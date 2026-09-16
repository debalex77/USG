#ifndef SYNCPATIENTWORKER_H
#define SYNCPATIENTWORKER_H

#include <QObject>
#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <common/globals.h>
#include <common/appmetatypes.h>
#include <threads/databaseprovider.h>

/** Clasa destinata pentru syncronizarea pacientului
 ** cu baza 'cloud' */
class SyncPatientWorker : public QObject
{
    Q_OBJECT
public:
    explicit SyncPatientWorker(DatabaseProvider *provider,
                               PatientDataStructure data,
                               QObject *parent = nullptr);

    void process();

signals:
    void syncError(const QString &error);
    void finished();

private:
    bool isUuidValid() const;
    QString uuidToHex() const;
    void logQueryError(const QString &context, const QSqlQuery &q) const;
    void bindPatientFields(QSqlQuery &q);

    bool patientExistsInDatabase(QSqlDatabase &dbConn, bool *ok, QString *error);
    bool updatePatient(QSqlDatabase &dbConn, QString *error);
    bool insertPatient(QSqlDatabase &dbConn, QString *error);

private:
    DatabaseProvider *m_db;
    PatientDataStructure m_data;
};

#endif // SYNCPATIENTWORKER_H
