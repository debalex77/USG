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

#include "logmanager.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QTextStream>

#include <cstdio>

QScopedPointer<QFile> LogManager::s_logFile;
QMutex LogManager::s_mutex;
QtMessageHandler LogManager::s_previousHandler = nullptr;
bool LogManager::s_handlerInstalled = false;
QString LogManager::s_logPath;
QDate LogManager::s_activeDate;
int LogManager::s_retainedArchives = -1;

QDate LogManager::firstEntryDate(const QFileInfo &fileInfo)
{
    QFile file(fileInfo.filePath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        for (int lineNumber = 0; lineNumber < 20 && !stream.atEnd(); ++lineNumber) {
            const QString line = stream.readLine().trimmed();
            if (line.size() < 10)
                continue;

            const QDate date = QDate::fromString(line.left(10), Qt::ISODate);
            if (date.isValid())
                return date;
        }
    }

    return fileInfo.lastModified().date();
}

void LogManager::rotateActiveLog(const QFileInfo &activeLogInfo, QStringList &errors)
{
    if (!activeLogInfo.isFile())
        return;

    const QDate logDate = firstEntryDate(activeLogInfo);
    if (!logDate.isValid() || logDate >= QDate::currentDate())
        return;

    QDir directory(activeLogInfo.absolutePath());
    const QString dateSuffix = logDate.toString(QStringLiteral("dd.MM.yyyy"));
    QString archivedName = QStringLiteral("%1_%2.log").arg(activeLogInfo.completeBaseName(), dateSuffix);
    QString archivedPath = directory.filePath(archivedName);
    int suffix = 1;
    while (QFileInfo::exists(archivedPath)) {
        archivedName = QStringLiteral("%1_%2_%3.log")
                           .arg(activeLogInfo.completeBaseName(), dateSuffix)
                           .arg(suffix++);

        archivedPath = directory.filePath(archivedName);
    }

    QFile activeLog(activeLogInfo.absoluteFilePath());
    if (!activeLog.rename(archivedPath))
        errors.append(
            QObject::tr("Fișierul de logare nu a putut fi arhivat: %1 (%2)")
                .arg(activeLogInfo.absoluteFilePath(),
                     activeLog.errorString())
            );
}

void LogManager::removeExpiredArchives(const QFileInfo &activeLogInfo,
                                       int retainedArchives, QStringList &errors)
{
    if (retainedArchives < 0 || activeLogInfo.completeBaseName().isEmpty())
        return;

    QDir directory(activeLogInfo.absolutePath());
    directory.setFilter(QDir::Files | QDir::NoSymLinks);
    directory.setSorting(QDir::Time);

    const QRegularExpression archivePattern(
        QStringLiteral("^%1_[0-9]{1,2}\\.[0-9]{1,2}\\.[0-9]{4}(?:_[0-9]+)?\\.log$")
            .arg(QRegularExpression::escape(activeLogInfo.completeBaseName()))
        );

    int archiveIndex = 0;
    const QFileInfoList files = directory.entryInfoList();
    for (const QFileInfo &fileInfo : files) {
        if (!archivePattern.match(fileInfo.fileName()).hasMatch())
            continue;

        if (++archiveIndex > retainedArchives && !QFile::remove(fileInfo.filePath()))
            errors.append(
                QObject::tr("Fișierul de logare vechi nu a putut fi șters: %1")
                    .arg(fileInfo.filePath())
                );
    }
}

void LogManager::rotateIfDateChanged(const QDate &currentDate, QStringList &errors)
{
    if (s_logPath.isEmpty() || !s_activeDate.isValid() || currentDate == s_activeDate)
        return;

    if (s_logFile && s_logFile->isOpen()) {
        s_logFile->flush();
        s_logFile->close();
    }
    s_logFile.reset();

    const QFileInfo activeLogInfo(s_logPath);
    rotateActiveLog(activeLogInfo, errors);
    removeExpiredArchives(activeLogInfo, s_retainedArchives, errors);

    s_logFile.reset(new QFile(activeLogInfo.absoluteFilePath()));
    if (!s_logFile->open(QFile::Append | QFile::Text)) {
        errors.append(QObject::tr("Fișierul de logare nu a putut fi redeschis după rotație: %1 (%2)")
                          .arg(activeLogInfo.absoluteFilePath(), s_logFile->errorString()));
        s_logFile.reset();
    }

    // Evitam repetarea rotatiei la fiecare mesaj daca redeschiderea esueaza.
    s_activeDate = currentDate;
}

