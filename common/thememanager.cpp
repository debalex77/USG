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

#include "thememanager.h"

#include <QProcess>
#ifdef Q_OS_WIN
#include <QSettings>
#elif defined(Q_OS_MACOS)
extern bool isDarkModeEnabledMacOS();
#endif

bool ThemeManager::isDark()
{
#ifdef Q_OS_WIN
    QSettings set("HKEY_CURRENT_USER/Software/Microsoft/Windows/CurrentVersion/Themes/Personalize", QSettings::NativeFormat);
    return set.value("AppsUseLightTheme", 1).toInt() == 0;
#elif defined(Q_OS_MACOS)
    return isDarkModeEnabledMacOS();
#else
    QProcess p; p.start("gsettings", {"get", "org.gnome.desktop.interface", "color-scheme"});
    p.waitForFinished();
    return QString(p.readAllStandardOutput()).contains("prefer-dark", Qt::CaseInsensitive);
#endif
}
