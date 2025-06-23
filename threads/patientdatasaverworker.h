#ifndef PATIENTDATASAVERWORKER_H
#define PATIENTDATASAVERWORKER_H

#include <QObject>
#include <threads/databaseprovider.h>
#include <common/appmetatypes.h>

class PatientDataSaverWorker : public QObject
{
    Q_OBJECT
public:
    PatientDataSaverWorker(DatabaseProvider *provider, DatesCatPatient &data, QObject *parent = nullptr);

public slots:
    void processInsert();
    void processUpdate();

signals:
    void finished();
    void finishedError(const QStringList &err);
    void finishedPatientExistInBD(const QVariantMap &map_data);

private:
    bool patientExistInDB(QSqlDatabase &dbConn);
    bool patientDataInsertInDB(QSqlDatabase &dbConn, QStringList &err);
    void patientDataUpdate(QSqlDatabase &dbConn, QStringList &err);

private:
    DatabaseProvider *m_db{nullptr};
    DatesCatPatient m_data;
};

#endif // PATIENTDATASAVERWORKER_H
