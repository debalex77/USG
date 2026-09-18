/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "appcontroller.h"

#include <QMessageBox>
#include <QDir>
#include <QScopeGuard>
#include <QSqlError>
#include <QSqlQuery>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_mainWin(nullptr)
    , m_db(this)
{

}

int AppController::run(int &argc, char **argv)
{
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    const auto closeLogManager = qScopeGuard([] { LogManager::shutdown(); });

    if (!QDir().mkpath(globals().json_dir)) {
        qWarning(logWarning())
            << tr("Directorul pentru starea interfeței nu a putut fi creat:")
            << globals().json_dir;
    }

    applyGlobalFont(); /** fontul aplicatiei */
    applyStyleSheet(); /** stilul aplicatiei */

    /** 1. Selectăm baza de date */
    DatabaseSelection dbSel;
    if (dbSel.exec() != QDialog::Accepted)
        return 0;

    /** 2. Setările rămân disponibile pe durata buclei aplicației. */
    AppSettings appSettings;

    /** 3. Determinăm dacă jurnalizarea în fișier este dezactivată explicit. */
    const bool isDebug = (argc >= 2 && QString(argv[1]).compare("/debug", Qt::CaseInsensitive) == 0);

    /** 4. Gestionăm fluxul primei lansări / mutării / normal */
    int code = handleLaunchFlow(appSettings, argv, isDebug);
    if (code >= 0)
        return code; // user a întrerupt

    /** 5. Fereastra principală */
    if (!ensureMainDatabaseConnected())
        return 0;

    m_mainWin = new MainWindow(m_db);
    WindowManager::resize(m_mainWin);
    SplashManager::show(*m_mainWin);
    m_mainWin->show();

    return app.exec();
}

void AppController::applyGlobalFont()
{
#ifdef Q_OS_WIN
    int id = QFontDatabase::addApplicationFont(":/fonts/segoeUI/segoeui.ttf");
    QFont f(QFontDatabase::applicationFontFamilies(id).value(0), 10);
    qApp->setFont(f);
#elif defined(Q_OS_LINUX)
    int id = QFontDatabase::addApplicationFont(":/fonts/segoeUI/segoeui.ttf");
    QFont f(QFontDatabase::applicationFontFamilies(id).value(0), 11);
    qApp->setFont(f);
#else
    qApp->setFont(QFont("San Francisco", 13));
#endif
}

