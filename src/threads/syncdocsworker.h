#ifndef SYNCDOCSWORKER_H
#define SYNCDOCSWORKER_H

#include <QObject>
#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <threads/databaseprovider.h>

class SyncDocsWorker : public QObject
{
    Q_OBJECT
public:
    struct DataDocs {
        int id_order;
        int id_report;
        int id_patient;
        QString nameConnection;
    };
    explicit SyncDocsWorker(DatabaseProvider *provider,
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
    bool syncAttachedImages(QSqlDatabase &dbSync, QSqlDatabase &dbImg);

    // Metode utilitare
    bool checkIfRecordExists(QSqlDatabase &db,
                             const QString &table,
                             const QByteArray &uuid);
    bool insertOrUpdateRecord(QSqlDatabase &dbTarget,
                              const QString &table,
                              const QVariantMap &data);

private:
    DatabaseProvider *m_db;
    DataDocs m_data;
};

#endif // SYNCDOCSWORKER_H
