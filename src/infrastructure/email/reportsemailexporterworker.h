#ifndef REPORTSEMAILEXPORTERWORKER_H
#define REPORTSEMAILEXPORTERWORKER_H

#include <QObject>
#include <QSet>
#include <database/database.h>
#include <infrastructure/database/databaseprovider.h>
#include <common/appmetatypes.h>

// Exportă în PDF mai multe rapoarte ecografice validate (și, opțional,
// imaginile lor) pentru o singură scrisoare către organizația trimițătoare.
//
// Rulează pe firul GUI (LimeReport), pornit prin QTimer::singleShot, ca
// DocEmailExporterWorker; DataBase servește doar la textele SQL și la
// ReportPrintService, conexiunea vine din DatabaseProvider.
class ReportsEmailExporterWorker : public QObject
{
    Q_OBJECT
public:
    ReportsEmailExporterWorker(DataBase &database, DatabaseProvider *provider,
                               const DatesReportsForExportEmail &data,
                               QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished(const ReportsForAgentEmail &result);
    void setTextInfo(const QString &txtInfo);

private:
    bool loadRecipient(QSqlDatabase &dbConn);
    bool exportReport(QSqlDatabase &dbConn, QSqlDatabase *dbImages, qint64 reportId);
    QString uniqueFileStem(const QString &reportNumber, qint64 reportId);

private:
    DataBase &m_database;
    DatabaseProvider *m_provider = nullptr;
    DatesReportsForExportEmail m_data;

    ReportsForAgentEmail m_result;
    QStringList m_errors;
    QSet<QString> m_usedFileStems;
};

#endif // REPORTSEMAILEXPORTERWORKER_H
