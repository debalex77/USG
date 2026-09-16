#ifndef PATIENTSAVERWORKER_H
#define PATIENTSAVERWORKER_H

#include <QObject>
#include <QUuid>

#include <common/appmetatypes.h>
#include <common/table_sections.h>
#include <common/globals.h>

#include <threads/databaseprovider.h>

/** Clasa destinata pentru salvarea datelor pacientului
 ** fara 'inghetarea' interfetei */
class PatientSaverWorker : public QObject
{
    Q_OBJECT
public:
    explicit PatientSaverWorker(DatabaseProvider *provider,
                                const PatientDataStructure &patientData,
                                QObject *parent = nullptr);

signals:
    void finished(const PatientDataStructure &data);
    void finishedError(const QStringList &err);
    void finishedPatientExistInBD(const PatientDataStructure &data);

public slots:
    void processInsert();
    void processUpdate();

private slots:
    bool loadPatientUuidById(QSqlDatabase &dbConn, QUuid &uuidOut);

private:
    bool patientExistInDB(QSqlDatabase &dbConn,
                          bool *queryOk = nullptr,
                          QString *queryError = nullptr);
    bool patientDataInsertInDB(QSqlDatabase &dbConn, QStringList &err);
    bool patientDataUpdate(QSqlDatabase &dbConn, QStringList &err);

private:
    DatabaseProvider *m_provider{nullptr};
    PatientDataStructure m_patientData;
};

#endif // PATIENTSAVERWORKER_H
