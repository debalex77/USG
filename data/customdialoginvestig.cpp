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

#include "customdialoginvestig.h"
#include <QGuiApplication>
#include <QToolButton>
#include <QMessageBox>
#include <QScreen>
#include <customs/custommessage.h>

CustomDialogInvestig::CustomDialogInvestig(QWidget *parent):
    QDialog(parent)
{
    setWindowTitle(tr("Alege investigațiile"));

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

void CustomDialogInvestig::highlightChecked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked)
        item->setBackground(QColor(255,255,178));
    else
        item->setBackground(QColor(255,255,255));
}

void CustomDialogInvestig::save()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        bool isChecked = (item->checkState() == Qt::Checked) ? true : false;

        switch (i) {
        case t_organsInternal:
            m_organs_internal = isChecked;
            break;
        case t_urinarySystem:
            m_urinary_system = isChecked;
            break;
        case t_prostate:
            m_prostate = isChecked;
            break;
        case t_thyroide:
            m_thyroide = isChecked;
            break;
        case t_breast:
            m_breast = isChecked;
            break;
        case t_gynecology:
            m_gynecology = isChecked;
            break;
        case t_gestation0:
            m_gestation0 = isChecked;
            break;
        case t_gestation1:
            m_gestation1 = isChecked;
            break;
        case t_gestation2:
            m_gestation2 = isChecked;
            break;
        case t_lymphNodes:
            m_lymphNodes = isChecked;
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
    if (m_organs_internal == false &&
        m_urinary_system == false &&
        m_prostate == false &&
        m_thyroide == false &&
        m_breast == false &&
        m_gynecology == false &&
        m_gestation0 == false &&
        m_gestation1 == false &&
        m_gestation2 == false &&
        m_lymphNodes == false){
        CustomMessage *msgBox = new CustomMessage(this);
        msgBox->setWindowTitle(tr("Selectarea sistemului"));
        msgBox->setTextTitle(tr("Nu este selectat nici un sistem !!!<br> Bifati sistemul necesar."));
        msgBox->exec();
        msgBox->deleteLater();
        return;
    }
    QDialog::accept();
}

void CustomDialogInvestig::createListWidget()
{
    widget = new QListWidget;
    QStringList strList;
    strList << tr("organe interne")
            << tr("sistemul urinar")
            << tr("prostata")
            << tr("ginecologia")
            << tr("gl.mamare")
            << tr("tiroida")
            << tr("sarcina până la 11 săptămâni")
            << tr("sarcina 11-14 săptămâni")
            << tr("sarcina 15-40 săptămâni")
            << tr("țes.moi și gangl.limfatici");

    widget->addItems(strList);

    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void CustomDialogInvestig::createOtherWidgets()
{
    viewBox   = new QGroupBox(tr("Lista investigațiilor"));
    buttonBox = new QDialogButtonBox;
    btnOK    = buttonBox->addButton(QDialogButtonBox::Ok);
    btnClose = buttonBox->addButton(QDialogButtonBox::Close);
    btnOK->setMinimumWidth(80);
    btnOK->setMaximumWidth(80);
    btnClose->setMinimumWidth(80);
    btnClose->setMaximumWidth(80);
}

void CustomDialogInvestig::createLayout()
{
    QToolButton *btnCheck          = new QToolButton(this);
    QToolButton *btnUncheck        = new QToolButton(this);
    if (globals().isSystemThemeDark){
        btnCheck->setIcon(QIcon(":/img/check-mark.png"));
    } else {
        btnCheck->setIcon(QIcon(":/img/checked_checkbox.png"));
    }
    btnCheck->setToolButtonStyle(Qt::ToolButtonIconOnly);
    if (globals().isSystemThemeDark){
        btnUncheck->setIcon(QIcon(":/img/unchecked_dark.png"));
    } else {
        btnUncheck->setIcon(QIcon(":/img/unchecked_checkbox.png"));
    }
    btnUncheck->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QSpacerItem *itemSpacer = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* horizontalLayout_btn = new QHBoxLayout;
    horizontalLayout_btn->addItem(itemSpacer);
    horizontalLayout_btn->addWidget(btnCheck);
    horizontalLayout_btn->addWidget(btnUncheck);

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

    connect(btnCheck, &QAbstractButton::clicked, this, [this]()
    {
        QListWidgetItem* item = 0;
        for(int i = 0; i < widget->count(); ++i){
            item = widget->item(i);
            switch (i) {
            case t_organsInternal:
                item->setCheckState(Qt::Checked);
                break;
            case t_urinarySystem:
                item->setCheckState(Qt::Checked);
                break;
            case t_prostate:
                item->setCheckState(Qt::Checked);
                break;
            case t_thyroide:
                item->setCheckState(Qt::Checked);
                break;
            case t_breast:
                item->setCheckState(Qt::Checked);
                break;
            case t_gynecology:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation0:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation1:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation2:
                item->setCheckState(Qt::Checked);
                break;
            case t_lymphNodes:
                item->setCheckState(Qt::Checked);
                break;
            default:
                qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
            }
        }
    });

    connect(btnUncheck, &QAbstractButton::clicked, this, [this]()
    {
        QListWidgetItem* item = 0;
        for(int i = 0; i < widget->count(); ++i){
            item = widget->item(i);
            switch (i) {
            case t_organsInternal:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_urinarySystem:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_prostate:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_thyroide:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_breast:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gynecology:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation0:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation1:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation2:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_lymphNodes:
                item->setCheckState(Qt::Unchecked);
                break;
            default:
                qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
            }
        }
    });
}

void CustomDialogInvestig::createConnections()
{
    connect(this, &CustomDialogInvestig::t_organs_internalChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_urinary_systemChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_prostateChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gynecologyChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_breastChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_thyroideChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation0Changed, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation1Changed, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation2Changed, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_lymphNodesChanged, this, &CustomDialogInvestig::setListWidget);

    connect(btnOK, &QPushButton::clicked, this, &CustomDialogInvestig::save);
    connect(btnClose, &QPushButton::clicked, this, &CustomDialogInvestig::close);
}

void CustomDialogInvestig::setListWidget()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        switch (i) {
        case t_organsInternal:
            item->setCheckState((m_organs_internal) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_urinarySystem:
            item->setCheckState((m_urinary_system) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_prostate:
            item->setCheckState((m_prostate) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_thyroide:
            item->setCheckState((m_thyroide) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_breast:
            item->setCheckState((m_breast) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gynecology:
            item->setCheckState((m_gynecology) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation0:
            item->setCheckState((m_gestation0) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation1:
            item->setCheckState((m_gestation1) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation2:
            item->setCheckState((m_gestation2) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_lymphNodes:
            item->setCheckState((m_lymphNodes) ? Qt::Checked : Qt::Unchecked);
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
}
