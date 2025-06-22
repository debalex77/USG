#ifndef DOCEMAILEXPORTERWORKER_H
#define DOCEMAILEXPORTERWORKER_H

#include <LimeReport>
#include <QObject>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <data/database.h>
#include <threads/databaseprovider.h>

class DocEmailExporterWorker : public QObject
{
    Q_OBJECT
public:

    struct DocData
    {
        int id_user;
        bool thisMySQL;
        int id_order;
        int id_report;
        QString nr_order;
        QString nr_report;
        int id_patient;
        QString unitMeasure;
        QByteArray logo_byteArray;
        QByteArray stamp_organization_byteArray;
        QByteArray stamp_doctor_byteArray;
        QByteArray signature_doctor_byteArray;
        QString pathTemplatesDocs;
        QString filePDF;
    };

    DocEmailExporterWorker(DatabaseProvider *provider, DocData &data, QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished();
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
    DocData m_data;
    DataBase *db;
    DatabaseProvider *m_db{nullptr};

    struct DatesForAgentEmail
    {
        QString nr_order;
        QString nr_report;
        QString emailTo;
        QString name_patient;
        QString name_doctor_execute;
        QString str_dateInvestigation;
    };
    DatesForAgentEmail m_datesExport;
    QVector<DatesForAgentEmail> datesExportForAgentEmail;

    int exist_logo = 0;
    int exist_signature = 0;
    int exist_stamp_doctor = 0;
    int exist_stamp_organization = 0;
    QStandardItemModel *model_img;

    LimeReport::ReportEngine *m_report;
    QSqlQueryModel *model_organization;
    QSqlQueryModel *model_patient;
    QSqlQueryModel *model_table;

    QSqlQueryModel *modelOrgansInternal;
    QSqlQueryModel *modelUrinarySystem;
    QSqlQueryModel *modelProstate;
    QSqlQueryModel *modelGynecology;
    QSqlQueryModel *modelBreast;
    QSqlQueryModel *modelThyroid;
    QSqlQueryModel *modelGestationO;
    QSqlQueryModel *modelGestation1;
    QSqlQueryModel *modelGestation2;
};

#endif // DOCEMAILEXPORTERWORKER_H
