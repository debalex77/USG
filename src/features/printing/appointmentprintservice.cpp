#include "appointmentprintservice.h"

#include <LimeReport>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>

#include <common/applicationpathscontext.h>
#include <core/loggingcategories.h>
#include <features/printing/printimagesservice.h>
#include <settings/settingsservice.h>

namespace {

// Numele câmpurilor sunt un contract cu AppointmentPatient.lrxml ($D{main_table.*}).
enum MainTableColumn {
    ColTime, ColPatient, ColInvestigations, ColOrganization,
    ColDoctor, ColComment, ColExecuted, ColCount
};

// Denumirea după id, citită o singură dată pentru fiecare id.
class NameLookup
{
public:
    NameLookup(const QSqlDatabase &connection, const QString &sql)
        : m_query(connection)
    {
        m_prepared = m_query.prepare(sql);
        if (!m_prepared)
            qWarning(logWarning()) << "AppointmentPrintService: prepare lookup error:"
                                   << m_query.lastError().text();
    }

    QString name(int id)
    {
        if (id <= 0 || !m_prepared)
            return QString();

        const auto it = m_cache.constFind(id);
        if (it != m_cache.constEnd())
            return it.value();

        QString value;
        m_query.bindValue(QStringLiteral(":id"), id);
        if (m_query.exec() && m_query.next())
            value = m_query.value(0).toString();
        else if (m_query.lastError().isValid())
            qWarning(logWarning()) << "AppointmentPrintService: lookup error:"
                                   << m_query.lastError().text();
        m_query.finish();

        m_cache.insert(id, value);
        return value;
    }

private:
    QSqlQuery m_query;
    bool m_prepared = false;
    QHash<int, QString> m_cache;
};

} // namespace

AppointmentPrintService::AppointmentPrintService(const QSqlDatabase &connection)
    : m_connection(connection)
{
}

AppointmentPrintService::Result AppointmentPrintService::print(const Request &request) const
{
    Result result;
    if (request.rows.isEmpty()) {
        result.error = tr("Nu sunt programări pentru data aleasă.");
        return result;
    }
    if (!m_connection.isValid() || !m_connection.isOpen()) {
        result.error = tr("Baza de date nu este deschisă pentru tipărirea programărilor.");
        return result;
    }

    // Modelele trebuie să rămână vii până după distrugerea motorului LimeReport.
    // Ordinea declarațiilor asigură distrugerea motorului înaintea modelelor.
    QStandardItemModel imageModel;
    QSqlQueryModel organizationModel;
    QStandardItemModel tableModel(0, ColCount);
    LimeReport::ReportEngine report(request.reportParent);

    const Settings::OrganizationSettings &settings =
        SettingsService::instance().organization();
    const PrintImagesService::Result images = PrintImagesService::fillModel(
        &imageModel,
        m_connection,
        settings.organizationId,
        settings.defaultDoctorId,
        PrintImagesService::ModelLayout::StatisticalReports);

    // antetul și subsolul – ca în rapoartele statistice
    QSqlQuery organizationQuery(m_connection);
    organizationQuery.prepare(QStringLiteral(R"(
        SELECT
            organizations.id AS id_organizations,
            organizations.IDNP,
            organizations.name,
            organizations.address,
            organizations.telephone,
            fullNameDoctors.nameAbbreviated AS doctor,
            organizations.email,
            organizations.site
        FROM
            organizations
        LEFT JOIN
            fullNameDoctors
                ON fullNameDoctors.id_doctors = ?
        WHERE
            organizations.id = ?
    )"));
    organizationQuery.addBindValue(settings.defaultDoctorId);
    organizationQuery.addBindValue(settings.organizationId);
    if (!organizationQuery.exec()) {
        result.error = tr("Datele organizației pentru tipărire nu au putut fi citite: %1")
                           .arg(organizationQuery.lastError().text());
        return result;
    }
    organizationModel.setQuery(std::move(organizationQuery));

    // rândurile programărilor
    NameLookup organizations(m_connection, QStringLiteral(R"(
        SELECT name FROM organizations WHERE id = :id
    )"));
    NameLookup doctors(m_connection, QStringLiteral(R"(
        SELECT nameAbbreviated FROM fullNameDoctors WHERE id_doctors = :id
    )"));

    tableModel.setHorizontalHeaderLabels({
        QStringLiteral("time"), QStringLiteral("patient"),
        QStringLiteral("investigations"), QStringLiteral("organization"),
        QStringLiteral("doctor"), QStringLiteral("comment"),
        QStringLiteral("executed")
    });
    for (const Row &row : request.rows) {
        QList<QStandardItem *> items(ColCount);
        items[ColTime]           = new QStandardItem(row.time);
        items[ColPatient]        = new QStandardItem(row.patient);
        items[ColInvestigations] = new QStandardItem(row.investigations);
        items[ColOrganization]   = new QStandardItem(organizations.name(row.organizationId));
        items[ColDoctor]         = new QStandardItem(doctors.name(row.doctorId));
        items[ColComment]        = new QStandardItem(row.comment);
        items[ColExecuted]       = new QStandardItem(row.executed ? tr("da") : QString());
        tableModel.appendRow(items);
    }

    const QLocale locale;
    const QString presentationDate =
        tr("pentru data de %1 (%2)")
            .arg(request.date.toString(QStringLiteral("dd.MM.yyyy")),
                 locale.dayName(request.date.dayOfWeek()));

    report.dataManager()->clearUserVariables();
    report.dataManager()->setReportVariable("v_presentation_date", presentationDate);
    report.dataManager()->setReportVariable("v_count", request.rows.size());
    report.dataManager()->setReportVariable("v_exist_logo", images.logo ? 1 : 0);
    report.dataManager()->setReportVariable("v_hide_logo", 0);
    report.dataManager()->setReportVariable("v_hide_data_organization", 0);
    report.dataManager()->setReportVariable("v_hide_name_doctor", 0);
    // lista programărilor nu se semnează/ștampilează
    report.dataManager()->setReportVariable("v_hide_signature", 1);
    report.dataManager()->addModel("table_img", &imageModel, false);
    report.dataManager()->addModel("main_organization", &organizationModel, false);
    report.dataManager()->addModel("main_table", &tableModel, false);
    report.setShowProgressDialog(true);
    report.setPreviewWindowTitle(
        tr("Programarea pacienților %1 (printare)").arg(presentationDate));

    const QString templatePath = QDir(
        ApplicationPathsContext::instance().data().templatesDirectory)
                                     .filePath(QStringLiteral("AppointmentPatient.lrxml"));
    if (!QFileInfo::exists(templatePath) || !report.loadFromFile(templatePath)) {
        result.error = tr("Nu a fost încărcat șablonul formei de tipar: %1")
                           .arg(QDir::toNativeSeparators(templatePath));
        return result;
    }

    switch (request.mode) {
    case PrintType::Designer:
        qInfo(logInfo()) << "AppointmentPrintService: designer;"
                         << request.date.toString(Qt::ISODate);
        report.designReport();
        result.success = true;
        break;
    case PrintType::Preview:
        qInfo(logInfo()) << "AppointmentPrintService: preview;"
                         << request.date.toString(Qt::ISODate);
        report.previewReport();
        result.success = true;
        break;
    case PrintType::ExportToPDF:
        result.error = tr("Exportul PDF nu este disponibil pentru programări.");
        break;
    }
    return result;
}
