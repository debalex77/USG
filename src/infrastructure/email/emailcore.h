#ifndef EMAILCORE_H
#define EMAILCORE_H

#include <QObject>
#include <QCoreApplication>
#include <QSslSocket>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <core/loggingcategories.h>

class EmailCore : public QObject
{
    Q_OBJECT
public:
    explicit EmailCore(QObject *parent = nullptr);
    ~EmailCore();

    void setEmailData(const QString &smtpServer, int port, const QString &emailFrom, const QString &userName,
                      const QString &password, const QString &emailTo, const QString &subiect,
                      const QString &body, const QStringList &attachments);

    // Verifică conexiunea SSL și autentificarea SMTP fără a trimite mesaj.
    static bool testConnection(const QString &smtpServer, int port,
                               const QString &userName, const QString &password,
                               int timeoutMs, QString *error);

public slots:
    void sendEmail();

signals:
    void emailSent(bool success);  // Semnal când emailul s-a trimis

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
};

#endif // EMAILCORE_H
