#ifndef DOCEMAILEXPORTERWORKER_H
#define DOCEMAILEXPORTERWORKER_H

#include <LimeReport>
#include <QObject>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <data/database.h>
#include <threads/databaseprovider.h>
#include <common/appmetatypes.h>

class DocEmailExporterWorker : public QObject
{
    Q_OBJECT
public:

    // DatesDocForExportEmail - metatype definit in common/appmetatypes.h
    DocEmailExporterWorker(DatabaseProvider *provider, DatesDocForExportEmail &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished(QVector<DatesForAgentEmail> datesExport); // DatesForAgentEmail - metatype definit in common/appmetatypes.h
    void setTextInfo(QString txtInfo);

private:
    // functiile pu export orderEcho
    void setModelImgForPrint();
    void setModelDatesOrganization(QSqlDatabase &dbConn);
    void setModelDatesPatient(QSqlDatabase &dbConn);
    void setModelDocTable(QSqlDatabase &dbConn, bool noncomercial);
    void cleaningUpModelInstances();
    void exportOrderEcho(QSqlDatabase &dbConn);

    // functiile pu export reportEcho
    void handlerComplex(QSqlDatabase &dbConn);
    void handlerOrgansInternal(QSqlDatabase &dbConn);
    void handlerUrinarySystem(QSqlDatabase &dbConn);
    void handlerProstate(QSqlDatabase &dbConn);
    void handlerGynecology(QSqlDatabase &dbConn);
    void handlerBreast(QSqlDatabase &dbConn);
    void handlerThyroid(QSqlDatabase &dbConn);
    void handlerGestation0(QSqlDatabase &dbConn);
    void handlerGestation1(QSqlDatabase &dbConn);
    void handlerGestation2(QSqlDatabase &dbConn);
    void cleaningUpModelInstancesReport();
    void exportReportEcho(QSqlDatabase &dbConn);

    // functiile pu export imaginilor
    void exportImagesDocument(QSqlDatabase &dbConn);

private:
    DatesDocForExportEmail m_data; // DatesDocForExportEmail - metatype definit in common/appmetatypes.h
    DataBase *db;
    DatabaseProvider *m_db{nullptr};

    DatesForAgentEmail m_datesExport; // DatesForAgentEmail - metatype definit in common/appmetatypes.h
    QVector<DatesForAgentEmail> datesExportForAgentEmail;

    int exist_logo = 0;
    int exist_signature = 0;
    int exist_stamp_doctor = 0;
    int exist_stamp_organization = 0;
    QStandardItemModel *model_img = nullptr;

    LimeReport::ReportEngine *m_report;
    QSqlQueryModel *model_organization = nullptr;
    QSqlQueryModel *model_patient      = nullptr;
    QSqlQueryModel *model_table        = nullptr;

    QSqlQueryModel *modelOrgansInternal = nullptr;
    QSqlQueryModel *modelUrinarySystem  = nullptr;
    QSqlQueryModel *modelProstate       = nullptr;
    QSqlQueryModel *modelGynecology     = nullptr;
    QSqlQueryModel *modelBreast         = nullptr;
    QSqlQueryModel *modelThyroid        = nullptr;
    QSqlQueryModel *modelGestationO     = nullptr;
    QSqlQueryModel *modelGestation1     = nullptr;
    QSqlQueryModel *modelGestation2     = nullptr;
};

#endif // DOCEMAILEXPORTERWORKER_H
