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

#include "processingaction.h"
#include "ui_processingaction.h"

ProcessingAction::ProcessingAction(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProcessingAction)
{
    ui->setupUi(this);

    setWindowTitle(tr("Procesarea ..."));
    QMovie *movie = new QMovie(":/img/Spinner200px-200px.gif");
    movie->setScaledSize(ui->loader->size()); // Setează dimensiunea GIF-ului la QLabel
    ui->loader->setMovie(movie);
    movie->start();

    // Șterge movie când QLabel este distrus
    connect(ui->loader, &QObject::destroyed, movie, &QObject::deleteLater);
    connect(this, &ProcessingAction::txtInfoChanged, this, &ProcessingAction::slot_txtInfoChanged);
}

ProcessingAction::~ProcessingAction()
{
    delete ui;
}

QString ProcessingAction::getTxtInfo()
{
    return m_txtInfo;
}

void ProcessingAction::setTxtInfo(QString txtInfo)
{
    m_txtInfo = txtInfo;
    emit txtInfoChanged();
}

void ProcessingAction::slot_txtInfoChanged()
{
    if (m_txtInfo.isEmpty())
        return;

    ui->txt_info->setText(m_txtInfo);
}
