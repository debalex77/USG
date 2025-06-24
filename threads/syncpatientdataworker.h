#ifndef SYNCPATIENTDATAWORKER_H
#define SYNCPATIENTDATAWORKER_H

#include <QObject>
#include <threads/databaseprovider.h>
#include <common/appmetatypes.h>

class SyncPatientDataWorker : public QObject
{
    Q_OBJECT
public:
    SyncPatientDataWorker(DatabaseProvider *provider, DatesCatPatient &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished();

private:
    bool patientExistInBD(QSqlDatabase &dbConn);
    void patientDataUpdate(QSqlDatabase &dbConn);
    void pacientDataInsert(QSqlDatabase &dbConn_cloud);

private:
    DatabaseProvider *m_db;
    DatesCatPatient m_data;
    QVariantMap map_data;
};

#endif // SYNCPATIENTDATAWORKER_H
