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

#include "docemailexporterworker.h"

#include <features/printing/orderprintservice.h>
#include <features/printing/reportprintservice.h>

#include <infrastructure/email/reportimagesexporter.h>

DocEmailExporterWorker::DocEmailExporterWorker(DataBase &database,
                                               DatabaseProvider *provider,
                                               DatesDocForExportEmail &data,
                                               QObject *parent)
    : QObject{parent}, m_data(data), db(&database), m_db(provider)
{

}

// **********************************************************************************
// --- functia principala de export

void DocEmailExporterWorker::process()
{
    const QString connName = QStringLiteral("connection_%1")
                                    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    const QString connNameImage = QStringLiteral("connectionImage_%1")
                                      .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    m_datesExport.exportDirectory = m_data.filePDF;
    bool exportSucceeded = false;

    { // Conexiunea trăieşte DOAR în acest bloc

        // 1. efectuam conectarea la bd int-un alt thread
        // La eșec nu se iese din funcție: conexiunea înregistrată de provider
        // trebuie eliminată mai jos, ca la orice alt rezultat.
        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        const bool connected = dbConn.isOpen() || dbConn.open();
        if (!connected) {
            qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                                            .arg(this->metaObject()->className())
                                     << dbConn.lastError().text();
            m_exportErrors << tr("Conexiunea cu baza de date nu a putut fi deschisă: %1")
                                  .arg(dbConn.lastError().text());
        }

        // 2. exportam ordercho; fără datele comenzii (și ale raportului
        //    validat asociat) nu are sens exportul raportului și al imaginilor
        const bool orderOk = connected && exportOrderEcho(dbConn);
        if (orderOk) {

            // 3. exportam reportEcho
            const bool reportOk = exportReportEcho(dbConn);

            // 4. exportam imaginile
            bool imagesOk = true;
            if (m_data.thisMySQL) {
                imagesOk = exportImagesDocument(dbConn);
            } else {
                QSqlDatabase dbConnImage = m_db->getDatabaseImagesThread(connNameImage);
                if (!dbConnImage.isOpen() && !dbConnImage.open()) {
                    imagesOk = false;
                    m_exportErrors << tr("Baza de date a imaginilor nu a putut fi deschisă: %1")
                                          .arg(dbConnImage.lastError().text());
                } else {
                    imagesOk = exportImagesDocument(dbConnImage);
                }
                dbConnImage.close();
            }
            exportSucceeded = reportOk && imagesOk;
        }

        // 5. inchidem conexiunea cu BD
        dbConn.close();

    } // <- destructor QSqlDatabase

    if (m_db->containConnection(connNameImage))
        m_db->removeDatabaseThread(connNameImage);

    m_db->removeDatabaseThread(connName);

    m_datesExport.attachments = m_exportedFiles;
    m_datesExport.success = exportSucceeded;
    if (!exportSucceeded) {
        m_datesExport.errorText = m_exportErrors.isEmpty()
            ? tr("Exportul documentelor nu s-a finalizat.")
            : m_exportErrors.join(QStringLiteral("\n"));
    }
    datesExportForAgentEmail = {m_datesExport};

    emit finished(datesExportForAgentEmail);
}

