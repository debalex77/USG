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

#include "settingsrepository.h"

#include <QMetaType>
#include <QSqlError>
#include <QSqlQuery>

#include <core/version.h>
#include <database/database.h>

SettingsRepository::SettingsRepository(DataBase &database)
    : m_database(database)
{
}

SettingsRepository::LoadResult SettingsRepository::loadForUser(int userId) const
{
    LoadResult result;
    result.data.values.user.userId = userId;

    if (userId <= 0) {
        result.error = QObject::tr("ID-ul utilizatorului nu este valid.");
        return result;
    }

    QVariantMap row;
    QString error;
    bool loadedFromNewSchema = false;
    if (hasNewSchema()) {
        const bool hasSynchronizationPreference =
            m_database.getDatabase().record(QStringLiteral("applicationSettings"))
                .contains(QStringLiteral("synchronization_enabled"));
        QSqlQuery q(m_database.getDatabase());
        q.prepare(QStringLiteral(R"(
            SELECT
                application.user_id AS application_user_id,
                application.check_for_updates_on_startup,
                application.show_user_manual_on_startup,
                application.show_assistant_on_startup,
                application.document_journal_refresh_interval_seconds,
                %1 AS synchronization_enabled,
                user_settings.user_id AS user_settings_user_id,
                user_settings.default_organization_id,
                user_settings.minimize_to_tray,
                user_settings.confirm_on_exit,
                user_settings.archive_sqlite_on_exit,
                user_settings.open_documents_in_separate_windows,
                user_settings.print_menu_mode,
                organization.default_doctor_id,
                organization.default_nurse_id,
                organization.ultrasound_device_name,
                organization.logo
            FROM
                users AS base_user
            LEFT JOIN
                applicationSettings AS application
                   ON application.user_id = base_user.id
            LEFT JOIN
                userSettings AS user_settings
                   ON user_settings.user_id = base_user.id
            LEFT JOIN
                organizationSettings AS organization
                   ON organization.organization_id =
                      user_settings.default_organization_id
            WHERE
                base_user.id = ?
        )").arg(hasSynchronizationPreference
                    ? QStringLiteral("application.synchronization_enabled")
                    : QStringLiteral("1")));
        q.addBindValue(userId);
        if (!q.exec()) {
            result.error = q.lastError().text();
            return result;
        }
        if (q.next()
            && !q.value(QStringLiteral("application_user_id")).isNull()
            && !q.value(QStringLiteral("user_settings_user_id")).isNull()) {
            const QSqlRecord record = q.record();
            for (int index = 0; index < record.count(); ++index)
                row.insert(record.fieldName(index), q.value(index));
            loadedFromNewSchema = true;
        }
    }

    if (!loadedFromNewSchema)
        row = m_database.selectJoinConstantsUserPreferencesByUserId(userId, &error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    if (row.isEmpty())
        return result;

    result.found = true;
    Settings::Snapshot &values = result.data.values;

    // functia suplimentara interna
    const auto value = [&row, loadedFromNewSchema](const char *legacy,
                                                   const char *current) {
        return row.value(QString::fromLatin1(loadedFromNewSchema ? current : legacy));
    };

    // check_for_updates_on_startup
    values.application.checkForUpdatesOnStartup =
        value("checkNewVersionApp", "check_for_updates_on_startup").toBool();

    // show_user_manual_on_startup
    values.application.showUserManualOnStartup =
        value("showUserManual", "show_user_manual_on_startup").toBool();

    // show_assistant_on_startup
    values.application.showAssistantOnStartup =
        value("showAsistantHelper", "show_assistant_on_startup").toBool();

    // document_journal_refresh_interval_seconds
    values.application.documentJournalRefreshIntervalSeconds =
        value("updateListDoc", "document_journal_refresh_interval_seconds").toInt();

    // Bazele anterioare acestei preferințe păstrează comportamentul existent:
    // sincronizarea este permisă dacă există o configurație cloud validă.
    values.synchronization.enabled = loadedFromNewSchema
        ? row.value(QStringLiteral("synchronization_enabled")).toBool()
        : true;

    // default_organization_id
    const QVariant organizationId =
        value("id_organizations", "default_organization_id");
    values.organization.organizationId = organizationId.isNull()
                                                 ? -1 : organizationId.toInt();

    // default_doctor_id
    const QVariant doctorId = value("id_doctors", "default_doctor_id");
    values.organization.defaultDoctorId = doctorId.isNull() ? -1 : doctorId.toInt();

    // default_nurse_id
    const QVariant nurseId = value("id_nurses", "default_nurse_id");
    values.organization.defaultNurseId = nurseId.isNull() ? -1 : nurseId.toInt();

    // ultrasound_device_name
    values.organization.ultrasoundDeviceName =
        value("brandUSG", "ultrasound_device_name").toString();

    // logo
    values.organization.logoData =
        QByteArray::fromBase64(row.value("logo").toByteArray());

    values.user.defaultOrganizationId = values.organization.organizationId;
    values.user.minimizeToTray        = value("minimizeAppToTray", "minimize_to_tray").toBool();
    values.user.confirmOnExit         = value("showQuestionCloseApp", "confirm_on_exit").toBool();
    values.user.archiveSqliteOnExit   = value("databasesArchiving", "archive_sqlite_on_exit").toBool();

    // open_documents_in_separate_windows
    values.user.openDocumentsInSeparateWindows =
        value("showDocumentsInSeparatWindow", "open_documents_in_separate_windows").toBool();

    // print_menu_mode
    values.user.printMenuMode = Settings::PrintMenuMode(
        value("showDesignerMenuPrint", "print_menu_mode").toInt());

    return result;
}

