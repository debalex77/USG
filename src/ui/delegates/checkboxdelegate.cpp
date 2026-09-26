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

#include "checkboxdelegate.h"

#include <QCheckBox>
#include <QApplication>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *editor = new QCheckBox(parent);

    connect(editor, &QCheckBox::toggled, this, [this, editor]() {
        auto *that = const_cast<CheckBoxDelegate *>(this);
        emit that->commitData(editor);
        emit that->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
    });

    return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *cb = qobject_cast<QCheckBox *>(editor);
    if (!cb)
        return;

    cb->setChecked(index.model()->data(index, Qt::EditRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto *cb = qobject_cast<QCheckBox *>(editor);
    if (!cb || !model)
        return;

    model->setData(index, cb->isChecked(), Qt::EditRole);
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const Qt::CheckState state =
        static_cast<Qt::CheckState>(index.model()->data(index, Qt::CheckStateRole).toInt());

    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(
        QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x()
                               + option.rect.width() / 2
                               - checkbox_rect.width() / 2);

    checkboxstyle.state = QStyle::State_Enabled;
    checkboxstyle.state |= (state == Qt::Checked)
                               ? QStyle::State_On
                               : QStyle::State_Off;

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    //center
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                               option.rect.width()/2 - checkbox_rect.width()/2);

    editor->setGeometry(checkboxstyle.rect);
}