void AppController::applyStyleSheet()
{
    /** determinam stilul */
    QFile f(ThemeManager::isDark()
                ? ":/styles/style_dark.qss"
                : ":/styles/style_main.qss");

    /** suplimentar variabila globala */
    globals().isSystemThemeDark = ThemeManager::isDark()
                                      ? true
                                      : false ;

    if (f.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
}

bool AppController::ensureMainDatabaseConnected()
{
    const QSqlDatabase currentDatabase = m_db.getDatabase();
    if (currentDatabase.isValid() && currentDatabase.isOpen())
        return true;

    if (m_db.connectToDataBase()) {
        const QSqlDatabase connectedDatabase = m_db.getDatabase();
        if (connectedDatabase.isValid() && connectedDatabase.isOpen())
            return true;
    }

    const QSqlDatabase failedDatabase = m_db.getDatabase();
    const QString databaseDescription = globals().thisMySQL
                                            ? tr("host: %1, baza: %2")
                                                  .arg(globals().mySQLhost,
                                                       globals().mySQLnameBase)
                                            : globals().sqliteDatabasePath;
    const QString errorText = failedDatabase.lastError().text();

    qCritical(logCritical())
        << tr("Conexiunea principală la baza de date nu a putut fi deschisă:")
        << databaseDescription
        << errorText;

    QMessageBox::critical(nullptr,
                          tr("Conectarea la baza de date"),
                          tr("Baza de date nu a putut fi deschisă:<br><b>%1</b><br><br>%2")
                              .arg(databaseDescription.toHtmlEscaped(),
                                   errorText.toHtmlEscaped()),
                          QMessageBox::Ok);
    return false;
}

bool AppController::initializeNewDatabase()
{
    DatabaseInit initializer(this);
    const bool initialized = initializer.run(nullptr, [] {
        bool success = false;
        {
            DataBase db;
            success = db.connectToDataBase() && db.creatingTables();
            if (success)
                success = db.loadNormogramsFromXml();

            if (success && !globals().thisMySQL)
                success = db.creatingTables_DbImage();

            if (success)
                success = db.verifyNewDatabaseSchema();
        }

        if (QSqlDatabase::contains(QStringLiteral("db_image"))) {
            {
                QSqlDatabase imageDb = QSqlDatabase::database(QStringLiteral("db_image"), false);
                imageDb.close();
            }
            QSqlDatabase::removeDatabase(QStringLiteral("db_image"));
        }
        if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

        return success;
    });

    if (initialized)
        return true;

    qCritical(logCritical()) << tr("Inițializarea bazei de date noi a eșuat.");
    QMessageBox::critical(nullptr,
                          tr("Crearea bazei de date"),
                          tr("Baza de date nu a putut fi inițializată complet. "
                             "Inițializarea va putea fi reluată la următoarea lansare. "
                             "Verificați jurnalul aplicației."),
                          QMessageBox::Ok);
    return false;
}

bool AppController::ensureInitialAdministrator()
{
    QSqlQuery query(m_db.getDatabase());
    if (!query.exec(QStringLiteral(
            "SELECT COUNT(*) FROM users WHERE deletionMark = 0"))
        || !query.next()) {
        qCritical(logCritical())
            << tr("Verificarea administratorului inițial a eșuat:")
            << query.lastError().text();
        QMessageBox::critical(nullptr,
                              tr("Crearea administratorului"),
                              tr("Nu s-a putut verifica dacă baza de date conține un administrator."),
                              QMessageBox::Ok);
        return false;
    }

    if (query.value(0).toInt() > 0)
        return true;

    // Recupereaza si profilele incomplete create inainte de introducerea
    // cheii initialSetupComplete. Fara niciun utilizator activ, autentificarea
    // nu este posibila si configurarea initiala trebuie reluata.
    if (!globals().firstLaunch && !AppSettings::saveInitialSetupComplete(false)) {
        qCritical(logCritical()) << tr("Starea configurării inițiale incomplete nu a putut fi salvată.");
        return false;
    }

    UserDialog userDialog(m_db, nullptr);
    userDialog.setIsNew(true);
    userDialog.setWindowTitle(tr("Crearea administratorului aplicației [*]"));
    if (userDialog.exec() == QDialog::Accepted)
        return true;

    qInfo(logInfo()) << tr("Crearea administratorului a fost amânată; configurarea inițială va fi reluată.");
    return false;
}

bool AppController::authorizeUser()
{
    AuthorizationUser auth(m_db, nullptr);
    auth.setId(globals().idUserApp);
    if (auth.exec() != QDialog::Accepted)
        return false;

    m_db.updateVariableFromTableSettingsUser();
    return true;
}

int AppController::handleLaunchFlow(AppSettings &appSettings, char **/*argv*/, bool isDebug)
{
    // 0️ Modul de lansare necunoscut -> rulam wizard-ul InitLaunch
    if (globals().unknowModeLaunch) {
        InitLaunch *initWizard = new InitLaunch();
        initWizard->setAttribute(Qt::WA_DeleteOnClose);
        if (initWizard->exec() != QDialog::Accepted)
            return 0; // utilizatorul a anulat
    }

    if (globals().firstLaunch || globals().moveApp == 1) {
        if (appSettings.exec() != QDialog::Accepted)
            return 0;

    } else if (!appSettings.loadSettings()) {
        return 0;

    }

    // Calea logului devine cunoscuta numai dupa citirea/salvarea profilului.
    if (!isDebug) {
        (void) LogManager::init(globals().logPath,
                                globals().numSavedFilesLog);
    }

    // Pentru un profil nou sau pentru o configurare initiala intrerupta,
    // reluam crearea si verificarea schemei.
    if (globals().firstLaunch && !initializeNewDatabase())
        return 0;

    if (!ensureMainDatabaseConnected())
        return 0;

    if (!ensureInitialAdministrator())
        return 0;

    // Dialogul se accepta numai dupa terminarea încarcarii asincrone a
    // constantelor și a datelor organizatiei/doctorului/cloud.
    if (!authorizeUser())
        return 0;

    return -1; // continuam
}