bool LogManager::init(const QString &path, int retainedArchives)
{
    if (path.isEmpty()) {
        qWarning("Calea fișierului de logare este goală; se păstrează doar ieșirea standard Qt.");
        return false;
    }

    const QFileInfo fileInfo(path);
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        qWarning().noquote()
            << QObject::tr("Directorul de logare nu a putut fi creat: %1")
                   .arg(fileInfo.absolutePath());
        return false;
    }

    QStringList errors;
    bool logOpened = false;
    {
        QMutexLocker locker(&s_mutex);
        // Inchidem fisierul curent inainte de o posibila redenumire la reinitializare.
        s_logFile.reset();
        s_logPath.clear();
        s_activeDate = QDate();
        s_retainedArchives = -1;
        rotateActiveLog(fileInfo, errors);
        removeExpiredArchives(fileInfo, retainedArchives, errors);

        s_logFile.reset(new QFile(fileInfo.absoluteFilePath()));
        if (!s_logFile->open(QFile::Append | QFile::Text)) {
            errors.append(QObject::tr("Fișierul de logare nu a putut fi deschis: %1 (%2)")
                              .arg(fileInfo.absoluteFilePath(),
                                   s_logFile->errorString()));
            s_logFile.reset();
        } else {
            logOpened    = true;
            s_logPath    = fileInfo.absoluteFilePath();
            s_activeDate = QDate::currentDate();
            s_retainedArchives = retainedArchives;
        }
    }

    if (logOpened) {
        const QtMessageHandler previous = qInstallMessageHandler(handler);
        QMutexLocker locker(&s_mutex);
        if (previous != handler) {
            s_previousHandler = previous;
            s_handlerInstalled = true;
        }
    }

    for (const QString &error : std::as_const(errors))
        qWarning().noquote() << error;

    return logOpened;
}

void LogManager::shutdown()
{
    QtMessageHandler previousHandler = nullptr;
    {
        QMutexLocker locker(&s_mutex);
        if (!s_handlerInstalled) {
            s_logFile.reset();
            return;
        }
        previousHandler = s_previousHandler;
    }

    // Oprim mesajele noi catre handler inainte de inchiderea fisierului.
    qInstallMessageHandler(previousHandler);

    QMutexLocker locker(&s_mutex);
    if (s_logFile && s_logFile->isOpen()) {
        s_logFile->flush();
        s_logFile->close();
    }
    s_logFile.reset();
    s_previousHandler = nullptr;
    s_handlerInstalled = false;
    s_logPath.clear();
    s_activeDate = QDate();
    s_retainedArchives = -1;
}

void LogManager::handler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    QtMessageHandler previousHandler = nullptr;
    QStringList rotationErrors;
    {
        QMutexLocker locker(&s_mutex);
        rotateIfDateChanged(QDate::currentDate(), rotationErrors);
        if (s_logFile && s_logFile->isOpen()) {
            QTextStream out(s_logFile.data());
            out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

            switch (type) {
            case QtInfoMsg: out     << "INF "; break;
            case QtDebugMsg: out    << "DBG "; break;
            case QtWarningMsg: out  << "WRN "; break;
            case QtCriticalMsg: out << "CRT "; break;
            case QtFatalMsg: out    << "FTL "; break;
            }

            out << ctx.category << ": " << msg << Qt::endl;
            out.flush();
        }
        previousHandler = s_previousHandler;
    }

    for (const QString &error : std::as_const(rotationErrors)) {
        const QByteArray encodedError = error.toLocal8Bit();
        std::fprintf(stderr, "%s\n", encodedError.constData());
    }
    if (!rotationErrors.isEmpty())
        std::fflush(stderr);

    // Pastram comportamentul standard Qt (Application Output/stderr).
    if (previousHandler)
        previousHandler(type, ctx, msg);
    else {
        const QByteArray formatted = qFormatLogMessage(type, ctx, msg).toLocal8Bit();
        std::fprintf(stderr, "%s\n", formatted.constData());
        std::fflush(stderr);
    }
}
