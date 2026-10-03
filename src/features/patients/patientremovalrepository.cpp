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

#include "patientremovalrepository.h"

#include <QSqlError>
#include <QSqlQuery>

#include "common/maindatabaseconnectioncontext.h"
#include "core/loggingcategories.h"
#include "database/database.h"

namespace {

QDateTime toDateTime(const QVariant &value)
{
    // MariaDB întoarce DATETIME/DATE, SQLite – text ISO.
    QDateTime dateTime = value.toDateTime();
    if (dateTime.isValid())
        return dateTime;

    const QString text = value.toString().trimmed();
    dateTime = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (!dateTime.isValid())
        dateTime = QDateTime::fromString(text, Qt::ISODate);
    if (!dateTime.isValid())
        dateTime = QDate::fromString(text, QStringLiteral("yyyy-MM-dd")).startOfDay();
    return dateTime;
}

}

PatientRemovalRepository::PatientRemovalRepository(DataBase &db)
    : m_db(db)
{}

bool PatientRemovalRepository::findReferences(qint64 patientId,
                                              QList<Reference> *references,
                                              QString *error) const
{
    if (error)
        error->clear();
    if (!references)
        return false;
    references->clear();

    QSqlQuery query(m_db.getDatabase());
    query.setForwardOnly(true);
    if (!query.prepare(m_db.getTextSQL(QStringLiteral(":/sql/queries/patient_references.sql")))) {
        if (error)
            *error = query.lastError().text();
        qWarning(logWarning()) << "PatientRemovalRepository: prepare referințe:"
                               << query.lastError().text();
        return false;
    }
    for (int i = 0; i < 3; ++i)
        query.addBindValue(patientId);

    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        qWarning(logWarning()) << "PatientRemovalRepository: citirea referințelor a eșuat:"
                               << query.lastError().text();
        return false;
    }

    while (query.next()) {
        Reference reference;
        reference.kind      = static_cast<Reference::Kind>(query.value(0).toInt());
        reference.numberDoc = query.value(1).toString().trimmed();
        reference.dateDoc   = toDateTime(query.value(2));
        references->append(reference);
    }
    return true;
}

PatientRemovalRepository::RemoveResult
PatientRemovalRepository::removePatient(qint64 patientId, QString *error) const
{
    if (error)
        error->clear();

    QSqlQuery query(m_db.getDatabase());
    if (!query.prepare(QStringLiteral(R"(
            DELETE FROM patients
            WHERE
                id = ?
                AND NOT EXISTS (SELECT 1 FROM orderEcho WHERE patient_id = ?)
                AND NOT EXISTS (SELECT 1 FROM reportEcho WHERE patient_id = ?)
                AND NOT EXISTS (SELECT 1 FROM patientAppointments WHERE patient_id = ?)
        )"))) {
        if (error)
            *error = query.lastError().text();
        qCritical(logCritical()) << "PatientRemovalRepository: prepare eliminare:"
                                 << query.lastError().text();
        return RemoveResult::Error;
    }
    for (int i = 0; i < 4; ++i)
        query.addBindValue(patientId);

    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        qCritical(logCritical()) << "PatientRemovalRepository: eliminarea pacientului"
                                 << patientId << "a eșuat:" << query.lastError().text();
        return RemoveResult::Error;
    }

    if (query.numRowsAffected() != 1) {
        // Pacientul figurează între timp într-un document sau nu mai există.
        QList<Reference> references;
        if (findReferences(patientId, &references) && !references.isEmpty())
            return RemoveResult::Referenced;
        if (error)
            *error = QStringLiteral("Pacientul cu id=%1 nu mai există în baza de date.")
                         .arg(patientId);
        return RemoveResult::Error;
    }

    qInfo(logInfo()) << "PatientRemovalRepository: pacientul" << patientId
                     << "a fost eliminat din baza de date.";

    removeOrphanImages(patientId);
    return RemoveResult::Removed;
}

void PatientRemovalRepository::removeOrphanImages(qint64 patientId) const
{
    // La SQLite imaginile sunt într-o bază separată, fără FK către patients.
    // Pacientul nu mai are documente, deci imaginile rămase sunt orfane.
    // La MariaDB FK-ul imagesReports -> patients nu permite astfel de rânduri.
    if (!MainDatabaseConnectionContext::instance().isSqlite())
        return;

    QSqlDatabase imageDatabase = m_db.getDatabaseImage();
    if (!imageDatabase.isOpen()
        || !imageDatabase.tables(QSql::Tables).contains(QStringLiteral("imagesReports"),
                                                         Qt::CaseInsensitive)) {
        return;
    }

    QSqlQuery query(imageDatabase);
    if (!query.prepare(QStringLiteral(R"(
            DELETE FROM imagesReports WHERE patient_id = ?
        )"))) {
        qWarning(logWarning()) << "PatientRemovalRepository: prepare imagini orfane:"
                               << query.lastError().text();
        return;
    }
    query.addBindValue(patientId);
    if (!query.exec()) {
        qWarning(logWarning()) << "PatientRemovalRepository: eliminarea imaginilor orfane a eșuat:"
                               << query.lastError().text();
        return;
    }
    if (query.numRowsAffected() > 0)
        qInfo(logInfo()) << "PatientRemovalRepository: eliminate" << query.numRowsAffected()
                         << "imagini orfane ale pacientului" << patientId;
}
