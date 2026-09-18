#include "appsettingsstore.h"

#include "legacysettingscodec.h"
#include "loggingcategories.h"

#include <QSettings>
#include <QFile>

namespace AppSettingsStore {

namespace {

    int readBoundedInt(QSettings &settings, const QString &key, int defaultValue,
                       int minimum, int maximum);
    bool readStrictBool(QSettings &settings, const QString &key, bool defaultValue);
    bool isValidLegacyEncodedValue(const QString &encodedValue);

}

namespace Key {
    const QString groupIndex      = QStringLiteral("index_init");
    const QString groupPaths      = QStringLiteral("path_app");
    const QString groupConnection = QStringLiteral("connect");
    const QString groupStartup    = QStringLiteral("on_start");
    const QString groupMessages   = QStringLiteral("show_msg");

    const QString indexLanguage    = QStringLiteral("indexLangApp");
    const QString indexDatabase    = QStringLiteral("indexTypeSQL");
    const QString indexUnitMeasure = QStringLiteral("indexUnitMeasure");

    const QString pathTemplates    = QStringLiteral("docsTemplatesPath");
    const QString pathReports      = QStringLiteral("reportsPath");
    const QString pathVideo        = QStringLiteral("videoDirectory");

    const QString mysqlHost        = QStringLiteral("MySQL_host");
    const QString mysqlDatabase    = QStringLiteral("MySQL_name_base");
    const QString mysqlPort        = QStringLiteral("MySQL_port");
    const QString mysqlUser        = QStringLiteral("MySQL_user");
    const QString mysqlPassword    = QStringLiteral("MySQL_passwd_user");
    const QString mysqlOptions     = QStringLiteral("MySQL_option_connect");

    const QString sqliteDatabase     = QStringLiteral("sqliteDatabaseName");
    const QString sqlitePath         = QStringLiteral("sqliteDatabasePath");
    const QString imageDatabasePath  = QStringLiteral("imageDatabasePath");
    const QString logPath            = QStringLiteral("logPath");

    const QString rememberedUserId   = QStringLiteral("idUserApp");
    const QString rememberedUserName = QStringLiteral("nameUserApp");
    const QString rememberUser       = QStringLiteral("memoryUser");
    const QString retainedLogFiles   = QStringLiteral("numSavedFilesLog");
    const QString initialSetupComplete = QStringLiteral("initialSetupComplete");

    const QString showVideoMessage   = QStringLiteral("showMsgVideo");
    const QString showReportsMessage = QStringLiteral("showMsgReports");
}

namespace {

struct RenamedKey {
    const char *oldKey;
    const char *newKey;
};

const RenamedKey renamedKeys[] = {
    {"connect/sqliteNameBase",     "connect/sqliteDatabaseName"},
    {"connect/sqlitePathBase",     "connect/sqliteDatabasePath"},
    {"connect/pathDBImage",        "connect/imageDatabasePath"},
    {"connect/pathLogApp",         "connect/logPath"},
    {"path_app/pathTemplatesDocs", "path_app/docsTemplatesPath"},
    {"path_app/pathReports",       "path_app/reportsPath"},
    {"path_app/pathVideo",         "path_app/videoDirectory"}
};

bool migrateProfileKeys(QSettings &settings)
{
    bool needsMigration = false;
    for (const auto &key : renamedKeys)
        needsMigration |= settings.contains(QLatin1String(key.oldKey));
    if (!needsMigration)
        return true;

    // Keep the original profile (including unknown keys) before removing aliases.
    const QString backupPath = settings.fileName() + QStringLiteral(".pre-4.1.2.bak");
    if (!settings.isWritable()
        || (!QFile::exists(backupPath) && !QFile::copy(settings.fileName(), backupPath))) {
        qWarning(logWarning()) << "Migrarea cheilor profilului 4.1.2: copia de siguranță nu poate fi pregătită.";
        return false;
    }
    for (const auto &key : renamedKeys) {
        const QString oldKey = QLatin1String(key.oldKey);
        const QString newKey = QLatin1String(key.newKey);
        if (!settings.contains(oldKey))
            continue;
        if (!settings.contains(newKey))
            settings.setValue(newKey, settings.value(oldKey));
        settings.remove(oldKey);
    }
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        qWarning(logWarning()) << "Migrarea cheilor profilului 4.1.2 nu a putut fi salvată.";
        return false;
    }
    qInfo(logInfo()) << "Cheile profilului .conf au fost actualizate pentru 4.1.2; copia originală a fost păstrată.";
    return true;
}

int readBoundedInt(QSettings &settings, const QString &key, int defaultValue,
                   int minimum, int maximum)
{
    bool converted = false;
    const QVariant storedValue = settings.value(key, defaultValue);
    const int value = storedValue.toInt(&converted);
    if (converted && value >= minimum && value <= maximum)
        return value;

    qWarning(logWarning()) << "Valoare incompatibilă în setările aplicației:"
                           << settings.group() + QLatin1Char('/') + key
                           << "=" << storedValue
                           << "; se folosește valoarea implicită" << defaultValue;
    return defaultValue;
}

bool readStrictBool(QSettings &settings, const QString &key, bool defaultValue)
{
    if (!settings.contains(key))
        return defaultValue;

    const QVariant storedValue = settings.value(key);
    const QString normalized = storedValue.toString().trimmed().toLower();

    if (normalized == QStringLiteral("true") || normalized == QStringLiteral("1"))
        return true;

    if (normalized == QStringLiteral("false") || normalized == QStringLiteral("0"))
        return false;

    qWarning(logWarning()) << "Valoare booleană incompatibilă în setările aplicației:"
                           << settings.group() + QLatin1Char('/') + key
                           << "; se folosește valoarea implicită" << defaultValue;
    return defaultValue;
}

bool isValidLegacyEncodedValue(const QString &encodedValue)
{
    if (encodedValue.isEmpty())
        return true;

    return LegacySettingsCodec::isValid(encodedValue);
}

}

