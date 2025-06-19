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

#include "infowindow.h"
#include <QScreen>
#include "ui_infowindow.h"

InfoWindow::InfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow)
{
    ui->setupUi(this);

    appSettings = new AppSettings(this);

    ui->txt_title->clear();
    ui->textBrowser->clear();

    (globals().show_content_info_video) ? ui->notShow->setChecked(false)
                                       : ui->notShow->setChecked(true);

    ui->textBrowser->setFocus();

    connect(this, &InfoWindow::typeInfoChanged, this, &InfoWindow::slot_typeInfoChanged);

    // ********************************************************
    // resize and move windows

    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    this->resize(680, 420);
    int x = (screenWidth / 2) - (width() / 2);   //*0.1;
    int y = (screenHeight / 2) - (height() / 2); //*0.1;
    move(x, y);
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

InfoWindow::TypeInfo InfoWindow::getTypeInfo()
{
    return m_typeInfo;
}

void InfoWindow::setTypeInfo(TypeInfo typeInfo)
{
    m_typeInfo = typeInfo;
    emit typeInfoChanged();
}

void InfoWindow::setTitle(const QString title)
{
    ui->txt_title->setText(title);
}

void InfoWindow::setTex(const QString content)
{
    QString str_style = globals().isSystemThemeDark
                            ? "<div style='color:#a6a6a6;'>"
                            : "<div>";
    QString str;
    str.append(str_style);
    str.append(content);
    str.append("</div>");

    if (m_typeInfo == INFO_REALEASE) {
        ui->textBrowser->setMarkdown(content);
    } else if (m_typeInfo == INFO_VIDEO || m_typeInfo == INFO_REPORT) {
        ui->textBrowser->setHtml(str);
    }
}

void InfoWindow::slot_typeInfoChanged()
{
    if (m_typeInfo == INFO_REALEASE) {
        setWindowTitle(tr("Istoria versiunilor"));
        ui->txt_title->setText(tr("Descrierea versiunilor"));
        ui->label_image->setPixmap(QPixmap(QString::fromUtf8(":/img/history.png")));
        ui->label_image->setMinimumSize(QSize(32, 32));
        ui->label_image->setMaximumSize(QSize(32, 32));
        ui->notShow->setVisible(false);
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(11);
        ui->textBrowser->setFont(font);
        ui->textBrowser->setOpenExternalLinks(true);
    }
}

void InfoWindow::closeEvent(QCloseEvent *event)
{
    if (m_typeInfo == INFO_REALEASE) {
        appSettings->deleteLater();
        event->accept();
    } else {
        if (m_typeInfo == INFO_VIDEO) {
            globals().show_content_info_video = !ui->notShow->isChecked();
            appSettings->setKeyAndValue("show_msg",
                                        "showMsgVideo",
                                        (globals().show_content_info_video) ? 1 : 0);
        } else if (m_typeInfo == INFO_REPORT) {
            globals().show_info_reports = !ui->notShow->isChecked();
            appSettings->setKeyAndValue("show_msg",
                                        "showMsgReports",
                                        (globals().show_info_reports) ? 1 : 0);
        };

        appSettings->deleteLater();

        event->accept();
    }
}
