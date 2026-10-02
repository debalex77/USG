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

#include "reportsemailexporterworker.h"

#include <features/printing/reportprintservice.h>
#include <infrastructure/email/reportimagesexporter.h>

ReportsEmailExporterWorker::ReportsEmailExporterWorker(DataBase &database,
                                                       DatabaseProvider *provider,
                                                       const DatesReportsForExportEmail &data,
                                                       QObject *parent)
    : QObject{parent}
    , m_database(database)
    , m_provider(provider)
    , m_data(data)
{
}

void ReportsEmailExporterWorker::process()
{
    // Numele conexiunilor includ adresa obiectului: pe firul GUI poate rula
    // în paralel și exportul din OrderView (connection_<threadId>).
    const QString connName = QStringLiteral("reportsEmail_%1")
                                 .arg(reinterpret_cast<quintptr>(this));
    const QString connNameImage = QStringLiteral("reportsEmailImage_%1")
                                      .arg(reinterpret_cast<quintptr>(this));

    m_result.exportDirectory = m_data.filePDF;
    m_result.organizationId = m_data.printOrganizationId;
    m_result.requestedCount = static_cast<int>(m_data.reportIds.size());

    { // Conexiunile trăiesc DOAR în acest bloc

        QSqlDatabase dbConn = m_provider->getDatabaseThread(connName, m_data.thisMySQL);
        const bool connected = dbConn.isOpen() || dbConn.open();
        if (!connected) {
            qCritical(logCritical()) << "[ReportsEmailExporterWorker] Nu pot deschide conexiunea DB:"
                                     << dbConn.lastError().text();
            m_errors << tr("Conexiunea cu baza de date nu a putut fi deschisă: %1")
                            .arg(dbConn.lastError().text());
        }

        // Imaginile: la MariaDB sunt în baza principală, la SQLite în db_image.
        QSqlDatabase dbConnImage;
        QSqlDatabase *dbImages = nullptr;
        if (connected && m_data.includeImages) {
            if (m_data.thisMySQL) {
                dbImages = &dbConn;
            } else {
                dbConnImage = m_provider->getDatabaseImagesThread(connNameImage);
                if (dbConnImage.isOpen() || dbConnImage.open()) {
                    dbImages = &dbConnImage;
                } else {
                    m_errors << tr("Baza de date a imaginilor nu a putut fi deschisă: %1")
                                    .arg(dbConnImage.lastError().text());
                }
            }
        }

        if (connected && loadRecipient(dbConn)) {
            int index = 0;
            for (const qint64 reportId : std::as_const(m_data.reportIds)) {
                ++index;
                emit setTextInfo(tr("Se exportă raportul %1 din %2 ...")
                                     .arg(index)
                                     .arg(m_result.requestedCount));
                if (exportReport(dbConn, dbImages, reportId))
                    ++m_result.exportedCount;
            }
        }

        if (dbConnImage.isValid())
            dbConnImage.close();
        dbConn.close();

    } // <- destructori QSqlDatabase

    if (m_provider->containConnection(connNameImage))
        m_provider->removeDatabaseThread(connNameImage);
    m_provider->removeDatabaseThread(connName);

    m_result.success = m_result.exportedCount > 0;
    if (!m_errors.isEmpty())
        m_result.errorText = m_errors.join(QStringLiteral("\n"));
    else if (!m_result.success)
        m_result.errorText = tr("Exportul rapoartelor nu s-a finalizat.");

    qInfo(logInfo()) << "[ReportsEmailExporterWorker] Rapoarte exportate:"
                     << m_result.exportedCount << "din" << m_result.requestedCount
                     << "fișiere=" << m_result.attachments.size();

    emit finished(m_result);
}

bool ReportsEmailExporterWorker::loadRecipient(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    qry.prepare(m_database.getTextSQL(QStringLiteral(":/sql/queries/organizations_byID_select.sql")));
    qry.addBindValue(m_data.recipientOrganizationId);
    if (!qry.exec()) {
        qCritical(logCritical()) << "[ReportsEmailExporterWorker] Datele organizației nu pot fi citite:"
                                 << qry.lastError().text();
        m_errors << tr("Datele organizației destinatare nu au putut fi citite: %1")
                        .arg(qry.lastError().text());
        return false;
    }
    if (!qry.next()) {
        m_errors << tr("Organizația destinatară nu a fost găsită.");
        return false;
    }

    m_result.recipientName = qry.value(QStringLiteral("name")).toString();
    m_result.emailTo = qry.value(QStringLiteral("email")).toString().trimmed();
    return true;
}

