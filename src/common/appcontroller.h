#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QApplication>
#include <QFontDatabase>
#include <QSslSocket>
#include <QSqlDatabase>
#include <QFile>
#include <QString>

#include <common/thememanager.h>
#include <common/windowmanager.h>
#include <common/splashmanager.h>
#include <common/logmanager.h>
#include <common/databaseinit.h>

#include <data/authorizationuser.h>
#include <data/databaseselection.h>
#include <data/mainwindow.h>
#include <data/appsettings.h>
#include <data/initlaunch.h>
#include <common/globals.h>

#include <catalogs/userdialog.h>

class QApplication;
class MainWindow;
class AppSettings; // definit în <data/appsettings.h>

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
};

#endif // APPCONTROLLER_H