bool SettingsRepository::saveForUser(const PersistedSettings &settings,
                                     QStringList &errors) const
{
    const Settings::Snapshot &values = settings.values;
    const int userId = values.user.userId;
    if (userId <= 0) {
        errors << QObject::tr("ID-ul utilizatorului nu este valid.");
        return false;
    }

    const bool mysql = m_database.getDatabase().driverName().contains(
        QStringLiteral("MYSQL"), Qt::CaseInsensitive);
    const auto databaseBoolean = [mysql](bool value) -> QVariant {
        return mysql ? QVariant(value) : QVariant(int(value));
    };
    const auto nullableId = [](int value) -> QVariant {
        return value > 0 ? QVariant(value) : QVariant();
    };

    if (hasNewSchema()) {

        // pu aplication
        QVariantMap applicationValues;
        applicationValues.insert("user_id", userId);
        applicationValues.insert("check_for_updates_on_startup",
                                 databaseBoolean(values.application.checkForUpdatesOnStartup));
        applicationValues.insert("show_user_manual_on_startup",
                                 databaseBoolean(values.application.showUserManualOnStartup));
        applicationValues.insert("show_assistant_on_startup",
                                 databaseBoolean(values.application.showAssistantOnStartup));
        applicationValues.insert("document_journal_refresh_interval_seconds",
                                 values.application.documentJournalRefreshIntervalSeconds);
        if (m_database.getDatabase().record(QStringLiteral("applicationSettings"))
                .contains(QStringLiteral("synchronization_enabled"))) {
            applicationValues.insert("synchronization_enabled",
                                     databaseBoolean(values.synchronization.enabled));
        }
        if (!upsert(QStringLiteral("applicationSettings"), QStringLiteral("user_id"),
                    userId, applicationValues, errors))
            return false;

        // pu user
        QVariantMap userSettingsValues;
        userSettingsValues.insert("user_id", userId);
        userSettingsValues.insert("default_organization_id",
                                  nullableId(values.user.defaultOrganizationId));
        userSettingsValues.insert("minimize_to_tray",
                                  databaseBoolean(values.user.minimizeToTray));
        userSettingsValues.insert("confirm_on_exit",
                                  databaseBoolean(values.user.confirmOnExit));
        userSettingsValues.insert("archive_sqlite_on_exit",
                                  databaseBoolean(values.user.archiveSqliteOnExit));
        userSettingsValues.insert("open_documents_in_separate_windows",
                                  databaseBoolean(values.user.openDocumentsInSeparateWindows));
        userSettingsValues.insert("print_menu_mode", int(values.user.printMenuMode));
        if (!upsert(QStringLiteral("userSettings"), QStringLiteral("user_id"),
                    userId, userSettingsValues, errors))
            return false;

        if (values.organization.organizationId > 0) {
            QVariantMap newOrganizationValues;
            newOrganizationValues.insert("organization_id",
                                         values.organization.organizationId);
            newOrganizationValues.insert("default_doctor_id",
                                         nullableId(values.organization.defaultDoctorId));
            newOrganizationValues.insert("default_nurse_id",
                                         nullableId(values.organization.defaultNurseId));
            newOrganizationValues.insert(
                "ultrasound_device_name",
                values.organization.ultrasoundDeviceName.trimmed().isEmpty()
                    ? QVariant()
                    : QVariant(values.organization.ultrasoundDeviceName.trimmed()));
            newOrganizationValues.insert(
                "logo", values.organization.logoData.isEmpty()
                            ? QVariant(QMetaType(QMetaType::QByteArray))
                            : QVariant(values.organization.logoData.toBase64()));
            if (!upsert(QStringLiteral("organizationSettings"),
                        QStringLiteral("organization_id"),
                        values.organization.organizationId,
                        newOrganizationValues, errors))
                return false;
        }
    }

    // Schema noua este sursa unica pentru bazele create sau migrate complet.
    if (hasNewSchema())
        return true;

    // Compatibilitate de scriere pentru bazele 4.1.x care nu au trecut inca
    // prin migrarea schemei de setari.
    QVariantMap organizationValues;
    organizationValues.insert("id_users", userId);
    organizationValues.insert("id_organizations",
                              nullableId(values.organization.organizationId));
    organizationValues.insert("id_doctors",
                              nullableId(values.organization.defaultDoctorId));
    organizationValues.insert("id_nurses",
                              nullableId(values.organization.defaultNurseId));
    organizationValues.insert(
        "brandUSG", values.organization.ultrasoundDeviceName.trimmed().isEmpty()
                        ? QVariant()
                        : QVariant(values.organization.ultrasoundDeviceName.trimmed()));
    organizationValues.insert(
        "logo", values.organization.logoData.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QByteArray))
                    : QVariant(values.organization.logoData.toBase64()));

    if (!upsert(QStringLiteral("constants"),
                QStringLiteral("id_users"),
                userId, organizationValues, errors))
        return false;

    // inseram valorile
    QVariantMap userValues;
    userValues.insert("id_users", userId);
    userValues.insert("showQuestionCloseApp",
                      databaseBoolean(values.user.confirmOnExit));
    userValues.insert("showUserManual",
                      databaseBoolean(values.application.showUserManualOnStartup));
    userValues.insert("updateListDoc",
                      values.application.documentJournalRefreshIntervalSeconds);
    userValues.insert("showDesignerMenuPrint",
                      databaseBoolean(values.user.printMenuMode == Settings::PrintMenuMode::PreviewAndDesigner));
    userValues.insert("checkNewVersionApp",
                      databaseBoolean(values.application.checkForUpdatesOnStartup));
    userValues.insert("databasesArchiving",
                      databaseBoolean(values.user.archiveSqliteOnExit));
    userValues.insert("showAsistantHelper",
                      databaseBoolean(values.application.showAssistantOnStartup));
    userValues.insert("showDocumentsInSeparatWindow",
                      databaseBoolean(values.user.openDocumentsInSeparateWindows));
    userValues.insert("minimizeAppToTray",
                      databaseBoolean(values.user.minimizeToTray));

    return upsert(QStringLiteral("userPreferences"),
                  QStringLiteral("id_users"),
                  userId, userValues, errors);
}

