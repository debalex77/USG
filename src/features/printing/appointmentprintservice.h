#ifndef APPOINTMENTPRINTSERVICE_H
#define APPOINTMENTPRINTSERVICE_H

#include <QCoreApplication>
#include <QDate>
#include <QSqlDatabase>
#include <QVector>

#include <common/table_sections.h>

/** Tipărirea programărilor unei zile (AppointmentPatient.lrxml). */
class AppointmentPrintService final
{
    Q_DECLARE_TR_FUNCTIONS(AppointmentPrintService)

public:
    struct Row {
        QString time;
        bool executed = false;
        QString patient;
        QString investigations;
        int organizationId = 0;
        int doctorId = 0;
        QString comment;
    };

    struct Request {
        QDate date;
        QVector<Row> rows;
        PrintType::Column mode = PrintType::Preview;
        QObject *reportParent = nullptr;
    };

    struct Result {
        bool success = false;
        QString error;
    };

    explicit AppointmentPrintService(const QSqlDatabase &connection);

    [[nodiscard]]
    Result print(const Request &request) const;

private:
    QSqlDatabase m_connection;
};

#endif // APPOINTMENTPRINTSERVICE_H
