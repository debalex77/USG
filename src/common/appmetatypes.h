#ifndef APPMETATYPES_H
#define APPMETATYPES_H

#pragma once

#include <QString>
#include <QDate>
#include <QVariant>
#include <QVector>
#include <QList>
#include <QUuid>
#include <common/table_sections.h>

//------ structuri

struct PatientDataStructure /** pu sincronizarea pacientului */
{
    int id;
    int deletionMark;
    QString idnp;
    QString name;
    QString firstName;
    QString middleName;
    QString medicalPolicy;
    QDate birthday;
    QString address;
    QString phone;
    QString email;
    QString comment;
    QUuid uuid;

    void clear()
    {
        *this = PatientDataStructure{};
    }
};

struct OrderDataStructure /** pu sincronizarea doc.OrderEcho */
{
    int id               = 0;
    int deletionMark     = 0;
    QString numberDoc;
    QDateTime dateDoc;
    int id_organization  = 0;
    int id_contract      = 0;
    int id_typePrice     = 0;
    int id_doctor        = 0;
    int id_doctor_execut = 0;
    int id_nurse         = 0;
    int id_patient       = 0;
    int id_user          = 0;
    double sum           = 0.0;
    QString comment;
    int card_payment     = 0;
    int attached_image   = 0;
    QUuid uuid;

    void clear()
    {
        *this = OrderDataStructure{};
    }
};

struct DatesCatPatient // de eliminat
{
    int id;
    QString name;
    QString firstName;
    QString idnp;
    QString medicalPolicy;
    QDate birthday;
    QString address;
    QString email;
    QString phone;
    QString comment;
    QUuid uuid;
    bool thisMySQL; // de eliminat

    void clear()
    {
        *this = DatesCatPatient{};
    }
};

struct DatesForAgentEmail
{
    QString nr_order;
    QString nr_report;
    QString emailTo;
    QString name_patient;
    QString name_doctor_execute;
    QString str_dateInvestigation;
};

struct DatesDocForExportEmail
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

struct DatesDocsOrderReportSync
{
    bool thisMySQL = false;
    QDateTime dateTime_doc;
    int id_order = 0;
    QString nr_order = nullptr;
    int id_report = 0;
    QString nr_report = nullptr;
    int id_patient = 0;
    QString name_parient = nullptr;
    QString fname_patient = nullptr;
    QString idnp_patient = nullptr;
    QDate birthday_patient;
    int id_doctor = 0;
    QString name_doctor = nullptr;
    QString fname_doctor = nullptr;
};

struct PricingJournalSettings
{
    QDateTime startDate;
    QDateTime endDate;

    bool saveFilter = false;
    int idOrganization = 0;
    int idContract     = 0;
    int idUser         = 0;
    QString nrDoc;

    int sortSection = 0;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    QMap<int, int> sectionSizes;
    QMap<int, bool> hiddenSections;
};

struct JournalFilter
{
    QDateTime startDate;
    QDateTime endDate;

    bool saveFilter = false;
    int idOrganization = 0;
    int idContract     = 0;
    int idUser         = 0;
    int patientId      = 0;
    QString nrDoc;
    QString patientName;

    int sortSection = 0;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    QMap<int, int> sectionSizes;
    QMap<int, bool> hiddenSections;
};

struct CatalogsCommon
{
    // common
    int id = -1;
    int deletionMark = -1;
    QString fullName;
    QString phone;
    QString email;

    // users
    QDateTime lastConnection;
    QString txtLastConnection;

    // organizations
    int idContract;

    // patients
    QDate birthday;
    QString txtBirthday;
    QString medicalPolicy;
    QString adress;
    QString idnp;
    QString comment;
    QByteArray uuid;
};

struct CatalogViewFilter
{
    int sortSection = 0;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    QMap<int, int> sectionSizes;
    QMap<int, bool> hiddenSections;
};

//------ declaratii metatype

Q_DECLARE_METATYPE(PatientDataStructure)
Q_DECLARE_METATYPE(OrderDataStructure)
Q_DECLARE_METATYPE(DatesCatPatient)
Q_DECLARE_METATYPE(DatesForAgentEmail)
Q_DECLARE_METATYPE(DatesDocForExportEmail)
Q_DECLARE_METATYPE(DatesDocsOrderReportSync)
Q_DECLARE_METATYPE(PricingJournalSettings)
Q_DECLARE_METATYPE(JournalFilter)
Q_DECLARE_METATYPE(CatalogsCommon)
Q_DECLARE_METATYPE(CatalogViewFilter)

//------ aliase

using m_DatesForAgentEmail = QVector<DatesForAgentEmail>;

#endif // APPMETATYPES_H
