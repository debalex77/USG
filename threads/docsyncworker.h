#ifndef DOCSYNCWORKER_H
#define DOCSYNCWORKER_H

#include <QObject>
#include <data/database.h>
#include <threads/databaseprovider.h>
#include <common/appmetatypes.h>

class docSyncWorker : public QObject
{
    Q_OBJECT
public:
    explicit docSyncWorker(DatabaseProvider *provider, DatesDocsOrderReportSync &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished();

private:
    int getLastNumberDoc(const QString name_table, QSqlDatabase &dbConn);
    void setIDdoctor(QSqlDatabase &dbConn_sync);
    void setIDpacient(QSqlDatabase &dbConn_sync);
    void syncOrder(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncReport(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

private:
    DatabaseProvider *m_db{nullptr};
    DatesDocsOrderReportSync m_data; // datele transmise

    struct Dates_Sync {
        int id_order      = 0;
        QString numberDoc = nullptr;
        QDateTime dateDoc;
        int id_doctor  = 0;
        int id_nurse   = 0;
        int id_patient = 0;
    };
    Dates_Sync m_datesSync;

};

#endif // DOCSYNCWORKER_H
