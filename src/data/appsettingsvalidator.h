#ifndef APPSETTINGSVALIDATOR_H
#define APPSETTINGSVALIDATOR_H

#include <QString>

#include "appsettingsstore.h"

namespace AppSettingsValidator {

enum class Field {
    None,
    DatabaseType,
    ProfileName,
    SettingsPath,
    LogPath,
    TemplatesPath,
    ReportsPath,
    VideoPath,
    MysqlHost,
    MysqlDatabase,
    MysqlPort,
    MysqlUser,
    SqliteDatabase,
    SqlitePath,
    ImageDatabasePath
};

struct ValidationResult {
    Field field = Field::None;
    QString value;
    QString reason;

    [[nodiscard]] bool isValid() const { return field == Field::None; }
};

[[nodiscard]] QString writableFileLocationError(const QString &path,
                                                 bool requireReadable);
[[nodiscard]] QString directoryLocationError(const QString &path,
                                              bool requireWritable);
[[nodiscard]] bool isSafeProfileName(const QString &name);
[[nodiscard]] ValidationResult validateProfile(
    const AppSettingsStore::ProfileData &data,
    const QString &settingsPath);

}

#endif // APPSETTINGSVALIDATOR_H
