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

#include "basesqltablemodel.h"
#include <QDebug>
BaseSqlTableModel::BaseSqlTableModel(QObject *parent) : QSqlTableModel(parent)
{
}

void BaseSqlTableModel::setMainFlag(Qt::ItemFlags flag)
{
    m_flag = flag;
}

Qt::ItemFlags BaseSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant BaseSqlTableModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);

    if (parent()->inherits("FirstRunWizard"))
        return dataFromCatForTableModel(index, role, value);
    return value;
}

void BaseSqlTableModel::setTable(const QString &tableName)
{
    QSqlTableModel::setTable(tableName);
}

void BaseSqlTableModel::setEditStrategy(EditStrategy strategy)
{
    QSqlTableModel::setEditStrategy(strategy);
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order)
{
    QSqlTableModel::setSort(column, order);
}

Qt::ItemFlags BaseSqlTableModel::flagsFromPricing(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    if (index.column() == 4 || index.column() == 5)
        flags |= Qt::ItemIsEnabled;
    else if (index.column() == 6)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant BaseSqlTableModel::dataFromCatForTableModel(const QModelIndex &index, int role, QVariant &value) const
{
    switch (role) {
    case Qt::DisplayRole: // datele prezentarii

        if (index.column() == 1)
            return "";

        return value;

    case Qt::EditRole:    // datele redactarii

        return value;

    case Qt::TextAlignmentRole: // alinierea

        return int(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::DecorationRole:

        if (index.column() == 1 && QSqlTableModel::data(index, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/element_x32.png");
        else if (index.column() == 1 && QSqlTableModel::data(index, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/element_delete_x32.png");
        else
            return value;

    case Qt::FontRole:

        if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 1), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlTableModel::data(index, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }

    case Qt::BackgroundRole:  // culoarea fondalului

        if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 1), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));

        // if (QSqlTableModel::data(QSqlTableModel::index(index.row(), 4), Qt::DisplayRole).toInt() == 0) // column 'use'
        //     return QBrush(QColor(224,224,224));

        return value;

    default:
        return value;
    }

    return value;
}
