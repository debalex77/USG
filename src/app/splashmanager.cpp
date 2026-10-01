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

#include "splashmanager.h"

#include <QSplashScreen>
#include <QElapsedTimer>
#include <QPixmap>
#include <QPainter>
#include <QFontMetrics>
#include <QCoreApplication>
#include <QDate>

void SplashManager::show(QWidget &mainWindow, int durationMs)
{
    QPixmap pm;
    QDate current_date = QDate::currentDate();
    int current_year = current_date.year();
    if (current_date >= QDate(current_year, 12, 15) || current_date <= QDate(current_year, 01, 15))
        pm.load(":/icons/splash_santa.png");
    else if (current_date >= QDate(current_date.year(), 12, 1)
             || current_date <= QDate(current_year, 03, 01).addDays(-1))
        pm.load(":/icons/splash_snow.png");
    else
        pm.load(":/icons/usg_splash.png");

    if (pm.isNull()) {
        mainWindow.show();
        return;
    }

    // Pozițiile pe X se calculează după lățimea reală a textului (traducerile
    // au lungimi diferite): toate textele se aliniază la aceeași margine dreaptă.
    #if defined(Q_OS_MACOS)
    constexpr int titlePointSize = 22;
    constexpr int infoPointSize  = 13;
    constexpr int yearPointSize  = 10;
    constexpr int versionY       = 74;
    #elif defined(Q_OS_WIN)
    constexpr int titlePointSize = 20;
    constexpr int infoPointSize  = 11;
    constexpr int yearPointSize  = 9;
    constexpr int versionY       = 70;
    #else
    constexpr int titlePointSize = 20;
    constexpr int infoPointSize  = 11;
    constexpr int yearPointSize  = 9;
    constexpr int versionY       = 74;
    #endif
    constexpr int minTitlePointSize = 12;
    constexpr int rightMargin       = 20;

    const int rightEdge     = pm.width() - rightMargin;
    const int maxTitleWidth = pm.width() - 2 * rightMargin;

    QPainter p(&pm);
    QFont f = p.font();
    f.setBold(true);

    const auto drawRightAligned = [&p, rightEdge](int y, const QString &text) {
        p.drawText(rightEdge - p.fontMetrics().horizontalAdvance(text), y, text);
    };

    // titlu – aliniat la dreapta; se micșorează fontul dacă textul nu încape
    const QString title = QCoreApplication::tr("USG - Evidența examinărilor ecografice");
    int titleSize = titlePointSize;
    f.setPointSize(titleSize);
    p.setFont(f);
    while (titleSize > minTitlePointSize
           && p.fontMetrics().horizontalAdvance(title) > maxTitleWidth) {
        f.setPointSize(--titleSize);
        p.setFont(f);
    }
    drawRightAligned(44, title);

    // versiune, autor – aliniate la dreapta
    f.setPointSize(infoPointSize);
    p.setFont(f);
    drawRightAligned(versionY, QCoreApplication::tr("versiunea ") + VERSION_FULL);
    drawRightAligned(242, QCoreApplication::tr("autor:"));
    drawRightAligned(259, COMPANY_EMAIL);

    f.setPointSize(yearPointSize);
    p.setFont(f);
    p.drawText(290, 310, "2021 - " + QString::number(QDate::currentDate().year())
                             + QCoreApplication::tr(" a."));

    p.end();

    QSplashScreen splash(pm);
    splash.show();

    QElapsedTimer t;
    t.start();

    while (t.elapsed() < durationMs) {
        int pr = static_cast<int>(t.elapsed() * 100.0 / durationMs);
        splash.showMessage(QObject::tr("Încărcat: %1%" )
                               .arg(pr), Qt::AlignBottom|Qt::AlignRight);
        QCoreApplication::processEvents();
    }
    splash.finish(&mainWindow);
}
