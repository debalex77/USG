#ifndef APPMETATYPES_H
#define APPMETATYPES_H

#pragma once

#include <QString>
#include <QDate>
#include <QVariant>
#include <QVector>
#include <QList>

//------ structuri
struct DatesForAgentEmail
{
    QString nr_order;
    QString nr_report;
    QString emailTo;
    QString name_patient;
    QString name_doctor_execute;
    QString str_dateInvestigation;
};

//------ declaratii metatype

Q_DECLARE_METATYPE(DatesForAgentEmail)

//------ aliase

using m_DatesForAgentEmail = QVector<DatesForAgentEmail>;

#endif // APPMETATYPES_H
