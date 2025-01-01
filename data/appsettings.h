#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QLocale>
#include <QTranslator>
#include <QDesktopServices>
#include <QApplication>
#include <QProcess>
#include <QStandardItemModel>

#include <QToolButton>
#include <QLineEdit>

#if defined(Q_OS_WIN)
#include <QStandardPaths>
#endif

#include <data/globals.h>
#include "database.h"
#include "popup.h"

// ***********************************

#if defined(Q_OS_LINUX)
    #define NAME_DIR_CONFIG_PATH  "/.config/USG"
    #define NAME_FILE_CONFIG_PATH "/.config/USG/settings.conf"
    #define NAME_DIR_LOG_PATH     "var/log/usg"
    #define NAME_FILE_LOG_PATH    "var/log/usg/usg.log"
#elif defined(Q_OS_MACOS)
    #define NAME_DIR_CONFIG_PATH  "/.config/USG"
    #define NAME_FILE_CONFIG_PATH "/.config/USG/settings.ini"
    #define NAME_DIR_LOG_PATH     "/Library/Logs/USG"
    #define NAME_FILE_LOG_PATH    "/Library/Logs/USG/usg.log"
#elif defined(Q_OS_WIN)

#endif

// ***********************************

namespace Ui {
class AppSettings;
}

class AppSettings : public QDialog
{
    Q_OBJECT

public:
    explicit AppSettings(QWidget *parent = nullptr);
    ~AppSettings();

    void readSettings();
    void loadSettings();
    void setLanguageApp();
    void setBeginGroup(const QString nameGroup);
    void setEndGroup();

    void setKeyAndValue(const QString nameGroup, const QString &_key, const QVariant &_value);
    QVariant getValuesSettings(const QString nameGroup, const QString &_key, const QVariant &_defaultValue);

signals:
    void changeLanguage();

private:
    enum IndexBaseSQL {idx_Unknow = 0, idx_MySQL = 1, idx_Sqlite = 2};
    void initBtnForm();
    void initBtnSettingsApp();
    void initBtnLogApp();
    void initBtnMainBase(const QString appStyleBtn);
    void initBtnImageBase(const QString appStyleBtn);
    void initBtnDirTemplets(const QString appStyleBtn);
    void initBtnDirReports(const QString appStyleBtn);
    void initBtnDirVideo(const QString appStyleBtn);

    void initConnections();

    void processingLoggingFiles(const QString nameMatches);
    void removeFilesLogOnStartApp();
    void setDefaultPath();
    void setDefaultPathSqlite();
    bool checkDataSettings();
    void saveSettings();
    void loadDataFromTableSettingsUsers();

private slots:
    void dataWasModified();

    void slot_currentIndexChangedTab(const int index);
    void slot_clickedTableLogs(const QModelIndex &index);

    void changeNameBaseSqlite();    // editarea datelor sqlite:
    void changePathBaseSqlite();    // name, path

    void changeNameHostMySQL();     // editarea datelor conectarii MySQL
    void changeNameBaseMySQL();
    void changePortConectionMySQL();
    void changeConnectOptionsMySQL();
    void changeUserNameMySQL();
    void changePasswdMySQL();

    void onAddPathSqlite();         // butoane p/u localizarea bd sqlite
    void onEditPathSqlite();
    void onClearPathSqlite();

    void onAddPathDBImage();        // butoane p/u localizarea bd image(sqlite)
    void onEditPathDBImage();
    void onClearPathDBImage();

    void openFileSettingsApp();     // btn setarile aplicatiei

    void openFileCurrentLogApp();   // btn fisierelor de logare

    void openDirTemplets();         // btn templets and reports
    void openDirReports();
    void openDirVideo();

    void onBtnOKSettings();         // butoane principale ale formai
    void onBtnWriteSettings();
    void onBtnCancelSettings();

    void changeIndexLangApp(const int _index);      // modificarea indexului combo ...
    void changeIndexTypeSQL(const int _index);      // LangApp, TypeSQL, UnitMeasure
    void changeIndexUnitMeasure(const int _index);

    void createNewBaseSqlite();    // butoanele p/u testarea conectariii
    void onTestConnectionMySQL();

    void setPathAppSettings(); // setam drumul spre setariile aplicatiei
    void setPathGlobalVariableAppSettings();
private:
    Ui::AppSettings *ui;
    QSettings       *settApp;
    QTranslator     *translator;
    DataBase        *db;
    PopUp           *popUp;

#if defined(Q_OS_LINUX)
    QString dirConfigPath  = QDir::homePath() + NAME_DIR_CONFIG_PATH;
    QString fileConfigPath = QDir::homePath() + NAME_FILE_CONFIG_PATH;
    QString dirLogPath     = QDir::rootPath() + NAME_DIR_LOG_PATH;
    QString fileLogPath    = QDir::rootPath() + NAME_FILE_LOG_PATH;
#elif defined(Q_OS_MACOS)
    QString dirConfigPath  = QDir::homePath() + NAME_DIR_CONFIG_PATH;
    QString fileConfigPath = QDir::homePath() + NAME_FILE_CONFIG_PATH;
    QString dirLogPath     = QDir::homePath() + NAME_DIR_LOG_PATH;
    QString fileLogPath    = QDir::homePath() + NAME_FILE_LOG_PATH;
#elif defined(Q_OS_WIN)
    QString dirConfigPath  = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config";
    QString fileConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config/settings.conf";
    QString dirLogPath     = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/logs";
    QString fileLogPath    = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/logs/usg.log";
#endif

    QLineEdit *lineEditPathTemplatesPrint;
    QLineEdit *lineEditPathReports;
    QLineEdit *lineEditPathDBSqlite;
    QLineEdit *lineEditPathDBImage;
    QLineEdit *lineEditPathVideo;

    QToolButton *btnAdd;
    QToolButton *btnEdit;
    QToolButton *btnClear;

    QToolButton *btnOpenDirTemplates;
    QToolButton *btnRemoveDirTemplates;

    QToolButton *btnOpenDirReports;
    QToolButton *btnRemoveDirReports;

    QToolButton *btnOpenDirVideo;
    QToolButton *btnRemoveDirVideo;

    QToolButton *btnAddImage;
    QToolButton *btnEditImage;
    QToolButton *btnClearImage;

    QStandardItemModel *model_logs;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
};

#endif // APPSETTINGS_H
