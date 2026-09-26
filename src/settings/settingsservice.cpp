/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 Codreanu Alexandru <alovada.med@gmail.com>
 *
 *****************************************************************************/

#include "settingsservice.h"

SettingsService &SettingsService::instance()
{
    static SettingsService service;
    return service;
}

SettingsService::SettingsService(QObject *parent)
    : QObject(parent)
{
}

const Settings::ApplicationPreferences &SettingsService::application() const
{
    return m_snapshot.application;
}

const Settings::OrganizationSettings &SettingsService::organization() const
{
    return m_snapshot.organization;
}

const Settings::UserPreferencesData &SettingsService::user() const
{
    return m_snapshot.user;
}

const Settings::SynchronizationSettings &SettingsService::synchronization() const
{
    return m_snapshot.synchronization;
}

Settings::Snapshot SettingsService::snapshot() const
{
    return m_snapshot;
}

void SettingsService::setApplication(const Settings::ApplicationPreferences &preferences)
{
    if (m_snapshot.application == preferences)
        return;

    m_snapshot.application = preferences;
    emit applicationChanged(m_snapshot.application);
}

void SettingsService::setOrganization(const Settings::OrganizationSettings &settings)
{
    if (m_snapshot.organization == settings)
        return;

    m_snapshot.organization = settings;
    emit organizationChanged(m_snapshot.organization);
}

void SettingsService::setUser(const Settings::UserPreferencesData &preferences)
{
    if (m_snapshot.user == preferences)
        return;

    const Settings::PrintMenuMode oldPrintMenuMode = m_snapshot.user.printMenuMode;
    m_snapshot.user = preferences;
    emit userChanged(m_snapshot.user);
    if (oldPrintMenuMode != m_snapshot.user.printMenuMode)
        emit printMenuModeChanged(m_snapshot.user.printMenuMode);
}

void SettingsService::setSynchronization(const Settings::SynchronizationSettings &settings)
{
    if (m_snapshot.synchronization == settings)
        return;

    m_snapshot.synchronization = settings;
    emit synchronizationChanged(m_snapshot.synchronization);
}

void SettingsService::setSnapshot(const Settings::Snapshot &snapshot)
{
    setApplication(snapshot.application);
    setOrganization(snapshot.organization);
    setUser(snapshot.user);
    setSynchronization(snapshot.synchronization);
}
