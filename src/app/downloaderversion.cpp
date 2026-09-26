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

#include "downloaderversion.h"
#include "core/loggingcategories.h"
#include <QScopeGuard>

DownloaderVersion::DownloaderVersion(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &DownloaderVersion::onResult);
}

void DownloaderVersion::getData()
{
    QUrl url(GITHUB_URL);    // URL
    QNetworkRequest request; // trimitem solicitarea
    request.setUrl(url);     // setam URL in solicitarea
    manager->get(request);   // solicitarea propriu-zisa
}

void DownloaderVersion::onResult(QNetworkReply *reply)
{
    const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });

    // daca eroarea
    if(reply->error()){
        // prezentam eroarea
        qInfo(logInfo()) << "ERROR";
        qInfo(logInfo()) << reply->errorString();
    } else {
        // in caz contrar cream fisierul
        QDir dir;
        QString path_file_version = dir.toNativeSeparators(QDir::tempPath() + "/usg_version.txt");
        QFile file(path_file_version);
        if (file.exists() && !QFile::remove(path_file_version)) {
            qWarning(logWarning()) << "Cannot replace downloaded version file:"
                                   << path_file_version;
            return;
        }

        if(file.open(QFile::WriteOnly)){
            file.write(reply->readAll());  // scrim datele in fisier
            file.close();                  // inchidem fisier
            qInfo(logInfo()) << "Downloading is completed";
            emit onReady(); // emitem signal
        } else {
            qWarning(logWarning()) << "Cannot write downloaded version file:"
                                   << file.errorString();
        }
    }
}
