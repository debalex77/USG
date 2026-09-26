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

#include "tabledocmodel.h"

TableDocModel::TableDocModel(QObject *parent)
    : QSqlTableModel{parent},
    m_flags(Qt::NoItemFlags)
{}

void TableDocModel::setPurpose(Purpose purpose)
{
    m_purpose = purpose;
}

void TableDocModel::setFlags(Qt::ItemFlags flag)
{
    m_flags = flag;
}

QVariant TableDocModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVariant value = QSqlTableModel::data(index, role);

    const double price = QSqlTableModel::data(
                             this->index(index.row(), PricingsTableSection::Price),
                             Qt::EditRole).toDouble();

#if defined(Q_OS_WIN)
    QFont font;
    font.setPointSize(9);
#endif

    if (role == Qt::DisplayRole && index.column() == PricingsTableSection::Price) {
        return QString::number(value.toDouble(), 'f', 2);
    }

    if (role == Qt::EditRole)
        return value;

    if (role == Qt::FontRole) {
#if defined(Q_OS_WIN)
        return font;
#else
        return QVariant();
#endif
    }

    if (role == Qt::BackgroundRole) {


        if (m_purpose == Table_destination) {
            if (qFuzzyIsNull(price))
                return QBrush(QColor(191,198,188));
            return globals().isSystemThemeDark
                       ? QBrush(QColor(52, 112, 93))
                       : QBrush(QColor(214,235,206));
        }

        if (index.column() != PricingsTableSection::Price) {
            return globals().isSystemThemeDark
                       ? QBrush(QColor(52, 112, 93))
                       : QBrush(QColor(217,255,210));
        }

        return QVariant();
    }

    if (role == Qt::ForegroundRole) {

        if (m_purpose == Table_destination && qFuzzyIsNull(price))
            return QBrush(QColor(88,94,86));

        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == PricingsTableSection::Cod ||
            index.column() == PricingsTableSection::Price) {
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        return QVariant();
    }

    return value;
}

Qt::ItemFlags TableDocModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // dacă flags sunt setate manual
    if (m_flags != Qt::NoItemFlags)
        return m_flags;

    Qt::ItemFlags flags = QSqlTableModel::flags(index);

    if (m_flags == Qt::NoItemFlags)
    if (index.column() == PricingsTableSection::Price)
        flags |= Qt::ItemIsEditable;

    return flags;
}
