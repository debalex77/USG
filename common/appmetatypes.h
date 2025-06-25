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

struct TagsSystemDocument
{
    int id_doc;
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

//------ declaratii metatype

Q_DECLARE_METATYPE(DatesCatPatient)
Q_DECLARE_METATYPE(DatesForAgentEmail)
Q_DECLARE_METATYPE(TagsSystemDocument)
Q_DECLARE_METATYPE(DatesDocForExportEmail)

//------ aliase

using m_DatesForAgentEmail = QVector<DatesForAgentEmail>;

#endif // APPMETATYPES_H
