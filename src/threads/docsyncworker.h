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

    bool existreportDoc(QSqlDatabase &dbConn_sync);

    void syncOrder(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncReport(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertOrgansInternal(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateOrgansInternal(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertUrinarySystem(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateUrinarySystem(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertProstate(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateProstate(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertGynecology(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateGynecology(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertBreast(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateBreast(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertThyroid(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateThyroid(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertGestation0(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateGestation0(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertGestation1(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateGestation1(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncInsertGestation2(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);
    void syncUpdateGestation2(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local);

    void syncImages(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConnImg);

private:
    DatabaseProvider *m_db{nullptr};
    DatesDocsOrderReportSync m_data; // datele transmise

    struct Dates_Sync {
        bool existDoc     = false;
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