bool SettingsRepository::hasNewSchema() const
{
    const QSqlDatabase database = m_database.getDatabase();
    const QStringList tables = database.tables(QSql::Tables);
    const QMap<QString, QStringList> requiredColumns{
        {QStringLiteral("applicationSettings"),
         {QStringLiteral("user_id"),
          QStringLiteral("check_for_updates_on_startup"),
          QStringLiteral("show_user_manual_on_startup"),
          QStringLiteral("show_assistant_on_startup"),
          QStringLiteral("document_journal_refresh_interval_seconds")}},
        {QStringLiteral("organizationSettings"),
         {QStringLiteral("organization_id"), QStringLiteral("default_doctor_id"),
          QStringLiteral("default_nurse_id"),
          QStringLiteral("ultrasound_device_name"), QStringLiteral("logo")}},
        {QStringLiteral("userSettings"),
         {QStringLiteral("user_id"), QStringLiteral("default_organization_id"),
          QStringLiteral("minimize_to_tray"), QStringLiteral("confirm_on_exit"),
          QStringLiteral("archive_sqlite_on_exit"),
          QStringLiteral("open_documents_in_separate_windows"),
          QStringLiteral("print_menu_mode")}}
    };

    for (auto table = requiredColumns.cbegin(); table != requiredColumns.cend(); ++table) {
        if (!tables.contains(table.key(), Qt::CaseInsensitive))
            return false;
        const QSqlRecord record = database.record(table.key());
        for (const QString &column : table.value()) {
            if (!record.contains(column))
                return false;
        }
    }
    return true;
}

