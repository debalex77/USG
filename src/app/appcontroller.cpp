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
#include "common/applicationpathscontext.h"
#include "common/cloudconnectioncontext.h"
#include "common/maindatabaseconnectioncontext.h"
#include "common/sessioncontext.h"
#include "core/version.h"

#include "settings/settingsrepository.h"
#include "settings/settingsservice.h"

#include <QMessageBox>
#include <QDir>
#include <QProcess>
#include <QScopeGuard>
#include <QSqlError>
#include <QSqlQuery>
#include <QTranslator>

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

    /** 1. directoriu cu setari */
    if (!QDir().mkpath(ApplicationPathsContext::instance().uiSettingsDirectory())) {
        qWarning(logWarning())
            << tr("Directorul pentru starea interfeței nu a putut fi creat:")
            << ApplicationPathsContext::instance().uiSettingsDirectory();
    }

    applyGlobalFont(); /** fontul aplicatiei */
    applyStyleSheet(); /** stilul aplicatiei */

    // Selectarea bazei este afișată înainte de citirea profilului. Limba ei
    // trebuie cunoscută din preferința comună a aplicației.
    QTranslator startupTranslator;
    const QString startupLanguage = AppSettingsStore::readStartupLanguage(
        ApplicationPathsContext::instance().startupSettingsFilePath());
    const QString language = startupLanguage.isEmpty()
                                 ? QLocale::system().name()
                                 : startupLanguage;
    const QString translationPath = QStringLiteral(":/i18n/USG_%1.qm")
                                        .arg(QLocale(language).name());
    if (startupTranslator.load(translationPath)) {
        app.installTranslator(&startupTranslator);
        if (language.startsWith(QLatin1String("ru"), Qt::CaseInsensitive)
            && QCoreApplication::translate("DatabaseSelection", "Alege/creează baza de date")
                   == QStringLiteral("Alege/creează baza de date")) {
            qWarning(logWarning())
                << "Catalogul rus nu a tradus dialogul de selectare a bazei de date:"
                << translationPath;
        }
    } else {
        qWarning(logWarning())
            << "Catalogul de traducere pentru selectarea bazei de date nu a putut fi încărcat:"
            << translationPath;
    }

    /** 2. Selectăm baza de date */
    DatabaseSelection dbSel;
    if (dbSel.exec() != QDialog::Accepted)
        return 0;

    /** 3. Setările rămân disponibile pe durata buclei aplicației. */
    AppSettings appSettings;

    /** 4. Determinăm dacă jurnalizarea în fișier este dezactivată explicit. */
    const bool isDebug = (argc >= 2 && QString(argv[1]).compare("/debug", Qt::CaseInsensitive) == 0);

    /** 5. Gestionăm fluxul primei lansări / mutării / normal */
    int code = handleLaunchFlow(appSettings, argv, isDebug);
    if (code >= 0)
        return code; // user a intrerupt

    /** 6. conectarea cu bd */
    if (!ensureMainDatabaseConnected())
        return 0;

    /** 7. Fereastra principală */
    m_mainWin = new MainWindow(m_db);
    connect(m_mainWin, &MainWindow::restartApproved,
            this, [this, &app]() {
                m_restartApproved = true;
                app.quit();
            });
    WindowManager::resize(m_mainWin);
    SplashManager::show(*m_mainWin);
    m_mainWin->show();

    // Fereastra este desenată pentru a prezenta progresul, dar fluxul de
    // lansare se finalizează sincron înainte de intrarea în event loop.
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    if (!m_mainWin->completeStartup()) {
        m_mainWin->hide();
        delete m_mainWin;
        m_mainWin = nullptr;
        return 0;
    }

    const QString program = app.applicationFilePath();
    QStringList arguments = app.arguments();
    if (!arguments.isEmpty())
        arguments.removeFirst();

    const int exitCode = app.exec();
    if (m_restartApproved) {
        delete m_mainWin;
        m_mainWin = nullptr;
        LogManager::shutdown();
        if (!QProcess::startDetached(program, arguments)) {
            qCritical(logCritical())
                << tr("Relansarea aplicației după schimbarea limbii a eșuat:")
                << program;
            QMessageBox::critical(nullptr,
                                  tr("Relansarea aplicației"),
                                  tr("Aplicația nu a putut fi pornită din nou. Lansați-o manual."));
        }
    }
    return exitCode;
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
    /** verificam daca bd e valabila si e deschisa */
    const QSqlDatabase currentDatabase = m_db.getDatabase();
    if (currentDatabase.isValid() && currentDatabase.isOpen())
        return true;

    /** conectarea propriu-zisa */
    if (m_db.connectToDataBase()) {
        const QSqlDatabase connectedDatabase = m_db.getDatabase();
        if (connectedDatabase.isValid() && connectedDatabase.isOpen())
            return true;
    }

    /** daca nu s-a efectuat conectarea prezentama textul */
    const QSqlDatabase failedDatabase = m_db.getDatabase();
    const MainDatabaseConnectionData connection = MainDatabaseConnectionContext::instance().data();

    const QString databaseDescription = connection.backend == MainDatabaseBackend::MariaDb
                                            ? tr("host: %1, baza: %2")
                                                  .arg(connection.hostName,
                                                       connection.databaseName)
                                            : connection.sqliteDatabasePath;
    const QString errorText = failedDatabase.lastError().text();

    /** nu uitam de log */
    qCritical(logCritical())
        << tr("Conexiunea principală la baza de date nu a putut fi deschisă:")
        << databaseDescription
        << errorText;

    /** prezentam */
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

            if (success) {
                QString schemaVersionError;
                success = db.setDatabaseSchemaVersion(QStringLiteral(VERSION_FULL), &schemaVersionError);
                if (!success)
                    qCritical(logCritical())
                        << tr("Inițializarea versiunii schemei a eșuat:")
                        << schemaVersionError;
            }

            if (success && !MainDatabaseConnectionContext::instance().isMariaDb())
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
    QSqlQuery q(m_db.getDatabase());
    if (!q.exec(QStringLiteral(R"(SELECT COUNT(*) FROM users WHERE deletionMark = 0)"))
        || !q.next()) {

        qCritical(logCritical())
            << tr("Verificarea administratorului inițial a eșuat:")
            << q.lastError().text();

        QMessageBox::critical(nullptr,
                              tr("Crearea administratorului"),
                              tr("Nu s-a putut verifica dacă baza de date conține un administrator."),
                              QMessageBox::Ok);
        return false;
    }

    if (q.value(0).toInt() > 0)
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
    /** autorizarea cu setarea ID userului */
    AuthorizationUser auth(m_db, nullptr);
    auth.setId(SessionContext::instance().userId());
    if (auth.exec() != QDialog::Accepted)
        return false;

    /** citim setarile userului */
    SettingsRepository repository(m_db);
    SettingsRepository::LoadResult loaded = repository.loadForUser(SessionContext::instance().userId());
    if (!loaded.isValid()) {
        qCritical(logCritical())
            << tr("Citirea setărilor utilizatorului a eșuat:")
            << loaded.error;
        return false;
    }

    if (loaded.found) {
        // DataConstantsWorker stabilește dacă configurația cloud este validă,
        // iar repository-ul păstrează alegerea utilizatorului. Sincronizarea
        // efectivă este activă numai dacă ambele condiții sunt îndeplinite.
        const Settings::SynchronizationSettings runtime =
            SettingsService::instance().synchronization();
        const CloudConnectionData cloudRuntime =
            CloudConnectionContext::instance().data();
        const bool cloudConfigurationUsable =
            runtime.configured && !cloudRuntime.password.isEmpty();
        loaded.data.values.synchronization.configured = cloudConfigurationUsable;
        loaded.data.values.synchronization.enabled =
            cloudConfigurationUsable && runtime.enabled
            && loaded.data.values.synchronization.enabled;
        SettingsService::instance().setSnapshot(loaded.data.values);

        CloudConnectionData cloud = cloudRuntime;
        cloud.enabled = loaded.data.values.synchronization.enabled;
        CloudConnectionContext::instance().setData(cloud);

    } else {
        qWarning(logWarning())
            << tr("Nu există setări persistate pentru utilizatorul cu id=%1; "
                  "se folosesc valorile încărcate la autentificare.")
                   .arg(SessionContext::instance().userId());
        // DataConstantsWorker a incarcat deja contextul organizatiei si al
        // conexiunii cloud. Preferintele inexistente raman la valorile implicite.

    }
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
        (void) LogManager::init(ApplicationPathsContext::instance().data().logFilePath,
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

    // Dialogul se accepta numai dupa terminarea incarcarii asincrone a
    // constantelor si a datelor organizatiei/doctorului/cloud.
    if (!authorizeUser())
        return 0;

    return -1; // continuam
}
