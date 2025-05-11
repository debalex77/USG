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
    //create the checkbox editor
    QCheckBox *editor = new QCheckBox(parent);
    return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //set if checked or not
    QCheckBox *cb = qobject_cast<QCheckBox *>(editor);
    cb->setChecked(index.data().toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QCheckBox *cb = static_cast<QCheckBox *>(editor);
    int value = (cb->checkState()==Qt::Checked)?1:0;
    model->setData(index, value, Qt::EditRole);
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //retrieve data
     bool data = index.model()->data(index, Qt::DisplayRole).toBool();

     //create CheckBox style
     QStyleOptionButton checkboxstyle;
     QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

     //center
     checkboxstyle.rect = option.rect;
     checkboxstyle.rect.setLeft(option.rect.x() +
                                option.rect.width()/2 - checkbox_rect.width()/2);
     //checked or not checked
     if(data)
         checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
     else
         checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;

     //done! we can draw!
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
