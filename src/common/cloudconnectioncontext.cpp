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

#include "cloudconnectioncontext.h"

#include <QReadLocker>
#include <QWriteLocker>

CloudConnectionContext &CloudConnectionContext::instance()
{
    static CloudConnectionContext context;
    return context;
}

CloudConnectionContext::CloudConnectionContext(QObject *parent)
    : QObject(parent)
{
}

CloudConnectionData CloudConnectionContext::data() const
{
    const QReadLocker locker(&m_lock);
    return m_data;
}

void CloudConnectionContext::setData(const CloudConnectionData &data)
{
    {
        const QWriteLocker locker(&m_lock);
        if (m_data == data)
            return;
        m_data = data;
    }

    emit changed(data);
}

void CloudConnectionContext::clear()
{
    setData({});
}
