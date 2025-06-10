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
#include "thememanager.h"
#include "windowmanager.h"
#include "splashmanager.h"
#include "logmanager.h"

#include <data/authorizationuser.h>
#include <data/databaseselection.h>
#include <data/mainwindow.h>
#include <data/appsettings.h>
#include <data/initlaunch.h>
#include <catalogs/catusers.h>
#include <data/globals.h>

#include <QApplication>
#include <QFontDatabase>
#include <QSslSocket>
#include <QSqlDatabase>
#include <QFile>
#include <QString>
#include <databaseinit.h>

int AppController::run(int &argc, char **argv)
{
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    applyGlobalFont();
    applyStyleSheet();

    // 0. Selectăm baza de date
    DatabaseSelection dbSel;
    if (dbSel.exec() != QDialog::Accepted)
        return 0;

    // 1. Instanțiem AppSettings existent
    m_appSettings = new AppSettings();

    // 2. Inițializăm fișierul de log (dacă nu suntem în /debug)
    if (argc < 2 || QString(argv[1]).compare("/debug", Qt::CaseInsensitive) != 0) {
        LogManager::init(globals().pathLogAppSettings);
    }

    // 3. Gestionăm fluxul primei lansări / mutării / normal
    int code = handleLaunchFlow(argv);
    if (code >= 0)
        return code; // user a întrerupt

    // 4. Fereastra principală
    m_mainWin = new MainWindow();
    WindowManager::resize(m_mainWin);
    SplashManager::show(*m_mainWin);
    m_mainWin->show();

    return app.exec();
}

void AppController::applyGlobalFont()
{
#ifdef Q_OS_WIN
    int id = QFontDatabase::addApplicationFont(":/Fonts/segoeUI/segoeui.ttf");
    QFont f(QFontDatabase::applicationFontFamilies(id).value(0), 10);
    qApp->setFont(f);
#elif defined(Q_OS_LINUX)
    int id = QFontDatabase::addApplicationFont(":/Fonts/segoeUI/segoeui.ttf");
    QFont f(QFontDatabase::applicationFontFamilies(id).value(0), 11);
    qApp->setFont(f);
#else
    qApp->setFont(QFont("San Francisco", 13));
#endif
}

void AppController::applyStyleSheet()
{
    // determinam stilul
    QFile f(ThemeManager::isDark()
                ? ":/styles/style_dark.qss"
                : ":/styles/style_main.qss");

    // suplimentar variabila globala
    globals().isSystemThemeDark = ThemeManager::isDark()
                                      ? true
                                      : false ;

    if (f.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
}

int AppController::handleLaunchFlow(char **/*argv*/)
{
    // 0️ Modul de lansare necunoscut -> rulăm wizard-ul InitLaunch
    if (globals().unknowModeLaunch) {
        InitLaunch *initWizard = new InitLaunch();
        initWizard->setAttribute(Qt::WA_DeleteOnClose);
        if (initWizard->exec() != QDialog::Accepted)
            return 0;                       // utilizatorul a anulat
    }

    if (globals().firstLaunch) {

        // 1️ Prima lansare
        if (m_appSettings->exec() != QDialog::Accepted)
            return 0;

        LogManager::init(globals().pathLogAppSettings); // reinitializam LogManager

        DatabaseInit *initor = new DatabaseInit(this);
        initor->runAsync(nullptr, []{
            DataBase db;                 // conexiune proprie thread-ului
            db.connectToDataBase();      // va crea fișierul goală dacă e SQLite
            db.creatingTables();         // schemă + date
            db.loadNormogramsFromXml();
            if (! globals().thisMySQL)
                db.creatingTables_DbImage();
            db.createIndexTables();
        },
                         // === callback în MAIN THREAD după terminare ===
            []{

                // 1.3 Creăm administratorul (acum BD este pregătită)
                CatUsers *cuDlg = new CatUsers();
                cuDlg->setProperty("ItNew", true);
                cuDlg->setWindowTitle(tr("Crearea administratorului aplicației [*]"));
                    if (cuDlg->exec() != QDialog::Accepted) {
                        qApp->quit();
                        return;
                    }

                // 1.4 Lansăm autorizarea imediat cu userul creat
                auto auth = new AuthorizationUser();
                auth->setAttribute(Qt::WA_DeleteOnClose);
                auth->setProperty("Id", globals().idUserApp);
                if (auth->exec() != QDialog::Accepted)
                    qApp->quit();
                    // altfel continuăm – AppController va afișa MainWindow după return
            }
        );

    } else if (globals().moveApp == 1) {

        // 2️ BD transferată
        if (m_appSettings->exec() != QDialog::Accepted)
            return 0;

        AuthorizationUser *auth = new AuthorizationUser();
        auth->setAttribute(Qt::WA_DeleteOnClose);
        auth->setProperty("Id", globals().idUserApp);
        if (auth->exec() != QDialog::Accepted)
            return 0;

    } else {

        // 3️ Lansare normală
        m_appSettings->loadSettings();

        AuthorizationUser *auth = new AuthorizationUser();
        auth->setAttribute(Qt::WA_DeleteOnClose);
        auth->setProperty("Id", globals().idUserApp);
        if (auth->exec() != QDialog::Accepted)
            return 0;

    }
    return -1; // continuăm
}
