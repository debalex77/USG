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

#include "lineeditopen.h"

LineEditOpen::LineEditOpen(QWidget *parent) : QLineEdit(parent)
{
    action_open = this->addAction(QIcon(":/img/open-search.png"), QLineEdit::ActionPosition::TrailingPosition);
    connect(action_open, &QAction::triggered, this, &LineEditOpen::onClickedButton);

    // Inițial ascunde acțiunea dacă textul este gol
    action_open->setVisible(!this->text().isEmpty());

    // Conectează semnalul textChanged la o funcție care schimbă vizibilitatea acțiunii
    connect(this, &QLineEdit::textChanged, this, &LineEditOpen::onTextChanged);
}

LineEditOpen::~LineEditOpen()
{

}

void LineEditOpen::onTextChanged(const QString &text)
{
    action_open->setVisible(! text.isEmpty());
}
