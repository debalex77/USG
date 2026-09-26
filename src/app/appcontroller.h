#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QApplication>
#include <QFontDatabase>
#include <QSslSocket>
#include <QSqlDatabase>
#include <QFile>
#include <QString>

#include <ui/services/thememanager.h>
#include <app/windowmanager.h>
#include <app/splashmanager.h>
#include <core/logging/logmanager.h>
#include <database/databaseinit.h>

#include <app/authorizationuser.h>
#include <app/databaseselection.h>
#include <app/mainwindow.h>
#include <settings/appsettings.h>
#include <app/initlaunch.h>
#include <common/globals.h>

#include <features/catalogs/userdialog.h>

class QApplication;
class MainWindow;
class AppSettings; // definit în <settings/appsettings.h>

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    int run(int &argc, char **argv);

private:
    void applyGlobalFont();
    void applyStyleSheet();
    bool ensureMainDatabaseConnected();
    bool initializeNewDatabase();
    bool ensureInitialAdministrator();
    bool authorizeUser();
    int  handleLaunchFlow(AppSettings &appSettings, char **, bool isDebug);

    MainWindow *m_mainWin = nullptr;
    DataBase m_db;
    bool m_restartApproved = false;
};

#endif // APPCONTROLLER_H
