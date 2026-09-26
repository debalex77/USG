/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "emailcore.h"
#include "core/loggingcategories.h"

#include <QFileInfo>
#include <QRegularExpression>

namespace {

// Citește un răspuns SMTP complet (inclusiv multi-linie: "250-...", "250 ...")
// și returnează codul din ultima linie.
int readSmtpReply(QSslSocket &socket, int timeoutMs, QByteArray *reply)
{
    static const QRegularExpression lastLine(QStringLiteral(R"((?:^|\r\n)(\d{3}) [^\r\n]*\r\n$)"));
    reply->clear();
    while (true) {
        const QRegularExpressionMatch match = lastLine.match(QString::fromUtf8(*reply));
        if (match.hasMatch())
            return match.captured(1).toInt();
        if (!socket.waitForReadyRead(timeoutMs))
            return -1;
        reply->append(socket.readAll());
    }
}

} // namespace

bool EmailCore::testConnection(const QString &smtpServer, int port,
                               const QString &userName, const QString &password,
                               int timeoutMs, QString *error)
{
    const auto fail = [error](const QString &text) {
        if (error)
            *error = text;
        qWarning(logWarning()) << "Verificarea conexiunii SMTP a eșuat:" << text;
        return false;
    };

    QSslSocket socket;
    socket.connectToHostEncrypted(smtpServer, port);
    if (!socket.waitForEncrypted(timeoutMs))
        return fail(QCoreApplication::translate("EmailCore",
                                                "Conexiunea securizată cu %1:%2 nu a putut fi stabilită: %3")
                        .arg(smtpServer).arg(port).arg(socket.errorString()));

    QByteArray reply;
    const auto step = [&](const QByteArray &command, int expectedCode, const QString &stage) {
        if (!command.isEmpty()) {
            socket.write(command);
            socket.flush();
        }
        const int code = readSmtpReply(socket, timeoutMs, &reply);
        if (code == expectedCode)
            return true;
        const QString details = code < 0
            ? QCoreApplication::translate("EmailCore", "serverul nu a răspuns")
            : QString::fromUtf8(reply).trimmed();
        return fail(QStringLiteral("%1: %2").arg(stage, details));
    };

    if (!step({}, 220, QStringLiteral("Greeting"))
        || !step("EHLO localhost\r\n", 250, QStringLiteral("EHLO"))
        || !step("AUTH LOGIN\r\n", 334, QStringLiteral("AUTH LOGIN"))
        || !step(userName.toUtf8().toBase64() + "\r\n", 334,
                 QCoreApplication::translate("EmailCore", "Utilizator"))
        || !step(password.toUtf8().toBase64() + "\r\n", 235,
                 QCoreApplication::translate("EmailCore", "Autentificare"))) {
        socket.abort();
        return false;
    }

    socket.write("QUIT\r\n");
    socket.flush();
    readSmtpReply(socket, timeoutMs, &reply);
    socket.disconnectFromHost();

    qInfo(logInfo()) << "Verificarea conexiunii SMTP reușită:" << smtpServer << port;
    return true;
}

EmailCore::EmailCore(QObject *parent)
    : QObject{parent}
{

}

EmailCore::~EmailCore()
{

}

void EmailCore::setEmailData(const QString &smtpServer, int port, const QString &emailFrom, const QString &userName,
                             const QString &password, const QString &emailTo, const QString &subiect,
                             const QString &body, const QStringList &attachments)
{
    m_smtpServer = smtpServer;
    m_port       = port;
    m_emailFrom  = emailFrom;
    m_userName   = userName;
    m_password   = password;
    m_emailTo    = emailTo;
    m_subiect    = subiect;
    m_body       = body;
    m_filesAttachments = attachments;
}

