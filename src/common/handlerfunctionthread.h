#ifndef HANDLERFUNCTIONTHREAD_H
#define HANDLERFUNCTIONTHREAD_H

#include <LimeReport>
#include <QObject>
#include <QStandardItemModel>
#include <QSqlQueryModel>

#include <data/database.h>

struct ExportData {
    QString nr_order;
    QString nr_report;
    QString emailTo;
    QString name_patient;
    QString name_doctor_execute;
    QString str_dateInvestigation;
};

class HandlerFunctionThread : public QObject
{
    Q_OBJECT
public:
    explicit HandlerFunctionThread(QObject *parent = nullptr);
    ~HandlerFunctionThread();

    //---- constante sau date generale
    struct GeneralData
    {
        int id_user;
        int id_organization;
        int id_doctor;
        bool thisMySQL;
    };
    void setGeneralData(const GeneralData &data);

    //---- date pacientului
    struct DataCatPatient
    {
        int id;
        QString name;
        QString fname;
        QString idnp;
        QString medicalPolicy;
        QDate birthday;
        QString address;
        QString email;
        QString phone;
        QString comment;
        bool thisMySQL;
    };
    void setDataCatPatient(const DataCatPatient &data);

    //---- date documentelor pu exportul prin e-mail
    struct DocumentExportEmailData
    {
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
    void setDocumentExportEmailData(const DocumentExportEmailData &data);

    //---- tag-le pe sisteme documentului orderEcho & reportEcho
    struct TagsSystemDocument
    {
        bool organs_internal;
        bool urinary_system;
        bool prostate;
        bool gynecology;
        bool breast;
        bool thyroide;
        bool gestation0;
        bool gestation1;
        bool gestation2;
        bool gestation3;
    };
    void setTagsSystemDocument(const TagsSystemDocument &data);

public slots:
    void setDataConstants();
    void saveDataPatient();
    void syncWithMySQL(int localPatientId);
    void updateDataPatientInDB();
    void exportDocumentsToPDF();

signals:
    void finishSetDataConstants();
    void finishExistPatientInBD(const QString patient_name, const QString patient_fname, QDate patient_birthday, const QString patient_idnp);
    void finishInsertDataCatPatient(const bool succes, const int patient_id);
    void finishUpdateDataCatPatient(const bool succes);
    void finishExportDocumenstToPDF(QVector<ExportData> data_agentEmail);
    void errorExportDocs(QString textError);
    void setTextInfo(QString txtInfo);

private:
    // insertia, actualizarea datelor pacientului
    bool patientExistsInDB(QSqlDatabase dbConnection);
    bool insertDataPatientInDB(QSqlDatabase dbConnection);

    // formarea modelului:
    //   - logotip,stampila organizatiei
    //   - stampila, semnatura doctorului
    void setModelImgForPrint();

    // exportul documentelor
    void exportDocumentOrder(QSqlDatabase dbConnection);
    void exportDocumentReport(QSqlDatabase dbConnection);
    void exportDocumentImages(QSqlDatabase dbImageConnection);

private:

    // structura generala
    GeneralData m_GeneralData;

    // structura datelor pacientului
    DataCatPatient m_DataCatPatient;

    // datele documentului pu export prin e-mail
    DocumentExportEmailData m_DocumentExportEmailData;

    int exist_logo = 0;
    int exist_stamp_organization = 0;
    int exist_stamp_doctor = 0;
    int exist_signature = 0;

    QString m_unitMeasure = nullptr;
    QByteArray m_logo_byteArray = nullptr;
    QByteArray m_stamp_main_organization = nullptr;
    QByteArray m_stamp_main_doctor = nullptr;
    QByteArray m_signature_main_doctor= nullptr;
    QString m_pathTemplatesDocs = nullptr;
    QString m_filePDF = nullptr;

    //tag documentului raport ecografic
    TagsSystemDocument m_TagsSystemDocument;

    // class
    DataBase *db;
    LimeReport::ReportEngine *m_report;
    QVector<ExportData> data_agentEmail;

    CryptoManager *crypto_manager;

    QStandardItemModel *model_img;
    QSqlQueryModel *model_organization;
    QSqlQueryModel *model_patient;
    QSqlQueryModel *model_table;
};

#endif // HANDLERFUNCTIONTHREAD_H