ReadResult readProfile(const QString &settingsPath, const QString &defaultLogPath)
{
    ReadResult result;
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.allKeys();

    if (settings.status() != QSettings::NoError) {
        result.error = settings.status() == QSettings::FormatError
                           ? ReadError::Format
                           : ReadError::Access;
        return result;
    }

    const auto validateEncodedKey = [&settings, &result](const QString &key) {
        if (!isValidLegacyEncodedValue(settings.value(key).toString()))
            result.invalidEncodedKeys.append(settings.group() + QLatin1Char('/') + key);
    };

    settings.beginGroup(Key::groupConnection);
    validateEncodedKey(Key::mysqlHost);
    validateEncodedKey(Key::mysqlDatabase);
    validateEncodedKey(Key::mysqlUser);
    validateEncodedKey(Key::mysqlPassword);
    settings.endGroup();

    settings.beginGroup(Key::groupStartup);
    validateEncodedKey(Key::rememberedUserId);
    validateEncodedKey(Key::rememberedUserName);
    settings.endGroup();

    if (!result.invalidEncodedKeys.isEmpty()) {
        result.error = ReadError::InvalidEncodedValue;
        return result;
    }

    if (!migrateProfileKeys(settings)) {
        result.error = ReadError::Access;
        return result;
    }

    ProfileData &data = result.data;
    // index
    settings.beginGroup(Key::groupIndex);
    data.languageIndex    = readBoundedInt(settings, Key::indexLanguage, Default::languageIndex, 0, 1);
    data.databaseIndex    = readBoundedInt(settings, Key::indexDatabase, Default::databaseIndex, 0, 2);
    data.unitMeasureIndex = readBoundedInt(settings, Key::indexUnitMeasure, Default::unitMeasureIndex, 0, 1);
    settings.endGroup();

    // paths
    settings.beginGroup(Key::groupPaths);
    data.pathTemplates = settings.value(Key::pathTemplates).toString();
    data.pathReports   = settings.value(Key::pathReports).toString();
    data.pathVideo     = settings.value(Key::pathVideo).toString();
    settings.endGroup();

    // data mysql/mariadb & sqlite
    settings.beginGroup(Key::groupConnection);
    data.mysqlHost          = LegacySettingsCodec::decode(settings.value(Key::mysqlHost).toString());
    data.mysqlDatabase      = LegacySettingsCodec::decode(settings.value(Key::mysqlDatabase).toString());
    data.mysqlPort          = readBoundedInt(settings, Key::mysqlPort, Default::mysqlPort, 1, 65535);
    data.mysqlOptions       = settings.value(Key::mysqlOptions).toString();
    data.mysqlUser          = LegacySettingsCodec::decode(settings.value(Key::mysqlUser).toString());
    data.mysqlPassword      = LegacySettingsCodec::decode(settings.value(Key::mysqlPassword).toString());
    data.imageDatabasePath  = settings.value(Key::imageDatabasePath).toString();
    data.logPath            = settings.value(Key::logPath, defaultLogPath).toString();
    data.sqliteDatabase     = settings.value(Key::sqliteDatabase).toString();
    data.sqlitePath         = settings.value(Key::sqlitePath).toString();
    settings.endGroup();

    // remembers
    settings.beginGroup(Key::groupStartup);
    const QString rememberedIdText = LegacySettingsCodec::decode(settings.value(Key::rememberedUserId).toString());
    data.rememberedUserName        = LegacySettingsCodec::decode(settings.value(Key::rememberedUserName).toString()).trimmed();
    bool rememberedIdValid         = false;
    data.rememberedUserId          = rememberedIdText.toInt(&rememberedIdValid);
    const bool rememberRequested   = readStrictBool(settings, Key::rememberUser,  Default::rememberUser);
    data.rememberUser = rememberRequested && rememberedIdValid
                        && data.rememberedUserId > 0
                        && !data.rememberedUserName.isEmpty();
    data.rememberedUserDataIncomplete = rememberRequested && !data.rememberUser;
    if (!data.rememberUser) {
        data.rememberedUserId = 0;
        data.rememberedUserName.clear();
    }
    data.retainedLogFiles = readBoundedInt(settings, Key::retainedLogFiles, Default::retainedLogFiles, 0, 99);
    data.initialSetupComplete = readStrictBool(settings, Key::initialSetupComplete, true);
    settings.endGroup();

    // shows
    settings.beginGroup(Key::groupMessages);
    data.showVideoMessage   = readStrictBool(settings, Key::showVideoMessage, Default::showVideoMessage);
    data.showReportsMessage = readStrictBool(settings, Key::showReportsMessage, Default::showReportsMessage);
    settings.endGroup();

    return result;
}

