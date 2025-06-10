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
#include <QTextStream>

QScopedPointer<QFile> LogManager::s_logFile;

void LogManager::init(const QString &path)
{
    if (path.isEmpty())
        return; // fallback

    s_logFile.reset(new QFile(path));

    if (s_logFile->open(QFile::Append | QFile::Text)) {
        qInstallMessageHandler(handler);
    }
}

void LogManager::handler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    if (!s_logFile || !s_logFile->isOpen())
        return;

    QTextStream out(s_logFile.data());
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

    switch (type) {
    case QtInfoMsg: out << "INF "; break;
    case QtDebugMsg: out << "DBG "; break;
    case QtWarningMsg: out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg: out << "FTL "; break;
    }

    out << ctx.category << ": " << msg << Qt::endl;
    out.flush();
}
