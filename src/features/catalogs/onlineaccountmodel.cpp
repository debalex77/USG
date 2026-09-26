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

#include "onlineaccountmodel.h"

OnlineAccountModel::OnlineAccountModel(QObject *parent)
    : BaseAbstractModel(parent)
{
    // inregistram coloanele

    // ID
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("ID"), QStringLiteral("id"), false, Qt::AlignCenter));

    // deletionMark cu decoratiuni
    auto statusColumn = std::make_unique<FieldColumn>(
        QString(), QStringLiteral("deletionMark"), false, Qt::AlignCenter);

    statusColumn->setDecorationRule([](const QVariantMap &row) -> QVariant {
        const int mark = row.value(QStringLiteral("deletionMark")).toInt();
        if (mark == 0)
            return QIcon(QStringLiteral(":/img/catalogs/item.png"));
        if (mark == 1)
            return QIcon(QStringLiteral(":/img/catalogs/item_delete.png"));
        return {};
    });

    statusColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(statusColumn));

    // organization
    auto organizationColumn = std::make_unique<FieldColumn>(
        QStringLiteral("Organization (ID)"), QStringLiteral("id_organizations"),
        false, Qt::AlignCenter);
    organizationColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(organizationColumn));

    // user
    auto userColumn = std::make_unique<FieldColumn>(
        QStringLiteral("User (ID)"), QStringLiteral("id_users"),
        false, Qt::AlignCenter);
    userColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    registerColumn(std::move(userColumn));

    // email
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("E-mail"), QStringLiteral("email"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));

    // smtp server
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("SMTP server"), QStringLiteral("smtp_server"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));

    // port
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("Port"), QStringLiteral("port"), false, Qt::AlignVCenter));

    // usename
    registerColumn(std::make_unique<FieldColumn>(
        QStringLiteral("User name"), QStringLiteral("username"), false,
        Qt::AlignVCenter | Qt::AlignHCenter));
}
