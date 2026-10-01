#include "reportprintservice.h"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <LimeReport>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QStandardItemModel>

#include <common/applicationpathscontext.h>
#include <common/globals.h>
#include <core/loggingcategories.h>
#include <database/database.h>
#include <features/printing/printimagesservice.h>
#include <settings/settingsservice.h>

ReportPrintService::ReportPrintService(DataBase &database,
                                       const QSqlDatabase &connection)
    : m_database(database), m_connection(connection)
{
}

ReportPrintService::Result ReportPrintService::print(const Request &request) const
{
    using namespace ReportSections;

    Result result;
    if (request.reportId <= 0) {
        result.error = tr("Identificatorul raportului ecografic nu este valid.");
        return result;
    }
    if (!m_connection.isValid() || !m_connection.isOpen()) {
        result.error = tr("Baza de date nu este deschisă pentru tipărirea raportului.");
        return result;
    }

    QString outputBase;
    if (request.mode == PrintType::ExportToPDF) {
        const QString requestedBase = request.pdfBase.trimmed();
        if (requestedBase.isEmpty()) {
            result.error = tr("Nu este indicată calea fișierelor PDF pentru raport.");
            return result;
        }
        outputBase = QDir::cleanPath(requestedBase);
    }

    QSqlQuery metadata(m_connection);
    metadata.prepare(QStringLiteral(R"(
        SELECT
            numberDoc,
            dateDoc,
            patient_id,
            t_organs_internal,
            t_urinary_system,
            t_prostate,
            t_gynecology,
            t_breast,
            t_thyroid,
            t_gestation0,
            t_gestation1,
            t_gestation2,
            t_lymphNodes
        FROM
            reportEcho
        WHERE
            id = ?
    )"));
    metadata.addBindValue(request.reportId);
    if (!metadata.exec() || !metadata.next()) {
        result.error = metadata.lastError().isValid()
                           ? tr("Datele raportului nu au putut fi citite: %1")
                                 .arg(metadata.lastError().text())
                           : tr("Raportul ecografic nu a fost găsit.");
        return result;
    }

    const QString number = metadata.value("numberDoc").toString();
    const QVariant documentDateValue = metadata.value("dateDoc");
    const int patientId = metadata.value("patient_id").toInt();
    ReportSystems systems;
    if (metadata.value("t_organs_internal").toBool())
        systems |= ReportSystem::OrgansInternal;
    if (metadata.value("t_urinary_system").toBool())
        systems |= ReportSystem::UrinarySystem;
    if (metadata.value("t_prostate").toBool())
        systems |= ReportSystem::Prostate;
    if (metadata.value("t_gynecology").toBool())
        systems |= ReportSystem::Gynecology;
    if (metadata.value("t_breast").toBool())
        systems |= ReportSystem::Breast;
    if (metadata.value("t_thyroid").toBool())
        systems |= ReportSystem::Thyroid;
    if (metadata.value("t_gestation0").toBool())
        systems |= ReportSystem::Gestation0;
    if (metadata.value("t_gestation1").toBool())
        systems |= ReportSystem::Gestation1;
    if (metadata.value("t_gestation2").toBool())
        systems |= ReportSystem::Gestation2;
    if (metadata.value("t_lymphNodes").toBool())
        systems |= ReportSystem::LymphNodes;

    const bool hasPrintableSystem =
        systems.testFlag(ReportSystem::OrgansInternal)
        || systems.testFlag(ReportSystem::UrinarySystem)
        || systems.testFlag(ReportSystem::Prostate)
        || systems.testFlag(ReportSystem::Gynecology)
        || systems.testFlag(ReportSystem::Breast)
        || systems.testFlag(ReportSystem::Thyroid)
        || systems.testFlag(ReportSystem::Gestation0)
        || systems.testFlag(ReportSystem::Gestation1)
        || systems.testFlag(ReportSystem::Gestation2)
        || systems.testFlag(ReportSystem::LymphNodes);
    if (!hasPrintableSystem) {
        result.error = tr("Raportul validat nu conține niciun sistem pentru tipărire.");
        return result;
    }

    // Modelele trebuie să rămână vii până după distrugerea motorului LimeReport.
    // Ordinea declarațiilor asigură distrugerea motorului înaintea modelelor.
    QStandardItemModel imageModel;
    QSqlQueryModel organizationModel;
    QSqlQueryModel patientModel;
    std::vector<std::unique_ptr<QSqlQueryModel>> sectionModels;
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

    report.dataManager()->clearUserVariables();
    report.dataManager()->addModel("table_logo", &imageModel, false);
    report.dataManager()->addModel("main_organization", &organizationModel, false);
    report.dataManager()->addModel("table_patient", &patientModel, false);
    report.dataManager()->setReportVariable("v_exist_logo", images.logo ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_stamp", images.organizationStamp ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_stamp_doctor", images.doctorStamp ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_exist_signature", images.doctorSignature ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_show_stamp_doctor",
        images.doctorStamp && request.showDoctorStamp ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_show_signature_doctor",
        images.doctorSignature && request.showDoctorSignature ? 1 : 0);
    report.dataManager()->setReportVariable(
        "v_export_pdf", request.mode == PrintType::ExportToPDF ? 1 : 0);
    report.dataManager()->setReportVariable(
        "unitMeasure",
        globals().unitMeasure == QStringLiteral("milimetru")
            ? QStringLiteral("mm")
            : QStringLiteral("cm"));
    report.dataManager()->setReportVariable("all_recomandation", QString());
    const QDateTime documentDate = documentDateValue.toDateTime();
    report.setPreviewWindowTitle(
        tr("Raport ecografic nr.%1 din %2 (printare)")
            .arg(number,
                 documentDate.isValid()
                     ? documentDate.toString(QStringLiteral("dd.MM.yyyy hh:mm:ss"))
                     : documentDateValue.toString()));
    report.setShowProgressDialog(true);

    const QString templatesDirectory =
        ApplicationPathsContext::instance().data().templatesDirectory;
    bool failed = false;
    const auto fail = [&](const QString &message) {
        if (result.error.isEmpty())
            result.error = message;
        failed = true;
    };
    const auto executeTemplate = [&](const QString &templateName,
                                     const QString &suffix,
                                     const QString &label) {
        if (failed)
            return;
        const QString templatePath = QDir(templatesDirectory).filePath(templateName);
        if (!QFileInfo::exists(templatePath) || !report.loadFromFile(templatePath)) {
            fail(tr("Nu a fost încărcat șablonul formei de tipar: %1")
                     .arg(QDir::toNativeSeparators(templatePath)));
            return;
        }

        if (request.mode == PrintType::Designer) {
            qInfo(logInfo()) << "ReportPrintService: designer; sistem=" << label
                             << "nr=" << number;
            report.designReport();
            return;
        }
        if (request.mode == PrintType::Preview) {
            qInfo(logInfo()) << "ReportPrintService: preview; sistem=" << label
                             << "nr=" << number;
            report.previewReport();
            return;
        }

        const QString pdfFile = outputBase + suffix;
        if (QFile::exists(pdfFile) && !QFile::remove(pdfFile)) {
            fail(tr("Fișierul PDF existent nu poate fi înlocuit: %1")
                     .arg(QDir::toNativeSeparators(pdfFile)));
            return;
        }
        qInfo(logInfo()) << "ReportPrintService: export PDF; sistem=" << label
                         << "nr=" << number;
        if (!report.printToPDF(pdfFile)) {
            fail(tr("Raportul ecografic nu a putut fi exportat în PDF: %1")
                     .arg(QDir::toNativeSeparators(pdfFile)));
            return;
        }
        const QFileInfo file(pdfFile);
        if (!file.exists() || !file.isFile() || file.size() <= 0) {
            fail(tr("Fișierul PDF al raportului nu a fost creat corect: %1")
                     .arg(QDir::toNativeSeparators(pdfFile)));
            return;
        }
        result.files << pdfFile;
    };

    const auto addModel = [&](const QString &queryText,
                              const char *modelName) -> QSqlQueryModel * {
        auto model = std::make_unique<QSqlQueryModel>();
        model->setQuery(queryText, m_connection);
        if (model->lastError().isValid()) {
            fail(tr("Datele sistemului nu au putut fi citite: %1")
                     .arg(model->lastError().text()));
            return nullptr;
        }
        QSqlQueryModel *modelPtr = model.get();
        report.dataManager()->addModel(modelName, modelPtr, false);
        sectionModels.push_back(std::move(model));
        return modelPtr;
    };

    const auto scalar = [&](const QString &sql, const QVariantList &binds) -> QVariantList {
        QSqlQuery query(m_connection);
        query.prepare(sql);
        for (const QVariant &bind : binds)
            query.addBindValue(bind);
        if (!query.exec()) {
            fail(tr("Parametrii raportului nu au putut fi citiți: %1")
                     .arg(query.lastError().text()));
            return {};
        }
        if (!query.next())
            return {};
        QVariantList values;
        for (int column = 0; column < query.record().count(); ++column)
            values << query.value(column);
        return values;
    };

    if (systems.testFlag(ReportSystem::OrgansInternal)
        && systems.testFlag(ReportSystem::UrinarySystem)) {
        addModel(m_database.getQryForTableOrgansInternalById(request.reportId),
                 "table_organs_internal");
        addModel(m_database.getQryForTableUrinarySystemById(request.reportId),
                 "table_urinary_system");
        report.dataManager()->setReportVariable("unit_measure_volum", "ml");
        executeTemplate(QStringLiteral("Complex.lrxml"),
                        QStringLiteral("_complex.pdf"),
                        QStringLiteral("complex"));
    } else {
        if (systems.testFlag(ReportSystem::OrgansInternal)) {
            addModel(m_database.getQryForTableOrgansInternalById(request.reportId),
                     "table_organs_internal");
            report.dataManager()->setReportVariable("unit_measure_volum", "ml");
            executeTemplate(QStringLiteral("Organs internal.lrxml"),
                            QStringLiteral("_organs_internal.pdf"),
                            QStringLiteral("organs_internal"));
        }
        if (systems.testFlag(ReportSystem::UrinarySystem)) {
            addModel(m_database.getQryForTableUrinarySystemById(request.reportId),
                     "table_urinary_system");
            report.dataManager()->setReportVariable("unit_measure_volum", "ml");
            executeTemplate(QStringLiteral("Urinary system.lrxml"),
                            QStringLiteral("_urinary_sistem.pdf"),
                            QStringLiteral("urinary_system"));
        }
    }

    if (systems.testFlag(ReportSystem::Prostate)) {
        addModel(m_database.getQryForTableProstateById(request.reportId),
                 "table_prostate");
        const QVariantList values = scalar(
            QStringLiteral("SELECT transrectal FROM tableProstate WHERE id_reportEcho = ?"),
            {request.reportId});
        report.dataManager()->setReportVariable(
            "method_examination",
            !values.isEmpty() && values.constFirst().toBool()
                ? QStringLiteral("transrectal")
                : QStringLiteral("transabdominal"));
        report.dataManager()->setReportVariable("unit_measure_volum", "cm3");
        executeTemplate(QStringLiteral("Prostate.lrxml"),
                        QStringLiteral("_prostate.pdf"),
                        QStringLiteral("prostate"));
    }

    if (systems.testFlag(ReportSystem::Gynecology)) {
        addModel(m_database.getQryForTableGynecologyById(request.reportId),
                 "table_gynecology");
        const QVariantList values = scalar(
            QStringLiteral("SELECT transvaginal FROM tableGynecology WHERE id_reportEcho = ?"),
            {request.reportId});
        report.dataManager()->setReportVariable(
            "method_examination",
            !values.isEmpty() && values.constFirst().toBool()
                ? QStringLiteral("transvaginal")
                : QStringLiteral("transabdominal"));
        report.dataManager()->setReportVariable("unit_measure_volum", "cm3");
        executeTemplate(QStringLiteral("Gynecology.lrxml"),
                        QStringLiteral("_gynecology.pdf"),
                        QStringLiteral("gynecology"));
    }

    if (systems.testFlag(ReportSystem::Breast)) {
        addModel(m_database.getQryForTableBreastById(request.reportId), "table_breast");
        executeTemplate(QStringLiteral("Breast.lrxml"),
                        QStringLiteral("_breast.pdf"),
                        QStringLiteral("breast"));
    }

    if (systems.testFlag(ReportSystem::Thyroid)) {
        addModel(m_database.getQryForTableThyroidById(request.reportId), "table_thyroid");
        report.dataManager()->setReportVariable("unit_measure_volum", "cm3");
        executeTemplate(QStringLiteral("Thyroid.lrxml"),
                        QStringLiteral("_thyroid.pdf"),
                        QStringLiteral("thyroid"));
    }

    const auto setGestationVariables = [&](const QString &tableName) {
        const QVariantList values = scalar(
            QStringLiteral("SELECT view_examination, lmp FROM %1 WHERE id_reportEcho = ?")
                .arg(tableName),
            {request.reportId});
        if (values.size() >= 2) {
            const QDate lmp = values.at(1).toDate();
            report.dataManager()->setReportVariable("ivestigation_view", values.at(0).toInt());
            report.dataManager()->setReportVariable("v_lmp", lmp.toString("dd.MM.yyyy"));
            report.dataManager()->setReportVariable(
                "v_probable_date_birth",
                lmp.isValid() ? lmp.addDays(280).toString("dd.MM.yyyy") : QString());
        }
    };

    if (systems.testFlag(ReportSystem::Gestation0)) {
        addModel(m_database.getQryForTableGestation0dById(request.reportId),
                 "table_gestation0");
        setGestationVariables(QStringLiteral("tableGestation0"));
        executeTemplate(QStringLiteral("Gestation0.lrxml"),
                        QStringLiteral("_gestation0.pdf"),
                        QStringLiteral("gestation0"));
    }

    if (systems.testFlag(ReportSystem::Gestation1)) {
        addModel(m_database.getQryForTableGestation1dById(request.reportId),
                 "table_gestation1");
        setGestationVariables(QStringLiteral("tableGestation1"));
        executeTemplate(QStringLiteral("Gestation1.lrxml"),
                        QStringLiteral("_gestation1.pdf"),
                        QStringLiteral("gestation1"));
    }

    if (systems.testFlag(ReportSystem::Gestation2)) {
        addModel(m_database.getQryForTableGestation2(request.reportId), "table_gestation2");
        const QVariantList values = scalar(
            QStringLiteral("SELECT dateMenstruation FROM tableGestation2 WHERE id_reportEcho = ?"),
            {request.reportId});
        if (!values.isEmpty()) {
            const QDate lmp = values.constFirst().toDate();
            report.dataManager()->setReportVariable("v_lmp", lmp.toString("dd.MM.yyyy"));
            report.dataManager()->setReportVariable(
                "v_probable_date_birth",
                lmp.isValid() ? lmp.addDays(280).toString("dd.MM.yyyy") : QString());
        }
        executeTemplate(QStringLiteral("Gestation2.lrxml"),
                        QStringLiteral("_gestation2.pdf"),
                        QStringLiteral("gestation2"));
    }

    if (systems.testFlag(ReportSystem::LymphNodes)) {
        addModel(m_database.getQryForTableLymphNodes(request.reportId), "table_lymph");
        const QVariantList values = scalar(
            QStringLiteral("SELECT section_type FROM tableSofTissuesLymphNodes WHERE id_reportEcho = ?"),
            {request.reportId});
        report.dataManager()->setReportVariable(
            "v_section_type_text",
            values.isEmpty() || values.constFirst().toString().isEmpty()
                ? QStringLiteral("lymph_nodes")
                : values.constFirst().toString());
        executeTemplate(QStringLiteral("LymphNodes.lrxml"),
                        QStringLiteral("_lymphNodes.pdf"),
                        QStringLiteral("lymph_nodes"));
    }

    if (failed) {
        if (request.mode == PrintType::ExportToPDF) {
            for (const QString &file : std::as_const(result.files))
                QFile::remove(file);
            result.files.clear();
        }
        return result;
    }

    result.success = true;
    return result;
}
