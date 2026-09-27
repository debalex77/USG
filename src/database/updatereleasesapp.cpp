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

#include "updatereleasesapp.h"
#include "uuidmigrationplan.h"
#include "common/cloudconnectioncontext.h"

#include "common/sessioncontext.h"
#include "settings/settingsservice.h"

#include <QMap>
#include <QRegularExpression>
#include <QUuid>

namespace {

    bool hasColumn(const QSqlDatabase &database,
                   const QString &table,
                   const QString &column)
    {
        return database.record(table).contains(column);
    }

    bool addColumnIfMissing(const QSqlDatabase &database,
                            const QString &table,
                            const QString &column,
                            const QString &definition,
                            const QString &migration)
    {
        if (hasColumn(database, table, column))
            return true;

        QSqlQuery query(database);
        const QString sql = QStringLiteral("ALTER TABLE `%1` ADD COLUMN `%2` %3")
                                .arg(table, column, definition);
        if (!query.exec(sql)) {
            qCritical(logCritical())
                << migration << QObject::tr("nu a putut adăuga coloana %1.%2:")
                                    .arg(table, column)
                << query.lastError().text();
            return false;
        }

        qInfo(logInfo()) << migration
                         << QObject::tr("coloană adăugată: %1.%2").arg(table, column);
        return true;
    }

    bool ensureGestation2Columns(QSqlDatabase currentDb)
    {
        QSqlQuery query(currentDb);
        // Read the schema through the active driver (SQLite or MariaDB/MySQL).
        if (!query.exec(QStringLiteral("SELECT * FROM tableGestation2 WHERE 1=0"))) {
            qCritical(logCritical()) << "Migrare Gestation2: schema nu poate fi citită:"
                                     << query.lastError().text();
            return false;
        }
        const QSqlRecord columns = query.record();
        query.finish();
        for (const QString &column : {QStringLiteral("fetalPrezentation"),
                                      QStringLiteral("multiplePregnancy")}) {
            if (columns.contains(column))
                continue;
            // These values are combo-box indices, matching the new-database schema.
            if (!query.exec(QStringLiteral(
                    "ALTER TABLE tableGestation2 ADD COLUMN `%1` INT DEFAULT 0").arg(column))) {
                qCritical(logCritical()) << "Migrare Gestation2: coloana nu poate fi adăugată:"
                                         << column << query.lastError().text();
                return false;
            }
        }
        qInfo(logInfo())
            << "Au fost adaugate coloanele 'fetalPrezentation' si 'multiplePregnancy' in tabela 'tableGestation2'.";
        return true;
    }

    bool migrateInvestigationsOwnerColumn(QSqlDatabase currentDb)
    {
        const bool sqlite = currentDb.driverName() == QStringLiteral("QSQLITE");
        QSqlQuery q(currentDb);
        QString ownerType;
        QString groupIdType = QStringLiteral("INTEGER");
        if (sqlite) {
            if (!q.exec(QStringLiteral("PRAGMA table_info(investigations)")))
                return false;
            while (q.next()) {
                if (q.value(1).toString() == QStringLiteral("owner"))
                    ownerType = q.value(2).toString();
            }
        } else {
            if (!q.exec(QStringLiteral("SHOW COLUMNS FROM investigations LIKE 'owner'")) || !q.next())
                return false;
            ownerType = q.value(1).toString();
            if (!q.exec(QStringLiteral("SHOW COLUMNS FROM investigationsGroup LIKE 'id'")) || !q.next())
                return false;
            groupIdType = q.value(1).toString();
            static const QRegularExpression integerType(
                QStringLiteral("^(tinyint|smallint|mediumint|int|bigint)(\\([0-9]+\\))?( unsigned)?$"),
                QRegularExpression::CaseInsensitiveOption);
            if (!integerType.match(groupIdType).hasMatch()) {
                qCritical(logCritical()) << "Migrare owner: tip incompatibil pentru investigationsGroup.id";
                return false;
            }
        }
        q.finish();
        if (ownerType.contains(QStringLiteral("INT"), Qt::CaseInsensitive))
            return true; // Already migrated (or created with the current schema).
        if (ownerType.isEmpty()) {
            qCritical(logCritical()) << "Migrare owner: coloana investigations.owner lipsește.";
            return false;
        }

        // Resolve old labels and numeric IDs without modifying the source column.
        const QString mapping = QStringLiteral(
            "COALESCE((SELECT MIN(g.id) FROM investigationsGroup g WHERE g.name = i.owner), "
            "(SELECT g.id FROM investigationsGroup g WHERE CAST(g.id AS CHAR) = CAST(i.owner AS CHAR)))");
        if (!q.exec(QStringLiteral(
                "SELECT i.id FROM investigations i WHERE i.owner IS NOT NULL AND "
                "(%1 IS NULL OR (SELECT COUNT(*) FROM investigationsGroup g WHERE g.name=i.owner)>1) LIMIT 1")
                        .arg(mapping))) {
            qCritical(logCritical()) << "Migrare owner: verificarea mapării a eșuat:" << q.lastError().text();
            return false;
        }
        if (q.next()) {
            qCritical(logCritical()) << "Migrare owner: sistem inexistent sau ambiguu pentru investigația" << q.value(0);
            return false;
        }
        q.finish();

        // SQLite savepoints also work inside the enclosing 4.0.1 transaction.
        if (sqlite && !q.exec(QStringLiteral("SAVEPOINT migrate_owner")))
            return false;
        const auto fail = [&]() {
            qCritical(logCritical()) << "Migrare owner:" << q.lastError().text();
            q.finish();
            if (sqlite) {
                QSqlQuery rollback(currentDb);
                rollback.exec(QStringLiteral("ROLLBACK TO migrate_owner"));
                rollback.exec(QStringLiteral("RELEASE migrate_owner"));
            }
            return false;
        };
        if (!currentDb.record(QStringLiteral("investigations")).contains(QStringLiteral("owner_id"))) {
            const QString reference = sqlite
                ? QStringLiteral(" REFERENCES investigationsGroup(id) ON DELETE SET NULL ON UPDATE RESTRICT")
                : QString();
            if (!q.exec(QStringLiteral("ALTER TABLE investigations ADD COLUMN owner_id %1 NULL%2")
                            .arg(groupIdType, reference)))
                return fail();
        }
        if (!q.exec(QStringLiteral("UPDATE investigations AS i SET owner_id = %1").arg(mapping)))
            return fail();

        if (sqlite) {
            // Do not drop/rebuild the parent table: that can cascade into pricing.
            // Temporarily remove only schema objects that reference the old column.
            QList<QPair<QString, QString>> objects;
            if (!q.exec(QStringLiteral(
                    "SELECT type, name, sql FROM sqlite_master WHERE sql IS NOT NULL "
                    "AND ((tbl_name='investigations' AND type IN ('index','trigger')) OR type='view')")))
                return fail();
            const QRegularExpression ownerWord(QStringLiteral("\\bowner\\b"), QRegularExpression::CaseInsensitiveOption);
            while (q.next()) {
                const QString definition = q.value(2).toString();
                if (!ownerWord.match(definition).hasMatch())
                    continue;
                if (q.value(0).toString() == "view" && !definition.contains("investigations", Qt::CaseInsensitive))
                    continue;
                QString name = q.value(1).toString();
                name.replace('"', QStringLiteral("\"\""));
                objects.append({QString("DROP %1 \"%2\"").arg(q.value(0).toString(), name), definition});
            }
            q.finish();
            for (const auto &object : objects) {
                if (!q.exec(object.first))
                    return fail();
            }
            if (!q.exec(QStringLiteral("ALTER TABLE investigations DROP COLUMN owner"))
                || !q.exec(QStringLiteral("ALTER TABLE investigations RENAME COLUMN owner_id TO owner")))
                return fail();
            for (const auto &object : objects) {
                if (!q.exec(object.second))
                    return fail();
            }
            if (!q.exec(QStringLiteral("RELEASE migrate_owner")))
                return fail();
        } else {
            // One ALTER avoids leaving the table without owner after a failed rename.
            if (!q.exec(QStringLiteral(
                    "ALTER TABLE investigations DROP COLUMN owner, "
                    "CHANGE COLUMN owner_id owner %1 NULL, "
                    "ADD CONSTRAINT investigations_investigationsGroup_FK FOREIGN KEY (owner) "
                    "REFERENCES investigationsGroup(id) ON DELETE SET NULL ON UPDATE RESTRICT")
                            .arg(groupIdType)))
                return fail();
        }
        qInfo(logInfo()) << "Migrarea investigations.owner la identificator numeric s-a finalizat.";
        return true;
    }

}

UpdateReleasesApp::UpdateReleasesApp(QObject *parent) :
    QObject(parent),
    db(new DataBase(this))
{
    connect(db, &DataBase::uuidProgress,
            this, &UpdateReleasesApp::migrationProgress);
}

UpdateReleasesApp::~UpdateReleasesApp()
{
}

bool UpdateReleasesApp::validateMigrationContext(const QVersionNumber &current,
                                                 const QVersionNumber &target) const
{
    const QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical())
            << "Actualizarea a fost anulată: conexiunea bazei de date nu este deschisă.";
        return false;
    }

    const QString driverName = currentDb.driverName().toUpper();
    const bool sqliteDriver = driverName == QStringLiteral("QSQLITE");
    const bool mysqlDriver = driverName == QStringLiteral("QMYSQL");

    if (!sqliteDriver && !mysqlDriver) {
        qCritical(logCritical())
            << tr("Actualizarea a fost anulată: driver SQL nesuportat: %1.")
                   .arg(currentDb.driverName());
        return false;
    }

    const MainDatabaseBackend backend =
        MainDatabaseConnectionContext::instance().backend();
    const bool sqliteSelected = backend == MainDatabaseBackend::SQLite;
    const bool mariaDbSelected = backend == MainDatabaseBackend::MariaDb;
    if (sqliteSelected == mariaDbSelected
        || sqliteDriver != sqliteSelected
        || mysqlDriver != mariaDbSelected) {
        qCritical(logCritical())
            << tr("Actualizarea a fost anulată: motorul selectat nu corespunde "
                  "driverului conexiunii (%1).")
                   .arg(currentDb.driverName());
        return false;
    }

    if (current > target) {
        qCritical(logCritical())
            << tr("Actualizarea a fost anulată: baza de date are versiunea %1, "
                  "mai nouă decât aplicația %2. Folosiți o versiune compatibilă a aplicației.")
                   .arg(current.toString(), target.toString());
        return false;
    }

    return true;
}

bool UpdateReleasesApp::validateMigrationPrerequisites(
    const QVersionNumber &migrationVersion) const
{
    // Migrarea 4.0.1 actualizează documentele și recreează view-urile aplicației.
    // Verificarea are loc înainte de primul DDL, ca o bază incompletă să nu rămână
    // actualizată doar parțial (în special pe MariaDB, unde DDL face auto-commit).
    if (migrationVersion != QVersionNumber(4, 0, 1)
        && migrationVersion != QVersionNumber(4, 1, 0))
        return true;

    const QSqlDatabase currentDb = db->getDatabase();
    const QStringList existingTables = currentDb.tables(QSql::Tables);
    if (migrationVersion == QVersionNumber(4, 1, 0)) {
        const bool hasOld = existingTables.contains(QStringLiteral("pacients"), Qt::CaseInsensitive);
        const bool hasNew = existingTables.contains(QStringLiteral("patients"), Qt::CaseInsensitive);
        if (hasOld == hasNew) {
            qCritical(logCritical())
                << "Migrarea la 4.1.0 necesită exact una dintre tabelele pacients/patients.";
            return false;
        }
        for (const QString &table : {QStringLiteral("orderEcho"), QStringLiteral("reportEcho")}) {
            if (!existingTables.contains(table, Qt::CaseInsensitive)) {
                qCritical(logCritical()) << "Migrarea la 4.1.0: lipsește tabela" << table;
                return false;
            }
        }
        return true;
    }
    const QStringList requiredTables = migrationVersion == QVersionNumber(4, 1, 0)
        ? QStringList{QStringLiteral("pacients"), QStringLiteral("orderEcho"),
                      QStringLiteral("reportEcho")}
        : QStringList{
        QStringLiteral("orderEcho"),
        QStringLiteral("reportEcho"),
        QStringLiteral("users"),
        QStringLiteral("doctors"),
        QStringLiteral("nurses"),
        QStringLiteral("typesPrices"),
        QStringLiteral("organizations"),
        QStringLiteral("contracts")};

    QStringList missingTables;
    for (const QString &requiredTable : requiredTables) {
        if (!existingTables.contains(requiredTable, Qt::CaseInsensitive))
            missingTables.append(requiredTable);
    }

    if (!missingTables.isEmpty()) {
        qCritical(logCritical())
            << tr("Migrarea la %1 a fost anulată înainte de modificarea schemei. "
                  "Lipsesc tabelele obligatorii: %2.")
                   .arg(migrationVersion.toString(), missingTables.join(", "));
        return false;
    }

    return true;
}

bool UpdateReleasesApp::validatePostMigration(
    const QVersionNumber &migrationVersion) const
{
    if (migrationVersion == QVersionNumber(4, 1, 0)) {
        const QSqlDatabase currentDb = db->getDatabase();
        const QStringList tables = currentDb.tables(QSql::Tables);
        const QSqlRecord patientRecord = currentDb.record(QStringLiteral("patients"));
        const QStringList requiredColumns = {
            QStringLiteral("deletion_mark"), QStringLiteral("idnp"),
            QStringLiteral("last_name"), QStringLiteral("first_name"),
            QStringLiteral("middle_name"), QStringLiteral("medical_policy")};
        if (!tables.contains(QStringLiteral("patients"), Qt::CaseInsensitive)
            || tables.contains(QStringLiteral("pacients"), Qt::CaseInsensitive)) {
            qCritical(logCritical()) << "Verificarea 4.1.0: tabela patients lipsește sau pacients încă există.";
            return false;
        }
        for (const QString &column : requiredColumns) {
            if (!patientRecord.contains(column)) {
                qCritical(logCritical()) << "Verificarea 4.1.0: lipsește patients." + column;
                return false;
            }
        }
        for (const QString &table : {QStringLiteral("orderEcho"), QStringLiteral("reportEcho")}) {
            if (!currentDb.record(table).contains(QStringLiteral("patient_id"))) {
                qCritical(logCritical()) << "Verificarea 4.1.0: lipsește" << table + ".patient_id";
                return false;
            }
        }
        if (tables.contains(QStringLiteral("imagesReports"), Qt::CaseInsensitive)
            && !currentDb.record(QStringLiteral("imagesReports")).contains(QStringLiteral("patient_id"))) {
            qCritical(logCritical()) << "Verificarea 4.1.0: lipsește imagesReports.patient_id.";
            return false;
        }
        const QSqlRecord appointmentRecord = currentDb.record(QStringLiteral("patientAppointments"));
        if (!tables.contains(QStringLiteral("patientAppointments"), Qt::CaseInsensitive)
            || !tables.contains(QStringLiteral("patientAppointmentInvestigations"), Qt::CaseInsensitive)
            || !appointmentRecord.contains(QStringLiteral("patient_id"))
            || !appointmentRecord.contains(QStringLiteral("investigation_id"))) {
            qCritical(logCritical()) << "Verificarea 4.1.0: schema patientAppointments este incompletă.";
            return false;
        }
        QSqlQuery integrity(currentDb);
        const QStringList checks = {
            QStringLiteral("SELECT COUNT(*) FROM orderEcho o LEFT JOIN patients p ON p.id=o.patient_id WHERE p.id IS NULL"),
            QStringLiteral("SELECT COUNT(*) FROM reportEcho r LEFT JOIN patients p ON p.id=r.patient_id WHERE p.id IS NULL")};
        for (const QString &sql : checks) {
            if (!integrity.exec(sql) || !integrity.next() || integrity.value(0).toLongLong() != 0) {
                qCritical(logCritical()) << "Verificarea relațiilor 4.1.0 a eșuat:" << integrity.lastError().text();
                return false;
            }
        }
        QSqlQuery viewQuery(currentDb);
        if (!viewQuery.exec(QStringLiteral("SELECT * FROM v_patients_completer_active LIMIT 1"))) {
            qCritical(logCritical()) << "Verificarea view-ului pacienților a eșuat:" << viewQuery.lastError().text();
            return false;
        }
        return true;
    }

    if (migrationVersion != QVersionNumber(4, 0, 1))
        return true;

    const QSqlDatabase currentDb = db->getDatabase();
    const QStringList existingTables = currentDb.tables(QSql::Tables);
    const QStringList existingViews = currentDb.tables(QSql::Views);

    const QStringList requiredTables = {
        QStringLiteral("doc_sequences"),
        QStringLiteral("orderEcho"),
        QStringLiteral("reportEcho"),
        QStringLiteral("patientAppointments"),
        QStringLiteral("patientAppointmentInvestigations")
    };
    const QStringList requiredViews = {
        QStringLiteral("v_users_combo_active"),
        QStringLiteral("v_doctors_active"),
        QStringLiteral("v_nurses_active"),
        QStringLiteral("v_types_prices_active"),
        QStringLiteral("v_organizations_active"),
        QStringLiteral("v_contracts_listView_active")
    };

    for (const QString &table : requiredTables) {
        if (!existingTables.contains(table, Qt::CaseInsensitive)) {
            qCritical(logCritical())
                << tr("Verificarea după migrarea la %1 a eșuat: lipsește tabela %2.")
                       .arg(migrationVersion.toString(), table);
            return false;
        }
    }

    const QSqlRecord appointmentRecord = currentDb.record(QStringLiteral("patientAppointments"));
    if (!appointmentRecord.contains(QStringLiteral("patient_id"))
        || !appointmentRecord.contains(QStringLiteral("investigation_id"))) {
        qCritical(logCritical())
            << "Verificarea 4.0.1: schema patientAppointments este incompletă.";
        return false;
    }

    for (const QString &view : requiredViews) {
        if (!existingViews.contains(view, Qt::CaseInsensitive)) {
            qCritical(logCritical())
                << tr("Verificarea după migrarea la %1 a eșuat: lipsește view-ul %2.")
                       .arg(migrationVersion.toString(), view);
            return false;
        }
    }

    if (!currentDb.record(QStringLiteral("orderEcho")).contains(QStringLiteral("docYear"))
        || !currentDb.record(QStringLiteral("reportEcho")).contains(QStringLiteral("docYear"))) {
        qCritical(logCritical())
            << tr("Verificarea după migrarea la %1 a eșuat: lipsește coloana docYear.")
                   .arg(migrationVersion.toString());
        return false;
    }

    const QString patientTable = existingTables.contains(QStringLiteral("patients"), Qt::CaseInsensitive)
                                     ? QStringLiteral("patients") : QStringLiteral("pacients");
    const QStringList documentTables = {QStringLiteral("orderEcho"), QStringLiteral("reportEcho")};
    QStringList patientColumns;
    for (const QString &table : documentTables) {
        const QSqlRecord record = currentDb.record(table);
        const QString column = record.contains(QStringLiteral("patient_id"))
                                   ? QStringLiteral("patient_id")
                               : record.contains(QStringLiteral("id_pacients"))
                                   ? QStringLiteral("id_pacients")
                               : record.contains(QStringLiteral("id_patients"))
                                   ? QStringLiteral("id_patients") : QString();
        if (column.isEmpty()) {
            qCritical(logCritical()) << "Verificarea 4.0.1: coloana pacientului lipsește în" << table;
            return false;
        }
        patientColumns.append(column);
    }

    QStringList uuidTables = {
        QStringLiteral("contracts"), QStringLiteral("doctors"),
        QStringLiteral("investigations"), QStringLiteral("investigationsGroup"),
        QStringLiteral("conclusionTemplates"), QStringLiteral("formationsSystemTemplates"),
        QStringLiteral("nurses"), QStringLiteral("orderEcho"),
        QStringLiteral("organizations"), patientTable,
        QStringLiteral("pricings"), QStringLiteral("reportEcho"),
        QStringLiteral("typesPrices"), QStringLiteral("users")
    };
    if (MainDatabaseConnectionContext::instance().isMariaDb())
        uuidTables.append(QStringLiteral("imagesReports"));

    const QString uuidLengthExpression = MainDatabaseConnectionContext::instance().isMariaDb()
                                             ? QStringLiteral("OCTET_LENGTH(uuid)")
                                             : QStringLiteral("length(uuid)");
    for (const QString &table : uuidTables) {
        if (!existingTables.contains(table, Qt::CaseInsensitive)
            || !currentDb.record(table).contains(QStringLiteral("uuid"))) {
            qCritical(logCritical())
                << tr("Verificarea UUID a eșuat: tabela sau coloana uuid lipsește în %1.")
                       .arg(table);
            return false;
        }

        QSqlQuery uuidQuery(currentDb);
        const QString sql = QStringLiteral(
            "SELECT "
            "SUM(CASE WHEN uuid IS NULL OR %1 <> 16 THEN 1 ELSE 0 END), "
            "COUNT(*) - COUNT(DISTINCT uuid) FROM %2")
                                .arg(uuidLengthExpression, table);
        if (!uuidQuery.exec(sql) || !uuidQuery.next()) {
            qCritical(logCritical())
                << tr("Verificarea UUID nu a putut fi executată pentru %1: %2")
                       .arg(table, uuidQuery.lastError().text());
            return false;
        }

        const qint64 invalidUuid = uuidQuery.value(0).toLongLong();
        const qint64 duplicateUuid = uuidQuery.value(1).toLongLong();
        if (invalidUuid > 0 || duplicateUuid > 0) {
            qCritical(logCritical())
                << tr("Verificarea UUID a eșuat pentru %1: %2 valori invalide, "
                      "%3 valori duplicate.")
                       .arg(table)
                       .arg(invalidUuid)
                       .arg(duplicateUuid);
            return false;
        }
    }

    const QStringList relationChecks = {
        QStringLiteral("SELECT COUNT(*) FROM orderEcho o LEFT JOIN %1 p "
                       "ON p.id=o.%2 WHERE p.id IS NULL")
            .arg(patientTable, patientColumns.at(0)),
        QStringLiteral("SELECT COUNT(*) FROM reportEcho r LEFT JOIN %1 p "
                       "ON p.id=r.%2 WHERE p.id IS NULL")
            .arg(patientTable, patientColumns.at(1)),
        QStringLiteral("SELECT COUNT(*) FROM reportEcho r LEFT JOIN orderEcho o "
                       "ON o.id=r.id_orderEcho WHERE o.id IS NULL")
    };
    for (const QString &sql : relationChecks) {
        QSqlQuery relationQuery(currentDb);
        if (!relationQuery.exec(sql) || !relationQuery.next()
            || relationQuery.value(0).toLongLong() != 0) {
            qCritical(logCritical())
                << tr("Verificarea relațiilor după migrarea la %1 a eșuat: %2")
                       .arg(migrationVersion.toString(), relationQuery.lastError().text());
            return false;
        }
    }

    qInfo(logInfo())
        << tr("Verificările după migrarea la %1 s-au finalizat cu succes.")
               .arg(migrationVersion.toString());
    return true;
}

