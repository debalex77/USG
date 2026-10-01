#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QDialog>
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

#include <database/database.h>
#include <settings/appsettingsstore.h>
#include <ui/widgets/loglevelbutton.h>
#include <common/globals.h>

// ***********************************

#if defined(Q_OS_LINUX)
    #define NAME_DIR_LOG_PATH     "var/log/usg"
    #define NAME_FILE_LOG_PATH    "var/log/usg/usg.log"
#elif defined(Q_OS_MACOS)
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
    [[nodiscard]] bool loadSettings();
    void setLanguageApp();

    static bool saveRememberedUser(int userId, const QString &userName, bool remember);
    static bool saveInitialSetupComplete(bool complete);

signals:
    void restartRequested();

private:
    enum IndexBaseSQL {idx_Unknow = 0, idx_MySQL = 1, idx_Sqlite = 2};
    void captureSettingsState();
    void restoreSettingsState();

    void persistStartupLanguage(const AppSettingsStore::ProfileData &data);
    [[nodiscard]] AppSettingsStore::ProfileData profileFromGlobals() const;
    [[nodiscard]] AppSettingsStore::ProfileData profileFromForm() const;

    void applyProfileToForm(const AppSettingsStore::ProfileData &data);
    void applyProfileToRuntime(const AppSettingsStore::ProfileData &data,
                               const QString &settingsPath = QString());
    void initBtnForm();
    void initBtnSettingsApp();
    void initBtnLogApp();
    void initBtnMainBase(const QString appStyleBtn);
    void initBtnImageBase(const QString appStyleBtn);
    void initBtnDirTemplets(const QString appStyleBtn);
    void initBtnDirReports(const QString appStyleBtn);
    void initBtnDirVideo(const QString appStyleBtn);

    void initConnections();

    void setDefaultPath();
    void setDefaultPathSqlite();
    bool checkDataSettings();
    bool saveSettings();
    void updateTableLog(QString level_log, QStringList level_exclude);

private slots:
    void dataWasModified();

    void slot_currentIndexChangedTab(const int index);
    void slot_clickedTableLogs(const QModelIndex &index);

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
    void selectedLevelLog(const QString level);

private:
    Ui::AppSettings *ui;
    QTranslator translator;
    QString dirConfigPath;

#if defined(Q_OS_LINUX)
    QString dirLogPath     = QDir::rootPath() + NAME_DIR_LOG_PATH;
    QString fileLogPath    = QDir::rootPath() + NAME_FILE_LOG_PATH;
#elif defined(Q_OS_MACOS)
    QString dirLogPath     = QDir::homePath() + NAME_DIR_LOG_PATH;
    QString fileLogPath    = QDir::homePath() + NAME_FILE_LOG_PATH;
#elif defined(Q_OS_WIN)
    QString dirLogPath     = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs";
    QString fileLogPath    = dirLogPath + "/usg.log";
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

    QStandardItemModel m_logModel;
    AppSettingsStore::ProfileData m_loadedProfile;
    AppSettingsStore::ProfileData m_initialProfile;
    QString m_initialSettingsPath;
    QString m_writtenSettingsPath; // profilul scris de acest dialog
    bool m_populatingForm = false;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
};

#endif // APPSETTINGS_H
