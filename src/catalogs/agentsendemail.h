#ifndef AGENTSENDEMAIL_H
#define AGENTSENDEMAIL_H

#include <QDialog>
#include <QDate>
#include <QMap>
#include <QStringList>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QSysInfo>

#include <data/database.h>

#include <common/globals.h>
#include <common/cryptomanager.h>
#include <common/emailcore.h>
#include <common/processingaction.h>

#include <customs/lineeditopen.h>

#include <models/queryrolesmodel.h>

namespace Ui {
class AgentSendEmail;
}

class AgentSendEmail : public QDialog
{
    Q_OBJECT

public:
    struct MailContext
    {
        QString nrOrder;
        QString nrReport;
        bool thisReports = false;
        QString nameReport;

        QString emailFrom;
        QString emailTo;
        QString namePatient;
        QString nameDoctor;
        QDate dateInvestigation = QDate::currentDate();

        QString smtpServer;
        int port = 0;
        QString username;
        QString password;

        QString subject;
        QString body;
        QStringList attachments;
    };

public:
    explicit AgentSendEmail(DataBase &db, QWidget *parent = nullptr);
    ~AgentSendEmail();

    void setContext(const MailContext &context);
    const MailContext &context() const;

private slots:
    void onOpenFile(const QString &typeFile);
    void onSend();
    void onEmailSent(bool success);
    void onClose();

private:
    void initModelAccount();
    void initConnections();
    void initEditorsMap();

    bool loadOnlineAccountSettings(bool logFailure = true);
    void selectFirstAvailableAccount();
    void refreshAttachmentsFromEditors();
    void buildMessage();
    void collectAttachments();
    void fillUiFromContext();
    void openFile(const QString &filePath);

private:
    Ui::AgentSendEmail *ui = nullptr;

    DataBase &m_db;
    MailContext m_ctx;

    QueryRolesModel *modelAccount = nullptr;

    QMap<QString, LineEditOpen*> fileInputs;
    QMap<QString, LineEditOpen*> imgInputs;

    ProcessingAction *loader = nullptr;
};

#endif // AGENTSENDEMAIL_H