WriteError writeProfile(const QString &settingsPath, const ProfileData &data)
{
    if (settingsPath.isEmpty())
        return WriteError::Access;

    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.allKeys();
    if (settings.status() == QSettings::FormatError)
        return WriteError::Format;
    if (settings.status() != QSettings::NoError || !migrateProfileKeys(settings))
        return WriteError::Access;

    settings.beginGroup(Key::groupIndex);
    settings.setValue(Key::indexLanguage,    data.languageIndex);
    settings.setValue(Key::indexDatabase,    data.databaseIndex);
    settings.setValue(Key::indexUnitMeasure, data.unitMeasureIndex);
    settings.endGroup();

    settings.beginGroup(Key::groupPaths);
    settings.setValue(Key::pathTemplates, data.pathTemplates);
    settings.setValue(Key::pathReports,   data.pathReports);
    settings.setValue(Key::pathVideo,     data.pathVideo);
    settings.endGroup();

    settings.beginGroup(Key::groupConnection);
    settings.setValue(Key::mysqlHost,          LegacySettingsCodec::encode(data.mysqlHost));
    settings.setValue(Key::mysqlDatabase,      LegacySettingsCodec::encode(data.mysqlDatabase));
    settings.setValue(Key::mysqlPort,          data.mysqlPort);
    settings.setValue(Key::mysqlUser,          LegacySettingsCodec::encode(data.mysqlUser));
    settings.setValue(Key::mysqlPassword,      LegacySettingsCodec::encode(data.mysqlPassword));
    settings.setValue(Key::mysqlOptions,       data.mysqlOptions);
    settings.setValue(Key::sqliteDatabase,     data.sqliteDatabase);
    settings.setValue(Key::sqlitePath,         data.sqlitePath);
    settings.setValue(Key::imageDatabasePath,  data.imageDatabasePath);
    settings.setValue(Key::logPath,            data.logPath);
    settings.endGroup();

    // ID-ul și numele utilizatorului memorat sunt administrate separat prin
    // writeGroup(); aici se actualizeaza doar preferinta generala de retentie.
    settings.beginGroup(Key::groupStartup);
    settings.setValue(Key::retainedLogFiles, data.retainedLogFiles);
    settings.setValue(Key::initialSetupComplete, data.initialSetupComplete);
    settings.endGroup();

    settings.beginGroup(Key::groupMessages);
    settings.setValue(Key::showVideoMessage,   data.showVideoMessage ? 1 : 0);
    settings.setValue(Key::showReportsMessage, data.showReportsMessage ? 1 : 0);
    settings.endGroup();

    settings.sync();
    if (settings.status() == QSettings::NoError)
        return WriteError::None;

    return settings.status() == QSettings::FormatError
               ? WriteError::Format
               : WriteError::Access;
}

bool writeGroup(const QString &settingsPath, const QString &group,
                const QVariantMap &values)
{
    if (settingsPath.isEmpty() || group.isEmpty() || values.isEmpty())
        return false;

    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.beginGroup(group);

    for (auto it = values.cbegin(); it != values.cend(); ++it)
        settings.setValue(it.key(), it.value());

    settings.endGroup();
    settings.sync();

    if (settings.status() == QSettings::NoError)
        return true;

    qWarning(logWarning()) << "Nu au putut fi salvate setările grupului"
                           << group << "în" << settingsPath;
    return false;
}

}
