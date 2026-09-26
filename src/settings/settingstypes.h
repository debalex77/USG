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

#ifndef SETTINGSTYPES_H
#define SETTINGSTYPES_H

#include <QByteArray>
#include <QMetaType>
#include <QString>

namespace Settings {

enum class PrintMenuMode {
    Standard           = 0,
    PreviewAndDesigner = 1
};

struct ApplicationPreferences {
    bool checkForUpdatesOnStartup = true;
    bool showUserManualOnStartup  = false;
    bool showAssistantOnStartup   = true;
    int documentJournalRefreshIntervalSeconds = 0;

    friend bool operator==(const ApplicationPreferences &,
                           const ApplicationPreferences &) = default;
};

struct OrganizationSettings {
    int organizationId  = -1;
    int defaultDoctorId = -1;
    int defaultNurseId  = -1;
    QString ultrasoundDeviceName;
    QByteArray logoData;

    friend bool operator==(const OrganizationSettings &,
                           const OrganizationSettings &) = default;
};

struct UserPreferencesData {
    int userId                = -1;
    int defaultOrganizationId = -1;
    bool minimizeToTray       = false;
    bool confirmOnExit        = true;
    bool archiveSqliteOnExit  = false;
    bool openDocumentsInSeparateWindows = false;
    PrintMenuMode printMenuMode = PrintMenuMode::Standard;

    friend bool operator==(const UserPreferencesData &,
                           const UserPreferencesData &) = default;
};

struct SynchronizationSettings {
    bool configured = false;
    bool enabled    = false;

    friend bool operator==(const SynchronizationSettings &,
                           const SynchronizationSettings &) = default;
};

struct Snapshot {
    ApplicationPreferences application;
    OrganizationSettings organization;
    UserPreferencesData user;
    SynchronizationSettings synchronization;

    friend bool operator==(const Snapshot &, const Snapshot &) = default;
};

} // namespace Settings

Q_DECLARE_METATYPE(Settings::ApplicationPreferences)
Q_DECLARE_METATYPE(Settings::OrganizationSettings)
Q_DECLARE_METATYPE(Settings::UserPreferencesData)
Q_DECLARE_METATYPE(Settings::SynchronizationSettings)
Q_DECLARE_METATYPE(Settings::PrintMenuMode)

#endif // SETTINGSTYPES_H
