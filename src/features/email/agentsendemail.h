#ifndef AGENTSENDEMAIL_H
#define AGENTSENDEMAIL_H

#include <QDialog>
#include <QDate>
#include <QMap>
#include <QPointer>
#include <QVector>
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

#include <database/database.h>

#include <common/globals.h>
#include <infrastructure/security/cryptomanager.h>
#include <infrastructure/email/emailcore.h>
#include <infrastructure/email/temporaryexportowner.h>
#include <ui/dialogs/processingaction.h>

#include <ui/widgets/lineeditopen.h>

#include <models/queryrolesmodel.h>

namespace Ui {
class AgentSendEmail;
}

class AgentSendEmail : public QDialog
{
    Q_OBJECT

public:
    // Tipul scrisorii; thisReports rămâne pentru apelurile existente
    // (true = raport statistic) când kind nu este MultipleReports.
    enum class MessageKind {
        Default,
        MultipleReports // mai multe rapoarte ecografice către organizația trimițătoare
    };

    struct MailContext
    {
        MessageKind kind = MessageKind::Default;

        int organizationId = 0;
        QString organizationName;
        QString organizationPhone;
        QString organizationEmail;

        QString nrOrder;
        QString nrReport;
        bool thisReports = false;
        QString nameReport;

        // MessageKind::MultipleReports
        QString recipientName;
        QStringList documentTitles;
        bool includesImages = false;

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
        QString exportDirectory;
    };

public:
    explicit AgentSendEmail(DataBase &db, QWidget *parent = nullptr);
    ~AgentSendEmail();

    void setContext(const MailContext &context);
    const MailContext &context() const;

    // Creează câte un subdirector privat și unic pentru fiecare export
    // (TemporaryExportOwner::prepare). Directorul transmis în MailContext este
    // revendicat de setContext() și șters la eliberarea ultimei referințe
    // (dialogul și, pe durata trimiterii, EmailCore).
    static bool prepareExportDirectory(QString *directory, QString *error = nullptr);
    // Șterge doar un director încă nerevendicat (export eșuat înainte de agent).
    static void removeExportDirectory(const QString &directory);

public slots:
    void done(int result) override;

private slots:
    void onOpenFile(const QString &typeFile);
    void onSend();
    void onEmailSent(bool success, const QString &errorText);
    void onClose();

private:
    void initModelAccount();
    void initConnections();
    void initEditorsMap();

    bool loadOnlineAccountSettings(bool logFailure = true);
    bool loadOrganizationDetails();
    bool selectAccountByEmail(const QString &email);
    void selectFirstAvailableAccount();
    void refreshAttachmentsFromEditors();
    void buildMessage();
    void collectAttachments();
    LineEditOpen *appendAttachmentEditor(bool image);
    void fillUiFromContext();
    void openFile(const QString &filePath);

private:
    Ui::AgentSendEmail *ui = nullptr;

    DataBase &m_db;
    MailContext m_ctx;
    std::shared_ptr<TemporaryExportOwner> m_exportOwner;

    QueryRolesModel *modelAccount = nullptr;

    QMap<QString, LineEditOpen*> fileInputs;
    QMap<QString, LineEditOpen*> imgInputs;
    QVector<LineEditOpen*> fileEditors;
    QVector<LineEditOpen*> imageEditors;

    QPointer<ProcessingAction> loader;
    QPointer<QThread> emailThread;
};

#endif // AGENTSENDEMAIL_H
