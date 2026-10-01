#ifndef EMAILCORE_H
#define EMAILCORE_H

#include <QObject>
#include <QCoreApplication>
#include <QSslSocket>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <core/loggingcategories.h>

#include <memory>

class TemporaryExportOwner;

class EmailCore : public QObject
{
    Q_OBJECT
public:
    explicit EmailCore(QObject *parent = nullptr);
    ~EmailCore();

    void setEmailData(const QString &smtpServer, int port, const QString &emailFrom, const QString &userName,
                      const QString &password, const QString &emailTo, const QString &subiect,
                      const QString &body, const QStringList &attachments);

    // Directorul temporar al atașamentelor rămâne pe disc cât timp EmailCore
    // păstrează referința (până la distrugerea obiectului în firul SMTP).
    void setExportOwner(std::shared_ptr<TemporaryExportOwner> owner);

    // Verifică conexiunea SSL și autentificarea SMTP fără a trimite mesaj.
    static bool testConnection(const QString &smtpServer, int port,
                               const QString &userName, const QString &password,
                               int timeoutMs, QString *error);

public slots:
    void sendEmail();

signals:
    // Emis o singură dată la finalul fiecărei încercări de trimitere;
    // errorText descrie cauza când success == false.
    void emailSent(bool success, const QString &errorText);

private:
    QString m_smtpServer;
    int     m_port;
    QString m_emailFrom;
    QString m_userName;
    QString m_password;
    QString m_emailTo;
    QString m_subiect;
    QString m_body;
    QStringList m_filesAttachments;
    std::shared_ptr<TemporaryExportOwner> m_exportOwner;
};

#endif // EMAILCORE_H
