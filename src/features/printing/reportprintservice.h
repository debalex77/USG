#ifndef REPORTPRINTSERVICE_H
#define REPORTPRINTSERVICE_H

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStringList>

#include <common/table_sections.h>

class DataBase;

class ReportPrintService final
{
    Q_DECLARE_TR_FUNCTIONS(ReportPrintService)

public:
    struct Request {
        int reportId = 0;
        PrintType::Column mode = PrintType::Preview;
        QString pdfBase;
        bool showDoctorStamp = false;
        bool showDoctorSignature = false;
        QObject *reportParent = nullptr;
    };

    struct Result {
        bool success = false;
        QString error;
        QStringList files;
    };

    ReportPrintService(DataBase &database, const QSqlDatabase &connection);

    [[nodiscard]] Result print(const Request &request) const;

private:
    DataBase &m_database;
    QSqlDatabase m_connection;
};

#endif // REPORTPRINTSERVICE_H
