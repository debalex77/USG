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
#include "data/loggingcategories.h"

DownloaderVersion::DownloaderVersion(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &DownloaderVersion::onResult);
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
    // daca eroarea
    if(reply->error()){
        // prezentam eroarea
        qInfo(logInfo()) << "ERROR";
        qInfo(logInfo()) << reply->errorString();
    } else {
        // in caz contrar cream fisierul
        QDir dir;
        QString path_file_version = dir.toNativeSeparators(QDir::tempPath() + "/usg_version.txt");
        QFile *file = new QFile(path_file_version);
        if (file->exists()){
#if defined(Q_OS_LINUX)
            QString str_cmd = "rm " + path_file_version;
            system(str_cmd.toStdString().c_str());
#elif defined (Q_OS_WIN)
            QString str_cmd = "del " + path_file_version;
            system(str_cmd.toStdString().c_str());
#endif
        }

        if(file->open(QFile::WriteOnly)){
            file->write(reply->readAll());  // scrim datele in fisier
            file->close();                  // inchidem fisier
            qInfo(logInfo()) << "Downloading is completed";
            emit onReady(); // emitem signal
        }
    }
}
