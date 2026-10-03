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

#include "dataconstantsworker.h"

DataConstantsWorker::DataConstantsWorker(const GeneralData &data, QObject *parent)
    : QObject{parent}, m_data(data)
{

}

void DataConstantsWorker::process()
{
    const QString connName = QStringLiteral("connection_%1")
                                .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    bool success = true;
    CloudConnectionData cloudConnection;
    int organizationId  = m_data.id_organization;
    int defaultDoctorId = m_data.id_doctor;
    int defaultNurseId  = -1;

    QString ultrasoundDeviceName;
    QByteArray logoData;
    OrganizationContextData organizationData;
    DoctorContextData doctorData;

    // Provider local: worker-ul nu depinde de durata de viață a dialogului
    // de autorizare care l-a pornit.
    DatabaseProvider provider;

    { // Conexiunea trăieşte DOAR în acest bloc

        QSqlDatabase dbConn = provider.getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                               .arg(this->metaObject()->className())
                        << dbConn.lastError().text();
            success = false;
        } else {
            // Setările organizației. Schema 4.1.x rămâne fallback numai până
            // la finalizarea migrării tuturor bazelor la 4.2.0.
            QSqlQuery qry(dbConn);
            const QStringList tables = dbConn.tables(QSql::Tables);
            const bool newSettingsSchema = tables.contains(QStringLiteral("userSettings"), Qt::CaseInsensitive) &&
                                           tables.contains(QStringLiteral("organizationSettings"), Qt::CaseInsensitive);

            qry.prepare(newSettingsSchema
                ? QStringLiteral(R"(
                    SELECT
                        user_settings.default_organization_id AS id_organizations,
                        organization.default_doctor_id AS id_doctors,
                        organization.default_nurse_id AS id_nurses,
                        organization.ultrasound_device_name AS brandUSG,
                        organization.logo AS logo
                    FROM userSettings AS user_settings
                    LEFT JOIN organizationSettings AS organization
                           ON organization.organization_id =
                              user_settings.default_organization_id
                    WHERE user_settings.user_id = ?
                )")
                : QStringLiteral("SELECT * FROM constants WHERE id_users = ?"));
            qry.addBindValue(m_data.id_user);
            if (! qry.exec()) {
                success = false;
                qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Eroare exec SELECT(constants):")
                                                .arg(this->metaObject()->className())
                                         << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    organizationId       = qry.value(rec.indexOf("id_organizations")).toInt();
                    defaultDoctorId      = qry.value(rec.indexOf("id_doctors")).toInt();
                    defaultNurseId       = qry.value(rec.indexOf("id_nurses")).toInt();
                    ultrasoundDeviceName = qry.value(rec.indexOf("brandUSG")).toString();
                    logoData = QByteArray::fromBase64(qry.value(rec.indexOf("logo")).toString().toUtf8());

                    qInfo(logInfo())
                        << "[THREAD] Actualizate datele organizației din"
                        << (newSettingsSchema ? "noua schemă de setări"
                                              : "schema legacy constants");
                }
            }

            // datele organizatiei implicite
            qry.prepare(R"(
                SELECT
                    *
                FROM
                    organizations
                WHERE
                    id = ?
            )");
            if (m_data.id_organization == -1)
                qry.addBindValue(organizationId);
            else
                qry.addBindValue(m_data.id_organization);
            if (! qry.exec()) {
                success = false;
                qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Eroare exec SELECT(organizations):")
                                                .arg(this->metaObject()->className())
                                         << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    organizationData.name    = qry.value(rec.indexOf("name")).toString();
                    organizationData.address = qry.value(rec.indexOf("address")).toString();
                    organizationData.phone   = qry.value(rec.indexOf("telephone")).toString();
                    organizationData.email   = qry.value(rec.indexOf("email")).toString();
                    organizationData.site    = rec.contains("site")
                                                ? qry.value(rec.indexOf("site")).toString()
                                                : QString();
                    organizationData.stampData = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());

                    qInfo(logInfo()) << "[THREAD] Actualizat contextul organizatiei implicite";
                }
            }

            // datele doctorului
            qry.prepare(R"(
                SELECT
                    doctors.id,
                    doctors.signature,
                    doctors.stamp,
                    fullNameDoctors.name AS fullName,
                    fullNameDoctors.nameAbbreviated
                FROM
                    doctors
                INNER JOIN
                    fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
                WHERE
                    doctors.deletionMark = 0 AND
                    doctors.id = ?
            )");
            if (m_data.id_doctor == -1)
                qry.addBindValue(defaultDoctorId);
            else
                qry.addBindValue(m_data.id_doctor);
            if (! qry.exec()) {
                success = false;
                qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Eroare exec SELECT(doctors):")
                                                .arg(this->metaObject()->className())
                                         << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    doctorData.fullName        = qry.value(rec.indexOf("fullName")).toString();
                    doctorData.abbreviatedName = qry.value(rec.indexOf("nameAbbreviated")).toString();
                    doctorData.stampData       = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());
                    doctorData.signatureData   = QByteArray::fromBase64(qry.value(rec.indexOf("signature")).toString().toUtf8());

                    qInfo(logInfo()) << "[THREAD] Actualizat contextul doctorului implicit";
                }
            }

            // datele conectarii la serverul pu syncronizare
            qry.prepare(R"(
                SELECT
                    cloudServer.*,
                    users.hash AS hashUser
                FROM
                    cloudServer
                INNER JOIN
                    users ON cloudServer.id_users = users.id
                WHERE
                    cloudServer.id_organizations = ? AND
                    cloudServer.id_users = ?
                )");
            qry.addBindValue(organizationId);
            qry.addBindValue(m_data.id_user);
            if (! qry.exec()) {
                success = false;
                qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Eroare exec SELECT(cloudServer):")
                                                .arg(this->metaObject()->className())
                                         << qry.lastError().text();
            } else {
                while (qry.next()) {
                    cloudConnection.configured = true;
                    QSqlRecord rec = qry.record();
                    cloudConnection.hostName          = qry.value(rec.indexOf("hostName")).toString();
                    cloudConnection.databaseName      = qry.value(rec.indexOf("databaseName")).toString();
                    cloudConnection.port              = qry.value(rec.indexOf("port")).toInt();
                    cloudConnection.connectionOptions = qry.value(rec.indexOf("connectionOption")).toString();
                    cloudConnection.userName          = qry.value(rec.indexOf("username")).toString();

                    // Până la migrarea 4.2.7 parola poate fi încă criptată cu
                    // cheia veche, derivată din hash-ul utilizatorului.
                    const int cloudOrganizationId = qry.value(rec.indexOf("id_organizations")).toInt();
                    const QString storedPassword = qry.value(rec.indexOf("password")).toString();
                    QString password;
                    QString decryptError;
                    const bool decrypted = CryptoManager::decryptCloudPassword(
                        dbConn,
                        cloudOrganizationId,
                        storedPassword,
                        qry.value(rec.indexOf("iv")).toString(),
                        qry.value(rec.indexOf("hashUser")).toString(),
                        &password,
                        nullptr,
                        &decryptError);
                    cloudConnection.password = decrypted ? password : QString();
                    cloudConnection.enabled = decrypted;
                    cloudConnection.passwordUnreadable =
                        !decrypted && CryptoManager::fromBase64(storedPassword).size() > 16;
                    if (!decrypted) {
                        qWarning(logWarning()) << "[THREAD] Parola cloud nu a putut fi decriptată:"
                                               << decryptError;
                    } else {
                        qInfo(logInfo()) << "[THREAD] Actualizat contextul conexiunei cloud.";
                    }
                }
            }

            dbConn.close();
        }
    } // <- destructor QSqlDatabase

    provider.removeDatabaseThread(connName);

    emit finished(success,
                  cloudConnection,
                  organizationId,
                  defaultDoctorId,
                  defaultNurseId,
                  ultrasoundDeviceName,
                  logoData,
                  organizationData,
                  doctorData);
}
