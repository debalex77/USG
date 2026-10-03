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

#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QByteArray>
#include <QString>

// Hash-ul parolei utilizatorului din users.hash.
//
// Format curent: pbkdf2-sha256$<iterații>$<salt base64>$<hash base64>,
// calculat peste SHA-256 hex al parolei. Astfel migrarea poate converti
// hash-urile vechi (SHA-256 hex fără salt) fără a cunoaște parolele.
namespace PasswordHasher {

// Hash nou pentru parola introdusă; șir gol la eroare.
QString hashPassword(const QString &password);

// Hash vechi (SHA-256 hex), doar pentru bazele MariaDB încă nemigrate, unde
// users.hash este CHAR(64); migrarea 4.2.7 îl convertește în formatul curent.
QString legacyHash(const QString &password);

// Convertește un hash vechi (SHA-256 hex) în formatul curent; șir gol la eroare.
QString upgradeLegacyHash(const QString &legacyHash);

// Hash vechi: 64 de caractere hex (SHA-256 fără salt).
bool isLegacyHash(const QString &storedHash);

// Hash în formatul curent, cu parametri valizi.
bool isCurrentHash(const QString &storedHash);

// Verifică parola în ambele formate. Logarea are loc înaintea migrării,
// deci hash-urile vechi trebuie acceptate până la actualizarea bazei.
bool verifyPassword(const QString &password, const QString &storedHash);

// Consumă timpul unei verificări PBKDF2 (utilizator inexistent), ca durata
// răspunsului să nu indice dacă numele există.
void simulateVerification(const QString &password);

// Lungimea minimă a parolei la crearea sau schimbarea ei.
inline constexpr int minPasswordLength = 8;

}

#endif // PASSWORDHASHER_H
