#ifndef APPSETTINGSSTORE_H
#define APPSETTINGSSTORE_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

/** Clasa pentru citirea/salvarea datelor profilului
  * utilizatorului
  ***************************************************/
namespace AppSettingsStore {

struct ProfileData {

    // indexuri lang, db, measure
    int languageIndex    = 1;
    int databaseIndex    = 0;
    int unitMeasureIndex = 0;

    // path templates (docs, report, video)
    QString pathTemplates;
    QString pathReports;
    QString pathVideo;

    // data mysql/maridb
    QString mysqlHost;
    QString mysqlDatabase;
    int mysqlPort = 3306;
    QString mysqlUser;
    QString mysqlPassword;
    QString mysqlOptions;

    // data sqlite
    QString sqliteDatabase;
    QString sqlitePath;
    QString imageDatabasePath;
    QString logPath;

    // remmembder data user
    bool rememberUser = false;
    int rememberedUserId = 0;
    QString rememberedUserName;
    bool rememberedUserDataIncomplete = false;
    int retainedLogFiles = 10;
    bool initialSetupComplete = true;

    // shows
    bool showVideoMessage = true;
    bool showReportsMessage = true;
};

enum class ReadError {
    None,
    Access,
    Format,
    InvalidEncodedValue
};

enum class WriteError {
    None,
    Access,
    Format
};

struct ReadResult {
    ProfileData data;
    ReadError error = ReadError::None;
    QStringList invalidEncodedKeys;

    [[nodiscard]] bool isValid() const { return error == ReadError::None; }
};

namespace Key {

    // grupe
    extern const QString groupIndex;
    extern const QString groupPaths;
    extern const QString groupConnection;
    extern const QString groupStartup;
    extern const QString groupMessages;

    // indexuri
    extern const QString indexLanguage;
    extern const QString indexDatabase;
    extern const QString indexUnitMeasure;

    // paths
    extern const QString pathTemplates;
    extern const QString pathReports;
    extern const QString pathVideo;

    // data mysql/maridb
    extern const QString mysqlHost;
    extern const QString mysqlDatabase;
    extern const QString mysqlPort;
    extern const QString mysqlUser;
    extern const QString mysqlPassword;
    extern const QString mysqlOptions;

    // data sqlite
    extern const QString sqliteDatabase;
    extern const QString sqlitePath;
    extern const QString imageDatabasePath;
    extern const QString logPath;

    // remember
    extern const QString rememberedUserId;
    extern const QString rememberedUserName;
    extern const QString rememberUser;
    extern const QString retainedLogFiles;
    extern const QString initialSetupComplete;

    // shows
    extern const QString showVideoMessage;
    extern const QString showReportsMessage;
}

namespace Default {
    inline constexpr int languageIndex       = 1;
    inline constexpr int databaseIndex       = 0;
    inline constexpr int unitMeasureIndex    = 0;
    inline constexpr int retainedLogFiles    = 10;
    inline constexpr bool rememberUser       = false;
    inline constexpr bool showVideoMessage   = true;
    inline constexpr bool showReportsMessage = true;
    inline constexpr int mysqlPort           = 3306;
}

ReadResult readProfile(const QString &settingsPath, const QString &defaultLogPath);
WriteError writeProfile(const QString &settingsPath, const ProfileData &data);
bool writeGroup(const QString &settingsPath, const QString &group,
                const QVariantMap &values);

}

#endif // APPSETTINGSSTORE_H
