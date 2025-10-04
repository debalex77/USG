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

#include "lineeditcustom.h"

LineEditCustom::LineEditCustom(QWidget *parent) : QLineEdit(parent)
{
    m_actionAddItem = this->addAction(QIcon(":/img/add_x32.png"), QLineEdit::ActionPosition::TrailingPosition);
    m_actionAddItem->setToolTip(tr("Adaug\304\203 \303\256n lista cu \310\231abloane"));
    connect(m_actionAddItem, &QAction::triggered, this, &LineEditCustom::onClickAddItem);

    m_actionOpenList = this->addAction(QIcon(":/img/select.png"), QLineEdit::ActionPosition::TrailingPosition);
    m_actionOpenList->setToolTip(tr("Selecteaz\304\203 din lista cu \310\231abloane"));
    connect(m_actionOpenList, &QAction::triggered, this, &LineEditCustom::onClickSelect);
}

LineEditCustom::~LineEditCustom()
{

}

QAction *LineEditCustom::actionAddItem() const
{
    return m_actionAddItem;
}

QAction *LineEditCustom::actionOpenList() const
{
    return m_actionOpenList;
}
