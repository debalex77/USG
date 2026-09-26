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

#include "sortmodel.h"

SortModel::SortModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setDynamicSortFilter(true);
}

bool SortModel::lessThan(const QModelIndex &source_left,
                         const QModelIndex &source_right) const
{
    QVariant l = sourceModel()->data(source_left, sortRole());
    QVariant r = sourceModel()->data(source_right, sortRole());

    switch (l.typeId()) {
    case QMetaType::Int:
        return l.toInt() < r.toInt();

    case QMetaType::QString:
        return QString::localeAwareCompare(l.toString(), r.toString()) < 0;

    case QMetaType::QDateTime:
        return l.toDateTime() < r.toDateTime();

    case QMetaType::Bool:
        return l.toBool() < r.toBool();

    default:
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}
