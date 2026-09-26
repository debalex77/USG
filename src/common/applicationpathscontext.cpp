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

#include "applicationpathscontext.h"

#include <QDir>
#include <QFileInfo>
#include <QReadLocker>
#include <QStandardPaths>
#include <QWriteLocker>

ApplicationPathsContext &ApplicationPathsContext::instance()
{
    static ApplicationPathsContext context;
    return context;
}

ApplicationPathsData ApplicationPathsContext::data() const
{
    const QReadLocker locker(&m_lock);
    return m_data;
}

void ApplicationPathsContext::setData(const ApplicationPathsData &data)
{
    const QWriteLocker locker(&m_lock);
    m_data = data;
}

void ApplicationPathsContext::setSettingsFilePath(const QString &path)
{
    const QWriteLocker locker(&m_lock);
    m_data.settingsFilePath = QDir::toNativeSeparators(path);
}

void ApplicationPathsContext::clear()
{
    setData({});
}

QString ApplicationPathsContext::configDirectory() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (QFileInfo(path).fileName().compare(QStringLiteral("USG"), Qt::CaseInsensitive) != 0) {
        path = QDir(path).filePath(QStringLiteral("USG"));
    }
    return QDir::toNativeSeparators(QDir::cleanPath(path));
}

QString ApplicationPathsContext::uiSettingsDirectory() const
{
    return QDir::toNativeSeparators(
        QDir(configDirectory()).filePath(QStringLiteral("settings"))
    );
}

QString ApplicationPathsContext::startupSettingsFilePath() const
{
    return QDir::toNativeSeparators(
        QDir(uiSettingsDirectory()).filePath(QStringLiteral("startup.ini"))
    );
}

QString ApplicationPathsContext::tableSettingsFilePath() const
{
    return QDir::toNativeSeparators(
        QDir(uiSettingsDirectory()).filePath(QStringLiteral("table_settings.json"))
    );
}

QString ApplicationPathsContext::reportSettingsFilePath() const
{
    return QDir::toNativeSeparators(
        QDir(uiSettingsDirectory()).filePath(QStringLiteral("report_settings.json"))
    );
}

QString ApplicationPathsContext::exportDirectory() const
{
    return QDir::toNativeSeparators(
        QDir(QDir::tempPath()).filePath(QStringLiteral("USG"))
    );
}
