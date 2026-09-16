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
    bool patientExistsInDatabase(QSqlDatabase &dbConn);
    void patientDataUpdate(QSqlDatabase &dbConn);
    void patientDataInsert(QSqlDatabase &dbConn);

private:
    DatabaseProvider *m_db;
    DatesCatPatient m_data;
    QVariantMap m_context;
};

#endif // SYNCPATIENTDATAWORKER_H
