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

#include "organizationcontractmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

#include <common/globals.h>

OrganizationContractModel::OrganizationContractModel(QObject *parent)
    : BaseAbstractModel(parent)
{
    //-----------------------------------------------------------------
    // 1. functii suplimentale interne

    // -- font
    const auto fontRule = [this](const QVariantMap &row) -> QVariant {
        QFont font;
        bool changed = false;

        if (row.value("deletionMark").toInt() == 1) {
            font.setItalic(true);
            font.setStrikeOut(true);
            changed = true;
        }

        if (row.value("id").toInt() == m_mainContractId) {
            font.setBold(true);
            changed = true;
        }

        return changed ? QVariant(font) : QVariant();
    };

    // -- forenground
    const auto foregroundRule = [this](const QVariantMap &row) -> QVariant {
        const bool isDeleted = row.value("deletionMark").toInt() == 1;
        const bool isMain = row.value("id").toInt() == m_mainContractId;

        if (isDeleted) {
            return globals().isSystemThemeDark
                       ? QVariant::fromValue(QBrush(QColor(200, 140, 180)))
                       : QVariant::fromValue(QBrush(QColor(255, 230, 255)));
        }

        if (isMain)
            return QBrush(QColor(0, 206, 209));

        return {};
    };

    //-------------------------------------------------------------
    // 2.inregistrarea coloanelor

    // ID
    auto idColumn = std::make_unique<FieldColumn>("ID", "id", false, Qt::AlignCenter);
    idColumn->setFontRule(fontRule);
    idColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(idColumn));

    // deletionMark
    auto deletionColumn = std::make_unique<FieldColumn>("", "deletionMark", false,
                                                        Qt::AlignCenter);
    deletionColumn->setDecorationRule([](const QVariantMap &row) -> QVariant {
        const int mark = row.value("deletionMark").toInt();
        if (mark == 0)
            return QIcon(":/img/catalogs/item.png");
        if (mark == 1)
            return QIcon(":/img/catalogs/item_delete.png");
        return {};
    });
    deletionColumn->setDisplayFormatter([](const QVariant &) -> QVariant { return {}; });
    deletionColumn->setFontRule(fontRule);
    deletionColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(deletionColumn));

    // name contract
    auto nameColumn = std::make_unique<FieldColumn>(
        "Denumirea contractului", "name", false, Qt::AlignLeft);
    nameColumn->setFontRule(fontRule);
    nameColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(nameColumn));

    // dateInit
    auto dateColumn = std::make_unique<DateColumn>(
        "Data încep.", "dateInit", "dd.MM.yyyy", false, Qt::AlignLeft);
    dateColumn->setFontRule(fontRule);
    dateColumn->setForegroundRule(foregroundRule);
    registerColumn(std::move(dateColumn));
}

void OrganizationContractModel::setMainContractId(int id)
{
    if (m_mainContractId == id)
        return;

    m_mainContractId = id;
    if (rowCount() > 0)
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                         {Qt::FontRole, Qt::ForegroundRole});
}