bool DocEmailExporterWorker::exportOrderEcho(QSqlDatabase &dbConn)
{
    // Citim numai metadatele necesare agentului și identificatorul raportului.
    // Generarea PDF este delegată aceleiași clase folosite de Preview/Designer.
    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        SELECT
                orderEcho.id_typesPrices,
                orderEcho.patient_id,
                orderEcho.sum,
                orderEcho.numberDoc  AS nr_order,
                orderEcho.dateDoc    AS dateInvestigation,
                reportEcho.id        AS id_report,
                reportEcho.numberDoc AS nr_report,
                typesPrices.noncomercial,
                patients.last_name AS patient_last_name,
                patients.first_name AS patient_first_name,
                fullNameDoctors.nameAbbreviated AS doctore_execute,
                patients.email AS emailTo
            FROM
                orderEcho
            INNER JOIN
                typesPrices ON typesPrices.id = orderEcho.id_typesPrices
            INNER JOIN
                reportEcho ON reportEcho.id_orderEcho = orderEcho.id AND
                              reportEcho.deletionMark = 2
            LEFT JOIN
                fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors_execute
            INNER JOIN
                patients ON patients.id = orderEcho.patient_id
            WHERE
                orderEcho.id = ?
        )");
    qry.addBindValue(m_data.id_order);
    if (! qry.exec()) {
        qCritical(logCritical()).noquote() << QStringLiteral("[THREAD %1] Eroare exec SELECT(orderEcho):")
                                        .arg(this->metaObject()->className())
                                 << qry.lastError().text();
        m_exportErrors << tr("Datele comenzii nu au putut fi citite: %1")
                              .arg(qry.lastError().text());
        return false;
    }

    if (!qry.next()) {
        m_exportErrors << tr("Comanda selectată nu are un raport ecografic validat.");
        qCritical(logCritical()).noquote()
            << QStringLiteral("[THREAD %1] Comanda cu id=%2 și raport validat nu a fost găsită pentru export.")
                   .arg(this->metaObject()->className())
                   .arg(m_data.id_order);
        return false;
    }

    const QSqlRecord rec = qry.record();
    m_data.id_patient = qry.value(rec.indexOf("patient_id")).toInt();
    m_data.id_report = qry.value(rec.indexOf("id_report")).toInt();
    m_data.nr_order = qry.value(rec.indexOf("nr_order")).toString();
    m_data.nr_report = qry.value(rec.indexOf("nr_report")).toString();
    qInfo(logInfo())
        << "[THREAD] Se initializeaza exportul documentului 'Comanda ecografica' nr."
        << m_data.nr_order;

    m_datesExport.nr_order = m_data.nr_order;
    m_datesExport.nr_report = m_data.nr_report;
    // Contul de expediere și identitatea documentelor aparțin cabinetului
    // configurat în UserPreference, nu organizației trimițătoare din comandă.
    m_datesExport.organizationId = m_data.printOrganizationId;
    m_datesExport.emailTo = qry.value(rec.indexOf("emailTo")).toString();
    m_datesExport.name_patient =
        QStringLiteral("%1 %2")
            .arg(qry.value(rec.indexOf("patient_last_name")).toString(),
                 qry.value(rec.indexOf("patient_first_name")).toString())
            .simplified();
    m_datesExport.name_doctor_execute =
        qry.value(rec.indexOf("doctore_execute")).toString();
    m_datesExport.str_dateInvestigation =
        qry.value(rec.indexOf("dateInvestigation")).toString();

    const QString pdfPath = m_data.filePDF + "/Comanda_ecografica_nr_"
                            + ReportImagesExporter::fileSafeDocumentNumber(m_data.nr_order) + ".pdf";
    OrderPrintService service(*db, dbConn);
    OrderPrintService::Request request;
    request.orderId = m_data.id_order;
    request.mode = PrintType::ExportToPDF;
    request.pdfFile = pdfPath;
    request.reportParent = this;
    const OrderPrintService::Result printResult = service.print(request);
    if (!printResult.success) {
        m_exportErrors << printResult.error;
        qCritical(logCritical())
            << "[THREAD] Eroare la exportul documentului 'Comanda ecografica' nr."
            << m_data.nr_order << printResult.error;
        return false;
    }

    m_exportedFiles.append(printResult.files);
    qInfo(logInfo()) << "[THREAD] Exportul cu succes a documentului 'Comanda ecografica' nr."
                     << m_data.nr_order;
    emit setTextInfo(tr("S-a exportat documentul „Comandă ecografică” nr.%1.")
                         .arg(m_data.nr_order));
    return true;
}

bool DocEmailExporterWorker::exportReportEcho(QSqlDatabase &dbConn)
{
    const QString reportBase = QDir(m_data.filePDF).filePath(
        QStringLiteral("Raport_ecografic_nr_%1")
            .arg(ReportImagesExporter::fileSafeDocumentNumber(m_data.nr_report)));

    ReportPrintService service(*db, dbConn);
    ReportPrintService::Request request;
    request.reportId = m_data.id_report;
    request.mode = PrintType::ExportToPDF;
    request.pdfBase = reportBase;
    request.showDoctorStamp = true;
    request.showDoctorSignature = true;
    request.reportParent = this;
    const ReportPrintService::Result printResult = service.print(request);
    if (!printResult.success) {
        m_exportErrors << (printResult.error.isEmpty()
                               ? tr("Raportul ecografic nu a putut fi exportat în PDF.")
                               : printResult.error);
        qCritical(logCritical())
            << "[THREAD] Eroare la exportul documentului 'Raport ecografic' nr."
            << m_data.nr_report << printResult.error;
        return false;
    }

    m_exportedFiles.append(printResult.files);
    qInfo(logInfo()) << "[THREAD] Exportul cu succes al documentului 'Raport ecografic' nr."
                     << m_data.nr_report << "fișiere=" << printResult.files.size();
    emit setTextInfo(tr("S-au exportat fișierele raportului ecografic nr.%1.")
                         .arg(m_data.nr_report));
    return true;

}

// **********************************************************************************
// --- exportul imaginilor

bool DocEmailExporterWorker::exportImagesDocument(QSqlDatabase &dbConn)
{
    ReportImagesExporter::Request request;
    request.orderId = m_data.id_order;
    request.reportId = m_data.id_report;
    request.reportNumber = m_data.nr_report;
    request.directory = m_data.filePDF;

    const ReportImagesExporter::Result result = ReportImagesExporter::exportImages(
        dbConn, request, [this](const QString &text) { emit setTextInfo(text); });

    // La eșec, cauza ajunge în m_exportErrors și este afișată de OrderView.
    m_exportedFiles << result.files;
    m_exportErrors << result.errors;
    return result.success;
}
