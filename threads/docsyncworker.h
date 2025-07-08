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

    int getIDdoctor(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local, const int m_ID);
    void setIDpacient(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncOrder(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncReport(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncOrgansInternal(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUrinarySystem(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncProstate(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncGynecology(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncBreast(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncThyroid(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncGestation0(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncGestation1(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncGestation2(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

private:
    DatabaseProvider *m_db{nullptr};
    DatesDocsOrderReportSync m_data; // datele transmise

    struct Dates_Sync {
        int id_order      = 0;
        QString nr_order  = nullptr;
        int id_report     = 0;
        QString nr_report = nullptr;
        QDateTime dateDoc;
        int id_doctor  = 0;
        int id_doctor_execut = 0;
        int id_nurse   = 0;
        int id_patient = 0;
    };
    Dates_Sync m_datesSync;

};

#endif // DOCSYNCWORKER_H
