#ifndef DOCEMAILEXPORTERWORKER_H
#define DOCEMAILEXPORTERWORKER_H

#include <QObject>
#include <database/database.h>
#include <infrastructure/database/databaseprovider.h>
#include <common/appmetatypes.h>

class DocEmailExporterWorker : public QObject
{
    Q_OBJECT
public:

    // DatesDocForExportEmail - metatype definit in common/appmetatypes.h
    // Rulează pe firul GUI (LimeReport); DataBase servește doar la textele SQL
    // de tipar (getQryForTable*), conexiunea vine din DatabaseProvider.
    DocEmailExporterWorker(DataBase &database, DatabaseProvider *provider,
                           DatesDocForExportEmail &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished(QVector<DatesForAgentEmail> datesExport); // DatesForAgentEmail - metatype definit in common/appmetatypes.h
    void setTextInfo(QString txtInfo);

private:
    bool exportOrderEcho(QSqlDatabase &dbConn);
    bool exportReportEcho(QSqlDatabase &dbConn);
    bool exportImagesDocument(QSqlDatabase &dbConn);

private:
    DatesDocForExportEmail m_data; // DatesDocForExportEmail - metatype definit in common/appmetatypes.h
    DataBase *db = nullptr;
    DatabaseProvider *m_db{nullptr};

    DatesForAgentEmail m_datesExport; // DatesForAgentEmail - metatype definit in common/appmetatypes.h
    QVector<DatesForAgentEmail> datesExportForAgentEmail;
    QStringList m_exportedFiles; // căile fișierelor exportate cu succes
    QStringList m_exportErrors;

};

#endif // DOCEMAILEXPORTERWORKER_H