QString ReportsEmailExporterWorker::uniqueFileStem(const QString &reportNumber, qint64 reportId)
{
    QString stem = ReportImagesExporter::fileSafeDocumentNumber(reportNumber.trimmed());
    if (stem.isEmpty() || m_usedFileStems.contains(stem))
        stem = QStringLiteral("%1_%2").arg(stem).arg(reportId);
    m_usedFileStems.insert(stem);
    return stem;
}

bool ReportsEmailExporterWorker::exportReport(QSqlDatabase &dbConn,
                                              QSqlDatabase *dbImages,
                                              qint64 reportId)
{
    // Datele se recitesc: raportul poate fi devalidat sau mutat între
    // selecția din dialog și export.
    QSqlQuery qry(dbConn);
    qry.prepare(m_database.getTextSQL(QStringLiteral(":/sql/queries_doc/reports_email_export_item.sql")));
    qry.bindValue(QStringLiteral(":idReport"), reportId);
    if (!qry.exec()) {
        qCritical(logCritical()) << "[ReportsEmailExporterWorker] Eroare la citirea raportului id="
                                 << reportId << qry.lastError().text();
        m_errors << tr("Raportul cu id=%1 nu a putut fi citit: %2")
                        .arg(reportId)
                        .arg(qry.lastError().text());
        return false;
    }
    if (!qry.next()) {
        m_errors << tr("Raportul cu id=%1 nu a fost găsit.").arg(reportId);
        return false;
    }

    const qint64 orderId = qry.value(QStringLiteral("id_orderEcho")).toLongLong();
    const QString number = qry.value(QStringLiteral("numberDoc")).toString().trimmed();
    const QDateTime dateDoc = qry.value(QStringLiteral("dateDoc")).toDateTime();
    const QString patientName =
        QStringLiteral("%1 %2")
            .arg(qry.value(QStringLiteral("last_name")).toString(),
                 qry.value(QStringLiteral("first_name")).toString())
            .simplified();

    if (qry.value(QStringLiteral("deletionMark")).toInt() != 2) {
        m_errors << tr("Raportul nr.%1 nu mai este validat.").arg(number);
        return false;
    }
    if (qry.value(QStringLiteral("id_organizations")).toInt() != m_data.recipientOrganizationId) {
        m_errors << tr("Raportul nr.%1 aparține altei organizații.").arg(number);
        return false;
    }

    const QString fileStem = uniqueFileStem(number, reportId);

    ReportPrintService service(m_database, dbConn);
    ReportPrintService::Request request;
    request.reportId = static_cast<int>(reportId);
    request.mode = PrintType::ExportToPDF;
    request.pdfBase = QDir(m_data.filePDF).filePath(
        QStringLiteral("Raport_ecografic_nr_%1").arg(fileStem));
    request.showDoctorStamp = true;
    request.showDoctorSignature = true;
    request.reportParent = this;
    const ReportPrintService::Result printResult = service.print(request);
    if (!printResult.success) {
        m_errors << tr("Raportul nr.%1: %2")
                        .arg(number,
                             printResult.error.isEmpty()
                                 ? tr("nu a putut fi exportat în PDF.")
                                 : printResult.error);
        qCritical(logCritical()) << "[ReportsEmailExporterWorker] Eroare la exportul raportului nr."
                                 << number << printResult.error;
        return false;
    }
    m_result.attachments << printResult.files;

    // Imaginile sunt opționale: o eroare aici nu exclude PDF-ul raportului,
    // dar este raportată utilizatorului.
    if (m_data.includeImages && dbImages) {
        ReportImagesExporter::Request imagesRequest;
        imagesRequest.orderId = orderId;
        imagesRequest.reportId = reportId;
        imagesRequest.reportNumber = fileStem;
        imagesRequest.directory = m_data.filePDF;
        const ReportImagesExporter::Result images =
            ReportImagesExporter::exportImages(*dbImages, imagesRequest);
        m_result.attachments << images.files;
        m_errors << images.errors;
    }

    m_result.documentTitles << tr("Raport ecografic nr.%1 din %2 – %3")
                                   .arg(number,
                                        dateDoc.toString(QStringLiteral("dd.MM.yyyy")),
                                        patientName);

    qInfo(logInfo()) << "[ReportsEmailExporterWorker] Exportul cu succes al raportului nr."
                     << number << "fișiere=" << printResult.files.size();
    return true;
}