bool SettingsRepository::recordExists(const QString &tableName,
                                      const QString &keyColumn, int keyValue,
                                      bool &exists, QStringList &errors) const
{
    exists = false;
    const QMap<QString, QString> allowedKeys{
        {QStringLiteral("constants"), QStringLiteral("id_users")},
        {QStringLiteral("userPreferences"), QStringLiteral("id_users")},
        {QStringLiteral("applicationSettings"), QStringLiteral("user_id")},
        {QStringLiteral("userSettings"), QStringLiteral("user_id")},
        {QStringLiteral("organizationSettings"), QStringLiteral("organization_id")}
    };
    if (!allowedKeys.contains(tableName) || allowedKeys.value(tableName) != keyColumn) {
        errors << QObject::tr("Tabela '%1' nu este acceptată de repository-ul setărilor.")
                      .arg(tableName);
        return false;
    }

    QSqlQuery query(m_database.getDatabase());
    if (!query.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = ?")
                           .arg(tableName, keyColumn))) {
        errors << query.lastError().text();
        return false;
    }
    query.addBindValue(keyValue);
    if (!query.exec() || !query.next()) {
        errors << QObject::tr("Verificarea tabelei '%1' a eșuat: %2")
                      .arg(tableName, query.lastError().text());
        return false;
    }

    const int count = query.value(0).toInt();
    if (count > 1) {
        errors << QObject::tr("Tabela '%1' conține %2 înregistrări pentru utilizatorul %3.")
                      .arg(tableName)
                      .arg(count)
                      .arg(keyValue);
        return false;
    }

    exists = count == 1;
    return true;
}

bool SettingsRepository::upsert(const QString &tableName,
                                const QString &keyColumn, int keyValue,
                                const QVariantMap &values,
                                QStringList &errors) const
{
    bool exists = false;
    if (!recordExists(tableName, keyColumn, keyValue, exists, errors))
        return false;

    const QString source = QStringLiteral("SettingsRepository");
    if (!exists)
        return m_database.insertIntoTable(source, tableName, values, errors);

    QVariantMap updateValues = values;
    updateValues.remove(keyColumn);
    const QMap<QString, QVariant> where{{keyColumn, keyValue}};
    return m_database.updateTable(source, tableName, updateValues, where, errors);
}
