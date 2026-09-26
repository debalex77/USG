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

#include "orderinvestigationmodel.h"

OrderInvestigationModel::OrderInvestigationModel(Kind kind, QObject *parent)
    : BaseAbstractModel(parent)
    , m_kind(kind)
{
    // ID
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("ID"), QStringLiteral("id"), false, Qt::AlignCenter));

    // deletionMark
    registerColumn(std::make_unique<FieldColumn>(
        QString(), QStringLiteral("deletionMark"), false, Qt::AlignCenter));

    // pricing ID or order ID
    if (kind == Kind::Available) {
        registerColumn(std::make_unique<FieldColumn>(
            QStringLiteral("Pricing (ID)"), QStringLiteral("id_pricings"),
            false, Qt::AlignCenter));
    } else {
        registerColumn(std::make_unique<FieldColumn>(
            QStringLiteral("Order (ID)"), QStringLiteral("id_orderEcho"),
            false, Qt::AlignCenter));
    }

    // cod MS
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Cod MS"), QStringLiteral("cod"), false,
        Qt::AlignHCenter | Qt::AlignVCenter));

    // name invastigation
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Denumirea investigației"), QStringLiteral("name"), false,
        Qt::AlignLeft | Qt::AlignVCenter));

    // price
    registerColumn(std::make_unique<NumberColumn>(
        QStringLiteral("Preț"), QStringLiteral("price"), 2,
        kind == Kind::Selected, Qt::AlignLeft | Qt::AlignVCenter));
}

OrderInvestigationModel::Kind OrderInvestigationModel::kind() const
{
    return m_kind;
}
