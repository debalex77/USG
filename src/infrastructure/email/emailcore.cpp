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
#include "temporaryexportowner.h"

#include <algorithm>

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QUuid>
#include <QUrl>

namespace {

// Răspunsul final după DATA sosește abia după ce serverul primește tot
// mesajul (inclusiv atașamentele), de aceea are un timeout mai mare.
constexpr int commandTimeoutMs = 10000;
constexpr int dataTimeoutMs    = 120000;

QByteArray wrappedBase64(const QByteArray &data)
{
    const QByteArray encoded = data.toBase64();
    QByteArray result;
    for (qsizetype offset = 0; offset < encoded.size(); offset += 76) {
        result += encoded.mid(offset, 76);
        result += "\r\n";
    }
    return result;
}

QString encodedHeader(const QString &value)
{
    const QString sanitized = QString(value).replace('\r', ' ').replace('\n', ' ');
    return QStringLiteral("=?UTF-8?B?%1?=")
        .arg(QString::fromLatin1(sanitized.toUtf8().toBase64()));
}

QString asciiFileName(QString name)
{
    name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    return name.isEmpty() ? QStringLiteral("attachment") : name;
}

// Citește un răspuns SMTP complet (inclusiv multi-linie: "250-...", "250 ...")
// și returnează codul din ultima linie.
int readSmtpReply(QSslSocket &socket, int timeoutMs, QByteArray *reply)
{
    static const QRegularExpression lastLine(QStringLiteral(R"((?:^|\r\n)(\d{3}) [^\r\n]*\r\n$)"));
    reply->clear();
    while (true) {
        // Datele pot fi deja în buffer (ex. greeting-ul primit odată cu
        // handshake-ul TLS); waitForReadyRead() așteaptă doar date noi.
        reply->append(socket.readAll());
        const QRegularExpressionMatch match = lastLine.match(QString::fromUtf8(*reply));
        if (match.hasMatch())
            return match.captured(1).toInt();
        if (!socket.waitForReadyRead(timeoutMs))
            return -1;
    }
}

bool openSecureSmtpSession(QSslSocket &socket, const QString &host, int port,
                           int timeoutMs, QByteArray *reply, QString *error)
{
    const auto setError = [error](const QString &text) {
        if (error)
            *error = text;
        return false;
    };
    const auto expect = [&](const QByteArray &command, int expected, const QString &stage) {
        if (!command.isEmpty()) {
            socket.write(command);
            socket.flush();
        }
        const int code = readSmtpReply(socket, timeoutMs, reply);
        if (code == expected)
            return true;
        const QString details = code < 0
            ? QCoreApplication::translate("EmailCore", "serverul nu a răspuns")
            : QString::fromUtf8(*reply).trimmed();
        return setError(QStringLiteral("%1: %2").arg(stage, details));
    };

    if (port == 465) {
        socket.connectToHostEncrypted(host, port);
        if (!socket.waitForEncrypted(timeoutMs))
            return setError(QCoreApplication::translate(
                                "EmailCore",
                                "Conexiunea securizată cu %1:%2 nu a putut fi stabilită: %3")
                                .arg(host).arg(port).arg(socket.errorString()));
        return expect({}, 220, QStringLiteral("Greeting"))
               && expect("EHLO localhost\r\n", 250, QStringLiteral("EHLO"));
    }

    socket.connectToHost(host, port);
    if (!socket.waitForConnected(timeoutMs))
        return setError(QCoreApplication::translate(
                            "EmailCore",
                            "Conexiunea cu %1:%2 nu a putut fi stabilită: %3")
                            .arg(host).arg(port).arg(socket.errorString()));
    if (!expect({}, 220, QStringLiteral("Greeting"))
        || !expect("EHLO localhost\r\n", 250, QStringLiteral("EHLO")))
        return false;
    if (!reply->contains("STARTTLS"))
        return setError(QCoreApplication::translate(
            "EmailCore", "Serverul SMTP nu oferă extensia STARTTLS."));
    if (!expect("STARTTLS\r\n", 220, QStringLiteral("STARTTLS")))
        return false;

    socket.startClientEncryption();
    if (!socket.waitForEncrypted(timeoutMs))
        return setError(QCoreApplication::translate(
                            "EmailCore", "Negocierea TLS cu %1:%2 a eșuat: %3")
                            .arg(host).arg(port).arg(socket.errorString()));
    return expect("EHLO localhost\r\n", 250, QStringLiteral("EHLO TLS"));
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
    QByteArray reply;
    QString connectionError;
    if (!openSecureSmtpSession(socket, smtpServer, port, timeoutMs, &reply, &connectionError))
        return fail(connectionError);
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

    if (!step("AUTH LOGIN\r\n", 334, QStringLiteral("AUTH LOGIN"))
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

void EmailCore::setExportOwner(std::shared_ptr<TemporaryExportOwner> owner)
{
    m_exportOwner = std::move(owner);
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
    qInfo(logInfo()) << "[THREAD] Se trimite emailul...";

    QSslSocket socket;
    QByteArray reply;

    // Orice ramură de eroare trebuie să emită emailSent(false, ...), altfel
    // firul de trimitere nu se oprește, iar dialogul apelant rămâne blocat.
    const auto fail = [this, &socket](const QString &text) {
        qCritical(logCritical()) << "[THREAD] Trimiterea e-mail-ului a eșuat:" << text;
        socket.abort();
        emit emailSent(false, text);
    };

    const auto step = [&](const QByteArray &command,
                          std::initializer_list<int> expectedCodes,
                          const QString &stage,
                          int timeoutMs = commandTimeoutMs) {
        if (!command.isEmpty()) {
            socket.write(command);
            socket.flush();
        }
        const int code = readSmtpReply(socket, timeoutMs, &reply);
        if (std::find(expectedCodes.begin(), expectedCodes.end(), code) != expectedCodes.end()) {
            qInfo(logInfo()) << "[THREAD] SMTP" << stage << "->" << code;
            return true;
        }
        const QString details = code < 0
            ? QCoreApplication::translate("EmailCore", "serverul nu a răspuns")
            : QString::fromUtf8(reply).trimmed();
        fail(QStringLiteral("%1: %2").arg(stage, details));
        return false;
    };

    // Adresele intră direct în comenzile MAIL FROM / RCPT TO: CR/LF sau
    // parantezele unghiulare ar permite injectarea unor comenzi SMTP.
    static const QRegularExpression invalidAddressChars(QStringLiteral("[\\r\\n<>]"));
    if (m_emailFrom.contains(invalidAddressChars) || m_emailTo.contains(invalidAddressChars)) {
        fail(QCoreApplication::translate("EmailCore",
                                         "Adresa expeditorului sau a destinatarului conține caractere nepermise."));
        return;
    }

    // Mesajul MIME se construiește înainte de conectare: un atașament care
    // nu poate fi citit oprește trimiterea, nu este omis în tăcere.
    const QString boundary = QStringLiteral("----=_USG_%1")
                                 .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QByteArray message;
    QTextStream stream(&message, QIODevice::WriteOnly);

    // Header-ele email-ului.
    stream << "From: " << QString(m_emailFrom).replace('\r', ' ').replace('\n', ' ') << "\r\n";
    stream << "To: " << QString(m_emailTo).replace('\r', ' ').replace('\n', ' ') << "\r\n";
    stream << "Subject: " << encodedHeader(m_subiect) << "\r\n";
    stream << "Date: " << QDateTime::currentDateTime().toString(Qt::RFC2822Date) << "\r\n";
    QString messageDomain = m_emailFrom.section('@', 1, 1).trimmed();
    messageDomain.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9.-]")),
                          QString());
    if (messageDomain.isEmpty())
        messageDomain = QStringLiteral("localhost");
    stream << "Message-ID: <" << QUuid::createUuid().toString(QUuid::WithoutBraces)
           << "@" << messageDomain << ">\r\n";
    stream << "MIME-Version: 1.0\r\n";
    stream << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n";
    stream << "\r\n";
    stream << "Acesta este un mesaj multipart în format MIME.\r\n";
    stream << "\r\n";

    // Partea 1: Corpul text al email-ului.
    stream << "--" << boundary << "\r\n";
    stream << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
    stream << "Content-Transfer-Encoding: base64\r\n";
    stream << "\r\n";
    stream << wrappedBase64(m_body.toUtf8());
    stream << "\r\n";

    // Partea 2: Atașamente multiple
    for (const QString &filePath : std::as_const(m_filesAttachments)) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            fail(QCoreApplication::translate("EmailCore", "Atașamentul nu poate fi citit: %1 (%2)")
                     .arg(QDir::toNativeSeparators(filePath), file.errorString()));
            return;
        }
        const QByteArray fileData = file.readAll();
        file.close();

