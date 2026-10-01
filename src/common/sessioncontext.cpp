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

#include "sessioncontext.h"

SessionContext &SessionContext::instance()
{
    static SessionContext context;
    return context;
}

SessionContext::SessionContext(QObject *parent)
    : QObject(parent)
{
}

int SessionContext::userId() const
{
    return m_userId;
}

QUuid SessionContext::userUuid() const
{
    return m_userUuid;
}

bool SessionContext::hasAuthenticatedUser() const
{
    return m_authenticated && m_userId > 0;
}

void SessionContext::setCandidateUserId(int userId)
{
    const bool userChanged = m_userId != userId;
    const bool authenticationChangedValue = m_authenticated;

    m_userId = userId;
    m_userUuid = QUuid();
    m_authenticated = false;

    if (userChanged)
        emit userIdChanged(m_userId);

    if (authenticationChangedValue)
        emit authenticationChanged(false);
}

void SessionContext::setAuthenticatedUserId(int userId, const QUuid &userUuid)
{
    const bool userChanged   = m_userId != userId;
    const bool authenticated = userId > 0;
    const bool authenticationChangedValue = m_authenticated != authenticated;

    m_userId        = userId;
    m_userUuid      = authenticated ? userUuid : QUuid();
    m_authenticated = authenticated;

    if (userChanged)
        emit userIdChanged(m_userId);

    if (authenticationChangedValue)
        emit authenticationChanged(m_authenticated);
}

void SessionContext::clear()
{
    setCandidateUserId(-1);
}
