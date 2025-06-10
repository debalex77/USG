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

    QPainter p(&pm);
    QFont f = p.font();
    f.setBold(true);
    f.setPointSize(14);

    #if defined(Q_OS_LINUX)
            f.setPointSize(20);
            p.setFont(f);
            p.drawText(96, 44, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
            f.setPointSize(11);
            p.setFont(f);
            p.drawText(464, 74, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
            p.drawText(536, 242, QCoreApplication::tr("autor:"));
            p.drawText(400, 259, USG_COMPANY_EMAIL);
            f.setPointSize(9);
            p.setFont(f);
            p.drawText(290, 310, "2021 - " + QString::number(QDate::currentDate().year())
                                     + QCoreApplication::tr(" a."));
    #elif defined(Q_OS_WIN)
            f.setPointSize(18);
            p.setFont(f);
            p.drawText(132, 48, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
            f.setPointSize(11);
            p.setFont(f);
            p.drawText(464, 68, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
            p.drawText(534, 242, QCoreApplication::tr("autor:"));
            p.drawText(400, 259, USG_COMPANY_EMAIL);
            f.setPointSize(9);
            p.setFont(f);
            p.drawText(290,310, "2021 - " + QString::number(QDate::currentDate().year())
                                     + QCoreApplication::tr(" a."));
    #elif defined(Q_OS_MACOS)
            f.setPointSize(22);
            p.setFont(f);
            p.drawText(182, 44, QCoreApplication::tr("USG - Evidența examinărilor ecografice"));
            f.setPointSize(13);
            p.setFont(f);
            p.drawText(468, 74, QCoreApplication::tr("versiunea ") + USG_VERSION_FULL);
            p.drawText(530, 242, QCoreApplication::tr("autor:"));
            p.drawText(408, 259, USG_COMPANY_EMAIL);
            f.setPointSize(10);
            p.setFont(f);
            p.drawText(290,310,"2021 - " + QString::number(QDate::currentDate().year())
                                     + QCoreApplication::tr(" a."));
    #endif

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