bool UpdateReleasesApp::ensurePatientAppointmentsSchema()
{
    QSqlDatabase currentDb = db->getDatabase();
    const QStringList tables = currentDb.tables(QSql::Tables);
    const auto actualTableName = [&tables](const QString &wanted) {
        for (const QString &table : tables) {
            if (table.compare(wanted, Qt::CaseInsensitive) == 0)
                return table;
        }
        return QString();
    };

    QString oldTable = actualTableName(QStringLiteral("registrationPatients"));
    QString newTable = actualTableName(QStringLiteral("patientAppointments"));
    if (!oldTable.isEmpty() && !newTable.isEmpty()) {
        qCritical(logCritical())
            << "Schema programărilor este ambiguă: există registrationPatients și patientAppointments.";
        return false;
    }

    QSqlQuery query(currentDb);
    if (newTable.isEmpty() && !oldTable.isEmpty()) {
        if (!query.exec(QStringLiteral("ALTER TABLE `%1` RENAME TO patientAppointments")
                            .arg(oldTable))) {
            qCritical(logCritical()) << "Redenumirea registrationPatients a eșuat:"
                                     << query.lastError().text();
            return false;
        }
        newTable = QStringLiteral("patientAppointments");
        qInfo(logInfo()) << "Tabela registrationPatients a fost redenumită în patientAppointments.";
    }

    if (newTable.isEmpty()) {
        const QString resource = MainDatabaseConnectionContext::instance().isSqlite()
            ? QStringLiteral(":/sql/sqlite/tables/patient_appointments.sql")
            : QStringLiteral(":/sql/mariadb/tables/patient_appointments.sql");
        QFile file(resource);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical(logCritical()) << "Schema patientAppointments nu poate fi citită:" << resource;
            return false;
        }
        const QStringList statements = QString::fromUtf8(file.readAll()).split(';', Qt::SkipEmptyParts);
        for (const QString &statement : statements) {
            if (!query.exec(statement.trimmed())) {
                qCritical(logCritical()) << "Crearea patientAppointments a eșuat:"
                                         << query.lastError().text();
                return false;
            }
        }
        qInfo(logInfo()) << "Tabela patientAppointments a fost creată.";
    }

    QSqlRecord record = currentDb.record(QStringLiteral("patientAppointments"));
    const QString integerType = MainDatabaseConnectionContext::instance().isMariaDb()
                                    ? QStringLiteral("BIGINT UNSIGNED NULL")
                                    : QStringLiteral("INTEGER");
    for (const QString &column : {QStringLiteral("patient_id"),
                                  QStringLiteral("investigation_id")}) {
        if (record.contains(column))
            continue;
        if (!query.exec(QStringLiteral("ALTER TABLE patientAppointments ADD COLUMN `%1` %2")
                            .arg(column, integerType))) {
            qCritical(logCritical()) << "Adăugarea patientAppointments." + column + " a eșuat:"
                                     << query.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Coloană adăugată în patientAppointments:" << column;
    }

    // In v3 this field was VARCHAR(150), sufficient for a single
    // investigation only. It remains as a readable/cached description, while
    // the actual multi-selection is stored in the child table below.
    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        QSqlQuery columnTypeQuery(currentDb);
        columnTypeQuery.prepare(QStringLiteral(
            "SELECT DATA_TYPE FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='patientAppointments' "
            "AND COLUMN_NAME='investigations'"));
        if (!columnTypeQuery.exec() || !columnTypeQuery.next()) {
            qCritical(logCritical())
                << "Tipul patientAppointments.investigations nu poate fi verificat:"
                << columnTypeQuery.lastError().text();
            return false;
        }
        if (columnTypeQuery.value(0).toString().compare(QStringLiteral("text"),
                                                        Qt::CaseInsensitive) != 0) {
            if (!query.exec(QStringLiteral(
                    "ALTER TABLE patientAppointments MODIFY COLUMN investigations TEXT NULL"))) {
                qCritical(logCritical())
                    << "Extinderea patientAppointments.investigations a eșuat:"
                    << query.lastError().text();
                return false;
            }
            qInfo(logInfo())
                << "Coloana patientAppointments.investigations a fost extinsă la TEXT.";
        }
    }

    if (!currentDb.tables(QSql::Tables).contains(
            QStringLiteral("patientAppointmentInvestigations"), Qt::CaseInsensitive)) {
        QString createRelations;
        if (MainDatabaseConnectionContext::instance().isMariaDb()) {
            const auto mariaDbColumnType = [&currentDb](const QString &table,
                                                        const QString &column) -> QString {
                QSqlQuery typeQuery(currentDb);
                typeQuery.prepare(QStringLiteral(
                    "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS "
                    "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME=? AND COLUMN_NAME=?"));
                typeQuery.addBindValue(table);
                typeQuery.addBindValue(column);
                if (!typeQuery.exec() || !typeQuery.next())
                    return {};
                return typeQuery.value(0).toString().trimmed();
            };

            // MariaDB requires both sides of a foreign key to have exactly the
            // same integer type, including the UNSIGNED attribute. Databases
            // migrated from v3 commonly use signed INT ids, while newly
            // created v4 databases use BIGINT UNSIGNED.
            const QString appointmentIdType =
                mariaDbColumnType(QStringLiteral("patientAppointments"), QStringLiteral("id"));
            const QString investigationIdType =
                mariaDbColumnType(QStringLiteral("investigations"), QStringLiteral("id"));
            if (appointmentIdType.isEmpty() || investigationIdType.isEmpty()) {
                qCritical(logCritical())
                    << "Nu au putut fi determinate tipurile cheilor pentru programări.";
                return false;
            }

            createRelations = QStringLiteral(
                  "CREATE TABLE patientAppointmentInvestigations ("
                  "appointment_id %1 NOT NULL,"
                  "investigation_id %2 NOT NULL,"
                  "position INT NOT NULL DEFAULT 0,"
                  "PRIMARY KEY(appointment_id, investigation_id),"
                  "KEY idx_patientAppointmentInvestigations_investigation(investigation_id),"
                  "CONSTRAINT fk_patientAppointmentInvestigations_appointment "
                  "FOREIGN KEY(appointment_id) REFERENCES patientAppointments(id) ON DELETE CASCADE,"
                  "CONSTRAINT fk_patientAppointmentInvestigations_investigation "
                  "FOREIGN KEY(investigation_id) REFERENCES investigations(id) ON DELETE RESTRICT"
                  ") ENGINE=InnoDB")
                                  .arg(appointmentIdType, investigationIdType);
        } else {
            createRelations = QStringLiteral(
                  "CREATE TABLE patientAppointmentInvestigations ("
                  "appointment_id INTEGER NOT NULL,"
                  "investigation_id INTEGER NOT NULL,"
                  "position INTEGER NOT NULL DEFAULT 0,"
                  "PRIMARY KEY(appointment_id, investigation_id),"
                  "FOREIGN KEY(appointment_id) REFERENCES patientAppointments(id) ON DELETE CASCADE,"
                  "FOREIGN KEY(investigation_id) REFERENCES investigations(id) ON DELETE RESTRICT)");
        }
        if (!query.exec(createRelations)) {
            qCritical(logCritical()) << "Crearea patientAppointmentInvestigations a eșuat:"
                                     << query.lastError().text();
            return false;
        }
        if (MainDatabaseConnectionContext::instance().isSqlite()
            && !query.exec(QStringLiteral(
                "CREATE INDEX IF NOT EXISTS idx_patientAppointmentInvestigations_investigation "
                "ON patientAppointmentInvestigations(investigation_id)"))) {
            qCritical(logCritical()) << "Crearea indexului investigațiilor programate a eșuat:"
                                     << query.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Tabela patientAppointmentInvestigations a fost creată.";
    }

    const QStringList refreshedTables = currentDb.tables(QSql::Tables);
    const bool hasPatients = refreshedTables.contains(QStringLiteral("patients"), Qt::CaseInsensitive);
    const bool hasPacients = refreshedTables.contains(QStringLiteral("pacients"), Qt::CaseInsensitive);
    QString patientMatch;
    if (hasPatients) {
        patientMatch = MainDatabaseConnectionContext::instance().isMariaDb()
            ? QStringLiteral("TRIM(CONCAT_WS(' ', p.last_name, NULLIF(p.first_name,''), NULLIF(p.middle_name,'')))=TRIM(a.dataPatient)")
            : QStringLiteral("TRIM(p.last_name || ' ' || p.first_name || CASE WHEN IFNULL(p.middle_name,'')='' THEN '' ELSE ' ' || p.middle_name END)=TRIM(a.dataPatient)");
    } else if (hasPacients) {
        patientMatch = MainDatabaseConnectionContext::instance().isMariaDb()
            ? QStringLiteral("TRIM(CONCAT_WS(' ', p.name, NULLIF(p.fName,''), NULLIF(p.mName,'')))=TRIM(a.dataPatient)")
            : QStringLiteral("TRIM(p.name || ' ' || IFNULL(p.fName,'') || CASE WHEN IFNULL(p.mName,'')='' THEN '' ELSE ' ' || p.mName END)=TRIM(a.dataPatient)");
    }
    if (!patientMatch.isEmpty()) {
        const QString patientTable = hasPatients ? QStringLiteral("patients") : QStringLiteral("pacients");
        const QString backfill = QStringLiteral(
            "UPDATE patientAppointments a SET patient_id=(SELECT MIN(p.id) FROM %1 p WHERE %2) "
            "WHERE patient_id IS NULL AND (SELECT COUNT(*) FROM %1 p WHERE %2)=1")
                                     .arg(patientTable, patientMatch);
        QString sql = backfill;
        if (MainDatabaseConnectionContext::instance().isSqlite())
            sql.replace(QStringLiteral("patientAppointments a"), QStringLiteral("patientAppointments AS a"));
        if (!query.exec(sql)) {
            qCritical(logCritical()) << "Completarea patientAppointments.patient_id a eșuat:"
                                     << query.lastError().text();
            return false;
        }
    }

    const QString investigationMatch = MainDatabaseConnectionContext::instance().isMariaDb()
        ? QStringLiteral("(TRIM(CONCAT(i.cod, ' - ', i.name))=TRIM(a.investigations) OR TRIM(i.name)=TRIM(a.investigations))")
        : QStringLiteral("(TRIM(i.cod || ' - ' || i.name)=TRIM(a.investigations) OR TRIM(i.name)=TRIM(a.investigations))");
    QString investigationSql = QStringLiteral(
        "UPDATE patientAppointments a SET investigation_id=(SELECT MIN(i.id) FROM investigations i WHERE %1) "
        "WHERE investigation_id IS NULL AND (SELECT COUNT(*) FROM investigations i WHERE %1)=1")
                                   .arg(investigationMatch);
    if (MainDatabaseConnectionContext::instance().isSqlite())
        investigationSql.replace(QStringLiteral("patientAppointments a"),
                                 QStringLiteral("patientAppointments AS a"));
    if (!query.exec(investigationSql)) {
        qCritical(logCritical()) << "Completarea patientAppointments.investigation_id a eșuat:"
                                 << query.lastError().text();
        return false;
    }

    const QString backfillRelations = MainDatabaseConnectionContext::instance().isMariaDb()
        ? QStringLiteral(
              "INSERT IGNORE INTO patientAppointmentInvestigations "
              "(appointment_id, investigation_id, position) "
              "SELECT id, investigation_id, 0 FROM patientAppointments "
              "WHERE investigation_id IS NOT NULL")
        : QStringLiteral(
              "INSERT OR IGNORE INTO patientAppointmentInvestigations "
              "(appointment_id, investigation_id, position) "
              "SELECT id, investigation_id, 0 FROM patientAppointments "
              "WHERE investigation_id IS NOT NULL");
    if (!query.exec(backfillRelations)) {
        qCritical(logCritical()) << "Transferul investigațiilor programate a eșuat:"
                                 << query.lastError().text();
        return false;
    }

    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        const QStringList indexes = {
            QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patientAppointments_date ON patientAppointments(dateDoc)"),
            QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patientAppointments_patient_id ON patientAppointments(patient_id)"),
            QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patientAppointments_investigation_id ON patientAppointments(investigation_id)")};
        for (const QString &sql : indexes) {
            if (!query.exec(sql)) {
                qCritical(logCritical()) << "Crearea indexului patientAppointments a eșuat:"
                                         << query.lastError().text();
                return false;
            }
        }
    } else {
        const QList<QPair<QString, QString>> indexes = {
            {QStringLiteral("idx_patientAppointments_date"), QStringLiteral("dateDoc")},
            {QStringLiteral("idx_patientAppointments_patient_id"), QStringLiteral("patient_id")},
            {QStringLiteral("idx_patientAppointments_investigation_id"), QStringLiteral("investigation_id")}};
        for (const auto &index : indexes) {
            QSqlQuery exists(currentDb);
            exists.prepare(QStringLiteral("SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS "
                                          "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='patientAppointments' "
                                          "AND INDEX_NAME=?"));
            exists.addBindValue(index.first);
            if (!exists.exec() || !exists.next())
                return false;
            if (exists.value(0).toInt() == 0
                && !query.exec(QStringLiteral("CREATE INDEX `%1` ON patientAppointments(`%2`)")
                                   .arg(index.first, index.second))) {
                qCritical(logCritical()) << "Crearea indexului" << index.first << "a eșuat:"
                                         << query.lastError().text();
                return false;
            }
        }
    }
    return true;
}

