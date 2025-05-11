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

#include "choicecolumns.h"
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QToolButton>

ChoiceColumns::ChoiceColumns(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Alege colonițe"));

    createListWidget();
    createOtherWidgets();
    createLayout();
    createConnections();

    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    int x = (screenWidth / 2) - (318 / 2);
    int y = (screenHeight / 2) - (302 / 2);
    move(x, y);
}

void ChoiceColumns::highlightChecked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked)
        item->setBackground(QColor(255,255,178));
    else
        item->setBackground(QColor(255,255,255));
}

void ChoiceColumns::save()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        bool isChecked = (item->checkState() == Qt::Checked) ? true : false;
        switch (i) {
        case 0:
            show_attachedImages = isChecked;
            break;
        case 1:
            show_cardPayment = isChecked;
            break;
        case 2:
            show_numberDoc = isChecked;
            break;
        case 3:
            show_dateDoc = isChecked;
            break;
        case 4:
            show_idOrganization = isChecked;
            break;
        case 5:
            show_Organization = isChecked;
            break;
        case 6:
            show_idContract = isChecked;
            break;
        case 7:
            show_Contract = isChecked;
            break;
        case 8:
            show_idPacient = isChecked;
            break;
        case 9:
            show_pacient = isChecked;
            break;
        case 10:
            show_IDNP = isChecked;
            break;
        case 11:
            show_idUser = isChecked;
            break;
        case 12:
            show_user = isChecked;
            break;
        case 13:
            show_sum = isChecked;
            break;
        case 14:
            show_comment = isChecked;
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
    emit saveData();
}

void ChoiceColumns::setListWidget()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        switch (i) {
        case 0:
            item->setCheckState((show_attachedImages) ? Qt::Checked : Qt::Unchecked);
            break;
        case 1:
            item->setCheckState((show_cardPayment) ? Qt::Checked : Qt::Unchecked);
            break;
        case 2:
            item->setCheckState((show_numberDoc) ? Qt::Checked : Qt::Unchecked);
            break;
        case 3:
            item->setCheckState((show_dateDoc) ? Qt::Checked : Qt::Unchecked);
            break;
        case 4:
            item->setCheckState((show_idOrganization) ? Qt::Checked : Qt::Unchecked);
            break;
        case 5:
            item->setCheckState((show_Organization) ? Qt::Checked : Qt::Unchecked);
            break;
        case 6:
            item->setCheckState((show_idContract) ? Qt::Checked : Qt::Unchecked);
            break;
        case 7:
            item->setCheckState((show_Contract) ? Qt::Checked : Qt::Unchecked);
            break;
        case 8:
            item->setCheckState((show_idPacient) ? Qt::Checked : Qt::Unchecked);
            break;
        case 9:
            item->setCheckState((show_pacient) ? Qt::Checked : Qt::Unchecked);
            break;
        case 10:
            item->setCheckState((show_IDNP) ? Qt::Checked : Qt::Unchecked);
            break;
        case 11:
            item->setCheckState((show_idUser) ? Qt::Checked : Qt::Unchecked);
            break;
        case 12:
            item->setCheckState((show_user) ? Qt::Checked : Qt::Unchecked);
            break;
        case 13:
            item->setCheckState((show_sum) ? Qt::Checked : Qt::Unchecked);
            break;
        case 14:
            item->setCheckState((show_comment) ? Qt::Checked : Qt::Unchecked);
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
}

void ChoiceColumns::createListWidget()
{
    widget = new QListWidget;
    QStringList strList;
    strList << tr("imaginea atasata")
            << tr("achitarea cu card")
            << tr("numarul documentului")
            << tr("data documentului")
            << tr("(id) organizatiei")
            << tr("organizatia")
            << tr("(id) contractului")
            << tr("contract")
            << tr("(id) pacientului")
            << tr("pacientul")
            << tr("IDNP pacientului")
            << tr("(id) utilizatorului")
            << tr("utilizatorul")
            << tr("suma")
            << tr("cometariu");

    widget->addItems(strList);

    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void ChoiceColumns::createOtherWidgets()
{
    viewBox   = new QGroupBox(tr("Lista colonițelor"));
    buttonBox = new QDialogButtonBox;
    btnOK    = buttonBox->addButton(QDialogButtonBox::Ok);
    btnClose = buttonBox->addButton(QDialogButtonBox::Close);
}

void ChoiceColumns::createLayout()
{
    QSpacerItem *itemSpacer = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* horizontalLayout_btn = new QHBoxLayout;
    horizontalLayout_btn->addItem(itemSpacer);

    QVBoxLayout* viewLayout = new QVBoxLayout;
    viewLayout->addLayout(horizontalLayout_btn);
    viewLayout->addWidget(widget);
    viewBox->setLayout(viewLayout);

    QHBoxLayout* horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(buttonBox);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(viewBox);
    mainLayout->addLayout(horizontalLayout);

    setLayout(mainLayout);
}

void ChoiceColumns::createConnections()
{
    connect(btnOK, &QPushButton::clicked, this, &ChoiceColumns::save);
    connect(btnClose, &QPushButton::clicked, this, &ChoiceColumns::close);
}
