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

#include "custommessage.h"
#include "ui_custommessage.h"

CustomMessage::CustomMessage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CustomMessage)
    , m_expandedIcon(globals().isSystemThemeDark
                         ? QIcon(QStringLiteral(":/img/common/arrow_down_dark.png"))
                         : QIcon(QStringLiteral(":/img/common/arrow_down_light.png")))
    , m_collapsedIcon(globals().isSystemThemeDark
                          ? QIcon(QStringLiteral(":/img/common/arrow_right_dark.png"))
                          : QIcon(QStringLiteral(":/img/common/arrow_right_light.png")))
{
    ui->setupUi(this);
    ui->detailedText->hide();
    ui->label_img->setPixmap(m_collapsedIcon.pixmap(QSize(16,16)));
    this->adjustSize();

    connect(ui->btnOK, &QToolButton::clicked,
            this, &CustomMessage::close, Qt::UniqueConnection);
    connect(ui->btnDetailed, &QToolButton::clicked,
            this, &CustomMessage::onShowDetailedText, Qt::UniqueConnection);
}

CustomMessage::~CustomMessage()
{
    delete ui;
}

void CustomMessage::setTextTitle(QString text)
{
    ui->txt_title->setText(text);
}

void CustomMessage::setDetailedText(QString text)
{
    ui->detailedText->setPlainText(text);
}

void CustomMessage::onShowDetailedText()
{
    if (ui->detailedText->isVisible()) {
        ui->detailedText->hide();
        ui->label_img->setPixmap(m_collapsedIcon.pixmap(QSize(16,16)));
        this->adjustSize();
    } else {
        ui->detailedText->show();
        ui->label_img->setPixmap(m_expandedIcon.pixmap(QSize(16,16)));
        this->adjustSize();
    }
}
