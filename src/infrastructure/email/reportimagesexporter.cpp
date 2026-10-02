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

#include "reportimagesexporter.h"

#include <core/loggingcategories.h>

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace {
// QImageReader::imageFormat() nu are supraîncărcare pentru QByteArray:
// fără QBuffer datele ar fi convertite implicit în QString (nume de fișier).
QByteArray imageFormatOf(QByteArray data)
{
    QBuffer buffer(&data);
    if (!buffer.open(QIODevice::ReadOnly))
        return {};
    return QImageReader::imageFormat(&buffer);
}
}

QString ReportImagesExporter::fileSafeDocumentNumber(QString number)
{
    number.replace('/', '_');
    number.replace('\\', '_');
    number.replace(':', '_');
    return number;
}

ReportImagesExporter::Result ReportImagesExporter::exportImages(QSqlDatabase &connection,
                                                                const Request &request,
                                                                const Progress &progress)
{
    const auto report = [&progress](const QString &text) {
        if (progress)
            progress(text);
    };

    Result result;
    report(tr("Se pregătește exportul imaginilor ..."));

    QSqlQuery qry(connection);
    qry.prepare(R"(
        SELECT
            image_1,
            image_2,
            image_3,
            image_4,
            image_5
        FROM
            imagesReports
        WHERE
            id_orderEcho = ? AND
            id_reportEcho = ?
    )");
    qry.addBindValue(request.orderId);
    qry.addBindValue(request.reportId);
    if (!qry.exec()) {
        qCritical(logCritical()) << "[ReportImagesExporter] Eroare la selectare din tabela 'imagesReports':"
                                 << qry.lastError().text();
        result.success = false;
        result.errors << tr("Imaginile atașate nu au putut fi citite: %1")
                             .arg(qry.lastError().text());
        return result;
    }

    qInfo(logInfo()) << "[ReportImagesExporter] Se inițializează exportul imaginilor documentului 'Raport ecografic' nr."
                     << request.reportNumber;

    if (!qry.next())
        return result; // raportul nu are imagini atașate

    const QSqlRecord rec = qry.record();
    const QVector<QString> imageNames = {"image_1", "image_2", "image_3", "image_4", "image_5"};

    for (int i = 0; i < imageNames.size(); ++i) {
        const QByteArray stored = qry.value(rec.indexOf(imageNames.at(i))).toByteArray();
        if (stored.isEmpty())
            continue;

        QByteArray imageData = stored;
        QByteArray format = imageFormatOf(imageData);
        if (format.isEmpty()) {
            const QByteArray decoded = QByteArray::fromBase64(
                stored, QByteArray::AbortOnBase64DecodingErrors);
            const QByteArray decodedFormat = imageFormatOf(decoded);
            if (!decodedFormat.isEmpty()) {
                imageData = decoded;
                format = decodedFormat;
            }
        }

        if (format.isEmpty()) {
            result.success = false;
            const QString error = tr("Imaginea atașată nr.%1 a raportului nr.%2 nu are un format recunoscut.")
                                      .arg(i + 1)
                                      .arg(request.reportNumber);
            result.errors << error;
            qCritical(logCritical()) << "[ReportImagesExporter]" << error;
            continue;
        }

        const QString extension = QString::fromLatin1(format).toLower() == QStringLiteral("jpeg")
            ? QStringLiteral("jpg")
            : QString::fromLatin1(format).toLower();
        const QString filePath = QDir(request.directory).filePath(
            QStringLiteral("Image_report_%1_nr_%2.%3")
                .arg(fileSafeDocumentNumber(request.reportNumber),
                     QString::number(i + 1), extension));
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly) || file.write(imageData) != imageData.size()) {
            result.success = false;
            const QString error = tr("Imaginea atașată nu a putut fi salvată: %1")
                                      .arg(QDir::toNativeSeparators(filePath));
            result.errors << error;
            qCritical(logCritical()) << "[ReportImagesExporter]" << error
                                     << file.errorString();
            continue;
        }
        file.close();
        result.files << filePath;
        qInfo(logInfo()) << "[ReportImagesExporter] Exportul cu succes al imaginii -" << filePath;
        report(tr("Imagine salvată: %1").arg(filePath));
    }

    if (result.success)
        report(tr("Imaginile sunt salvate cu succes ..."));
    return result;
}