        const QString fileName = QFileInfo(filePath).fileName();
        const QString fallbackName = asciiFileName(fileName);
        const QString encodedName = QString::fromLatin1(QUrl::toPercentEncoding(fileName));
        const QString mimeType = QMimeDatabase()
                                     .mimeTypeForFile(filePath, QMimeDatabase::MatchExtension)
                                     .name();

        stream << "--" << boundary << "\r\n";
        stream << "Content-Type: " << (mimeType.isEmpty() ? QStringLiteral("application/octet-stream") : mimeType)
               << "; name=\"" << fallbackName << "\"\r\n";
        stream << "Content-Transfer-Encoding: base64\r\n";
        stream << "Content-Disposition: attachment; filename=\"" << fallbackName
               << "\"; filename*=UTF-8''" << encodedName << "\r\n";
        stream << "\r\n";
        stream << wrappedBase64(fileData);
        stream << "\r\n";
    }

    // Închidem secțiunea MIME și semnalăm sfârșitul datelor.
    stream << "--" << boundary << "--\r\n";
    stream << "\r\n";
    stream.flush();

    // SMTP dot-stuffing: orice linie MIME care începe cu punct primește încă
    // un punct; terminatorul DATA este adăugat numai după această transformare.
    if (message.startsWith('.'))
        message.prepend('.');
    message.replace("\r\n.", "\r\n..");
    message += ".\r\n";

    QString connectionError;
    if (!openSecureSmtpSession(socket, m_smtpServer, m_port,
                               commandTimeoutMs, &reply, &connectionError)) {
        fail(connectionError);
        return;
    }

    if (!step("AUTH LOGIN\r\n", {334}, QStringLiteral("AUTH LOGIN"))
        || !step(m_userName.toUtf8().toBase64() + "\r\n", {334}, QCoreApplication::translate("EmailCore", "Utilizator"))
        || !step(m_password.toUtf8().toBase64() + "\r\n", {235}, QCoreApplication::translate("EmailCore", "Autentificare"))
        || !step(QStringLiteral("MAIL FROM:<%1>\r\n").arg(m_emailFrom).toUtf8(),
                 {250}, QStringLiteral("MAIL FROM"))
        || !step(QStringLiteral("RCPT TO:<%1>\r\n").arg(m_emailTo).toUtf8(),
                 {250, 251}, QStringLiteral("RCPT TO"))
        || !step("DATA\r\n", {354}, QStringLiteral("DATA"))
        || !step(message, {250}, QCoreApplication::translate("EmailCore", "Transmiterea mesajului"), dataTimeoutMs)) {
        return;
    }

    // Mesajul a fost acceptat de server; un QUIT eșuat nu mai schimbă rezultatul.
    socket.write("QUIT\r\n");
    socket.flush();
    readSmtpReply(socket, commandTimeoutMs, &reply);
    socket.disconnectFromHost();

    qInfo(logInfo()) << "[THREAD] Email trimis!";

    emit emailSent(true, QString());
}
