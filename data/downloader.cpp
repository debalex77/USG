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

#include "downloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>

Downloader::Downloader(QObject* parent) :
    BaseClass(parent)
{
    connect(&m_manager, &QNetworkAccessManager::finished, this, &Downloader::onReply);
}

bool Downloader::get(const QString& targetFolder, const QUrl& url)
{
    if (targetFolder.isEmpty() || url.isEmpty())
        return false;

    m_file = new QFile(targetFolder + QDir::separator() + url.fileName());
    if (!m_file->open(QIODevice::WriteOnly)) {
        delete m_file;
        m_file = nullptr;
        return false;
    }

    QNetworkRequest request(url);
    // request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true); // permitem redirectionarea
    m_currentReply = m_manager.get(request);                               // lansam descarcarea

    // conectarea pu citirea datelor si progresului
    connect(m_currentReply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &Downloader::updateDownloadProgress);
    return true;
}

void Downloader::onReadyRead()
{
    if (m_file)
        m_file->write(m_currentReply->readAll());
}

void Downloader::cancelDownload()
{
    if (m_currentReply)
        m_currentReply->abort();
}

void Downloader::onReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        m_file->flush();
        m_file->close();
    } else {
        qInfo(logWarning()) << "ERROR";
        qInfo(logWarning()) << reply->errorString();
        m_file->remove();
    }

    delete m_file;
    m_file = nullptr;
    reply->deleteLater();

    qInfo(logInfo()) << "Downloading new version App is completed.";
    emit finishedDownload();
}
