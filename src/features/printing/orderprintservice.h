#ifndef ORDERPRINTSERVICE_H
#define ORDERPRINTSERVICE_H

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStringList>

#include <common/table_sections.h>

class DataBase;

class OrderPrintService final
{
    Q_DECLARE_TR_FUNCTIONS(OrderPrintService)

public:
    struct Request {
        int orderId = 0;
        PrintType::Column mode = PrintType::Preview;
        QString pdfFile;
        QObject *reportParent = nullptr;
    };

    struct Result {
        bool success = false;
        QString error;
        QStringList files;
    };

    OrderPrintService(DataBase &database, const QSqlDatabase &connection);

    [[nodiscard]]
    Result print(const Request &request) const;

private:
    DataBase &m_database;
    QSqlDatabase m_connection;
};

#endif // ORDERPRINTSERVICE_H
