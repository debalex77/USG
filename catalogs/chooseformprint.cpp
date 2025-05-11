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

#include "chooseformprint.h"
#include "ui_chooseformprint.h"
#include <QGuiApplication>
#include <QScreen>

ChooseFormPrint::ChooseFormPrint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseFormPrint)
{
    ui->setupUi(this);

    setWindowTitle(tr("Alege forma de tipar"));

    QButtonGroup* groupBtn = new QButtonGroup(this);
    groupBtn->addButton(ui->btnComplex);
    groupBtn->addButton(ui->btnOrgasInternal);
    groupBtn->addButton(ui->btnUrinarySystem);
    groupBtn->addButton(ui->btnProstate);
    groupBtn->addButton(ui->btnGynecology);
    groupBtn->addButton(ui->btnBreast);
    groupBtn->addButton(ui->btnThyroid);
    groupBtn->addButton(ui->btnOB_0);
    groupBtn->addButton(ui->btnOB_1);
    groupBtn->addButton(ui->btnOB_2);
    ui->btnComplex->setChecked(true);

    connect(ui->btnOK, &QPushButton::clicked, this, &ChooseFormPrint::onAccept);
    connect(ui->btnClose, &QPushButton::clicked, this, &ChooseFormPrint::close);

    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;

    move(x, y);
}

ChooseFormPrint::~ChooseFormPrint()
{
    delete ui;
}

ChooseFormPrint::TypeFormPrint ChooseFormPrint::getTypeFormPrint()
{
    if (ui->btnComplex->isChecked())
        return complex;
    if (ui->btnOrgasInternal->isChecked())
        return organs_internal;
    if (ui->btnUrinarySystem->isChecked())
        return urinary_system;
    if (ui->btnProstate->isChecked())
        return prostate;
    if (ui->btnGynecology->isChecked())
        return gynecology;
    if (ui->btnBreast->isChecked())
        return breast;
    if (ui->btnThyroid->isChecked())
        return thyroid;
    if (ui->btnOB_0->isChecked())
        return gestation0;
    if (ui->btnOB_1->isChecked())
        return gestation1;
    if (ui->btnOB_2->isChecked())
        return gestation2;

    return complex;
}

void ChooseFormPrint::setTypeFormPrint(const TypeFormPrint typePrint)
{
    if (typePrint == complex)
        ui->btnComplex->setChecked(true);
    if (typePrint == organs_internal)
        ui->btnOrgasInternal->setChecked(true);
    if (typePrint == urinary_system)
        ui->btnUrinarySystem->setChecked(true);
    if (typePrint == prostate)
        ui->btnProstate->setChecked(true);
    if (typePrint == gynecology)
        ui->btnGynecology->setChecked(true);
    if (typePrint == breast)
        ui->btnBreast->setChecked(true);
    if (typePrint == thyroid)
        ui->btnThyroid->setChecked(true);
    if (typePrint == gestation0)
        ui->btnOB_0->setChecked(true);
    if (typePrint == gestation1)
        ui->btnOB_1->setChecked(true);
    if (typePrint == gestation2)
        ui->btnOB_2->setChecked(true);
}

void ChooseFormPrint::onAccept()
{
    emit mChooseFormPrint();
    QDialog::accept();
}
