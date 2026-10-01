#include "orderprintservice.h"

#include <LimeReport>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>

#include <common/applicationpathscontext.h>
#include <core/loggingcategories.h>
#include <database/database.h>
#include <features/printing/printimagesservice.h>
#include <settings/settingsservice.h>

OrderPrintService::OrderPrintService(DataBase &database,
                                     const QSqlDatabase &connection)
    : m_database(database), m_connection(connection)
{
}

OrderPrintService::Result OrderPrintService::print(const Request &request) const
{
    Result result;
    if (request.orderId <= 0) {
        result.error = tr("Identificatorul comenzii ecografice nu este valid.");
        return result;
    }
    if (!m_connection.isValid() || !m_connection.isOpen()) {
        result.error = tr("Baza de date nu este deschisă pentru tipărirea comenzii.");
        return result;
    }

    QString outputFile;
    if (request.mode == PrintType::ExportToPDF) {
        const QString requestedFile = request.pdfFile.trimmed();
        if (requestedFile.isEmpty()) {
            result.error = tr("Nu este indicată calea fișierului PDF pentru comandă.");
            return result;
        }
        outputFile = QDir::cleanPath(requestedFile);
        if (QFile::exists(outputFile) && !QFile::remove(outputFile)) {
            result.error = tr("Fișierul PDF existent nu poate fi înlocuit: %1")
                               .arg(QDir::toNativeSeparators(outputFile));
            return result;
        }
    }

    QSqlQuery metadata(m_connection);
    metadata.prepare(QStringLiteral(R"(
        SELECT
            orderEcho.numberDoc,
            orderEcho.dateDoc,
            orderEcho.patient_id,
            orderEcho.sum,
            typesPrices.noncomercial
        FROM
            orderEcho
        INNER JOIN
            typesPrices ON typesPrices.id = orderEcho.id_typesPrices
        WHERE
            orderEcho.id = ?
    )"));
    metadata.addBindValue(request.orderId);
    if (!metadata.exec() || !metadata.next()) {
        result.error = metadata.lastError().isValid()
                           ? tr("Datele comenzii nu au putut fi citite: %1")
                                 .arg(metadata.lastError().text())
                           : tr("Comanda ecografică nu a fost găsită.");
        return result;
    }

    const QString number         = metadata.value("numberDoc").toString();
    const QDateTime documentDate = metadata.value("dateDoc").toDateTime();
    const int patientId          = metadata.value("patient_id").toInt();
    const double documentSum     = metadata.value("sum").toDouble();
    const bool noncommercial     = metadata.value("noncomercial").toBool();

    QStringList investigationCodes;
    QSqlQuery codesQuery(m_connection);
    codesQuery.prepare(QStringLiteral(R"(
        SELECT
            cod
        FROM
            orderEchoTable
        WHERE
            id_orderEcho = ?
        ORDER BY
            id
    )"));
    codesQuery.addBindValue(request.orderId);
    if (!codesQuery.exec()) {
        result.error = tr("Investigațiile comenzii nu au putut fi citite: %1")
                           .arg(codesQuery.lastError().text());
        return result;
    }
    while (codesQuery.next()) {
        const QString code = codesQuery.value(0).toString().trimmed();
        if (!code.isEmpty())
            investigationCodes.append(code);
    }

    // Modelele trebuie să rămână vii până după distrugerea motorului LimeReport.
    // Ordinea declarațiilor asigură distrugerea motorului înaintea modelelor.
    QStandardItemModel imageModel;
    QSqlQueryModel organizationModel;
    QSqlQueryModel patientModel;
    QSqlQueryModel tableModel;
    LimeReport::ReportEngine report(request.reportParent);

    const Settings::OrganizationSettings &settings =
        SettingsService::instance().organization();
    const PrintImagesService::Result images = PrintImagesService::fillModel(
        &imageModel,
        m_connection,
        settings.organizationId,
        settings.defaultDoctorId);

    QSqlQuery organizationQuery(m_connection);
    organizationQuery.prepare(QStringLiteral(R"(
        SELECT
            org.id AS id_organizations,
            org.IDNP,
            org.name,
            org.address,
            org.telephone,
            doctor.nameAbbreviated AS doctor,
            nurse.nameAbbreviated AS nurse,
            org.email,
            org.site
        FROM
            organizations org
        LEFT JOIN
            fullNameDoctors doctor ON doctor.id_doctors = ?
        LEFT JOIN
            fullNameNurses nurse ON nurse.id_nurses = ?
        WHERE
            org.id = ?
    )"));
    organizationQuery.addBindValue(settings.defaultDoctorId);
    organizationQuery.addBindValue(settings.defaultNurseId);
    organizationQuery.addBindValue(settings.organizationId);
    if (!organizationQuery.exec()) {
        result.error = tr("Datele organizației pentru tipărire nu au putut fi citite: %1")
                           .arg(organizationQuery.lastError().text());
        return result;
    }
    organizationModel.setQuery(std::move(organizationQuery));

    DataBase::setModelQuery(patientModel,
                            m_connection,
                            m_database.getTextSQL(
                                QStringLiteral(":/sql/queries_print/tablePatientByID.sql")),
                            {patientId});
    if (patientModel.lastError().isValid()) {
        result.error = tr("Datele pacientului pentru tipărire nu au putut fi citite: %1")
                           .arg(patientModel.lastError().text());
        return result;
    }

    QVariantMap replacements;
    replacements.insert(QStringLiteral("noncomercial"), noncommercial);
    DataBase::setModelQuery(tableModel,
                            m_connection,
                            m_database.getTextSQL(
                                QStringLiteral(":/sql/queries_print/orderTable.sql")),
                            {request.orderId},
                            replacements);
    if (tableModel.lastError().isValid()) {
        result.error = tr("Pozițiile comenzii pentru tipărire nu au putut fi citite: %1")
                           .arg(tableModel.lastError().text());
        return result;
    }

    report.dataManager()->clearUserVariables();
    report.dataManager()->setReportVariable(
        "sume_total",
        QString::number(noncommercial ? 0.0 : documentSum, 'f', 2));
    report.dataManager()->setReportVariable("v_exist_logo", images.logo ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_stamp", images.organizationStamp ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_stamp_doctor", images.doctorStamp ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_signature", images.doctorSignature ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_consent", ReportSections::informedConsentTextByCodes(investigationCodes));
    report.dataManager()->addModel("table_img", &imageModel, false);
    report.dataManager()->addModel("main_organization", &organizationModel, false);
    report.dataManager()->addModel("table_pacient", &patientModel, false);
    report.dataManager()->addModel("table_table", &tableModel, false);
    report.setShowProgressDialog(true);
    report.setPreviewWindowTitle(
        tr("Comanda ecografică nr.%1 din %2 (printare)")
            .arg(number,
                 documentDate.isValid()
                     ? documentDate.toString(QStringLiteral("dd.MM.yyyy hh:mm:ss"))
                     : metadata.value(1).toString()));

    const QString templatePath = QDir(
        ApplicationPathsContext::instance().data().templatesDirectory)
                                     .filePath(QStringLiteral("Order.lrxml"));
    if (!QFileInfo::exists(templatePath) || !report.loadFromFile(templatePath)) {
        result.error = tr("Nu a fost încărcat șablonul formei de tipar: %1")
                           .arg(QDir::toNativeSeparators(templatePath));
        return result;
    }

    switch (request.mode) {
    case PrintType::Designer:
        qInfo(logInfo()) << "OrderPrintService: designer; nr=" << number;
        report.designReport();
        result.success = true;
        break;
    case PrintType::Preview:
        qInfo(logInfo()) << "OrderPrintService: preview; nr=" << number;
        report.previewReport();
        result.success = true;
        break;
    case PrintType::ExportToPDF:
        qInfo(logInfo()) << "OrderPrintService: export PDF; nr=" << number;
        result.success = report.printToPDF(outputFile);
        if (result.success) {
            const QFileInfo file(outputFile);
            result.success = file.exists() && file.isFile() && file.size() > 0;
        }
        if (result.success)
            result.files << outputFile;
        else
            result.error = tr("Comanda ecografică nu a putut fi exportată în PDF.");
        break;
    }
    return result;
}
