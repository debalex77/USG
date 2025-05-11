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

#include "combodelegate.h"
#include <QDebug>

ComboDelegate::ComboDelegate(QString str_qry, QObject *parent) :
    QItemDelegate(parent), m_str_qry(str_qry)
{
}

QWidget *ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    combo = new QComboBox(parent);
    combo->addItem(tr("<<- selectează ->>"), 0);
    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&ComboDelegate::setData));
    QSqlQuery query;
    query.prepare(m_str_qry);
    query.exec();

    while (query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0)); // query.value(0) - userData (id)
    }

    return combo;
}

void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);               // get the index of the text in the combobox
    const QString currentText = index.data(Qt::EditRole).toString(); // that matches the current value of the item
    const int cbIndex = cb->findText(currentText);   
    if (cbIndex >= 0)                                                // if it is valid, adjust the combobox
       cb->setCurrentIndex(cbIndex);
}

void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    model->setData(index, cb->currentText(), Qt::EditRole);
}

void ComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void ComboDelegate::setData(const int val)
{
    if (val == 0)
        return;
    emit commitData(combo);
}


