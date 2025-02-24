#ifndef HANDLERFUNCTIONTHREAD_H
#define HANDLERFUNCTIONTHREAD_H

#include <LimeReport>
#include <QObject>
#include <QStandardItemModel>
#include <QSqlQueryModel>

#include <data/database.h>

struct ConstantsData {
    // constants
    int        c_id_organizations;
    int        c_id_doctor;
    int        c_id_nurse;
    QString    c_brandUSG;
    QByteArray c_logo;
    // organization
    QString    organization_name;
    QString    organization_address;
    QString    organization_phone;
    QString    organization_email;
    QByteArray organization_stamp;
    // doctor
    QByteArray doctor_signatute;
    QByteArray doctor_stamp;
    QString    doctor_nameFull;
    QString    doctor_nameAbbreviate;
};

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

    void setRequiredVariabile(const int id_mainUser,
                              const int id_mainOrganization,
                              const int id_mainDoctor);

    void setRequiredVariableForCatPatient(const int patient_id,
                                          const QString patient_name,
                                          const QString patient_fname,
                                          const QString patient_idnp,
                                          const QString patient_medicalPolicy,
                                          const QDate patient_birthday,
                                          const QString patient_address,
                                          const QString patient_email,
                                          const QString patient_telephone,
                                          const QString patient_comment);

    void setRequiredVariableForExportDocuments(const bool thisMySQL,
                                               const int id_order,
                                               const int id_report,
                                               const QString unitMeasure,
                                               const QByteArray logo_byteArray,
                                               const QByteArray stamp_main_organization,
                                               const QByteArray stamp_main_doctor,
                                               const QByteArray signature_main_doctor,
                                               const QString pathTemplatesDocs,
                                               const QString filePDF);

    void setTagsDocumentReport(const bool organs_internal,
                               const bool urinary_system,
                               const bool prostate,
                               const bool gynecology,
                               const bool breast,
                               const bool thyroide,
                               const bool gestation0,
                               const bool gestation1,
                               const bool gestation2,
                               const bool gestation3);

public slots:
    void setDataConstants();
    void saveDataPatient();
    void updateDataPatientInDB();
    void exportDocumentsToPDF();

signals:
    void finishSetDataConstants(QVector<ConstantsData> data_constants);
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

    // common
    int m_id_user = -1;
    int m_id_mainDoctor = -1;
    int m_id_mainOrganization = -1;

    // save data patient
    int cat_patient_id = -1;
    QString cat_patient_name = nullptr;
    QString cat_patient_fname = nullptr;
    QString cat_patient_idnp = nullptr;
    QDate   cat_patient_birthday;
    QString cat_patient_address = nullptr;
    QString cat_patient_email = nullptr;
    QString cat_patient_medicalPolicy = nullptr;
    QString cat_patient_telephone = nullptr;
    QString cat_patient_comment = nullptr;

    // documents
    bool m_thisMySQL = false;
    int m_id_order  = -1;
    int m_nr_order  = -1;
    int m_id_report = -1;
    int m_nr_report = -1;
    int m_id_patient = -1;
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
    bool m_organs_internal = false;
    bool m_urinary_system  = false;
    bool m_prostate        = false;
    bool m_gynecology      = false;
    bool m_breast          = false;
    bool m_thyroide        = false;
    bool m_gestation0      = false;
    bool m_gestation1      = false;
    bool m_gestation2      = false;
    bool m_gestation3      = false;

    // class
    DataBase *db;
    LimeReport::ReportEngine *m_report;
    QVector<ConstantsData> data_constants;
    QVector<ExportData> data_agentEmail;

    QStandardItemModel *model_img;
    QSqlQueryModel *model_organization;
    QSqlQueryModel *model_patient;
    QSqlQueryModel *model_table;
};

#endif // HANDLERFUNCTIONTHREAD_H
