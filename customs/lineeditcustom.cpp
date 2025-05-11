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
    QAction *action_add_item = this->addAction(QIcon(":/img/add_x32.png"), QLineEdit::ActionPosition::TrailingPosition);
    action_add_item->setToolTip(tr("Adaug\304\203 \303\256n lista cu \310\231abloane"));
    connect(action_add_item, &QAction::triggered, this, &LineEditCustom::onClickAddItem);

    QAction *action_open_list = this->addAction(QIcon(":/img/select.png"), QLineEdit::ActionPosition::TrailingPosition);
    action_open_list->setToolTip(tr("Selecteaz\304\203 din lista cu \310\231abloane"));
    connect(action_open_list, &QAction::triggered, this, &LineEditCustom::onClickSelect);
}

LineEditCustom::~LineEditCustom()
{

}
