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

#include "legacysettingscodec.h"

#include <QByteArray>

namespace {
constexpr char encodingKey = 073;
}

QString LegacySettingsCodec::encode(const QString &value)
{
    QByteArray bytes(value.toUtf8());
    for (char &byte : bytes)
        byte ^= encodingKey;

    return QString::fromLatin1(bytes.toBase64());
}

QString LegacySettingsCodec::decode(const QString &value)
{
    QByteArray bytes = QByteArray::fromBase64(value.toLatin1());
    for (char &byte : bytes)
        byte ^= encodingKey;

    return QString::fromUtf8(bytes);
}

bool LegacySettingsCodec::isValid(const QString &encodedValue)
{
    return encodedValue.isEmpty() ||
           encode(decode(encodedValue)) == encodedValue;
}
