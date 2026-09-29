#include "reportdialogcontext.h"

#include <QSqlError>
#include <QSqlQuery>

const ReportDocumentContextData &ReportDialogContext::data() const
{
    return m_data;
}

bool ReportDialogContext::loadFromOrder(const QSqlDatabase &database,
                                        int orderId,
                                        QString *error)
{
    clear();
    if (error)
        error->clear();

    if (!database.isValid() || !database.isOpen()) {
        if (error)
            *error = QStringLiteral("Conexiunea la baza de date nu este deschisă.");
        return false;
    }

    if (orderId <= 0) {
        if (error)
            *error = QStringLiteral("Identificatorul comenzii ecografice nu este valid.");
        return false;
    }

    QSqlQuery query(database);
    query.prepare(QStringLiteral(R"(
        SELECT
            id,
            id_organizations,
            id_users,
            COALESCE(id_doctors_execute, id_doctors) AS executing_doctor_id,
            id_nurses
        FROM
            orderEcho
        WHERE
            id = ?
    )"));
    query.addBindValue(orderId);

    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }

    if (!query.next()) {
        if (error) {
            *error = QStringLiteral("Comanda ecografică cu ID=%1 nu a fost găsită.")
                         .arg(orderId);
        }
        return false;
    }

    m_data.orderId           = query.value(0).toInt();
    m_data.organizationId    = query.value(1).toInt();
    m_data.orderUserId       = query.value(2).toInt();
    m_data.executingDoctorId = query.value(3).toInt();
    m_data.nurseId           = query.value(4).toInt();

    if (!m_data.isValid()) {
        if (error) {
            *error = QStringLiteral(
                         "Comanda ecografică ID=%1 nu conține o organizație validă.")
                         .arg(orderId);
        }
        clear();
        return false;
    }

    return true;
}

void ReportDialogContext::clear()
{
    m_data = {};
}