bool UpdateReleasesApp::ensureReportVideoSchema()
{
    if (!MainDatabaseConnectionContext::instance().isMariaDb())
        return true;

    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical()) << "reportVideo: conexiunea MariaDB nu este deschisă.";
        return false;
    }

    const auto parentIdType = [&](const QString &table) -> QString {
        QSqlQuery typeQuery(currentDb);
        typeQuery.prepare(QStringLiteral(R"(
            SELECT COLUMN_TYPE
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = ?
              AND COLUMN_NAME = 'id'
        )"));
        typeQuery.addBindValue(table);
        if (!typeQuery.exec() || !typeQuery.next()) {
            qCritical(logCritical()) << "reportVideo: tipul ID-ului nu a putut fi citit pentru"
                                     << table << typeQuery.lastError().text();
            return {};
        }
        const QString type = typeQuery.value(0).toString().trimmed();
        const QRegularExpression integerType(
            QStringLiteral(R"(^(?:tinyint|smallint|mediumint|int|integer|bigint)(?:\(\d+\))?(?: unsigned)?$)"),
            QRegularExpression::CaseInsensitiveOption);
        if (!integerType.match(type).hasMatch()) {
            qCritical(logCritical()) << "reportVideo: tip de ID incompatibil în" << table << type;
            return {};
        }
        return type;
    };

    const QString reportIdType = parentIdType(QStringLiteral("reportEcho"));
    if (reportIdType.isEmpty())
        return false;

    const QString table = QStringLiteral("reportVideo");
    if (!currentDb.tables(QSql::Tables).contains(table, Qt::CaseInsensitive)) {
        const QString orderIdType = parentIdType(QStringLiteral("orderEcho"));
        QString createSql = DataBaseCommon::getTextQryFromResource(
            QStringLiteral(":/sql/mariadb/tables/report_video.sql"));
        if (orderIdType.isEmpty() || createSql.isEmpty())
            return false;
        for (const auto &columnAndType : {
                 QPair<QString, QString>{QStringLiteral("id_orderEcho"), orderIdType},
                 QPair<QString, QString>{QStringLiteral("id_reportEcho"), reportIdType}}) {
            const QRegularExpression definition(
                QStringLiteral(R"((`%1`\s+)BIGINT UNSIGNED\b)")
                    .arg(QRegularExpression::escape(columnAndType.first)));
            if (!createSql.contains(definition)) {
                qCritical(logCritical()) << "reportVideo: definiția coloanei lipsește:"
                                         << columnAndType.first;
                return false;
            }
            createSql.replace(definition,
                              QStringLiteral("\\1") + columnAndType.second);
        }
        QSqlQuery create(currentDb);
        if (!create.exec(createSql)) {
            qCritical(logCritical()) << "reportVideo: tabela nu a putut fi creată:"
                                     << create.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Tabela reportVideo a fost creată.";
        return true;
    }

    QSqlRecord record = currentDb.record(table);
    if (!record.contains(QStringLiteral("id"))) {
        qCritical(logCritical()) << "reportVideo: lipsește cheia primară id.";
        return false;
    }

    const QStringList requiredColumns = {QStringLiteral("id_orderEcho"),
                                         QStringLiteral("relative_path"),
                                         QStringLiteral("file_name")};
    bool missingRequiredColumn = false;
    for (const QString &column : requiredColumns)
        missingRequiredColumn |= !record.contains(column);

    // Some pre-4.2.0 MariaDB installations have an unused reportVideo table
    // containing only id/nameVideo/path_file. Extend it without removing the
    // legacy columns. Never invent a mapping for rows in a populated table.
    if (missingRequiredColumn) {
        QSqlQuery countQuery(currentDb);
        if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM reportVideo"))
            || !countQuery.next()) {
            qCritical(logCritical()) << "reportVideo: numărul rândurilor nu poate fi verificat:"
                                     << countQuery.lastError().text();
            return false;
        }
        if (countQuery.value(0).toLongLong() != 0) {
            qCritical(logCritical())
                << "reportVideo: tabela conține date și nu poate fi extinsă automat;"
                   " lipsește cel puțin o coloană obligatorie.";
            return false;
        }

        QString orderIdType;
        if (!record.contains(QStringLiteral("id_orderEcho"))) {
            orderIdType = parentIdType(QStringLiteral("orderEcho"));
            if (orderIdType.isEmpty())
                return false;
        }
        const QMap<QString, QString> definitions = {
            {QStringLiteral("id_orderEcho"), orderIdType + QStringLiteral(" NOT NULL")},
            {QStringLiteral("relative_path"), QStringLiteral("VARCHAR(512) NOT NULL")},
            {QStringLiteral("file_name"), QStringLiteral("VARCHAR(80) NOT NULL")}};
        QSqlQuery alter(currentDb);
        for (const QString &column : requiredColumns) {
            if (record.contains(column))
                continue;
            if (!alter.exec(QStringLiteral("ALTER TABLE `reportVideo` ADD COLUMN `%1` %2")
                                .arg(column, definitions.value(column)))) {
                qCritical(logCritical()) << "reportVideo: coloana nu a putut fi adăugată:"
                                         << column << alter.lastError().text();
                return false;
            }
            qInfo(logInfo()) << "reportVideo: coloană adăugată în tabela istorică goală:"
                             << column;
        }
        record = currentDb.record(table);
    }

    QSqlQuery query(currentDb);
    if (!record.contains(QStringLiteral("id_reportEcho"))) {
        if (!query.exec(QStringLiteral("ALTER TABLE reportVideo ADD COLUMN id_reportEcho %1 NULL")
                            .arg(reportIdType))) {
            qCritical(logCritical()) << "reportVideo.id_reportEcho nu a putut fi adăugată:"
                                     << query.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Coloana reportVideo.id_reportEcho a fost adăugată.";
    }
    if (!record.contains(QStringLiteral("uuid"))) {
        if (!query.exec(QStringLiteral("ALTER TABLE reportVideo ADD COLUMN uuid BINARY(16) NULL"))) {
            qCritical(logCritical()) << "reportVideo.uuid nu a putut fi adăugată:"
                                     << query.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "Coloana reportVideo.uuid a fost adăugată.";
    }

    QSqlQuery indexQuery(currentDb);
    if (!indexQuery.exec(QStringLiteral(R"(
        SELECT COUNT(*)
        FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'reportVideo'
          AND COLUMN_NAME = 'id_reportEcho'
          AND SEQ_IN_INDEX = 1
    )")) || !indexQuery.next()) {
        qCritical(logCritical()) << "reportVideo: indexul raportului nu poate fi verificat:"
                                 << indexQuery.lastError().text();
        return false;
    }
    if (indexQuery.value(0).toInt() == 0
        && !query.exec(QStringLiteral(
            "CREATE INDEX idx_reportVideo_reportEcho ON reportVideo(id_reportEcho)"))) {
        qCritical(logCritical()) << "reportVideo: indexul raportului nu poate fi creat:"
                                 << query.lastError().text();
        return false;
    }

    // O comandă poate avea mai multe rapoarte. Legăm video-urile istorice
    // numai când există exact un raport pentru comanda respectivă.
    if (!query.exec(QStringLiteral(R"(
        UPDATE reportVideo AS video
        INNER JOIN (
            SELECT id_orderEcho, MIN(id) AS report_id
            FROM reportEcho
            GROUP BY id_orderEcho
            HAVING COUNT(*) = 1
        ) AS report ON report.id_orderEcho = video.id_orderEcho
        SET video.id_reportEcho = report.report_id
        WHERE video.id_reportEcho IS NULL
    )"))) {
        qCritical(logCritical()) << "reportVideo: asocierea sigură a video-urilor a eșuat:"
                                 << query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() > 0)
        qInfo(logInfo()) << "reportVideo: video-uri istorice asociate cu raportul unic:"
                         << query.numRowsAffected();
    if (!query.exec(QStringLiteral(R"(
        UPDATE reportVideo
        SET uuid = UNHEX(REPLACE(UUID(), '-', ''))
        WHERE uuid IS NULL OR OCTET_LENGTH(uuid) <> 16
    )"))) {
        qCritical(logCritical()) << "reportVideo: completarea UUID a eșuat:"
                                 << query.lastError().text();
        return false;
    }
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM reportVideo WHERE id_reportEcho IS NULL"))
        || !query.next()) {
        qCritical(logCritical()) << "reportVideo: rândurile neasociate nu pot fi numărate:"
                                 << query.lastError().text();
        return false;
    }
    if (query.value(0).toLongLong() > 0)
        qWarning(logWarning()) << "reportVideo: video-uri istorice fără raport unic; datele au fost păstrate:"
                               << query.value(0).toLongLong();
    return true;
}

bool UpdateReleasesApp::ensureCryptoSplitKeySchema()
{
    // Tabela era creată numai în bazele noi; bazele actualizate la 4.2.0
    // nu o aveau, iar parolele conturilor de e-mail nu puteau fi salvate.
    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical())
            << "cryptoSplitKey: baza de date nu este deschisă.";
        return false;
    }

    if (currentDb.tables(QSql::Tables).contains(QStringLiteral("cryptoSplitKey"),
                                                Qt::CaseInsensitive))
        return true;

    const bool sqlite = currentDb.driverName() == QStringLiteral("QSQLITE");
    bool created = false;
    if (sqlite) {
        created = db_common.execFileBatch(currentDb,
                                          QStringLiteral(":/sql/sqlite/tables/cryptoSplitKey.sql"),
                                          QStringLiteral("cryptoSplitKey"));
    } else {
        // Bazele MariaDB 3.x pot avea organizations.id de tip INT;
        // tipul coloanei FK trebuie să fie identic cu cel al cheii părinte.
        QSqlQuery typeQuery(currentDb);
        typeQuery.prepare(QStringLiteral(R"(
            SELECT COLUMN_TYPE
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'organizations'
              AND COLUMN_NAME = 'id'
        )"));
        const QRegularExpression integerType(
            QStringLiteral(R"(^(?:tinyint|smallint|mediumint|int|integer|bigint)(?:\(\d+\))?(?: unsigned)?$)"),
            QRegularExpression::CaseInsensitiveOption);
        if (!typeQuery.exec() || !typeQuery.next()
            || !integerType.match(typeQuery.value(0).toString().trimmed()).hasMatch()) {
            qCritical(logCritical())
                << "cryptoSplitKey: tipul organizations.id nu poate fi determinat:"
                << typeQuery.lastError().text();
            return false;
        }
        const QString parentIdType = typeQuery.value(0).toString().trimmed();

        QString createSql = DataBaseCommon::getTextQryFromResource(
            QStringLiteral(":/sql/mariadb/tables/cryptoSplitKey.sql"));
        const QRegularExpression columnDefinition(
            QStringLiteral(R"((`id_organizations`\s+)BIGINT UNSIGNED\b)"));
        if (createSql.isEmpty() || !createSql.contains(columnDefinition)) {
            qCritical(logCritical())
                << "cryptoSplitKey: scriptul SQL lipsește sau are o definiție neașteptată.";
            return false;
        }
        createSql.replace(columnDefinition, QStringLiteral("\\1") + parentIdType);

        QSqlQuery createQuery(currentDb);
        created = createQuery.exec(createSql);
        if (!created)
            qCritical(logCritical()) << "cryptoSplitKey: crearea tabelei a eșuat:"
                                     << createQuery.lastError().text();
    }

    if (!created) {
        qCritical(logCritical()) << "cryptoSplitKey: tabela nu poate fi creată.";
        return false;
    }
    qInfo(logInfo()) << "Tabela lipsă cryptoSplitKey a fost creată.";
    return true;
}

bool UpdateReleasesApp::ensureRequiredViews()
{
    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical())
            << "Verificarea view-urilor: baza de date nu este deschisă.";
        return false;
    }

    if (!ensurePatientAppointmentsSchema())
        return false;
    if (!ensureCryptoSplitKeySchema())
        return false;
    if (!addColumnIfMissing(currentDb, QStringLiteral("tableGestation1"),
                            QStringLiteral("multiplePregnancy"),
                            QStringLiteral("INTEGER NOT NULL DEFAULT 0"),
                            QStringLiteral("Gestation1")))
        return false;

    const QString lymphRecommendation = QStringLiteral("Recomandari (gangl.limfatici)");
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        QSqlQuery schemaQuery(currentDb);
        schemaQuery.prepare(QStringLiteral(
            "SELECT sql FROM sqlite_master WHERE type='table' "
            "AND name='formationsSystemTemplates'"));
        if (!schemaQuery.exec() || !schemaQuery.next()) {
            qCritical(logCritical())
                << "Verificarea formationsSystemTemplates a eșuat:"
                << schemaQuery.lastError().text();
            return false;
        }

        QString createSql = schemaQuery.value(0).toString();
        schemaQuery.finish();
        if (!createSql.contains(lymphRecommendation)) {
            const QString oldTail = QStringLiteral("'Recomandari (gestatation2)')");
            const QString newTail = QStringLiteral(
                "'Recomandari (gestatation2)', 'Recomandari (gangl.limfatici)')");
            if (!createSql.replace(oldTail, newTail).contains(lymphRecommendation)) {
                qCritical(logCritical())
                    << "Schema formationsSystemTemplates nu poate fi extinsă automat.";
                return false;
            }

            QStringList indexSql;
            QSqlQuery indexQuery(currentDb);
            indexQuery.prepare(QStringLiteral(
                "SELECT sql FROM sqlite_master WHERE type='index' "
                "AND tbl_name='formationsSystemTemplates' AND sql IS NOT NULL"));
            if (!indexQuery.exec()) {
                qCritical(logCritical()) << "Indexurile formationsSystemTemplates nu pot fi citite:"
                                         << indexQuery.lastError().text();
                return false;
            }
            while (indexQuery.next())
                indexSql.append(indexQuery.value(0).toString());
            indexQuery.finish();

            if (!currentDb.transaction())
                return false;

            QSqlQuery alter(currentDb);
            bool ok = alter.exec(QStringLiteral(
                          "ALTER TABLE formationsSystemTemplates "
                          "RENAME TO formationsSystemTemplates_old"))
                      && alter.exec(createSql)
                      && alter.exec(QStringLiteral(
                          "INSERT INTO formationsSystemTemplates "
                          "(id, deletionMark, name, typeSystem, uuid) "
                          "SELECT id, deletionMark, name, typeSystem, uuid "
                          "FROM formationsSystemTemplates_old"))
                      && alter.exec(QStringLiteral(
                          "DROP TABLE formationsSystemTemplates_old"));
            for (const QString &sql : std::as_const(indexSql)) {
                if (ok && !alter.exec(sql))
                    ok = false;
            }

            if (!ok || !currentDb.commit()) {
                const QString error = alter.lastError().text();
                currentDb.rollback();
                qCritical(logCritical())
                    << "Extinderea formationsSystemTemplates a eșuat:" << error;
                return false;
            }
            qInfo(logInfo())
                << "Schema formationsSystemTemplates a fost extinsă pentru recomandările LymphNodes.";
        }
    } else {
        QSqlQuery columnQuery(currentDb);
        columnQuery.prepare(QStringLiteral(
            "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='formationsSystemTemplates' "
            "AND COLUMN_NAME='typeSystem'"));
        if (!columnQuery.exec() || !columnQuery.next()) {
            qCritical(logCritical())
                << "Verificarea ENUM formationsSystemTemplates a eșuat:"
                << columnQuery.lastError().text();
            return false;
        }
        if (!columnQuery.value(0).toString().contains(lymphRecommendation)) {
            QSqlQuery alter(currentDb);
            if (!alter.exec(QStringLiteral(R"(
                ALTER TABLE formationsSystemTemplates
                MODIFY COLUMN typeSystem ENUM(
                    'Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
                    'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
                    'Prostata', 'Recomandari (prostata)',
                    'Tiroida', 'Recomandari (tiroida)',
                    'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
                    'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
                    'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)',
                    'Recomandari (gangl.limfatici)'
                ) NOT NULL DEFAULT 'Unknow'
            )"))) {
                qCritical(logCritical())
                    << "Extinderea ENUM formationsSystemTemplates a eșuat:"
                    << alter.lastError().text();
                return false;
            }
            qInfo(logInfo())
                << "ENUM formationsSystemTemplates a fost extins pentru recomandările LymphNodes.";
        }
    }

    const QString basePath = MainDatabaseConnectionContext::instance().isSqlite()
                                 ? QStringLiteral(":/sql/sqlite/views/")
                                 : QStringLiteral(":/sql/mariadb/views/");
    const QList<QPair<QString, QString>> requiredViews = {
        {QStringLiteral("v_users_combo_active"), QStringLiteral("users_combo_view.sql")},
        {QStringLiteral("v_doctors_active"), QStringLiteral("doctors_combo_view.sql")},
        {QStringLiteral("v_nurses_active"), QStringLiteral("nurses_combo_view.sql")},
        {QStringLiteral("v_types_prices_active"), QStringLiteral("typePrices_combo_view.sql")},
        {QStringLiteral("v_organizations_active"), QStringLiteral("organizations_combo_view.sql")},
        {QStringLiteral("v_contracts_listView_active"), QStringLiteral("contracts_listForm_view.sql")}
    };

    const QStringList existingViews = currentDb.tables(QSql::Views);
    for (const auto &view : requiredViews) {
        if (existingViews.contains(view.first, Qt::CaseInsensitive))
            continue;

        QFile viewFile(basePath + view.second);
        if (!viewFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical(logCritical())
                << "Verificarea view-urilor: resursa nu poate fi citită:"
                << basePath + view.second;
            return false;
        }

        QSqlQuery createQuery(currentDb);
        if (!createQuery.exec(QString::fromUtf8(viewFile.readAll()).trimmed())) {
            qCritical(logCritical())
                << "Verificarea view-urilor: nu poate fi creat" << view.first
                << createQuery.lastError().text();
            return false;
        }
        qInfo(logInfo()) << "View lipsă creat:" << view.first;
    }

    return true;
}

bool UpdateReleasesApp::execUpdateCurrentRelease(const QString currentRelease)
{
    if (globals().firstLaunch)
        return true;

    if (currentRelease.trimmed().isEmpty()) {
        qCritical(logCritical())
            << "Actualizarea a fost anulată: versiunea bazei de date lipsește.";
        return false;
    }

    const QString versionText = currentRelease.trimmed();
    qsizetype suffixIndex = 0;
    QVersionNumber current = QVersionNumber::fromString(versionText, &suffixIndex);
    QVersionNumber target(VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

    // A trailing zero (e.g. 4.1.0) is valid, although not "normalized" in Qt.
    if (current.isNull() || suffixIndex != versionText.size()) {
        qCritical(logCritical()) << "Versiune invalida:" << currentRelease;
        return false;
    }

    qInfo(logInfo()) << "DB version:" << current.toString()
                     << "App version:" << target.toString();

    if (!validateMigrationContext(current, target))
        return false;

    struct MigrationStep {
        QVersionNumber version;
        bool (UpdateReleasesApp::*execute)();
    };

    const MigrationStep migrations[] = {
        {QVersionNumber(2, 0, 4), &UpdateReleasesApp::update_2_0_4},
        {QVersionNumber(2, 0, 5), &UpdateReleasesApp::update_2_0_5},
        {QVersionNumber(2, 0, 6), &UpdateReleasesApp::update_2_0_6},
        {QVersionNumber(2, 0, 7), &UpdateReleasesApp::update_2_0_7},
        {QVersionNumber(2, 0, 9), &UpdateReleasesApp::update_2_0_9},
        {QVersionNumber(3, 0, 1), &UpdateReleasesApp::update_3_0_1},
        {QVersionNumber(3, 0, 3), &UpdateReleasesApp::update_3_0_3},
        {QVersionNumber(3, 0, 6), &UpdateReleasesApp::update_3_0_6},
        {QVersionNumber(3, 0, 7), &UpdateReleasesApp::update_3_0_7},
        {QVersionNumber(4, 0, 1), &UpdateReleasesApp::update_4_0_1},
        {QVersionNumber(4, 1, 0), &UpdateReleasesApp::update_4_1_0},
        {QVersionNumber(4, 1, 2), &UpdateReleasesApp::update_4_1_2},
        {QVersionNumber(4, 2, 0), &UpdateReleasesApp::update_4_2_0}
    };

    for (const MigrationStep &migration : migrations) {
        if (current >= migration.version || migration.version > target)
            continue;

        const QString version = migration.version.toString();
        if (!validateMigrationPrerequisites(migration.version))
            return false;

        qInfo(logInfo()) << tr("Începe migrarea bazei de date la versiunea %1.").arg(version);
        emit migrationProgress(0, 0,
                               tr("Se execută migrarea bazei de date la versiunea %1...")
                                   .arg(version));

        if (!(this->*migration.execute)()) {
            qCritical(logCritical())
                << tr("Migrarea bazei de date la versiunea %1 a eșuat.").arg(version);
            return false;
        }

        // MariaDB confirmă implicit multe operații DDL. Dacă o etapă ulterioară
        // eșuează, relansarea trebuie să înceapă de la ultima etapă completă,
        // nu să reaplice migrările vechi pe o schemă deja redenumită.
        QString checkpointError;
        if (!db->setDatabaseSchemaVersion(version, &checkpointError)) {
            qCritical(logCritical())
                << tr("Versiunea intermediară %1 nu a putut fi salvată: %2")
                       .arg(version, checkpointError);
            return false;
        }

        current = migration.version;
        qInfo(logInfo()) << tr("Migrarea bazei de date la versiunea %1 s-a finalizat.")
                                .arg(version);
    }

    return true;
}

bool UpdateReleasesApp::update_2_0_4()
{
    db->getDatabase().transaction();
    //--------------------------------------------------
    // crearea tabelei noi
    QSqlQuery qry;
    if (MainDatabaseConnectionContext::instance().isSqlite()){
        if (! db_common.execFileBatch(":/sql/sqlite/tables/user_preferences.sql",
                                     "user_preferences")) {
            db->getDatabase().rollback();
            return false;
        }
    } else if (MainDatabaseConnectionContext::instance().isMariaDb()){
        if (!db_common.execFileBatch(":/sql/mariadb/tables/user_preferences.sql",
                                     "user_preferences")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    //-------------------------------------------------
    // inserarea datelor
    QVector<QVariant> data;
    data.append(1); // id
    data.append(SessionContext::instance().userId()); // id utilizatorului
    data.append(""); // versionsApp
    data.append(1);  // showQuestionCloseApp
    data.append(0);  // showUserManual
    data.append(0);  // showHistoryVersion
    data.append(10); // updateListDoc
    data.append(0);  // showDesignerMenuPrint
    data.append(0);  // checkNewVersionApp
    data.append(0);  // databasesArchiving
    data.append(1);  // showAsistantHelper

    QString err;
    if (! db_common.execPreparedFromFile(db->getDatabase(),
                                        ":/sql/queries/userPreferences_insert.sql",
                                        data,
                                        &err))
    {
        // logarea
        qCritical(logCritical()) << err;
        db->getDatabase().rollback();
        return false;
    }
    //------------------------------------------------
    // eliminarea tabelei settingsUsers

    qry.prepare("DROP TABLE settingsUsers;");
    if (! qry.exec()){
        qCritical(logCritical()) << tr("Nu este eliminata tabela 'settingsUsers' %1 ")
        .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
        db->getDatabase().rollback();
        return false;
    }

    return db->getDatabase().commit();
}

bool UpdateReleasesApp::update_2_0_5()
{
    db->getDatabase().transaction();

    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        if (! db_common.execFileBatch(":/sql/sqlite/tables/gestation2.sql",
                                     "gestation2")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        if (! db_common.execFileBatch(":/sql/mariadb/tables/gestation2.sql",
                                     "gestation2")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    return db->getDatabase().commit();
}

bool UpdateReleasesApp::update_2_0_6()
{
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        if (! db_common.execFileBatch(":/sql/sqlite/tables/normograms.sql",
                                     "normograms")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        if (! db_common.execFileBatch(":/sql/mariadb/tables/normograms.sql",
                                     "normograms")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    db->loadNormogramsFromXml();

    return db->getDatabase().commit();
}

bool UpdateReleasesApp::update_2_0_7()
{
    db->getDatabase().transaction();
    if (db->deleteDataFromTable("normograms")) {
        db->loadNormogramsFromXml();
    } else {
        db->getDatabase().rollback();
        return false;
    }
    return db->getDatabase().commit();
}

bool UpdateReleasesApp::update_2_0_9()
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    // crearea tabelei temporare
    qry.prepare("CREATE TABLE sqlitestudio_temp_table0 AS SELECT * FROM userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical())
        << tr("Eroare de actualizare a relizului '2.0.9' (crearea tabelei 'sqlitestudio_temp_table0'): ")
                + qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }

    // eliminarea tabelei vechi
    qry.prepare("DROP TABLE userPreferences;");
    if (! qry.exec()){
        qCritical(logCritical())
            << tr("Eroare de actualizare a relizului '2.0.9' (eliminarea tabelei 'userPreferences'): ")
                   + qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }

    // crearea tabelei noi cu modificari
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        if (! db_common.execFileBatch(":/sql/sqlite/tables/user_preferences.sql",
                                     "user_preferences")) {
            db->getDatabase().rollback();
            return false;
        }
    } else {
        if (! db_common.execFileBatch(":/sql/mariadb/tables/user_preferences.sql",
                                     "user_preferences")) {
            db->getDatabase().rollback();
            return false;
        }
    }

    // inserarea datelor
    qry.prepare("INSERT INTO userPreferences ("
                " id,"
                " id_users,"
                " versionApp,"
                " showQuestionCloseApp,"
                " showUserManual,"
                " showHistoryVersion,"
                " updateListDoc,"
                " showDesignerMenuPrint,"
                " checkNewVersionApp,"
                " databasesArchiving,"
                " showAsistantHelper ) "
                "SELECT "
                " id,"
                " id_users,"
                " versionApp,"
                " showQuestionCloseApp,"
                " showUserManual,"
                " showHistoryVersion,"
                " updateListDoc,"
                " showDesignerMenuPrint,"
                " checkNewVersionApp,"
                " databasesArchiving,"
                " showAsistantHelper FROM sqlitestudio_temp_table0;");
    if (! qry.exec()){
        qCritical(logCritical())
            << tr("Eroare de actualizare a relizului '2.0.9' (inserarea datelor din tabela 'sqlitestudio_temp_table0' in tabela 'userPreferences'): ")
                   + qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }

    // eliminarea tabelei temporare
    qry.prepare("DROP TABLE sqlitestudio_temp_table0;");
    if (! qry.exec()){
        qCritical(logCritical())
            << tr("Eroare de actualizare a relizului '" USG_VERSION_FULL "' (eliminarea tabelei 'sqlitestudio_temp_table0'): ")
                   + qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }

    return db->getDatabase().commit();
}

bool UpdateReleasesApp::update_3_0_1()
{
    if (!db->updateInvestigationFromXML_2024())
        return false;
    if (!updateTablePacients_release_3_0_1()
        || !updateTableKidney_release_3_0_1()
        || !updateTableIntestinalLoops_release_3_0_1()
        || !updateTableGynecology_release_3_0_1()
        || !updateTableformationsSystemTemplates_release_3_0_1())
        return false;
    createIndex_release_3_0_1();
    createIndexForBaseImage_3_0_1();

    return true;
}

bool UpdateReleasesApp::updateTablePacients_release_3_0_1()
{
    QSqlQuery qry;

    // SQLite nu impune lungimea câmpului TEXT. Nu reconstruim tabela aici:
    // implementarea istorică o ștergea fără să o recreeze, cu risc de pierdere
    // a tuturor pacienților. Redenumirea și normalizarea reală se fac în 4.1.0.
    if (MainDatabaseConnectionContext::instance().isSqlite())
        return true;

    if (MainDatabaseConnectionContext::instance().isSqlite()){

        if (qry.exec(R"(PRAGMA foreign_keys = 0;)"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: foreign_keys = 0";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este setat - foreign_keys = 0";

        if (qry.exec(R"(CREATE TABLE sqlitestudio_temp_table AS SELECT * FROM pacients;)"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creata tabela 'sqlitestudio_temp_table'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creata tabela 'sqlitestudio_temp_table'.";

        if (qry.exec("DROP TABLE pacients;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: eliminata tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este eliminata tabela 'pacients'.";

        if (qry.exec(R"(
                INSERT INTO pacients ( id, deletionMark, IDNP, name, fName, mName,  medicalPolicy, birthday, address,
                    telephone, email, comment )
                SELECT id, deletionMark, IDNP, name, fName, mName, medicalPolicy, birthday, address,
                     telephone, email, comment FROM sqlitestudio_temp_table;)"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: copierea datelor din tabela 'sqlitestudio_temp_table' in tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu au fost copiate datele din tabela 'sqlitestudio_temp_table' in tabela 'pacients'.";

        if (qry.exec("DROP TABLE sqlitestudio_temp_table;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: eliminata tabela 'sqlitestudio_temp_table'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este eliminata tabela 'sqlitestudio_temp_table'.";

        if (qry.exec(R"(
            CREATE TRIGGER create_full_name_pacient
            AFTER INSERT ON pacients
            FOR EACH ROW
            BEGIN
                INSERT INTO fullNamePacients
                    (id_pacients, name, nameIDNP, nameBirthday, nameTelephone, nameBirthdayIDNP)
                VALUES (
                    NEW.id,
                    NEW.name || ' ' || NEW.fName,
                    NEW.name || ' ' || NEW.fName || ' (' || NEW.IDNP || ')',
                    NEW.name || ' ' || NEW.fName || ' (' || NEW.birthday || ')',
                    NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,
                    NEW.name || ' ' || NEW.fName || ' (' || NEW.birthday || ', ' || NEW.IDNP || ')'
                );
            END;
            )"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: crearea trigger-lui 'create_full_name_pacient'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creat trigger-ul 'create_full_name_pacient'.";

        if (qry.exec(R"(
            CREATE TRIGGER update_full_name_pacient
            AFTER UPDATE ON pacients
            FOR EACH ROW
            BEGIN
                UPDATE fullNamePacients SET
                    name             = NEW.name || ' ' || NEW.fName,
                    nameIDNP         = NEW.name || ' ' || NEW.fName || ' (' || NEW.IDNP || ')',
                    nameBirthday     = NEW.name || ' ' || NEW.fName || ' (' || NEW.birthday || ')',
                    nameTelephone    = NEW.name || ' ' || NEW.fName || ', tel.: ' || NEW.telephone,
                    nameBirthdayIDNP = NEW.name || ' ' || NEW.fName || ' (' || NEW.birthday || ', ' || NEW.IDNP || ')'
                WHERE
                    id_pacients = NEW.id;
            END;
            )"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: crearea trigger-lui 'update_full_name_pacient'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este creat trigger-ul 'update_full_name_pacient'";

        if (qry.exec("PRAGMA foreign_keys = 1;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: instalat 'foreign_keys = 1'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu este setat - foreign_keys = 1";

    } else {

        if (qry.exec("ALTER TABLE `pacients` MODIFY `medicalPolicy` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL;"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: modificata sectia 'medicalPolicy' in tabela 'pacients'.";
        else
            qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu s-a efectuat modifcarea tabelei 'pacients' la bd MySQL.";

    }
    return !qry.lastError().isValid();
}

bool UpdateReleasesApp::updateTableKidney_release_3_0_1()
{
    const QSqlDatabase currentDb = db->getDatabase();
    const QList<QPair<QString, QString>> columns = MainDatabaseConnectionContext::instance().isSqlite()
        ? QList<QPair<QString, QString>>{
              {QStringLiteral("contour_right"),
               QStringLiteral("TEXT CHECK(contour_right IN ('clar','sters','regulat','neregulat')) DEFAULT 'clar'")},
              {QStringLiteral("contour_left"),
               QStringLiteral("TEXT CHECK(contour_left IN ('clar','sters','regulat','neregulat')) DEFAULT 'clar'")},
              {QStringLiteral("suprarenal_formations"), QStringLiteral("TEXT DEFAULT NULL")}}
        : QList<QPair<QString, QString>>{
              {QStringLiteral("contour_right"),
               QStringLiteral("ENUM('clar','sters','regulat','neregulat') DEFAULT 'clar'")},
              {QStringLiteral("contour_left"),
               QStringLiteral("ENUM('clar','sters','regulat','neregulat') DEFAULT 'clar'")},
              {QStringLiteral("suprarenal_formations"),
               QStringLiteral("VARCHAR(500) COLLATE utf8mb4_general_ci DEFAULT NULL")}};
    for (const auto &column : columns) {
        if (!addColumnIfMissing(currentDb, QStringLiteral("tableKidney"),
                                column.first, column.second,
                                QStringLiteral("Migrarea 3.0.1:")))
            return false;
    }
    return true;
}

bool UpdateReleasesApp::updateTableIntestinalLoops_release_3_0_1()
{
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        if (db_common.execFileBatch(":/sql/sqlite/tables/organs_internal.sql", "tableIntestinalLoop"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'tableIntestinalLoop'";
        else
            return false;
    } else {
        if (db_common.execFileBatch(":/sql/mariadb/tables/organs_internal.sql", "tableIntestinalLoop"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'tableIntestinalLoop'";
        else
            return false;
    }
    return true;
}

bool UpdateReleasesApp::updateTableGynecology_release_3_0_1()
{
    const QSqlDatabase currentDb = db->getDatabase();
    const QList<QPair<QString, QString>> columns = MainDatabaseConnectionContext::instance().isSqlite()
        ? QList<QPair<QString, QString>>{
              {QStringLiteral("junctional_zone"),
               QStringLiteral("TEXT CHECK(junctional_zone IN ('contur clar','contur sters')) DEFAULT 'contur clar'")},
              {QStringLiteral("junctional_zone_description"), QStringLiteral("TEXT DEFAULT NULL")},
              {QStringLiteral("cervical_canal"),
               QStringLiteral("TEXT CHECK(cervical_canal IN ('nedilatat','dilatat')) DEFAULT 'nedilatat'")},
              {QStringLiteral("cervical_canal_formations"), QStringLiteral("TEXT DEFAULT NULL")},
              {QStringLiteral("fallopian_tubes"),
               QStringLiteral("TEXT CHECK(fallopian_tubes IN ('nonvizibile','vizibile')) DEFAULT 'nonvizibile'")},
              {QStringLiteral("fallopian_tubes_formations"), QStringLiteral("TEXT DEFAULT NULL")}}
        : QList<QPair<QString, QString>>{
              {QStringLiteral("junctional_zone"),
               QStringLiteral("ENUM('contur clar','contur sters') DEFAULT 'contur clar'")},
              {QStringLiteral("junctional_zone_description"), QStringLiteral("VARCHAR(256) DEFAULT NULL")},
              {QStringLiteral("cervical_canal"),
               QStringLiteral("ENUM('nedilatat','dilatat') DEFAULT 'nedilatat'")},
              {QStringLiteral("cervical_canal_formations"), QStringLiteral("VARCHAR(256) DEFAULT NULL")},
              {QStringLiteral("fallopian_tubes"),
               QStringLiteral("ENUM('nonvizibile','vizibile') DEFAULT 'nonvizibile'")},
              {QStringLiteral("fallopian_tubes_formations"), QStringLiteral("VARCHAR(256) DEFAULT NULL")}};
    for (const auto &column : columns) {
        if (!addColumnIfMissing(currentDb, QStringLiteral("tableGynecology"),
                                column.first, column.second,
                                QStringLiteral("Migrarea 3.0.1:")))
            return false;
    }
    return true;

}

bool UpdateReleasesApp::updateTableformationsSystemTemplates_release_3_0_1()
{
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        if (db_common.execFileBatch(":/sql/sqlite/tables/conclusion_formations_templates.sql",
                                    "formationsSystemTemplates"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'formationsSystemTemplates'";
        else
            return false;
    } else {
        if (db_common.execFileBatch(":/sql/mariadb/tables/conclusion_formations_templates.sql",
                                    "formationsSystemTemplates"))
            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: a fost creata tabela 'formationsSystemTemplates'";
        else
            return false;
    }
    return true;
}

void UpdateReleasesApp::createIndex_release_3_0_1()
{
    QSqlQuery qry;

    if (qry.exec("CREATE INDEX idx_name_fName_pacients ON pacients(name, fName);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_fName_pacients'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_fName_pacients'.";

    if (qry.exec("CREATE INDEX idx_name_fName_IDNP_pacients ON pacients(name, fName, IDNP);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_fName_IDNP_pacients'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_fName_IDNP_pacients'.";

    //-----------------------------------------------------------------
    //---------------- pricing

    if (qry.exec("CREATE INDEX idx_dateDoc_pricing ON pricings(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_organizations_pricing ON pricings(id_organizations);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_organizations_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_organizations_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_pricing ON pricings(id_contracts);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_contracts_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_contracts_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_users_pricing ON pricings(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_typesPrices_pricing ON pricings(id_typesPrices);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_typesPrices_pricing'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_typesPrices_pricing'.";

    //-----------------------------------------------------------------
    //---------------- pricingsTable

    if (qry.exec("CREATE INDEX idx_id_pricings_pricingTable ON pricingsTable(id_pricings);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pricings_pricingTable'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pricings_pricingTable'.";

    //-----------------------------------------------------------------
    //---------------- orderEcho

    if (qry.exec("CREATE INDEX idx_id_organizations_order ON orderEcho(id_organizations);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_organizations_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_organizations_order'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_order ON orderEcho(id_contracts);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_contracts_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_contracts_order'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_order ON orderEcho(id_pacients);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pacients_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pacients_order'.";

    if (qry.exec("CREATE INDEX idx_id_doctors_order ON orderEcho(id_doctors);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_doctors_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_doctors_order'.";

    if (qry.exec("CREATE INDEX idx_id_users_order ON orderEcho(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_order'.";

    if (qry.exec("CREATE INDEX idx_dateDoc_order ON orderEcho(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_order'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_order'.";

    //----------------------------------------------------------------
    //----------------- orderEchoTable

    if (qry.exec("CREATE INDEX idx_id_orderEcho_orderTable ON orderEchoTable(id_orderEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_orderEcho_orderTable'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_orderEcho_orderTable'.";

    //----------------------------------------------------------------
    //----------------- reportEcho

    if (qry.exec("CREATE INDEX idx_dateDoc_report ON reportEcho(dateDoc);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_dateDoc_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_dateDoc_report'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_report ON reportEcho(id_pacients);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_pacients_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_pacients_report'.";

    if (qry.exec("CREATE INDEX idx_id_orderEcho_report ON reportEcho(id_orderEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_orderEcho_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_orderEcho_report'.";

    if (qry.exec("CREATE INDEX idx_id_users_report ON reportEcho(id_users);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_users_report'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_users_report'.";

    //---------------------------------------------------------------
    //----------------- tableCholecist

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cholecist ON tableCholecist(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_cholecist'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_cholecist'.";

    //---------------------------------------------------------------
    //----------------- tablePancreas

    if (qry.exec("CREATE INDEX idx_id_reportEcho_pancreas ON tablePancreas(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_pancreas'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_pancreas'.";

    //---------------------------------------------------------------
    //----------------- tableSpleen

    if (qry.exec("CREATE INDEX idx_id_reportEcho_spleen ON tableSpleen(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_spleen'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_spleen'.";


    //---------------------------------------------------------------
    //----------------- tableIntestinalLoop

    if (qry.exec("CREATE INDEX idx_id_reportEcho_intestin ON tableIntestinalLoop(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_intestin'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_intestin'.";

    //---------------------------------------------------------------
    //----------------- tableKidney

    if (qry.exec("CREATE INDEX idx_id_reportEcho_kidney ON tableKidney(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_kidney'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_kidney'.";

    //---------------------------------------------------------------
    //----------------- tableBladder

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bladder ON tableBladder(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_bladder'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_bladder'.";

    //---------------------------------------------------------------
    //----------------- tableProstate

    if (qry.exec("CREATE INDEX idx_id_reportEcho_prostate ON tableProstate(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_prostate'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_prostate'.";

    //---------------------------------------------------------------
    //----------------- tableGynecology

    if (qry.exec("CREATE INDEX idx_id_reportEcho_gynecology ON tableGynecology(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_gynecology'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_gynecology'.";

    //---------------------------------------------------------------
    //----------------- tableBreast

    if (qry.exec("CREATE INDEX idx_id_reportEcho_breast ON tableBreast(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_breast'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_breast'.";

    //---------------------------------------------------------------
    //----------------- tableThyroid

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thyroid ON tableThyroid(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_thyroid'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_thyroid'.";

    //---------------------------------------------------------------
    //----------------- tableGestation0

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges0 ON tableGestation0(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges0'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges0'.";

    //---------------------------------------------------------------
    //----------------- tableGestation1

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges1 ON tableGestation1(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges1'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges1'.";

    //---------------------------------------------------------------
    //----------------- tables Gestation2
    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges2 ON tableGestation2(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_ges2'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_ges2'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bio ON tableGestation2_biometry(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_bio'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_bio'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cr ON tableGestation2_cranium(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_cr'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_cr'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_snc ON tableGestation2_SNC(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_snc'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_snc'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_heart ON tableGestation2_heart(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_heart'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_heart'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thorax ON tableGestation2_thorax(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_thorax'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_thorax'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_abd ON tableGestation2_abdomen(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_abd'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_abd'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_us ON tableGestation2_urinarySystem(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_us'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_us'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_other ON tableGestation2_other(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_other'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_other'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_doppler ON tableGestation2_doppler(id_reportEcho);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_reportEcho_doppler'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_reportEcho_doppler'.";

    //---------------------------------------------------------------
    //----------------- formationsSystemTemplates

    if (qry.exec("CREATE INDEX idx_name_typeSystem_formationsSystemTemplates ON formationsSystemTemplates(name, typeSystem);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";
}

void UpdateReleasesApp::createIndexForBaseImage_3_0_1()
{
    QSqlQuery qry(db->getDatabaseImage());

    if (qry.exec("CREATE INDEX idx_id_documents_imagesReports ON imagesReports(id_reportEcho, id_orderEcho, id_patients, id_user);"))
        qInfo(logInfo()) << "Actualizarea la versiunea 3.0.1: creat indexul 'idx_id_documents_imagesReports'.";
    else
        qCritical(logCritical()) << "Eroare actualizarii la versiunea '3.0.1': nu a fost creat indexul 'idx_id_documents_imagesReports'.";
}

bool UpdateReleasesApp::update_3_0_3()
{
    const QSqlDatabase currentDb = db->getDatabase();
    if (MainDatabaseConnectionContext::instance().isSqlite()){

        if (! db_common.execFileBatch(":/sql/sqlite/tables/online_account.sql",
                                    "conts_online"))
            return false;

        if (! db_common.execFileBatch(":/sql/sqlite/tables/cloud_server.sql",
                                     "cloud_server"))
            return false;

        if (!addColumnIfMissing(currentDb, QStringLiteral("tableGestation0"),
                                QStringLiteral("lmp"), QStringLiteral("TEXT"),
                                QStringLiteral("Migrarea 3.0.3:"))
            || !addColumnIfMissing(currentDb, QStringLiteral("tableGestation1"),
                                   QStringLiteral("lmp"), QStringLiteral("TEXT"),
                                   QStringLiteral("Migrarea 3.0.3:")))
            return false;

    } else {
        // Tabelele de configurare cloud sunt create în migrarea 4.0.1.
        // Acolo tipurile FK sunt preluate din baza 3.x reală; folosirea aici a
        // BIGINT UNSIGNED din schema curentă eșuează pentru bazele vechi cu INT.

        if (!addColumnIfMissing(currentDb, QStringLiteral("tableGestation0"),
                                QStringLiteral("lmp"), QStringLiteral("VARCHAR(10)"),
                                QStringLiteral("Migrarea 3.0.3:"))
            || !addColumnIfMissing(currentDb, QStringLiteral("tableGestation1"),
                                   QStringLiteral("lmp"), QStringLiteral("VARCHAR(10)"),
                                   QStringLiteral("Migrarea 3.0.3:")))
            return false;

    }

    return true;
}

bool UpdateReleasesApp::existErrFormationsSystemTemplates()
{
    QSqlQuery qry;
    QString ddl;

    if (MainDatabaseConnectionContext::instance().isMariaDb()) {

        qry.prepare(R"(
            SHOW CREATE TABLE formationsSystemTemplates
        )");
        if (qry.exec() && qry.next())
            ddl = qry.value(0).toString();

        if (ddl.isEmpty()) {
            qry.prepare(R"(
                SELECT COLUMN_TYPE
                FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_SCHEMA = DATABASE()
                  AND TABLE_NAME = 'formationsSystemTemplates'
                  AND COLUMN_NAME = 'typeSystem'
            )");
            if (qry.exec() && qry.next())
                ddl = qry.value(0).toString();
        }

    } else {

        qry.prepare(R"(
            SELECT sql
              FROM sqlite_master
             WHERE type = 'table' AND
                   name = 'formationsSystemTemplates';
        )");
        if (qry.exec() && qry.next())
            ddl = qry.value(0).toString();

    }

    if (ddl.isEmpty())
        return false;

    if (ddl.contains("Rinici", Qt::CaseSensitive))
        return true;
    else
        return false;

}

bool UpdateReleasesApp::update_3_0_6()
{
    QSqlQuery qry;

    if (MainDatabaseConnectionContext::instance().isSqlite()) {

        /** 1. gasim si corectam eroarea in tabela 'formationsSystemTemplates' */
        if (existErrFormationsSystemTemplates()) {
            QSqlDatabase currentDb = db->getDatabase();
            if (!currentDb.transaction()) {
                qCritical(logCritical())
                    << "Migrarea 3.0.6: tranzacția SQLite nu a putut fi pornită:"
                    << currentDb.lastError().text();
                return false;
            }

            const auto failRebuild = [&]() {
                const QString error = qry.lastError().text();
                currentDb.rollback();
                qCritical(logCritical())
                    << "Migrarea 3.0.6: reconstruirea formationsSystemTemplates a eșuat:"
                    << error;
                return false;
            };

            if (!qry.exec(QStringLiteral(
                    "ALTER TABLE formationsSystemTemplates "
                    "RENAME TO formationsSystemTemplates_old")))
                return failRebuild();

            // Cream din nou tabela cu denumirea corecta.
            if (!db_common.execFileBatch(
                    ":/sql/sqlite/tables/conclusion_formations_templates.sql",
                    "formationsSystemTemplates"))
                return failRebuild();

            const bool oldHasUuid = currentDb.record(
                                        QStringLiteral("formationsSystemTemplates_old"))
                                        .contains(QStringLiteral("uuid"));
            const QString uuidExpression = oldHasUuid
                                               ? QStringLiteral(
                                                     "CASE WHEN uuid IS NULL OR length(uuid)<>16 "
                                                     "THEN randomblob(16) ELSE uuid END")
                                               : QStringLiteral("randomblob(16)");
            if (!qry.exec(QStringLiteral(R"(
                INSERT INTO formationsSystemTemplates
                    (id, deletionMark, name, typeSystem, uuid)
                SELECT id, deletionMark, name,
                       CASE WHEN typeSystem='Rinici' THEN 'Rinichi' ELSE typeSystem END,
                       %1
                FROM formationsSystemTemplates_old
            )").arg(uuidExpression)))
                return failRebuild();

            if (!qry.exec(QStringLiteral(
                    "DROP TABLE formationsSystemTemplates_old")))
                return failRebuild();

            if (!currentDb.commit()) {
                qCritical(logCritical())
                    << "Migrarea 3.0.6: commit-ul SQLite a eșuat:"
                    << currentDb.lastError().text();
                currentDb.rollback();
                return false;
            }

            qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: s-a corectat enumerarea in tabela 'formationsSystemTemplates'.";
        }

        /** 2. cream tabela noua 'tableSofTissuesLymphNodes' */
        if (! db_common.execFileBatch(":/sql/sqlite/tables/lymph_nodes.sql",
                                     "lymph_nodes"))
            return false;

        /** 3. Modificam tabela 'reportEcho' - adaugam coloana 't_lymphNodes' */
        if (!addColumnIfMissing(db->getDatabase(), QStringLiteral("reportEcho"),
                                QStringLiteral("t_lymphNodes"),
                                QStringLiteral("INTEGER DEFAULT 0"),
                                QStringLiteral("Migrarea 3.0.6:")))
            return false;


    } else {

        /** 1. gasim si corectam eroarea in tabela 'formationsSystemTemplates' */
        if (existErrFormationsSystemTemplates()) {

            // Modificam doar coloana
            if (! qry.exec(R"(
                ALTER TABLE formationsSystemTemplates
                MODIFY COLUMN typeSystem ENUM(
                    'Unknow', 'Ficat', 'Colecist', 'Pancreas', 'Splina', 'Intestine', 'Recomandari (org.interne)',
                    'Rinichi', 'V.urinara', 'Gl.suprarenale', 'Recomandari (s.urinar)',
                    'Prostata', 'Recomandari (prostata)',
                    'Tiroida', 'Recomandari (tiroida)',
                    'Gl.mamara (stanga)', 'Gl.mamara (dreapta)', 'Recomandari (gl.mamare)',
                    'Ginecologia (uter)', 'Ginecologia (ovar stang)', 'Ginecologia (ovar drept)', 'Recomandari (ginecologia)',
                    'Recomandari (gestatation0)', 'Recomandari (gestatation1)', 'Recomandari (gestatation2)'
                ) NOT NULL DEFAULT 'Unknow';
            )")) {
                qCritical(logCritical()) << "Eroare ALTER TABLE MySQL:" << qry.lastError().text();
                return false;
            }

            // Corectăm eventualele valori greșite deja introduse
            if (! qry.exec("UPDATE formationsSystemTemplates SET typeSystem='Rinichi' WHERE typeSystem='Rinici';")) {
                qCritical(logCritical()) << "Eroare UPDATE MySQL:" << qry.lastError().text();
                return false;
            } else {
                qInfo(logInfo()) << "Actualizarea la versiunea 3.0.6: s-a corectat valori greșite din tabela 'formationsSystemTemplates'.";
            }
        }

        /** 2. cream tabela noua 'tableSofTissuesLymphNodes' */
        if (! db_common.execFileBatch(":/sql/mariadb/tables/lymph_nodes.sql",
                                     "lymph_nodes"))
            return false;

        /** 3. Modificam tabela 'reportEcho' - adaugam coloana 't_lymphNodes' */
        if (!addColumnIfMissing(db->getDatabase(), QStringLiteral("reportEcho"),
                                QStringLiteral("t_lymphNodes"),
                                QStringLiteral("BOOLEAN DEFAULT 0"),
                                QStringLiteral("Migrarea 3.0.6:")))
            return false;
    }

    qInfo(logInfo()) << "Actualizarea pana la versiunea 3.0.6 s-a finisat.";

    return true;
}

bool UpdateReleasesApp::update_3_0_7()
{
    /** Cerinte pentru release:
     **
     ** 1. Modularea documentului 'Comanda ecografica'
     ** 2. Crearea formei de 'Parametri' pentru documentul 'Comanda ecografica':
     **    - vizualizarea/ascunderea in tabel a investigatiilor unde este indicat
     **      costul si nu este indicat costul
     ** 3. Procesarea:
     **    -> selectarea mai multor documente
     **    -> export PDF
     **    -> transmiterea prin e-mail.
     **
     ******************************************************************************/
    return true;
}

bool UpdateReleasesApp::transferSqliteUuidsToCloud(bool auditOnly)
{
    QSqlDatabase localDb = db->getDatabase();
    QSqlDatabase imageDb = db->getDatabaseImage();
    if (!localDb.isOpen() ||
        localDb.driverName().compare(QStringLiteral("QSQLITE"),
        Qt::CaseInsensitive) != 0) {
        qCritical(logCritical())
            << "Transferul UUID necesită baza locală SQLite deschisă.";
        return false;
    }

    const QString connectionName = QStringLiteral("migration_uuid_cloud_%1")
                                       .arg(reinterpret_cast<quintptr>(this));
    bool success = true;
    QString failure;

    {
        const CloudConnectionData cloud = CloudConnectionContext::instance().data();
        QSqlDatabase cloudDb = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"),
                                                          connectionName);
        cloudDb.setHostName(cloud.hostName);
        cloudDb.setDatabaseName(cloud.databaseName);
        cloudDb.setPort(cloud.port);
        cloudDb.setConnectOptions(cloud.connectionOptions);
        cloudDb.setUserName(cloud.userName);
        cloudDb.setPassword(cloud.password);

        if (!cloudDb.open()) {
            success = false;
            failure = tr("Conexiunea MariaDB pentru transferul UUID nu poate fi deschisă: %1")
                          .arg(cloudDb.lastError().text());
        }

        qint64 totalTransferred = 0;
        bool transactionStarted = false;
        const auto cloudTableName = [&cloudDb](const QString &requested) {
            for (const QString &name : cloudDb.tables(QSql::Tables))
                if (name.compare(requested, Qt::CaseInsensitive) == 0)
                    return name;
            if (requested == QStringLiteral("patients"))
                for (const QString &name : cloudDb.tables(QSql::Tables))
                    if (name.compare(QStringLiteral("pacients"), Qt::CaseInsensitive) == 0)
                        return name;
            return QString();
        };
        const bool transferImages = imageDb.isOpen()
                                    && imageDb.tables(QSql::Tables).contains(
                                           QStringLiteral("imagesReports"), Qt::CaseInsensitive);
        if (success) {

            const auto audit = UuidMigration::build(localDb, imageDb, cloudDb, !auditOnly);
            for (const QString &line : audit.summary)
                qInfo(logInfo()).noquote() << "[UUID audit]" << line;

            for (const QString &line : audit.warnings)
                qWarning(logWarning()).noquote() << "[UUID audit]" << line;

            for (const QString &line : audit.conflicts)
                qCritical(logCritical()).noquote() << "[UUID audit]" << line;

            success = audit.valid();
            if (!success)
                failure = tr("Transferul UUID a fost oprit: conflict UUID sau eroare de schemă. Verificați jurnalul.");

            // DDL only after the complete read-only audit has passed. MariaDB
            // commits DDL implicitly; the following data transaction is separate.
            QStringList tables;
            const QStringList expectedTables = {
                QStringLiteral("contracts"), QStringLiteral("doctors"),
                QStringLiteral("investigations"), QStringLiteral("investigationsGroup"),
                QStringLiteral("conclusionTemplates"),
                QStringLiteral("formationsSystemTemplates"), QStringLiteral("nurses"),
                QStringLiteral("orderEcho"), QStringLiteral("organizations"),
                QStringLiteral("patients"), QStringLiteral("pricings"),
                QStringLiteral("reportEcho"), QStringLiteral("typesPrices"),
                QStringLiteral("users")
            };
            for (const QString &table : expectedTables) {
                if (!cloudTableName(table).isEmpty())
                    tables.append(cloudTableName(table));
                else {
                    success = false;
                    failure = QStringLiteral("UUID transfer: missing cloud table %1").arg(table);
                    break;
                }
            }
            if (transferImages) {
                if (!cloudTableName(QStringLiteral("imagesReports")).isEmpty())
                    tables.append(cloudTableName(QStringLiteral("imagesReports")));
                else {
                    success = false;
                    failure = QStringLiteral("UUID transfer: missing cloud table imagesReports");
                }
            }

            for (const QString &table : tables) {

                if (!success || auditOnly)
                    break;

                QSqlQuery engine(cloudDb);
                engine.prepare(QStringLiteral(R"(
                    SELECT ENGINE FROM information_schema.TABLES
                    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = ?
                )"));
                engine.addBindValue(table);
                if (!engine.exec() || !engine.next()
                    || engine.value(0).toString().compare("InnoDB", Qt::CaseInsensitive) != 0) {
                    success = false;
                    failure = QStringLiteral("UUID transfer requires InnoDB: %1").arg(table);
                    break;
                }

                if (!hasColumn(cloudDb, table, QStringLiteral("uuid"))) {
                    QSqlQuery alter(cloudDb);
                    if (!alter.exec(QStringLiteral(R"(
                        ALTER TABLE `%1` ADD COLUMN uuid BINARY(16) NULL
                    )").arg(table))) {
                        success = false;
                        failure = QStringLiteral("UUID column creation failed: %1").arg(table);
                    }
                }
            }
        }

        if (success && !auditOnly) {
            transactionStarted = cloudDb.transaction();
            success = transactionStarted;
            if (!success)
                failure = QStringLiteral("UUID transfer: transaction failed");
        }

        if (success && !auditOnly) {
            // Rebuild under row locks: a plan from before DDL must not authorize
            // writes against data changed by another connection in the meantime.
            const auto plan = UuidMigration::build(localDb, imageDb, cloudDb, true, true);
            success = plan.valid();
            if (!success) {
                failure = QStringLiteral("UUID transfer: locked revalidation failed");
                for (const QString &line : plan.conflicts)
                    qCritical(logCritical()).noquote() << "[UUID audit]" << line;
            }

            for (const auto &match : plan.matches) {
                if (!success) break;
                QSqlQuery update(cloudDb);
                update.prepare(QStringLiteral(R"(
                    UPDATE `%1`
                    SET uuid = ?
                    WHERE id = ? AND (uuid IS NULL OR OCTET_LENGTH(uuid) = 0)
                )").arg(match.cloudTable));
                update.addBindValue(match.uuid, QSql::Binary);
                update.addBindValue(match.cloudId);

                if (!update.exec()) {
                    success = false;
                    failure = QStringLiteral("UUID transfer failed: %1 cloud_id=%2 (%3)")
                                  .arg(match.cloudTable).arg(match.cloudId)
                                  .arg(update.lastError().nativeErrorCode());
                    break;
                }

                totalTransferred += update.numRowsAffected();

                QSqlQuery verify(cloudDb);
                verify.prepare(QStringLiteral(R"(
                    SELECT uuid FROM `%1` WHERE id = ?
                )").arg(match.cloudTable));
                verify.addBindValue(match.cloudId);
                if (!verify.exec() || !verify.next() || verify.value(0).toByteArray() != match.uuid) {
                    success = false;
                    failure = QStringLiteral("UUID transfer verification failed: %1 cloud_id=%2")
                                  .arg(match.cloudTable).arg(match.cloudId);
                }
            }

            if (success && !cloudDb.commit()) {
                success = false;
                failure = QStringLiteral("UUID transfer: commit failed");
            }
        }

        if (!success && transactionStarted)
            cloudDb.rollback();

        if (success) {
            if (auditOnly)
                qInfo(logInfo()) << "[UUID] Read-only audit passed.";
            else
                qInfo(logInfo()) << "[UUID] Transfer committed. Rows:" << totalTransferred;
        }

        cloudDb.close();
    }

    QSqlDatabase::removeDatabase(connectionName);
    if (!success)
        qCritical(logCritical()).noquote() << failure;
    return success;
}


bool UpdateReleasesApp::update_4_0_1()
{
    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical()) << "Actualizarea la 4.0.1: baza de date nu este deschisă.";
        return false;
    }

    // Verify cross-database identities before this migration creates UUIDs or
    // changes either schema. No fallback to equal numeric IDs is permitted.
    if (MainDatabaseConnectionContext::instance().isSqlite()
        && SettingsService::instance().synchronization().enabled
        && !transferSqliteUuidsToCloud(true))
        return false;

    emit migrationProgress(0, 0, tr("Se actualizează schema programărilor pacienților..."));
    if (!ensurePatientAppointmentsSchema())
        return false;

    //----------------------------------------------------------------------
    // I. UUID
    //----------------------------------------------------------------------
    /** SQLite este sursa a UUID-lor pentru local/cloud.
     ** Generarea in ambele baze, independent, ar rupe inregistrari istorice. */
    emit migrationProgress(0, 0, tr("UUID: se generează identificatorii în baza locală..."));

    if (!db->ensureUUIDs()) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1: generarea UUID-urilor a eșuat.";
        return false;
    }
    if (!db->ensureIndexUUIDs()) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1: crearea indexurilor UUID a eșuat.";
        return false;
    }

    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        QSqlDatabase imageDb = db->getDatabaseImage();
        if (imageDb.isOpen()
            && imageDb.tables(QSql::Tables).contains(QStringLiteral("imagesReports"),
                                                      Qt::CaseInsensitive)) {

            QSqlRecord imageRecord = imageDb.record(QStringLiteral("imagesReports"));
            QSqlQuery imageQuery(imageDb);

            /** 1. adaugam column UUID  */
            if (!imageRecord.contains(QStringLiteral("uuid"))
                && !imageQuery.exec(QStringLiteral(
                    "ALTER TABLE imagesReports ADD COLUMN uuid BLOB"))) {
                qCritical(logCritical())
                    << "Actualizarea la 4.0.1: uuid nu a putut fi adăugat în db_image.imagesReports:"
                    << imageQuery.lastError().text();
                return false;
            }

            /** 2. verificam daca sunt imagini fara UUID */
            if (!imageQuery.exec(QStringLiteral(
                    "SELECT id FROM imagesReports WHERE uuid IS NULL OR length(uuid) <> 16"))) {
                qCritical(logCritical())
                    << "Actualizarea la 4.0.1: imaginile fără UUID nu au putut fi citite:"
                    << imageQuery.lastError().text();
                return false;
            }
            QList<qint64> imageIds;
            while (imageQuery.next())
                imageIds.append(imageQuery.value(0).toLongLong());

            /** 3. salvam imaginile */
            QSqlQuery updateImage(imageDb);
            updateImage.prepare(QStringLiteral("UPDATE imagesReports SET uuid=? WHERE id=?"));
            for (qint64 imageId : std::as_const(imageIds)) {
                updateImage.bindValue(0, QUuid::createUuid().toRfc4122(), QSql::Binary);
                updateImage.bindValue(1, imageId);
                if (!updateImage.exec()) {
                    qCritical(logCritical())
                        << "Actualizarea la 4.0.1: UUID imagine nu a putut fi salvat, id="
                        << imageId << updateImage.lastError().text();
                    return false;
                }
            }

            /** 4. cream indexul */
            if (!imageQuery.exec(QStringLiteral(
                    "CREATE UNIQUE INDEX IF NOT EXISTS uq_imagesReports_uuid "
                    "ON imagesReports(uuid)"))) {
                qCritical(logCritical())
                    << "Actualizarea la 4.0.1: indexul UUID pentru imagini nu a putut fi creat:"
                    << imageQuery.lastError().text();
                return false;
            }
            qInfo(logInfo())
                << tr("UUID generate în db_image.imagesReports: %1.").arg(imageIds.size());
        }
    }

    /** efectuam transfer in mariaDB */
    if (MainDatabaseConnectionContext::instance().isSqlite()
        && SettingsService::instance().synchronization().enabled) {
        emit migrationProgress(0, 0, tr("UUID: se transferă identificatorii în MariaDB..."));
        if (!transferSqliteUuidsToCloud())
            return false;
    }

    // emitem finalizarea
    emit migrationProgress(1, 1, tr("Actualizarea UUID s-a finalizat."));

    //----------------------------------------------------------------------
    // Functii auxiliare
    //----------------------------------------------------------------------

    const bool useTransaction = MainDatabaseConnectionContext::instance().isSqlite();
    if (useTransaction && !currentDb.transaction()) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1: tranzacția SQLite nu a putut fi pornită:"
            << currentDb.lastError().text();
        return false;
    }

    const auto rollbackMigration = [&]() {
        if (useTransaction && !currentDb.rollback())
            qCritical(logCritical())
                << "Actualizarea la 4.0.1: rollback-ul SQLite a eșuat:"
                << currentDb.lastError().text();
        return false;
    };

    QSqlQuery query(currentDb);
    const auto execSql = [&](const QString &sql, const QString &context) {
        if (query.exec(sql))
            return true;

        qCritical(logCritical()).noquote()
        << QStringLiteral("Actualizarea la 4.0.1 (%1) a eșuat: %2")
               .arg(context, query.lastError().text());
        return false;
    };

    // Bazele 3.x nu conțin întotdeauna configurarea e-mail/cloud. Pentru
    // MariaDB, tipurile FK trebuie să coincidă exact cu ID-urile bazei vechi.
    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        const QStringList existingTables = currentDb.tables(QSql::Tables);
        const auto mariaDbIdType = [&currentDb](const QString &table) -> QString {
            QSqlQuery typeQuery(currentDb);
            typeQuery.prepare(QStringLiteral(
                "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS "
                "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME=? AND COLUMN_NAME='id'"));
            typeQuery.addBindValue(table);
            if (!typeQuery.exec() || !typeQuery.next())
                return {};
            return typeQuery.value(0).toString().trimmed();
        };
        const QString organizationIdType = mariaDbIdType(QStringLiteral("organizations"));
        const QString userIdType = mariaDbIdType(QStringLiteral("users"));
        if (organizationIdType.isEmpty() || userIdType.isEmpty()) {
            qCritical(logCritical())
                << "Migrarea configurării cloud: tipurile organizations.id/users.id nu pot fi determinate.";
            return rollbackMigration();
        }

        if (!existingTables.contains(QStringLiteral("onlineAccount"), Qt::CaseInsensitive)) {
            const QString createOnlineAccount = QStringLiteral(
                "CREATE TABLE onlineAccount ("
                "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
                "id_organizations %1 NOT NULL,id_users %2 NOT NULL,"
                "email VARCHAR(255) NOT NULL,smtp_server VARCHAR(255) NOT NULL,"
                "port VARCHAR(5) NOT NULL,username VARCHAR(50) NOT NULL,"
                "password VARCHAR(255) NOT NULL,iv VARCHAR(24) NOT NULL,"
                "tag VARCHAR(24) NOT NULL,PRIMARY KEY(id),"
                "UNIQUE KEY uq_onlineAccount_email(email),"
                "KEY idx_onlineAccount_organizations(id_organizations),"
                "KEY idx_onlineAccount_users(id_users),"
                "CONSTRAINT fk_onlineAccount_organizations FOREIGN KEY(id_organizations) "
                "REFERENCES organizations(id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                "CONSTRAINT fk_onlineAccount_users FOREIGN KEY(id_users) "
                "REFERENCES users(id) ON DELETE CASCADE ON UPDATE RESTRICT"
                ") ENGINE=InnoDB").arg(organizationIdType, userIdType);
            if (!execSql(createOnlineAccount, QStringLiteral("creare onlineAccount")))
                return rollbackMigration();
        }

        if (!existingTables.contains(QStringLiteral("cloudServer"), Qt::CaseInsensitive)) {
            const QString createCloudServer = QStringLiteral(
                "CREATE TABLE cloudServer ("
                "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
                "id_organizations %1 NOT NULL,id_users %2 NOT NULL,"
                "hostName VARCHAR(30) NOT NULL,databaseName VARCHAR(30) NOT NULL,"
                "port VARCHAR(5),connectionOption VARCHAR(255),"
                "username VARCHAR(50) NOT NULL,password VARCHAR(255) NOT NULL,"
                "iv VARCHAR(24) NOT NULL,PRIMARY KEY(id),"
                "UNIQUE KEY uq_cloudServer_organization_user(id_organizations,id_users),"
                "KEY idx_cloudServer_organizations(id_organizations),"
                "KEY idx_cloudServer_users(id_users),"
                "CONSTRAINT fk_cloudServer_organizations FOREIGN KEY(id_organizations) "
                "REFERENCES organizations(id) ON DELETE CASCADE ON UPDATE RESTRICT,"
                "CONSTRAINT fk_cloudServer_users FOREIGN KEY(id_users) "
                "REFERENCES users(id) ON DELETE CASCADE ON UPDATE RESTRICT"
                ") ENGINE=InnoDB").arg(organizationIdType, userIdType);
            if (!execSql(createCloudServer, QStringLiteral("creare cloudServer")))
                return rollbackMigration();
        }
    } else {
        if (!DataBaseCommon::execFileBatch(currentDb,
                                           QStringLiteral(":/sql/sqlite/tables/online_account.sql"),
                                           QStringLiteral("onlineAccount (migrare 4.0.1)")))
            return rollbackMigration();
        if (!DataBaseCommon::execFileBatch(currentDb,
                                           QStringLiteral(":/sql/sqlite/tables/cloud_server.sql"),
                                           QStringLiteral("cloudServer (migrare 4.0.1)")))
            return rollbackMigration();
    }

    emit migrationProgress(0, 0, tr("Tabelele conturilor e-mail și cloud sunt pregătite."));

    if (!ensureGestation2Columns(currentDb))
        return rollbackMigration();

    if (!migrateInvestigationsOwnerColumn(currentDb))
        return rollbackMigration();

    const auto recreateView = [&](const QString &viewName,
                                  const QString &resourcePath) {
        if (!execSql(QStringLiteral("DROP VIEW IF EXISTS %1").arg(viewName),
                     tr("eliminare view %1").arg(viewName)))
            return false;

        QFile viewFile(resourcePath);
        if (!viewFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical(logCritical())
                << tr("Actualizarea la 4.0.1: resursa view-ului %1 nu poate fi citită: %2")
                       .arg(viewName, resourcePath);
            return false;
        }

        const QString createSql = QString::fromUtf8(viewFile.readAll()).trimmed();
        return execSql(createSql, tr("creare view %1").arg(viewName));
    };

    const auto recreateApplicationViews = [&]() {
        const QString basePath = MainDatabaseConnectionContext::instance().isSqlite()
                                     ? QStringLiteral(":/sql/sqlite/views/")
                                     : QStringLiteral(":/sql/mariadb/views/");
        const QList<QPair<QString, QString>> views = {
            {
                QStringLiteral("v_users_combo_active"),
                QStringLiteral("users_combo_view.sql")
            },
            {
                QStringLiteral("v_doctors_active"),
                QStringLiteral("doctors_combo_view.sql")
            },
            {
                QStringLiteral("v_nurses_active"),
                QStringLiteral("nurses_combo_view.sql")
            },
            {
                QStringLiteral("v_types_prices_active"),
                QStringLiteral("typePrices_combo_view.sql")
            },
            {
                QStringLiteral("v_organizations_active"),
                QStringLiteral("organizations_combo_view.sql")
            },
            {
                QStringLiteral("v_contracts_listView_active"),
                QStringLiteral("contracts_listForm_view.sql")
            }
        };

        for (const auto &view : views) {
            if (!recreateView(view.first, basePath + view.second))
                return false;
        }
        return true;
    };

    const auto rowCount = [&](const QString &tableName) {
        QSqlQuery countQuery(currentDb);
        if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(tableName))
            || !countQuery.next())
            return qint64(-1);
        return countQuery.value(0).toLongLong();
    };

    const auto duplicateGroupCount = [&](const QString &tableName,
                                          const QString &groupColumns) {
        QSqlQuery duplicateQuery(currentDb);
        const QString sql = QStringLiteral(
            "SELECT COUNT(*) FROM ("
            "SELECT 1 FROM %1 "
            "WHERE docYear IS NOT NULL AND numberDoc IS NOT NULL AND numberDoc <> '' "
            "GROUP BY %2 HAVING COUNT(*) > 1) duplicate_groups")
                                .arg(tableName, groupColumns);
        if (!duplicateQuery.exec(sql) || !duplicateQuery.next()) {
            qCritical(logCritical())
                << tr("Nu s-au putut verifica duplicatele istorice din %1: %2")
                       .arg(tableName, duplicateQuery.lastError().text());
            return qint64(-1);
        }
        return duplicateQuery.value(0).toLongLong();
    };

    const auto convertNumbersInBatches = [&](const QString &tableName,
                                              const QString &condition,
                                              const QString &valueExpression,
                                              qint64 progressOffset,
                                              qint64 progressMaximum,
                                              const QString &documentLabel,
                                              qint64 *convertedCount) {
        QSqlQuery idQuery(currentDb);
        const QString selectSql = QStringLiteral("SELECT id FROM %1 WHERE %2 ORDER BY id")
                                      .arg(tableName, condition);
        if (!idQuery.exec(selectSql)) {
            qCritical(logCritical()).noquote()
                << QStringLiteral("Actualizarea la 4.0.1 (selectare %1) a eșuat: %2")
                       .arg(documentLabel, idQuery.lastError().text());
            return false;
        }

        QList<qint64> ids;
        while (idQuery.next())
            ids.append(idQuery.value(0).toLongLong());
        idQuery.finish();

        constexpr qsizetype batchSize = 250;
        qint64 processed = 0;
        for (qsizetype begin = 0; begin < ids.size(); begin += batchSize) {
            const qsizetype end = qMin(begin + batchSize, ids.size());
            QStringList batchIds;
            batchIds.reserve(end - begin);
            for (qsizetype index = begin; index < end; ++index)
                batchIds.append(QString::number(ids.at(index)));

            const QString updateSql = QStringLiteral(
                                          "UPDATE %1 SET numberDoc = %2 WHERE id IN (%3)")
                                          .arg(tableName,
                                               valueExpression,
                                               batchIds.join(QLatin1Char(',')));
            if (!execSql(updateSql,
                         tr("formatare lot %1").arg(documentLabel)))
                return false;

            processed += query.numRowsAffected();
            emit migrationProgress(
                static_cast<int>(progressOffset + processed),
                static_cast<int>(progressMaximum),
                tr("Numerotare anuală: %1 din %2 %3 convertite...")
                    .arg(processed)
                    .arg(ids.size())
                    .arg(documentLabel));
        }

        *convertedCount = processed;
        return true;
    };

    //----------------------------------------------------------------------
    // II. Actualizarea
    //----------------------------------------------------------------------

    const qint64 orderCount = rowCount(QStringLiteral("orderEcho"));
    const qint64 reportCount = rowCount(QStringLiteral("reportEcho"));
    const QSqlRecord reportRecord = currentDb.record(QStringLiteral("reportEcho"));
    const QString reportPatientColumn = reportRecord.contains(QStringLiteral("patient_id"))
                                            ? QStringLiteral("patient_id")
                                        : reportRecord.contains(QStringLiteral("id_pacients"))
                                            ? QStringLiteral("id_pacients")
                                        : reportRecord.contains(QStringLiteral("id_patients"))
                                            ? QStringLiteral("id_patients") : QString();
    if (reportPatientColumn.isEmpty()) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1: coloana pacientului lipsește în reportEcho.";
        return false;
    }

    emit migrationProgress(
        0, 0,
        tr("Numerotare anuală: %1 comenzi și %2 rapoarte de procesat...")
            .arg(orderCount)
            .arg(reportCount));

    if (MainDatabaseConnectionContext::instance().isSqlite()) {

        // 1. cream tabela 'doc_sequences'
        if (!execSql(QStringLiteral(
                         "CREATE TABLE IF NOT EXISTS doc_sequences ("
                         "name TEXT NOT NULL, "
                         "year INTEGER NOT NULL, "
                         "value INTEGER NOT NULL DEFAULT 0, "
                         "PRIMARY KEY (name, year))"),
                     QStringLiteral("creare doc_sequences")))
            return rollbackMigration();
        emit migrationProgress(0, 0, tr("Tabela secvențelor anuale este pregătită."));

        // 2. adaugam clumn 'docYear' in orderEcho
        if (!currentDb.record("orderEcho").contains("docYear")
            && !execSql(QStringLiteral(
                            "ALTER TABLE orderEcho ADD COLUMN docYear INTEGER "
                            "GENERATED ALWAYS AS (CAST(strftime('%Y', dateDoc) AS INTEGER)) VIRTUAL"),
                        QStringLiteral("adăugare orderEcho.docYear")))
            return rollbackMigration();

        // 2. adaugam clumn 'docYear' in reportEcho
        if (!currentDb.record("reportEcho").contains("docYear")
            && !execSql(QStringLiteral(
                            "ALTER TABLE reportEcho ADD COLUMN docYear INTEGER "
                            "GENERATED ALWAYS AS (CAST(strftime('%Y', dateDoc) AS INTEGER)) VIRTUAL"),
                        QStringLiteral("adăugare reportEcho.docYear")))
            return rollbackMigration();
        emit migrationProgress(0, 0, tr("Coloanele pentru anul documentelor sunt pregătite."));

        // 3. pregatim conditiile pu nr. documentelor
        const QString orderCondition = QStringLiteral(
            "docYear IS NOT NULL AND numberDoc <> '' AND numberDoc NOT GLOB '*[^0-9]*'");
        const QString reportCondition = orderCondition;
        const qint64 convertibleOrders = rowCount(QStringLiteral(
            "orderEcho WHERE docYear IS NOT NULL AND numberDoc <> '' "
            "AND numberDoc NOT GLOB '*[^0-9]*'"));
        const qint64 convertibleReports = rowCount(QStringLiteral(
            "reportEcho WHERE docYear IS NOT NULL AND numberDoc <> '' "
            "AND numberDoc NOT GLOB '*[^0-9]*'"));
        if (convertibleOrders < 0 || convertibleReports < 0) {
            qCritical(logCritical())
                << "Actualizarea la 4.0.1: nu s-a putut determina numărul documentelor convertibile.";
            return rollbackMigration();
        }
        const qint64 progressMaximum = qMax<qint64>(1, convertibleOrders + convertibleReports + 5);

        //--- order
        qint64 convertedOrders = 0;
        if (!convertNumbersInBatches(
                QStringLiteral("orderEcho"), orderCondition,
                QStringLiteral("CAST(CAST(numberDoc AS INTEGER) AS TEXT) || '/' || docYear"),
                2, progressMaximum, tr("comenzi"), &convertedOrders))
            return rollbackMigration();
        emit migrationProgress(static_cast<int>(2 + convertibleOrders),
                               static_cast<int>(progressMaximum),
                               tr("Numerotare anuală: %1 comenzi convertite.")
                                   .arg(convertedOrders));

        //--- report
        qint64 convertedReports = 0;
        if (!convertNumbersInBatches(
                QStringLiteral("reportEcho"), reportCondition,
                QStringLiteral("CAST(CAST(numberDoc AS INTEGER) AS TEXT) || '/' || docYear"),
                2 + convertibleOrders, progressMaximum, tr("rapoarte"), &convertedReports))
            return rollbackMigration();
        emit migrationProgress(static_cast<int>(2 + convertibleOrders + convertibleReports),
                               static_cast<int>(progressMaximum),
                               tr("Numerotare anuală: %1 rapoarte convertite.")
                                   .arg(convertedReports));

        // 4. crearea indexurilor
        if (!execSql(QStringLiteral("DROP INDEX IF EXISTS uq_orderEcho_org_number"),
                     QStringLiteral("eliminare index comenzi"))
            || !execSql(QStringLiteral(
                            "CREATE UNIQUE INDEX uq_orderEcho_org_number "
                            "ON orderEcho(id_organizations, numberDoc, docYear)"),
                        QStringLiteral("creare index anual comenzi"))
            || !execSql(QStringLiteral("DROP INDEX IF EXISTS uq_reportEcho_pacients_number"),
                        QStringLiteral("eliminare index rapoarte"))
            || !execSql(QStringLiteral(
                            "CREATE UNIQUE INDEX uq_reportEcho_pacients_number "
                            "ON reportEcho(%1, numberDoc, docYear)")
                            .arg(reportPatientColumn),
                        QStringLiteral("creare index anual rapoarte")))
            return rollbackMigration();
        emit migrationProgress(static_cast<int>(3 + convertibleOrders + convertibleReports),
                               static_cast<int>(progressMaximum),
                               tr("Indexurile unice anuale au fost create."));

        // 5. executarea
        if (!execSql(QStringLiteral(
                         "INSERT INTO doc_sequences(name, year, value) "
                         "SELECT 'orderEcho', docYear, MAX(CAST(numberDoc AS INTEGER)) "
                         "FROM orderEcho WHERE docYear IS NOT NULL GROUP BY docYear "
                         "ON CONFLICT(name, year) DO UPDATE SET value = MAX(value, excluded.value)"),
                     QStringLiteral("inițializare secvențe comenzi"))

            || !execSql(QStringLiteral(
                            "INSERT INTO doc_sequences(name, year, value) "
                            "SELECT 'reportEcho', docYear, MAX(CAST(numberDoc AS INTEGER)) "
                            "FROM reportEcho WHERE docYear IS NOT NULL GROUP BY docYear "
                            "ON CONFLICT(name, year) DO UPDATE SET value = MAX(value, excluded.value)"),
                        QStringLiteral("inițializare secvențe rapoarte")))
            return rollbackMigration();

        emit migrationProgress(static_cast<int>(4 + convertibleOrders + convertibleReports),
                               static_cast<int>(progressMaximum),
                               tr("Secvențele anuale au fost inițializate."));

    } else if (MainDatabaseConnectionContext::instance().isMariaDb()) {

        if (!execSql(QStringLiteral(
                         "CREATE TABLE IF NOT EXISTS doc_sequences ("
                         "name VARCHAR(255) NOT NULL, "
                         "year SMALLINT NOT NULL, "
                         "value BIGINT UNSIGNED NOT NULL DEFAULT 0, "
                         "PRIMARY KEY (name, year)) ENGINE=InnoDB"),
                     QStringLiteral("creare doc_sequences")))

            return false;

        if (!currentDb.record("orderEcho").contains("docYear")
            && !execSql(QStringLiteral(
                            "ALTER TABLE orderEcho ADD COLUMN docYear SMALLINT "
                            "GENERATED ALWAYS AS (YEAR(dateDoc)) STORED"),
                        QStringLiteral("adăugare orderEcho.docYear")))
            return false;

        if (!currentDb.record("reportEcho").contains("docYear")
            && !execSql(QStringLiteral(
                            "ALTER TABLE reportEcho ADD COLUMN docYear SMALLINT "
                            "GENERATED ALWAYS AS (YEAR(dateDoc)) STORED"),
                        QStringLiteral("adăugare reportEcho.docYear")))
            return false;

        if (!execSql(QStringLiteral(
                         "UPDATE orderEcho "
                         "SET numberDoc = CONCAT(CAST(numberDoc AS UNSIGNED), '/', docYear) "
                         "WHERE docYear IS NOT NULL AND numberDoc REGEXP '^[0-9]+$'"),
                     QStringLiteral("formatare numere existente comenzi"))
            || !execSql(QStringLiteral(
                            "UPDATE reportEcho "
                            "SET numberDoc = CONCAT(CAST(numberDoc AS UNSIGNED), '/', docYear) "
                            "WHERE docYear IS NOT NULL AND numberDoc REGEXP '^[0-9]+$'"),
                        QStringLiteral("formatare numere existente rapoarte")))
            return false;

        const qint64 duplicateOrderGroups = duplicateGroupCount(
            QStringLiteral("orderEcho"),
            QStringLiteral("id_organizations, numberDoc, docYear"));

        const qint64 duplicateReportGroups = duplicateGroupCount(
            QStringLiteral("reportEcho"),
            QStringLiteral("%1, numberDoc, docYear").arg(reportPatientColumn));
        if (duplicateOrderGroups < 0 || duplicateReportGroups < 0)
            return false;

        if (!execSql(QStringLiteral(
                         "DROP INDEX IF EXISTS uq_orderEcho_org_number ON orderEcho"),
                     QStringLiteral("eliminare index unic comenzi"))
            || !execSql(QStringLiteral(
                            "DROP INDEX IF EXISTS idx_orderEcho_org_number_year ON orderEcho"),
                        QStringLiteral("eliminare index normal comenzi")))
            return false;

        if (duplicateOrderGroups == 0) {

            if (!execSql(QStringLiteral(
                             "CREATE UNIQUE INDEX uq_orderEcho_org_number "
                             "ON orderEcho(id_organizations, numberDoc, docYear)"),
                         QStringLiteral("creare index unic anual comenzi")))
                return false;

        } else {
            qWarning(logWarning())
                << tr("În orderEcho au fost găsite %1 grupuri de numere istorice duplicate. "
                      "Numerele sunt păstrate; se creează index normal.")
                       .arg(duplicateOrderGroups);
            if (!execSql(QStringLiteral(
                             "CREATE INDEX idx_orderEcho_org_number_year "
                             "ON orderEcho(id_organizations, numberDoc, docYear)"),
                         QStringLiteral("creare index normal anual comenzi")))
                return false;
        }

        if (!execSql(QStringLiteral(
                         "DROP INDEX IF EXISTS uq_reportEcho_pacients_number ON reportEcho"),
                     QStringLiteral("eliminare index unic rapoarte"))
            || !execSql(QStringLiteral(
                            "DROP INDEX IF EXISTS idx_reportEcho_patient_number_year ON reportEcho"),
                        QStringLiteral("eliminare index normal rapoarte")))
            return false;

        if (duplicateReportGroups == 0) {
            if (!execSql(QStringLiteral(
                             "CREATE UNIQUE INDEX uq_reportEcho_pacients_number "
                             "ON reportEcho(%1, numberDoc, docYear)")
                             .arg(reportPatientColumn),
                         QStringLiteral("creare index unic anual rapoarte")))
                return false;
        } else {
            qWarning(logWarning())
                << tr("În reportEcho au fost găsite %1 grupuri de numere istorice duplicate. "
                      "Numerele sunt păstrate; se creează index normal.")
                       .arg(duplicateReportGroups);
            if (!execSql(QStringLiteral(
                             "CREATE INDEX idx_reportEcho_patient_number_year "
                             "ON reportEcho(%1, numberDoc, docYear)")
                             .arg(reportPatientColumn),
                         QStringLiteral("creare index normal anual rapoarte")))
                return false;
        }

        if (!execSql(QStringLiteral(
                         "INSERT INTO doc_sequences(name, year, value) "
                         "SELECT 'orderEcho', docYear, "
                         "MAX(CAST(SUBSTRING_INDEX(numberDoc, '/', 1) AS UNSIGNED)) "
                         "FROM orderEcho WHERE docYear IS NOT NULL "
                         "AND numberDoc REGEXP '^[0-9]+/[0-9]{4}$' GROUP BY docYear "
                         "ON DUPLICATE KEY UPDATE value = GREATEST(value, VALUES(value))"),
                     QStringLiteral("inițializare secvențe comenzi"))
            || !execSql(QStringLiteral(
                            "INSERT INTO doc_sequences(name, year, value) "
                            "SELECT 'reportEcho', docYear, "
                            "MAX(CAST(SUBSTRING_INDEX(numberDoc, '/', 1) AS UNSIGNED)) "
                            "FROM reportEcho WHERE docYear IS NOT NULL "
                            "AND numberDoc REGEXP '^[0-9]+/[0-9]{4}$' GROUP BY docYear "
                            "ON DUPLICATE KEY UPDATE value = GREATEST(value, VALUES(value))"),
                        QStringLiteral("inițializare secvențe rapoarte")))
            return false;
    } else {
        qCritical(logCritical()) << "Actualizarea la 4.0.1: tip de bază de date nesuportat.";
        return rollbackMigration();
    }

    if (!recreateApplicationViews())
        return rollbackMigration();

    emit migrationProgress(0, 0, tr("View-urile pentru liste și selectoare au fost actualizate."));

    if (rowCount(QStringLiteral("orderEcho")) != orderCount
        || rowCount(QStringLiteral("reportEcho")) != reportCount) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1 a modificat neașteptat numărul documentelor.";
        return rollbackMigration();
    }

    emit migrationProgress(0, 0, tr("Se verifică integritatea bazei actualizate..."));
    if (!validatePostMigration(QVersionNumber(4, 0, 1)))
        return rollbackMigration();

    if (useTransaction && !currentDb.commit()) {
        qCritical(logCritical())
            << "Actualizarea la 4.0.1: commit-ul SQLite a eșuat:"
            << currentDb.lastError().text();
        currentDb.rollback();
        return false;
    }

    emit migrationProgress(7, 7,
                           tr("Numerotarea anuală s-a finalizat: %1 comenzi, %2 rapoarte.")
                               .arg(orderCount)
                               .arg(reportCount));

    qInfo(logInfo()) << "Actualizarea numerotării anuale pentru versiunea 4.0.1 s-a finalizat.";
    return true;
}

bool UpdateReleasesApp::update_4_1_0()
{
    if (!ensurePatientAppointmentsSchema())
        return false;

    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: baza de date nu este deschisă.";
        return false;
    }

    QSqlQuery query(currentDb);
    const auto execSql = [&](const QString &sql, const QString &context) {
        if (query.exec(sql))
            return true;
        qCritical(logCritical()).noquote()
            << QStringLiteral("Actualizarea la 4.1.0 (%1) a eșuat: %2; SQL: %3")
                   .arg(context, query.lastError().text(), sql);
        return false;
    };
    const auto tableExists = [&](const QString &name) {
        return currentDb.tables(QSql::Tables).contains(name, Qt::CaseInsensitive);
    };
    const auto columnExists = [&](const QString &table, const QString &column) {
        if (!tableExists(table))
            return false;
        const QSqlRecord record = currentDb.record(table);
        for (int index = 0; index < record.count(); ++index) {
            if (record.fieldName(index).compare(column, Qt::CaseSensitive) == 0)
                return true;
        }
        return false;
    };
    const auto rowCount = [&](const QString &table) -> qint64 {
        QSqlQuery countQuery(currentDb);
        if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(table))
            || !countQuery.next())
            return -1;
        return countQuery.value(0).toLongLong();
    };
    const auto renameColumn = [&](const QString &table, const QString &oldName,
                                  const QString &newName) {
        const bool oldExists = columnExists(table, oldName);
        const bool newExists = columnExists(table, newName);
        if (oldExists == newExists) {
            qCritical(logCritical())
                << "Actualizarea la 4.1.0: stare ambiguă pentru"
                << table + "." + oldName << "->" << newName;
            return false;
        }
        if (newExists)
            return true;
        return execSql(QStringLiteral("ALTER TABLE %1 RENAME COLUMN %2 TO %3")
                           .arg(table, oldName, newName),
                       QStringLiteral("redenumire %1.%2").arg(table, oldName));
    };
    const auto recreatePatientView = [&]() {
        if (!execSql(QStringLiteral("DROP VIEW IF EXISTS v_pacients_completer_active"),
                     QStringLiteral("eliminare view vechi"))
            || !execSql(QStringLiteral("DROP VIEW IF EXISTS v_patients_completer_active"),
                        QStringLiteral("eliminare view nou")))
            return false;
        const QString path = MainDatabaseConnectionContext::instance().isSqlite()
            ? QStringLiteral(":/sql/sqlite/views/patients_completer_view.sql")
            : QStringLiteral(":/sql/mariadb/views/patients_completer_view.sql");
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical(logCritical()) << "Actualizarea la 4.1.0: resursa view nu poate fi citită:" << path;
            return false;
        }
        return execSql(QString::fromUtf8(file.readAll()).trimmed(),
                       QStringLiteral("creare view pacienți"));
    };

    const QString sourceTable = tableExists(QStringLiteral("pacients"))
        ? QStringLiteral("pacients") : QStringLiteral("patients");
    const qint64 patientsBefore = rowCount(sourceTable);
    const qint64 ordersBefore   = rowCount(QStringLiteral("orderEcho"));
    const qint64 reportsBefore  = rowCount(QStringLiteral("reportEcho"));

    if (patientsBefore < 0 || ordersBefore < 0 || reportsBefore < 0) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: nu pot fi citite valorile de control.";
        return false;
    }

    emit migrationProgress(0, 8, tr("4.1.0: verificarea schemei pacienților..."));
    const bool transactional = MainDatabaseConnectionContext::instance().isSqlite();
    if (transactional && !currentDb.transaction()) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: tranzacția SQLite nu poate fi pornită:"
                                 << currentDb.lastError().text();
        return false;
    }

    const auto fail = [&]() {
        if (transactional)
            currentDb.rollback();
        return false;
    };

    if (!execSql(QStringLiteral("DROP VIEW IF EXISTS v_pacients_completer_active"),
                 QStringLiteral("eliminare view vechi")))

        return fail();

    emit migrationProgress(1, 8, tr("4.1.0: view-ul vechi a fost eliminat."));

    const QStringList oldTriggers =
    {
        QStringLiteral("create_full_name_pacient"),
        QStringLiteral("update_full_name_pacient"),
        // Nume folosite de unele versiuni intermediare de dezvoltare.
        QStringLiteral("pacients_after_insert"), QStringLiteral("pacients_after_update"),
        QStringLiteral("patients_after_insert"), QStringLiteral("patients_after_update")
    };

    for (const QString &trigger : oldTriggers) {
        const QString quotedTrigger = MainDatabaseConnectionContext::instance().isMariaDb()
            ? QStringLiteral("`%1`").arg(trigger)
            : trigger;
        if (!execSql(QStringLiteral("DROP TRIGGER IF EXISTS %1").arg(quotedTrigger),
                     QStringLiteral("eliminare trigger %1").arg(trigger)))
            return fail();
    }

    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        // FK-urile către tabela veche sunt descoperite, nu presupuse după nume.
        QSqlQuery fkQuery(currentDb);
        if (!fkQuery.exec(QStringLiteral(R"(
            SELECT DISTINCT
                TABLE_NAME,
                CONSTRAINT_NAME
            FROM
                information_schema.KEY_COLUMN_USAGE
            WHERE CONSTRAINT_SCHEMA=DATABASE() AND
                REFERENCED_TABLE_NAME='pacients'
            )"))) {
            qCritical(logCritical()) << "Actualizarea la 4.1.0: FK-urile nu pot fi inventariate:"
                                     << fkQuery.lastError().text();
            return false;
        }
        QList<QPair<QString, QString>> foreignKeys;
        while (fkQuery.next())
            foreignKeys.append({fkQuery.value(0).toString(), fkQuery.value(1).toString()});
        for (const auto &foreignKey : std::as_const(foreignKeys)) {
            if (!execSql(QStringLiteral("ALTER TABLE `%1` DROP FOREIGN KEY `%2`")
                             .arg(foreignKey.first, foreignKey.second),
                         QStringLiteral("eliminare FK %1").arg(foreignKey.second)))
                return false;
        }
    }
    emit migrationProgress(2, 8, tr("4.1.0: dependențele schemei vechi au fost eliminate."));

    const bool oldTable = tableExists(QStringLiteral("pacients"));
    const bool newTable = tableExists(QStringLiteral("patients"));
    if (oldTable == newTable) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: starea tabelelor pacients/patients este ambiguă.";
        return fail();
    }
    if (oldTable && !execSql(QStringLiteral("ALTER TABLE pacients RENAME TO patients"),
                             QStringLiteral("redenumire tabelă pacienți")))
        return fail();

    const QList<QPair<QString, QString>> patientColumns = {
        {QStringLiteral("deletionMark"),  QStringLiteral("deletion_mark")},
        {QStringLiteral("IDNP"),          QStringLiteral("idnp")},
        {QStringLiteral("name"),          QStringLiteral("last_name")},
        {QStringLiteral("fName"),         QStringLiteral("first_name")},
        {QStringLiteral("mName"),         QStringLiteral("middle_name")},
        {QStringLiteral("medicalPolicy"), QStringLiteral("medical_policy")}};

    for (const auto &column : patientColumns) {
        if (!renameColumn(QStringLiteral("patients"), column.first, column.second))
            return fail();
    }

    emit migrationProgress(4, 8, tr("4.1.0: tabela și coloanele pacienților au fost redenumite."));

    for (const QString &table : {QStringLiteral("orderEcho"), QStringLiteral("reportEcho")}) {
        const QString oldColumn = columnExists(table, QStringLiteral("id_pacients"))
            ? QStringLiteral("id_pacients") : QStringLiteral("id_patients");

        if (!renameColumn(table, oldColumn, QStringLiteral("patient_id")))
            return fail();
    }
    if (tableExists(QStringLiteral("imagesReports"))) {
        const QString oldColumn = columnExists(QStringLiteral("imagesReports"), QStringLiteral("id_pacients"))
            ? QStringLiteral("id_pacients") : QStringLiteral("id_patients");

        if (!renameColumn(QStringLiteral("imagesReports"), oldColumn, QStringLiteral("patient_id")))
            return fail();
    }

    emit migrationProgress(5, 8, tr("4.1.0: referințele documentelor au fost redenumite."));

    if (!execSql(QStringLiteral("DROP TABLE IF EXISTS fullNamePacients"),
                 QStringLiteral("eliminare cache nume pacienți")))
        return fail();

    if (MainDatabaseConnectionContext::instance().isMariaDb()) {

        const auto constraintExists = [&](const QString &name) {
            QSqlQuery check(currentDb);
            check.prepare(QStringLiteral(R"(
                SELECT 1
                FROM information_schema.TABLE_CONSTRAINTS
                WHERE
                    CONSTRAINT_SCHEMA=DATABASE() AND
                    CONSTRAINT_NAME=? LIMIT 1
            )"));
            check.addBindValue(name);
            return check.exec() && check.next();
        };

        const QStringList foreignKeySql = {
            QStringLiteral(R"(
                ALTER TABLE
                    orderEcho
                ADD CONSTRAINT
                    fk_orderEcho_patient
                FOREIGN KEY (patient_id)
                REFERENCES patients(id)
                    ON DELETE RESTRICT
                    ON UPDATE RESTRICT
            )"), QStringLiteral(R"(
                ALTER TABLE
                    reportEcho
                ADD CONSTRAINT fk_reportEcho_patient
                FOREIGN KEY (patient_id)
                REFERENCES patients(id)
                    ON DELETE RESTRICT
                    ON UPDATE RESTRICT
            )")};

        const QStringList foreignKeyNames = {
            QStringLiteral("fk_orderEcho_patient"),
            QStringLiteral("fk_reportEcho_patient")
        };

        for (qsizetype i = 0; i < foreignKeySql.size(); ++i) {
            if (!constraintExists(foreignKeyNames.at(i))
                && !execSql(foreignKeySql.at(i), QStringLiteral("recreare FK pacient")))
                return false;
        }

        if (tableExists(QStringLiteral("imagesReports"))
            && !constraintExists(QStringLiteral("fk_imagesReports_patient"))
            && !execSql(QStringLiteral(R"(
                ALTER TABLE
                    imagesReports
                ADD CONSTRAINT fk_imagesReports_patient
                FOREIGN KEY (patient_id)
                REFERENCES patients(id)
                    ON DELETE RESTRICT
                    ON UPDATE RESTRICT)"),
                QStringLiteral("recreare FK imagini-pacient")))
            return false;
    }

    if (MainDatabaseConnectionContext::instance().isMariaDb()
        && (!execSql(QStringLiteral("DROP INDEX IF EXISTS idx_patients_name ON patients"),
                     QStringLiteral("eliminare index nume existent"))
            || !execSql(QStringLiteral("DROP INDEX IF EXISTS idx_patients_idnp ON patients"),
                        QStringLiteral("eliminare index IDNP existent"))))
        return false;

    if (!execSql(MainDatabaseConnectionContext::instance().isSqlite()
                     ? QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patients_name ON patients(last_name, first_name)")
                     : QStringLiteral("CREATE INDEX idx_patients_name ON patients(last_name, first_name)"),
                 QStringLiteral("creare index nume pacienți"))
        || !execSql(MainDatabaseConnectionContext::instance().isSqlite()
                        ? QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patients_idnp ON patients(idnp)")
                        : QStringLiteral("CREATE INDEX idx_patients_idnp ON patients(idnp)"),
                    QStringLiteral("creare index IDNP pacienți")))
        return fail();

    if (!recreatePatientView())
        return fail();

    emit migrationProgress(7, 8, tr("4.1.0: indexurile și view-ul pacienților au fost create."));

    if (rowCount(QStringLiteral("patients")) != patientsBefore
        || rowCount(QStringLiteral("orderEcho")) != ordersBefore
        || rowCount(QStringLiteral("reportEcho")) != reportsBefore
        || !validatePostMigration(QVersionNumber(4, 1, 0))) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: valorile de control nu coincid.";
        return fail();
    }

    if (transactional && !currentDb.commit()) {
        qCritical(logCritical()) << "Actualizarea la 4.1.0: commit-ul SQLite a eșuat:"
                                 << currentDb.lastError().text();
        currentDb.rollback();
        return false;
    }

    emit migrationProgress(8, 8, tr("Migrarea pacienților la versiunea 4.1.0 s-a finalizat."));

    qInfo(logInfo()) << "Actualizarea schemei pacienților pentru versiunea 4.1.0 s-a finalizat.";

    return true;
}


bool UpdateReleasesApp::update_4_1_2()
{
    QSqlDatabase currentDb = db->getDatabase();
    if (!currentDb.isOpen()
        || !currentDb.tables(QSql::Tables).contains(QStringLiteral("organizations"), Qt::CaseInsensitive)
        || !currentDb.tables(QSql::Tables).contains(QStringLiteral("constants"), Qt::CaseInsensitive)
        || !currentDb.tables(QSql::Tables).contains(QStringLiteral("userPreferences"), Qt::CaseInsensitive)) {
        qCritical(logCritical())
            << "Migrarea 4.1.2: una dintre tabelele organizations, constants sau userPreferences lipsește.";
        return false;
    }

    const bool sqlite = currentDb.driverName() == QStringLiteral("QSQLITE");

    const QStringList settingsTables = {
        QStringLiteral("userPreferences"),
        QStringLiteral("constants")
    };

    // Verificăm toate precondițiile înaintea primei modificări de schemă.
    // MariaDB face implicit COMMIT pentru DDL, deci această ordine evită pe cât
    // posibil o migrare aplicată doar parțial.
    for (const QString &tableName : settingsTables) {
        QSqlQuery duplicateQuery(currentDb);
        duplicateQuery.prepare(QStringLiteral(
            "SELECT id_users, COUNT(*) FROM `%1` "
            "GROUP BY id_users HAVING COUNT(*) > 1 LIMIT 1").arg(tableName));

        if (!duplicateQuery.exec()) {
            qCritical(logCritical()) << "Migrarea 4.1.2: verificarea duplicatelor din"
                                     << tableName << "a eșuat:"
                                     << duplicateQuery.lastError().text();
            return false;
        }

        if (duplicateQuery.next()) {
            qCritical(logCritical())
                << QStringLiteral("Migrarea 4.1.2: tabela %1 conține %2 rânduri pentru "
                                  "id_users=%3. Datele trebuie verificate înainte de "
                                  "crearea indexului unic.")
                       .arg(tableName)
                       .arg(duplicateQuery.value(1).toInt())
                       .arg(duplicateQuery.value(0).toInt());
            return false;
        }
    }

    auto hasUniqueUserIndex = [&](const QString &tableName, bool *queryOk) -> bool {
        *queryOk = false;
        if (sqlite) {
            QSqlQuery tableQuery(currentDb);
            if (!tableQuery.exec(QStringLiteral("PRAGMA table_info(`%1`)").arg(tableName))) {
                qCritical(logCritical()) << "Migrarea 4.1.2: citirea cheii primare SQLite a eșuat:"
                                         << tableQuery.lastError().text();
                return false;
            }

            QStringList primaryKeyColumns;
            while (tableQuery.next()) {
                if (tableQuery.value(5).toInt() > 0)
                    primaryKeyColumns.append(tableQuery.value(1).toString());
            }
            if (primaryKeyColumns.size() == 1
                && primaryKeyColumns.constFirst().compare(QStringLiteral("id_users"),
                                                          Qt::CaseInsensitive) == 0) {
                *queryOk = true;
                return true;
            }

            QSqlQuery indexQuery(currentDb);
            if (!indexQuery.exec(QStringLiteral("PRAGMA index_list(`%1`)").arg(tableName))) {
                qCritical(logCritical()) << "Migrarea 4.1.2: citirea indexurilor SQLite a eșuat:"
                                         << indexQuery.lastError().text();
                return false;
            }

            while (indexQuery.next()) {
                if (indexQuery.value(2).toInt() == 0)
                    continue;

                QString indexName = indexQuery.value(1).toString();
                indexName.replace(QLatin1Char('`'), QStringLiteral("``"));
                QSqlQuery columnsQuery(currentDb);
                if (!columnsQuery.exec(QStringLiteral("PRAGMA index_info(`%1`)")
                                           .arg(indexName))) {
                    qCritical(logCritical())
                        << "Migrarea 4.1.2: citirea coloanelor indexului SQLite a eșuat:"
                        << columnsQuery.lastError().text();
                    return false;
                }

                QStringList columns;
                while (columnsQuery.next())
                    columns.append(columnsQuery.value(2).toString());
                if (columns.size() == 1
                    && columns.constFirst().compare(QStringLiteral("id_users"),
                                                    Qt::CaseInsensitive) == 0) {
                    *queryOk = true;
                    return true;
                }
            }

            *queryOk = true;
            return false;
        }

        QSqlQuery indexQuery(currentDb);
        indexQuery.prepare(QStringLiteral(R"(
            SELECT index_name
            FROM information_schema.statistics
            WHERE table_schema = DATABASE()
              AND table_name = ?
              AND non_unique = 0
            GROUP BY index_name
            HAVING COUNT(*) = 1 AND LOWER(MAX(column_name)) = 'id_users'
            LIMIT 1
        )"));
        indexQuery.addBindValue(tableName);
        if (!indexQuery.exec()) {
            qCritical(logCritical()) << "Migrarea 4.1.2: citirea indexurilor MariaDB a eșuat:"
                                     << indexQuery.lastError().text();
            return false;
        }
        *queryOk = true;
        return indexQuery.next();
    };

    const bool transactional = sqlite;
    if (transactional && !currentDb.transaction()) {
        qCritical(logCritical()) << "Migrarea 4.1.2: tranzacția SQLite nu poate fi pornită:"
                                 << currentDb.lastError().text();
        return false;
    }
    auto fail = [&]() {
        if (transactional)
            currentDb.rollback();
        return false;
    };

    const bool existed = hasColumn(currentDb,
                                   QStringLiteral("organizations"),
                                   QStringLiteral("site"));

    emit migrationProgress(0, 3, tr("4.1.2: se verifică site-ul organizațiilor..."));

    const QString definition = sqlite
        ? QStringLiteral("TEXT DEFAULT NULL")
        : QStringLiteral("VARCHAR(255) DEFAULT NULL");

    if (!addColumnIfMissing(currentDb, QStringLiteral("organizations"), QStringLiteral("site"),
                            definition, QStringLiteral("4.1.2"))
        || !hasColumn(currentDb, QStringLiteral("organizations"), QStringLiteral("site")))
        return fail();

    emit migrationProgress(1, 3, existed
        ? tr("4.1.2: coloana organizations.site există deja; valorile au fost păstrate.")
        : tr("4.1.2: a fost adăugată coloana site în tabela organizations."));

    auto ensureUniqueUserIndex = [&](const QString &tableName,
                                     const QString &indexName,
                                     int progress) -> bool {
        bool queryOk = false;
        const bool indexExists = hasUniqueUserIndex(tableName, &queryOk);
        if (!queryOk)
            return false;

        if (!indexExists) {
            QSqlQuery createIndex(currentDb);
            const QString sql = QStringLiteral("CREATE UNIQUE INDEX `%1` ON `%2`(`id_users`)")
                                    .arg(indexName, tableName);
            if (!createIndex.exec(sql)) {
                qCritical(logCritical()) << "Migrarea 4.1.2: crearea indexului"
                                         << indexName << "a eșuat:"
                                         << createIndex.lastError().text();
                return false;
            }
            qInfo(logInfo()) << "Migrarea 4.1.2: index unic creat:" << indexName;
        } else {
            qInfo(logInfo()) << "Migrarea 4.1.2: indexul există deja:" << indexName;
        }

        emit migrationProgress(progress, 3,
                               tr("4.1.2: verificată unicitatea %1.id_users.").arg(tableName));
        return true;
    };

    if (!ensureUniqueUserIndex(QStringLiteral("userPreferences"),
                               QStringLiteral("uq_userPreferences_users"), 2))
        return fail();

    if (!ensureUniqueUserIndex(QStringLiteral("constants"),
                               QStringLiteral("uq_constants_users"), 3))
        return fail();

    bool userPreferencesIndexOk = false;

    bool constantsIndexOk = false;
    if (!hasUniqueUserIndex(QStringLiteral("userPreferences"), &userPreferencesIndexOk)
        || !userPreferencesIndexOk
        || !hasUniqueUserIndex(QStringLiteral("constants"), &constantsIndexOk)
        || !constantsIndexOk
        || !hasColumn(currentDb, QStringLiteral("organizations"), QStringLiteral("site"))) {
        qCritical(logCritical()) << "Migrarea 4.1.2: verificarea finală a schemei a eșuat.";
        return fail();
    }

    if (transactional && !currentDb.commit()) {
        qCritical(logCritical()) << "Migrarea 4.1.2: commit-ul SQLite a eșuat:"
                                 << currentDb.lastError().text();
        currentDb.rollback();
        return false;
    }

    return true;
}

bool UpdateReleasesApp::update_4_2_0()
{
    QSqlDatabase currentDb = db->getDatabase();
    const QStringList tables = currentDb.tables(QSql::Tables);
    if (!currentDb.isOpen()
        || !tables.contains(QStringLiteral("userPreferences"), Qt::CaseInsensitive)
        || !tables.contains(QStringLiteral("constants"), Qt::CaseInsensitive)) {
        qCritical(logCritical())
            << "Migrarea 4.2.0: tabelele userPreferences sau constants lipsesc.";
        return false;
    }

    const bool sqlite = currentDb.driverName() == QStringLiteral("QSQLITE");
    const QString sqlRoot = sqlite ? QStringLiteral(":/sql/sqlite/tables/")
                                   : QStringLiteral(":/sql/mariadb/tables/");

    if (tables.contains(QStringLiteral("cloudServer"), Qt::CaseInsensitive)) {
        QSqlQuery duplicateCheck(currentDb);
        if (!duplicateCheck.exec(QStringLiteral(R"(
                SELECT
                    id_organizations,
                    id_users,
                    COUNT(*)
                FROM
                    cloudServer
                GROUP BY
                    id_organizations,
                    id_users
                HAVING
                    COUNT(*) > 1
                LIMIT 1
            )"))) {
            qCritical(logCritical())
                << "Migrarea 4.2.0: verificarea configurațiilor cloud duplicate a eșuat:"
                << duplicateCheck.lastError().text();
            return false;
        }
        if (duplicateCheck.next()) {
            qCritical(logCritical())
                << "Migrarea 4.2.0: există configurații cloud duplicate pentru organizația"
                << duplicateCheck.value(0).toInt() << "și utilizatorul"
                << duplicateCheck.value(1).toInt()
                << ". Eliminați duplicatul înainte de actualizare.";
            return false;
        }

        bool hasOrganizationUserUnique = false;
        if (sqlite) {
            QSqlQuery indexes(currentDb);
            if (!indexes.exec(QStringLiteral("PRAGMA index_list(cloudServer)"))) {
                qCritical(logCritical())
                    << "Migrarea 4.2.0: citirea indexurilor cloudServer a eșuat:"
                    << indexes.lastError().text();
                return false;
            }
            while (indexes.next() && !hasOrganizationUserUnique) {
                if (indexes.value(QStringLiteral("unique")).toInt() != 1)
                    continue;
                QSqlQuery columns(currentDb);
                if (!columns.exec(QStringLiteral("PRAGMA index_info(%1)")
                                      .arg(indexes.value(QStringLiteral("name")).toString()))) {
                    qCritical(logCritical())
                        << "Migrarea 4.2.0: citirea coloanelor indexului cloudServer a eșuat:"
                        << columns.lastError().text();
                    return false;
                }
                QStringList names;
                while (columns.next())
                    names.append(columns.value(QStringLiteral("name")).toString());
                hasOrganizationUserUnique = names == QStringList{
                    QStringLiteral("id_organizations"), QStringLiteral("id_users")};
            }
        } else {
            QSqlQuery uniqueIndex(currentDb);
            uniqueIndex.prepare(QStringLiteral(R"(
                SELECT
                    INDEX_NAME
                FROM
                    information_schema.STATISTICS
                WHERE
                    TABLE_SCHEMA = DATABASE() AND
                    TABLE_NAME = 'cloudServer' AND
                    NON_UNIQUE = 0
                GROUP BY
                    INDEX_NAME
                HAVING
                    COUNT(*) = 2 AND
                    COUNT(DISTINCT CASE
                        WHEN COLUMN_NAME IN ('id_organizations', 'id_users')
                        THEN COLUMN_NAME
                    END) = 2
                LIMIT 1
            )"));
            if (!uniqueIndex.exec()) {
                qCritical(logCritical())
                    << "Migrarea 4.2.0: verificarea indexului unic cloudServer a eșuat:"
                    << uniqueIndex.lastError().text();
                return false;
            }
            hasOrganizationUserUnique = uniqueIndex.next();
        }

        if (!hasOrganizationUserUnique) {
            QSqlQuery createUnique(currentDb);
            const QString createUniqueSql = sqlite
                ? QStringLiteral(R"(
                    CREATE UNIQUE INDEX uq_cloudServer_organization_user
                    ON cloudServer(id_organizations, id_users)
                )")
                : QStringLiteral(R"(
                    ALTER TABLE cloudServer
                    ADD UNIQUE KEY uq_cloudServer_organization_user
                        (id_organizations, id_users)
                )");
            if (!createUnique.exec(createUniqueSql)) {
                qCritical(logCritical())
                    << "Migrarea 4.2.0: crearea indexului unic cloudServer a eșuat:"
                    << createUnique.lastError().text();
                return false;
            }
        }
    }

    emit migrationProgress(0, 6, tr("4.2.0: se creează noua schemă de setări..."));
    const struct {
        const char *file;
        const char *table;
    } newSettingsTables[] = {
        {"application_settings.sql", "applicationSettings"},
        {"organization_settings.sql", "organizationSettings"},
        {"user_settings.sql", "userSettings"}
    };

    QMap<QString, QString> parentIdTypes;
    if (!sqlite) {
        const QRegularExpression integerType(
            QStringLiteral(R"(^(?:tinyint|smallint|mediumint|int|integer|bigint)(?:\(\d+\))?(?: unsigned)?$)"),
            QRegularExpression::CaseInsensitiveOption);
        for (const QString &parentTable : {QStringLiteral("users"),
                                           QStringLiteral("organizations"),
                                           QStringLiteral("doctors"),
                                           QStringLiteral("nurses")}) {
            QSqlQuery typeQuery(currentDb);
            typeQuery.prepare(QStringLiteral(R"(
                SELECT COLUMN_TYPE
                FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_SCHEMA = DATABASE()
                  AND TABLE_NAME = ?
                  AND COLUMN_NAME = 'id'
            )"));
            typeQuery.addBindValue(parentTable);
            if (!typeQuery.exec() || !typeQuery.next()
                || !integerType.match(typeQuery.value(0).toString().trimmed()).hasMatch()) {
                qCritical(logCritical())
                    << "Migrarea 4.2.0: tipul cheii părinte nu poate fi determinat:"
                    << parentTable << typeQuery.lastError().text();
                return false;
            }
            parentIdTypes.insert(parentTable, typeQuery.value(0).toString().trimmed());
        }
    }

    for (const auto &entry : newSettingsTables) {
        const QString resourcePath = sqlRoot + QString::fromLatin1(entry.file);
        bool created = false;
        if (sqlite) {
            created = db_common.execFileBatch(currentDb, resourcePath,
                                              QString::fromLatin1(entry.table));
        } else {
            QString createSql = DataBaseCommon::getTextQryFromResource(resourcePath);
            const QMap<QString, QString> columnParents =
                QString::fromLatin1(entry.table) == QStringLiteral("applicationSettings")
                    ? QMap<QString, QString>{{QStringLiteral("user_id"), QStringLiteral("users")}}
                : QString::fromLatin1(entry.table) == QStringLiteral("organizationSettings")
                    ? QMap<QString, QString>{{QStringLiteral("organization_id"), QStringLiteral("organizations")},
                                             {QStringLiteral("default_doctor_id"), QStringLiteral("doctors")},
                                             {QStringLiteral("default_nurse_id"), QStringLiteral("nurses")}}
                    : QMap<QString, QString>{{QStringLiteral("user_id"), QStringLiteral("users")},
                                             {QStringLiteral("default_organization_id"), QStringLiteral("organizations")}};
            if (createSql.isEmpty()) {
                qCritical(logCritical()) << "Migrarea 4.2.0: scriptul SQL lipsește:"
                                         << resourcePath;
                return false;
            }
            for (auto it = columnParents.cbegin(); it != columnParents.cend(); ++it) {
                const QRegularExpression columnDefinition(
                    QStringLiteral(R"((`%1`\s+)BIGINT UNSIGNED\b)")
                        .arg(QRegularExpression::escape(it.key())));
                if (!createSql.contains(columnDefinition)) {
                    qCritical(logCritical())
                        << "Migrarea 4.2.0: definiția coloanei lipsește în script:"
                        << entry.table << it.key();
                    return false;
                }
                createSql.replace(columnDefinition,
                                  QStringLiteral("\\1") + parentIdTypes.value(it.value()));
            }
            QSqlQuery createQuery(currentDb);
            created = createQuery.exec(createSql);
            if (!created)
                qCritical(logCritical()) << "Migrarea 4.2.0: crearea tabelei a eșuat:"
                                         << entry.table << createQuery.lastError().text();
        }
        if (!created) {
            qCritical(logCritical())
                << "Migrarea 4.2.0: nu poate crea/verifica tabela"
                << entry.table;
            return false;
        }
    }
    if (!ensureCryptoSplitKeySchema())
        return false;
    emit migrationProgress(1, 6, tr("4.2.0: noua schemă de setări este pregătită."));

    if (!currentDb.transaction()) {
        qCritical(logCritical())
            << "Migrarea 4.2.0: tranzacția de copiere nu poate fi pornită:"
            << currentDb.lastError().text();
        return false;
    }

    const QString insertPrefix = sqlite ? QStringLiteral("INSERT OR IGNORE")
                                        : QStringLiteral("INSERT IGNORE");

    const QString refreshExpression = sqlite
        ? QStringLiteral(
              "CASE WHEN CAST(COALESCE(updateListDoc, 0) AS INTEGER) "
              "BETWEEN 0 AND 86400 "
              "THEN CAST(COALESCE(updateListDoc, 0) AS INTEGER) ELSE 0 END")
        : QStringLiteral(
              "CASE WHEN CAST(COALESCE(updateListDoc, 0) AS SIGNED) "
              "BETWEEN 0 AND 86400 "
              "THEN CAST(COALESCE(updateListDoc, 0) AS SIGNED) ELSE 0 END");

    QSqlQuery query(currentDb);
    const QString copyApplication = QStringLiteral(R"(
        %1 INTO applicationSettings (
            user_id,
            check_for_updates_on_startup,
            show_user_manual_on_startup,
            show_assistant_on_startup,
            document_journal_refresh_interval_seconds
        )
        SELECT
            id_users,
            CASE WHEN COALESCE(checkNewVersionApp, 1) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(showUserManual, 0) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(showAsistantHelper, 1) <> 0 THEN 1 ELSE 0 END,
            %2
        FROM userPreferences
    )").arg(insertPrefix, refreshExpression);
    if (!query.exec(copyApplication)) {
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: copierea setărilor aplicației a eșuat:"
            << query.lastError().text();
        return false;
    }
    emit migrationProgress(2, 6, tr("4.2.0: setările aplicației au fost copiate."));

    const QString copyUser = QStringLiteral(R"(
        %1 INTO userSettings (
            user_id,
            default_organization_id,
            minimize_to_tray,
            confirm_on_exit,
            archive_sqlite_on_exit,
            open_documents_in_separate_windows,
            print_menu_mode
        )
        SELECT
            preferences.id_users,
            constants.id_organizations,
            CASE WHEN COALESCE(preferences.minimizeAppToTray, 0) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(preferences.showQuestionCloseApp, 1) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(preferences.databasesArchiving, 0) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(preferences.showDocumentsInSeparatWindow, 0) <> 0 THEN 1 ELSE 0 END,
            CASE WHEN COALESCE(preferences.showDesignerMenuPrint, 0) <> 0 THEN 1 ELSE 0 END
        FROM userPreferences AS preferences
        LEFT JOIN constants ON constants.id_users = preferences.id_users
    )").arg(insertPrefix);
    if (!query.exec(copyUser)) {
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: copierea setărilor utilizatorilor a eșuat:"
            << query.lastError().text();
        return false;
    }
    emit migrationProgress(3, 6, tr("4.2.0: setările utilizatorilor au fost copiate."));

    QSqlQuery sourceOrganizations(currentDb);
    if (!sourceOrganizations.exec(QStringLiteral(R"(
            SELECT id_users, id_organizations, id_doctors, id_nurses, brandUSG, logo
            FROM constants
            ORDER BY id_users
        )"))) {
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: citirea setărilor organizațiilor a eșuat:"
            << sourceOrganizations.lastError().text();
        return false;
    }

    while (sourceOrganizations.next()) {
        if (sourceOrganizations.value(1).isNull())
            continue;

        const int userId = sourceOrganizations.value(0).toInt();
        const int organizationId = sourceOrganizations.value(1).toInt();
        QSqlQuery existing(currentDb);
        existing.prepare(QStringLiteral(R"(
            SELECT default_doctor_id, default_nurse_id,
                   ultrasound_device_name, logo
            FROM organizationSettings
            WHERE organization_id = ?
        )"));
        existing.addBindValue(organizationId);
        if (!existing.exec()) {
            currentDb.rollback();
            qCritical(logCritical())
                << "Migrarea 4.2.0: verificarea organizationSettings a eșuat:"
                << existing.lastError().text();
            return false;
        }

        if (!existing.next()) {
            QSqlQuery insert(currentDb);
            insert.prepare(QStringLiteral(R"(
                INSERT INTO organizationSettings (
                    organization_id, default_doctor_id, default_nurse_id,
                    ultrasound_device_name, logo
                ) VALUES (?, ?, ?, ?, ?)
            )"));
            for (int column = 1; column <= 5; ++column)
                insert.addBindValue(sourceOrganizations.value(column));
            if (!insert.exec()) {
                currentDb.rollback();
                qCritical(logCritical())
                    << "Migrarea 4.2.0: copierea organizației"
                    << organizationId << "a eșuat:" << insert.lastError().text();
                return false;
            }
            continue;
        }

        const bool sameDoctor = existing.value(0).toLongLong()
                                == sourceOrganizations.value(2).toLongLong()
                                && existing.value(0).isNull()
                                   == sourceOrganizations.value(2).isNull();
        const bool sameNurse = existing.value(1).toLongLong()
                               == sourceOrganizations.value(3).toLongLong()
                               && existing.value(1).isNull()
                                  == sourceOrganizations.value(3).isNull();
        const bool sameDevice = existing.value(2).toString()
                                == sourceOrganizations.value(4).toString();
        const bool sameLogo = existing.value(3).toByteArray()
                              == sourceOrganizations.value(5).toByteArray();
        if (!sameDoctor || !sameNurse || !sameDevice || !sameLogo) {
            currentDb.rollback();
            qCritical(logCritical())
                << "Migrarea 4.2.0: date organizaționale diferite pentru aceeași "
                   "organizație. organization_id="
                << organizationId << "user_id=" << userId;
            return false;
        }
    }
    emit migrationProgress(4, 6, tr("4.2.0: setările organizațiilor au fost copiate."));

    const QString verifyApplication = QStringLiteral(R"(
        SELECT COUNT(*)
        FROM userPreferences AS legacy
        LEFT JOIN applicationSettings AS current
               ON current.user_id = legacy.id_users
        WHERE current.user_id IS NULL
           OR current.check_for_updates_on_startup <>
              CASE WHEN COALESCE(legacy.checkNewVersionApp, 1) <> 0 THEN 1 ELSE 0 END
           OR current.show_user_manual_on_startup <>
              CASE WHEN COALESCE(legacy.showUserManual, 0) <> 0 THEN 1 ELSE 0 END
           OR current.show_assistant_on_startup <>
              CASE WHEN COALESCE(legacy.showAsistantHelper, 1) <> 0 THEN 1 ELSE 0 END
           OR current.document_journal_refresh_interval_seconds <>
              %1
    )").arg(refreshExpression);
    if (!query.exec(verifyApplication) || !query.next()
        || query.value(0).toInt() != 0) {
        const QString error = query.lastError().text();
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: verificarea applicationSettings a eșuat:"
            << error;
        return false;
    }

    const QString verifyUser = QStringLiteral(R"(
        SELECT COUNT(*)
        FROM userPreferences AS legacy
        LEFT JOIN constants ON constants.id_users = legacy.id_users
        LEFT JOIN userSettings AS current ON current.user_id = legacy.id_users
        WHERE current.user_id IS NULL
           OR COALESCE(current.default_organization_id, -1) <>
              COALESCE(constants.id_organizations, -1)
           OR current.minimize_to_tray <>
              CASE WHEN COALESCE(legacy.minimizeAppToTray, 0) <> 0 THEN 1 ELSE 0 END
           OR current.confirm_on_exit <>
              CASE WHEN COALESCE(legacy.showQuestionCloseApp, 1) <> 0 THEN 1 ELSE 0 END
           OR current.archive_sqlite_on_exit <>
              CASE WHEN COALESCE(legacy.databasesArchiving, 0) <> 0 THEN 1 ELSE 0 END
           OR current.open_documents_in_separate_windows <>
              CASE WHEN COALESCE(legacy.showDocumentsInSeparatWindow, 0) <> 0 THEN 1 ELSE 0 END
           OR current.print_menu_mode <>
              CASE WHEN COALESCE(legacy.showDesignerMenuPrint, 0) <> 0 THEN 1 ELSE 0 END
    )");
    if (!query.exec(verifyUser) || !query.next() || query.value(0).toInt() != 0) {
        const QString error = query.lastError().text();
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: verificarea userSettings a eșuat:" << error;
        return false;
    }

    if (!currentDb.commit()) {
        const QString error = currentDb.lastError().text();
        currentDb.rollback();
        qCritical(logCritical())
            << "Migrarea 4.2.0: confirmarea copierii a eșuat:" << error;
        return false;
    }
    emit migrationProgress(5, 6, tr("4.2.0: datele migrate au fost verificate."));

    if (hasColumn(currentDb, QStringLiteral("userPreferences"),
                  QStringLiteral("order_splitFullName"))) {
        if (!query.exec(QStringLiteral(
                "ALTER TABLE userPreferences DROP COLUMN order_splitFullName"))) {
            qCritical(logCritical())
                << "Migrarea 4.2.0: eliminarea coloanei "
                   "userPreferences.order_splitFullName a eșuat:"
                << query.lastError().text();
            return false;
        }
    }

    if (!ensureReportVideoSchema()) {
        qCritical(logCritical())
            << "Migrarea 4.2.0: schema reportVideo nu a putut fi verificată.";
        return false;
    }

    qInfo(logInfo())
        << "Migrarea 4.2.0: datele din constants și userPreferences au fost "
           "copiate și verificate în noua schemă de setări.";

    emit migrationProgress(6, 6, tr("4.2.0: noua schemă de setări este pregătită."));

    return true;
}
