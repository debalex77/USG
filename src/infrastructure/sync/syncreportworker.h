#ifndef SYNCREPORTWORKER_H
#define SYNCREPORTWORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>

class SyncReportWorker : public QObject
{
    Q_OBJECT
public:
    explicit SyncReportWorker(int localReportId, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void syncProgress(const QString &message);
    void syncError(const QString &error);
    void finished();

private:
    int remoteIdByUuid(QSqlDatabase &cloudDb,
                       QSqlDatabase &localDb,
                       const QString &table,
                       int localId);
    bool syncReportHeader(QSqlDatabase &cloudDb, QSqlDatabase &localDb);
    bool ensureRemoteReportVideoSchema(QSqlDatabase &cloudDb);
    bool replaceChildTable(QSqlDatabase &cloudDb,
                           QSqlDatabase &localDb,
                           const QString &table);
    bool writeByUuid(QSqlDatabase &cloudDb,
                     const QString &table,
                     const QVariantMap &data);
    bool insertMap(QSqlDatabase &db,
                   const QString &table,
                   const QVariantMap &data);

    int m_localReportId = 0;
    int m_localOrderId = 0;
    int m_localPatientId = 0;
    int m_remoteReportId = 0;
    int m_remoteOrderId = 0;
    int m_remotePatientId = 0;
    int m_remoteUserId = 0;
    QString m_lastError;
};

#endif // SYNCREPORTWORKER_H
