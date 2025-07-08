#ifndef APPMETATYPES_H
#define APPMETATYPES_H

#pragma once

#include <QString>
#include <QDate>
#include <QVariant>
#include <QVector>
#include <QList>

//------ structuri

struct DatesCatPatient
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

//------ declaratii metatype

Q_DECLARE_METATYPE(DatesCatPatient)
Q_DECLARE_METATYPE(DatesForAgentEmail)
Q_DECLARE_METATYPE(DatesDocForExportEmail)
Q_DECLARE_METATYPE(DatesDocsOrderReportSync)

//------ aliase

using m_DatesForAgentEmail = QVector<DatesForAgentEmail>;

#endif // APPMETATYPES_H
