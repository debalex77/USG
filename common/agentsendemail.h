#ifndef AGENTSENDEMAIL_H
#define AGENTSENDEMAIL_H

#include "common/cryptomanager.h"
#include "common/emailcore.h"
#include "common/processingaction.h"
#include <QDialog>

#include <data/database.h>
#include <data/globals.h>

#include <QLineEdit>

#include <customs/lineeditopen.h>

namespace Ui {
class AgentSendEmail;
}

class AgentSendEmail : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString NrOrder READ getNrOrder WRITE setNrOrder NOTIFY NrOrderChanged FINAL)
    Q_PROPERTY(QString NrReport READ getNrReport WRITE setNrReport NOTIFY NrReportChanged FINAL)
    Q_PROPERTY(bool ThisReports READ getThisReports WRITE setThisReports NOTIFY ThisReportsChanged FINAL)
    Q_PROPERTY(QString NameReport READ getNameReport WRITE setNameReport NOTIFY NameReportChanged FINAL)
    Q_PROPERTY(QString EmailFrom READ getEmailFrom WRITE setEmailFrom NOTIFY EmailFromChanged FINAL)
    Q_PROPERTY(QString EmailTo READ getEmailTo WRITE setEmailTo NOTIFY EmailToChanged FINAL)
    Q_PROPERTY(QString NamePatient READ getNamePatient WRITE setNamePatient NOTIFY NamePatientChanged FINAL)
    Q_PROPERTY(QString NameDoctor READ getNameDoctor WRITE setNameDoctor NOTIFY NameDoctorChanged FINAL)
    Q_PROPERTY(QDate DateInvestigation READ getDateInvestigation WRITE setDateInvestigation NOTIFY DateInvestigationChanged FINAL)
public:
    explicit AgentSendEmail(QWidget *parent = nullptr);
    ~AgentSendEmail();

    QString getNrOrder();
    void setNrOrder(QString NrOrder);

    QString getNrReport();
    void setNrReport(QString NrReport);

    QString getEmailFrom();
    void setEmailFrom(QString EmailFrom);

    QString getEmailTo();
    void setEmailTo(QString EmailTo);

    QString getNamePatient();
    void setNamePatient(QString NamePatient);

    QString getNameDoctor();
    void setNameDoctor(QString NameDoctor);

    QDate getDateInvestigation();
    void setDateInvestigation(QDate DateInvestigation);

    bool getThisReports();
    void setThisReports(bool ThisReports);

    QString getNameReport();
    void setNameReport(QString NameReport);

signals:
    void NrOrderChanged();
    void NrReportChanged();
    void EmailFromChanged();
    void EmailToChanged();
    void NamePatientChanged();
    void NameDoctorChanged();
    void DateInvestigationChanged();
    void ThisReportsChanged();
    void NameReportChanged();

private slots:
    void slot_NrOrderChanged();
    void slot_NrReportChanged();
    void slot_EmailFromChanged();
    void slot_EmailToChanged();
    void slot_DateInvestigationChanged();
    void slot_NameReportChanged();

    void onOpenFile(const QString type_file);
    void openFile(const QString &filePath);

    void onSend();
    void onEmailSent(bool success);
    void onClose();

private:
    Ui::AgentSendEmail *ui;

    QStringList filesAttachments;

    QMap<QString, LineEditOpen*> fileInputs;
    QMap<QString, LineEditOpen*> imgInputs;

    QString m_subiect   = nullptr;
    QString m_body      = nullptr;
    QString m_nrOrder   = nullptr;
    QString m_nrReport  = nullptr;
    QString m_emailFrom = nullptr;
    QString m_emailTo   = nullptr;
    QString m_namePatient = nullptr;
    QString m_nameDoctor = nullptr;
    QDate m_dateInvestigation = QDate(2000, 01, 01);
    bool m_thisReports = false;
    QString m_nameReport = nullptr;

    QString m_smtp_server = nullptr;
    int m_port = -1;
    QString m_password = nullptr;

    DataBase *db;
    EmailCore *email_core;
    CryptoManager *crypto_manager;
    ProcessingAction *loader;
};

#endif // AGENTSENDEMAIL_H