/*****************************************************************
**
** Funcţia de trimitere email cu ataşamentul PDF.
** Parametrii:
** - smtpServer: adresa serverului SMTP (ex: "smtp.gmail.com")
** - port:       portul de conectare (ex: 465 pentru SSL)
** - username:   contul de email (ex: "exemplu@gmail.com")
** - password:   parola contului (sau tokenul de autentificare)
** - from:       adresa expeditorului
** - to:         adresa destinatarului
** - subject:    subiectul mesajului
** - body:       textul mesajului
** - pdfPath:    calea completă către fişierul PDF ce se atașează
**
******************************************************************/
void EmailCore::sendEmail()
{
    qInfo(logInfo()) << "Se trimite emailul...";

    // Creăm un socket SSL pentru a stabili conexiunea securizată.
    QSslSocket socket;

    // Conectăm socket-ul la serverul SMTP folosind conexiune criptată.
    socket.connectToHostEncrypted(m_smtpServer, m_port);
    if (! socket.waitForConnected(10000)) {
        qCritical(logCritical()) << "Eroare la conectare:" << socket.errorString();
        return;
    }

    // Așteptăm mesajul de bun venit de la server.
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit greeting de la server";
        return;
    }
    QByteArray response = socket.readAll();
    qInfo(logInfo()) << "Greeting de la server:" << response;

    // Trimiterea comenzii EHLO pentru a iniţia conversaţia SMTP.
    QString ehloCommand = "EHLO localhost\r\n";
    socket.write(ehloCommand.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la EHLO";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns EHLO:" << response;

    // Pornim autentificarea cu AUTH LOGIN.
    QString authCommand = "AUTH LOGIN\r\n";
    socket.write(authCommand.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la AUTH LOGIN";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns AUTH LOGIN:" << response;

    // Trimiterea username-ului în format Base64.
    QByteArray usernameEncoded = m_userName.toUtf8().toBase64();
    socket.write(usernameEncoded + "\r\n");
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns după username";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns după username:" << response;

    // Trimiterea parolei în format Base64.
    QByteArray passwordEncoded = m_password.toUtf8().toBase64();
    socket.write(passwordEncoded + "\r\n");
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns după parolă";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns după parolă:" << response;

    // Specificăm adresa expeditorului.
    QString mailFrom = QString("MAIL FROM:<%1>\r\n").arg(m_emailFrom);
    socket.write(mailFrom.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la MAIL FROM";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns MAIL FROM:" << response;

    // Specificăm adresa destinatarului.
    QString rcptTo = QString("RCPT TO:<%1>\r\n").arg(m_emailTo);
    socket.write(rcptTo.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la RCPT TO";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns RCPT TO:" << response;

    // Comanda DATA pentru a începe trimiterea conținutului email-ului.
    QString dataCommand = "DATA\r\n";
    socket.write(dataCommand.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la DATA";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns DATA:" << response;

    // Creăm mesajul MIME ce va conține corpul text și atașamentul PDF.
    // Stabilim un boundary unic pentru delimitarea părţilor.
    QString boundary = "----=_Part_boundary_123456789";
    QByteArray message;
    QTextStream stream(&message, QIODevice::WriteOnly);

    // Header-ele email-ului.
    stream << "From: " << m_emailFrom << "\r\n";
    stream << "To: " << m_emailTo << "\r\n";
    stream << "Subject: " << m_subiect << "\r\n";
    stream << "MIME-Version: 1.0\r\n";
    stream << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n";
    stream << "\r\n";
    stream << "Acesta este un mesaj multipart în format MIME.\r\n";
    stream << "\r\n";

    // Partea 1: Corpul text al email-ului.
    stream << "--" << boundary << "\r\n";
    stream << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
    stream << "Content-Transfer-Encoding: 7bit\r\n";
    stream << "\r\n";
    stream << m_body << "\r\n";
    stream << "\r\n";

    // Partea 2: Atașamente multiple
    for (const QString &filePath : std::as_const(m_filesAttachments)) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Nu pot deschide fișierul:" << filePath;
            // Poți alege să returnezi false sau să continui cu următorul fișier.
            continue;
        }
        QByteArray fileData = file.readAll();
        file.close();
        QByteArray fileEncoded = fileData.toBase64();

        // Obţinem numele fişierului din calea completă
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();

        stream << "--" << boundary << "\r\n";
        // Poţi schimba tipul MIME în funcţie de extensie; folosim application/octet-stream ca fallback.
        stream << "Content-Type: application/octet-stream; name=\"" << fileName << "\"\r\n";
        stream << "Content-Transfer-Encoding: base64\r\n";
        stream << "Content-Disposition: attachment; filename=\"" << fileName << "\"\r\n";
        stream << "\r\n";
        // Scriem datele codificate în linii de 76 de caractere
        for (int i = 0; i < fileEncoded.size(); i += 76) {
            stream << fileEncoded.mid(i, 76) << "\r\n";
        }
        stream << "\r\n";
    }

    // Închidem secțiunea MIME.
    stream << "--" << boundary << "--\r\n";
    stream << "\r\n";

    // Semnalăm sfârșitul datelor cu o linie care conține doar un punct.
    stream << ".\r\n";
    stream.flush();

    // Trimiterea mesajului complet către server.
    socket.write(message);
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns după trimiterea email-ului";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns după trimiterea mesajului:" << response;

    // Comanda QUIT pentru a închide conexiunea.
    QString quitCommand = "QUIT\r\n";
    socket.write(quitCommand.toUtf8());
    socket.flush();
    if (! socket.waitForReadyRead(10000)) {
        qWarning(logWarning()) << "Nu am primit răspuns la QUIT";
        return;
    }
    response = socket.readAll();
    qInfo(logInfo()) << "Răspuns QUIT:" << response;

    socket.disconnectFromHost();

    qInfo(logInfo()) << "Email trimis!";

    emit emailSent(true);
}
