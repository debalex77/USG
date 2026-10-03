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

#include "appsettingsstore.h"

#include "legacysettingscodec.h"
#include "profilesecretcodec.h"
#include "core/loggingcategories.h"

#include <QSettings>
#include <QFile>

namespace AppSettingsStore {

namespace {

    int readBoundedInt(QSettings &settings, const QString &key, int defaultValue,
                       int minimum, int maximum);
    bool readStrictBool(QSettings &settings, const QString &key, bool defaultValue);
    bool isValidLegacyEncodedValue(const QString &encodedValue);
    bool decodeProfilePassword(const QString &encodedValue, const QString &settingsPath,
                               QString *plainText);
    bool secureProfilePermissions(const QString &settingsPath);

}

namespace Key {
    const QString groupIndex      = QStringLiteral("index_init");
    const QString groupPaths      = QStringLiteral("path_app");
    const QString groupConnection = QStringLiteral("connect");
    const QString groupStartup    = QStringLiteral("on_start");

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
    if (!settings.isWritable() || (!QFile::exists(backupPath) && !QFile::copy(settings.fileName(), backupPath))) {
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

bool decodeProfilePassword(const QString &encodedValue, const QString &settingsPath,
                           QString *plainText)
{
    if (!plainText)
        return false;

    if (encodedValue.isEmpty()) {
        plainText->clear();
        return true;
    }

    if (ProfileSecretCodec::isEncrypted(encodedValue))
        return ProfileSecretCodec::decrypt(encodedValue, settingsPath, plainText);

    if (!LegacySettingsCodec::isValid(encodedValue))
        return false;

    *plainText = LegacySettingsCodec::decode(encodedValue);
    return true;
}

bool secureProfilePermissions(const QString &settingsPath)
{
#if defined(Q_OS_UNIX)
    return QFile::setPermissions(settingsPath,
                                 QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#else
    Q_UNUSED(settingsPath)
    return true;
#endif
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
    QString decodedPassword;
    if (!decodeProfilePassword(settings.value(Key::mysqlPassword).toString(),
                               settingsPath, &decodedPassword)) {
        result.invalidEncodedKeys.append(settings.group() + QLatin1Char('/')
                                         + Key::mysqlPassword);
    }
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
    // Valoarea a fost deja validată mai sus, inclusiv autentificarea AES-GCM.
    decodeProfilePassword(settings.value(Key::mysqlPassword).toString(),
                          settingsPath, &data.mysqlPassword);
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
    data.retainedLogFiles     = readBoundedInt(settings, Key::retainedLogFiles, Default::retainedLogFiles, 0, 99);
    data.initialSetupComplete = readStrictBool(settings, Key::initialSetupComplete, true);
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

    QString encryptedPassword;
    QString encryptionError;
    if (!ProfileSecretCodec::encrypt(data.mysqlPassword, settingsPath,
                                     &encryptedPassword, &encryptionError)) {
        qCritical(logCritical()) << "Criptarea parolei profilului a eșuat:"
                                 << encryptionError;
        return WriteError::Access;
    }

    settings.beginGroup(Key::groupConnection);
    settings.setValue(Key::mysqlHost,          LegacySettingsCodec::encode(data.mysqlHost));
    settings.setValue(Key::mysqlDatabase,      LegacySettingsCodec::encode(data.mysqlDatabase));
    settings.setValue(Key::mysqlPort,          data.mysqlPort);
    settings.setValue(Key::mysqlUser,          LegacySettingsCodec::encode(data.mysqlUser));
    settings.setValue(Key::mysqlPassword,      encryptedPassword);
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

    // Elimină definitiv preferințele locale obsolete din profilele existente.
    settings.remove(QStringLiteral("show_msg"));

    settings.sync();
    if (settings.status() == QSettings::NoError && secureProfilePermissions(settingsPath))
        return WriteError::None;

    if (settings.status() == QSettings::NoError)
        qCritical(logCritical()) << "Permisiunile profilului nu au putut fi restricționate:"
                                 << settingsPath;

    return settings.status() == QSettings::FormatError
               ? WriteError::Format
               : WriteError::Access;
}

QString readStartupLanguage(const QString &settingsPath)
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    const QString language = settings.value(QStringLiteral("ui/language")).toString();
    if (settings.status() != QSettings::NoError)
        return {};
    return language == QStringLiteral("ru-RU") || language == QStringLiteral("ro-RO")
               ? language : QString();
}

bool writeStartupLanguage(const QString &settingsPath, const QString &language)
{
    if (settingsPath.isEmpty()
        || (language != QStringLiteral("ru-RU") && language != QStringLiteral("ro-RO")))
        return false;

    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("ui/language"), language);
    settings.sync();
    return settings.status() == QSettings::NoError
           && secureProfilePermissions(settingsPath);
}

ArchiveOptions readArchiveOptions(const QString &settingsPath)
{
    ArchiveOptions options;
    if (settingsPath.isEmpty())
        return options;

    QSettings settings(settingsPath, QSettings::IniFormat);
    options.includeSettings = settings.value(QStringLiteral("archive/includeSettings"), false).toBool();
    options.includeCrypto   = settings.value(QStringLiteral("archive/includeCrypto"), false).toBool();
    options.encrypt         = settings.value(QStringLiteral("archive/encrypt"), false).toBool();
    return options;
}

bool readArchivePassword(const QString &settingsPath, const QString &keySettingsPath,
                         QString *password, QString *error)
{
    if (error)
        error->clear();
    if (!password || settingsPath.isEmpty() || keySettingsPath.isEmpty()) {
        if (error)
            *error = QStringLiteral("Parametri invalizi pentru citirea parolei arhivei.");
        return false;
    }
    password->clear();

    QSettings settings(settingsPath, QSettings::IniFormat);
    const QString encoded = settings.value(QStringLiteral("archive/password")).toString();
    if (encoded.isEmpty()) {
        if (error)
            *error = QStringLiteral("Parola arhivei nu este setată.");
        return false;
    }
    if (!ProfileSecretCodec::isEncrypted(encoded)) {
        if (error)
            *error = QStringLiteral("Parola arhivei are un format necunoscut.");
        return false;
    }
    return ProfileSecretCodec::decrypt(encoded, keySettingsPath, password, error)
           && !password->isEmpty();
}

bool writeArchivePassword(const QString &settingsPath, const QString &keySettingsPath,
                          const QString &password, QString *error)
{
    if (error)
        error->clear();
    if (settingsPath.isEmpty() || keySettingsPath.isEmpty()) {
        if (error)
            *error = QStringLiteral("Parametri invalizi pentru salvarea parolei arhivei.");
        return false;
    }

    QString encoded;
    if (!ProfileSecretCodec::encrypt(password, keySettingsPath, &encoded, error))
        return false;

    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("archive/password"), encoded);
    settings.sync();
    if (settings.status() != QSettings::NoError || !secureProfilePermissions(settingsPath)) {
        if (error)
            *error = QStringLiteral("Parola arhivei nu a putut fi salvată în %1").arg(settingsPath);
        return false;
    }
    return true;
}

bool writeArchiveOptions(const QString &settingsPath, const ArchiveOptions &options)
{
    if (settingsPath.isEmpty())
        return false;

    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("archive/includeSettings"), options.includeSettings);
    settings.setValue(QStringLiteral("archive/includeCrypto"), options.includeCrypto);
    settings.setValue(QStringLiteral("archive/encrypt"), options.encrypt);
    settings.sync();
    return settings.status() == QSettings::NoError
           && secureProfilePermissions(settingsPath);
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

    if (settings.status() == QSettings::NoError && secureProfilePermissions(settingsPath))
        return true;

    qWarning(logWarning()) << "Nu au putut fi salvate setările grupului"
                           << group << "în" << settingsPath;
    return false;
}

}
