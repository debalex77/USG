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

#include "treeitem.h"

#include <QIcon>

TreeItem::TreeItem (const QVector<QVariant> &data, TreeItemType type, TreeItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
    m_type = type;
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    m_childItems.append(item);
}

TreeItem *TreeItem::child (int row)
{
    return m_childItems.value(row);
}

int TreeItem::childCount() const
{
    return m_childItems.count();
}

int TreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant TreeItem::data (int column, int role) const
{
    if (column < 0 || column >= columnCount())
        return QVariant();

    if (column == 0) {
        if (role == Qt::DecorationRole && m_type == TreeItemGroup)
            return QIcon(":/img/folder_yellow.png");
        else if (role == Qt::DecorationRole && m_type == TreeItemItem)
            return QIcon(":/img/element.png");
    }

    return m_itemData.value(column);
}

TreeItem *TreeItem::parentItem()
{
    return m_parentItem;
}

int TreeItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, m_type, this);
        m_childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.insert(position, QVariant());

    foreach (TreeItem *child, m_childItems)
        child->insertColumns(position, columns);

    return true;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;
    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.removeAt(position);

    foreach (TreeItem *child, m_childItems)
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= m_itemData.size())
        return false;

    m_itemData[column] = value;

    return true;
}
