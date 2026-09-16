#ifndef SYNCORDERWORKER_H
#define SYNCORDERWORKER_H

#include <QObject>
#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <common/appmetatypes.h>
#include <threads/databaseprovider.h>

class SyncOrderWorker : public QObject
{
    Q_OBJECT
public:
    explicit SyncOrderWorker(DatabaseProvider *provider,
                             PatientDataStructure dataPatient,
                             OrderDataStructure dataOrder,
                             QObject *parent = nullptr);
    void process();

signals:
    void syncProgress(const QString &message);
    void syncError(const QString &error);
    void finished();

private:
    // Metode de sincronizare
    bool syncPatientData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal);
    bool syncOrderData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal);
    bool syncOrderTableData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal);
    bool resolveRemoteForeignKeys(QSqlDatabase &dbSync,
                                  QSqlDatabase &dbLocal,
                                  QVariantMap &orderData,
                                  QString *error);
    int remoteIdForLocalId(QSqlDatabase &dbSync,
                           QSqlDatabase &dbLocal,
                           const QString &table,
                           int localId,
                           bool required,
                           QString *error);

    // Metode utilitare
    bool checkIfRecordExists(QSqlDatabase &db,
                             const QString &table,
                             const QByteArray &uuid);
    bool insertOrUpdateRecord(QSqlDatabase &dbTarget,
                              const QString &table,
                              const QVariantMap &data);
    // bool deleteRecordIfExists(QSqlDatabase &db,
    //                           const QString &table,
    //                           const QString &idField,
    //                           int id);

private:
    DatabaseProvider *m_db;
    PatientDataStructure m_dataPatient;
    OrderDataStructure   m_dataOrder;
    int m_remoteOrderId = 0;
    QString m_lastError;
};

#endif // SYNCORDERWORKER_H
