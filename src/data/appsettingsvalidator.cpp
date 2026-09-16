#include "appsettingsvalidator.h"

#include <QFileInfo>
#include <QObject>

namespace AppSettingsValidator {

QString writableFileLocationError(const QString &path, bool requireReadable)
{
    if (path.isEmpty())
        return QObject::tr("calea este goală");

    const QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        if (!fileInfo.isFile())
            return QObject::tr("calea nu indică un fișier");
        if (requireReadable && !fileInfo.isReadable())
            return QObject::tr("fișierul nu poate fi citit");
        if (!fileInfo.isWritable())
            return QObject::tr("fișierul nu poate fi scris");
        return {};
    }

    const QFileInfo directoryInfo(fileInfo.absolutePath());
    if (!directoryInfo.exists() || !directoryInfo.isDir())
        return QObject::tr("directorul părinte nu există");
    if (!directoryInfo.isWritable())
        return QObject::tr("directorul părinte nu permite crearea fișierului");
    return {};
}

QString directoryLocationError(const QString &path, bool requireWritable)
{
    if (path.isEmpty())
        return QObject::tr("calea este goală");

    const QFileInfo directoryInfo(path);
    if (!directoryInfo.exists())
        return QObject::tr("directorul nu există");
    if (!directoryInfo.isDir())
        return QObject::tr("calea nu indică un director");
    if (!directoryInfo.isReadable())
        return QObject::tr("directorul nu poate fi citit");
    if (requireWritable && !directoryInfo.isWritable())
        return QObject::tr("directorul nu poate fi scris");
    return {};
}

bool isSafeProfileName(const QString &name)
{
    if (name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')))
        return false;

    for (const QChar character : name) {
        if (!character.isPrint())
            return false;
    }

    return name != QLatin1String(".") && name != QLatin1String("..");
}

ValidationResult validateProfile(const AppSettingsStore::ProfileData &data,
                                 const QString &settingsPath)
{
    const auto invalid = [](Field field, const QString &value, const QString &reason) {
        return ValidationResult{field, value, reason};
    };
    const auto invalidFile = [&invalid](Field field, const QString &path,
                                        bool requireReadable) {
        const QString reason = writableFileLocationError(path, requireReadable);
        return reason.isEmpty() ? ValidationResult{} : invalid(field, path, reason);
    };
    const auto invalidDirectory = [&invalid](Field field, const QString &path,
                                             bool requireWritable) {
        const QString reason = directoryLocationError(path, requireWritable);
        return reason.isEmpty() ? ValidationResult{} : invalid(field, path, reason);
    };

    if (data.databaseIndex != 1 && data.databaseIndex != 2)
        return invalid(Field::DatabaseType, {}, QObject::tr("tipul bazei de date nu este indicat"));

    const QString profileName = data.databaseIndex == 1
                                    ? data.mysqlDatabase
                                    : data.sqliteDatabase;
    if (!profileName.isEmpty() && !isSafeProfileName(profileName))
        return invalid(Field::ProfileName, profileName,
                       QObject::tr("denumirea conține caractere nepermise"));

    ValidationResult result = invalidFile(Field::SettingsPath, settingsPath, false);
    if (!result.isValid())
        return result;
    result = invalidFile(Field::LogPath, data.logPath, false);
    if (!result.isValid())
        return result;
    result = invalidDirectory(Field::TemplatesPath, data.pathTemplates, false);
    if (!result.isValid())
        return result;
    result = invalidDirectory(Field::ReportsPath, data.pathReports, false);
    if (!result.isValid())
        return result;
    if (!data.pathVideo.isEmpty()) {
        result = invalidDirectory(Field::VideoPath, data.pathVideo, true);
        if (!result.isValid())
            return result;
    }

    if (data.databaseIndex == 1) {
        if (data.mysqlHost.isEmpty())
            return invalid(Field::MysqlHost, {}, QObject::tr("numele hostului nu este indicat"));
        if (data.mysqlDatabase.isEmpty())
            return invalid(Field::MysqlDatabase, {}, QObject::tr("denumirea bazei de date nu este indicată"));
        if (data.mysqlUser.isEmpty())
            return invalid(Field::MysqlUser, {}, QObject::tr("utilizatorul nu este indicat"));
        if (data.mysqlPort < 1 || data.mysqlPort > 65535)
            return invalid(Field::MysqlPort, QString::number(data.mysqlPort),
                           QObject::tr("portul trebuie să fie între 1 și 65535"));
        return {};
    }

    if (data.sqliteDatabase.isEmpty())
        return invalid(Field::SqliteDatabase, {}, QObject::tr("denumirea bazei SQLite nu este indicată"));
    if (data.sqlitePath.isEmpty())
        return invalid(Field::SqlitePath, {}, QObject::tr("calea bazei SQLite nu este indicată"));
    result = invalidFile(Field::SqlitePath, data.sqlitePath, true);
    if (!result.isValid())
        return result;
    if (data.imageDatabasePath.isEmpty())
        return invalid(Field::ImageDatabasePath, {},
                       QObject::tr("calea bazei pentru imagini nu este indicată"));
    return invalidFile(Field::ImageDatabasePath, data.imageDatabasePath, true);
}

}
