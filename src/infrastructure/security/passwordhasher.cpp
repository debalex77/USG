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

#include "passwordhasher.h"

#include <QCryptographicHash>
#include <QStringList>

#include <core/loggingcategories.h>

#if defined(Q_OS_WIN)
#include <3rdparty/openssl/include/openssl/crypto.h>
#include <3rdparty/openssl/include/openssl/evp.h>
#include <3rdparty/openssl/include/openssl/rand.h>
#elif defined(Q_OS_LINUX)
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

namespace PasswordHasher {

namespace {

constexpr auto scheme = "pbkdf2-sha256";
constexpr int iterations = 600000;
constexpr int minIterations = 100000;
constexpr int maxIterations = 10000000;
constexpr int saltSize = 16;
constexpr int digestSize = 32;

QByteArray legacyDigest(const QString &password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool derive(const QByteArray &input, const QByteArray &salt, int rounds, QByteArray *digest)
{
    QByteArray out(digestSize, Qt::Uninitialized);
    if (PKCS5_PBKDF2_HMAC(input.constData(), int(input.size()),
                          reinterpret_cast<const unsigned char *>(salt.constData()),
                          int(salt.size()), rounds, EVP_sha256(),
                          int(out.size()),
                          reinterpret_cast<unsigned char *>(out.data())) != 1) {
        qWarning(logWarning()) << "PasswordHasher: PKCS5_PBKDF2_HMAC a eșuat.";
        return false;
    }
    *digest = out;
    return true;
}

bool decodeBase64(const QString &encoded, QByteArray *decoded)
{
    const QByteArray source = encoded.toLatin1();
    *decoded = QByteArray::fromBase64(source, QByteArray::AbortOnBase64DecodingErrors);
    return !decoded->isEmpty() && decoded->toBase64() == source;
}

struct ParsedHash
{
    int rounds = 0;
    QByteArray salt;
    QByteArray digest;
};

bool parse(const QString &storedHash, ParsedHash *parsed)
{
    const QStringList parts = storedHash.split(QLatin1Char('$'));
    if (parts.size() != 4 || parts.at(0) != QLatin1String(scheme))
        return false;

    bool ok = false;
    parsed->rounds = parts.at(1).toInt(&ok);
    return ok
           && parsed->rounds >= minIterations
           && parsed->rounds <= maxIterations
           && decodeBase64(parts.at(2), &parsed->salt)
           && parsed->salt.size() >= saltSize
           && decodeBase64(parts.at(3), &parsed->digest)
           && parsed->digest.size() == digestSize;
}

QString hashDigest(const QByteArray &legacyHex)
{
    QByteArray salt(saltSize, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char *>(salt.data()), int(salt.size())) != 1) {
        qWarning(logWarning()) << "PasswordHasher: RAND_bytes a eșuat.";
        return QString();
    }

    QByteArray digest;
    if (!derive(legacyHex, salt, iterations, &digest))
        return QString();

    return QStringLiteral("%1$%2$%3$%4")
        .arg(QLatin1String(scheme))
        .arg(iterations)
        .arg(QString::fromLatin1(salt.toBase64()),
             QString::fromLatin1(digest.toBase64()));
}

bool constantTimeEquals(const QByteArray &left, const QByteArray &right)
{
    return left.size() == right.size()
           && CRYPTO_memcmp(left.constData(), right.constData(), size_t(left.size())) == 0;
}

}

QString hashPassword(const QString &password)
{
    return hashDigest(legacyDigest(password));
}

QString legacyHash(const QString &password)
{
    return QString::fromLatin1(legacyDigest(password));
}

QString upgradeLegacyHash(const QString &legacyHash)
{
    if (!isLegacyHash(legacyHash))
        return QString();
    return hashDigest(legacyHash.toLower().toLatin1());
}

bool isLegacyHash(const QString &storedHash)
{
    if (storedHash.size() != 64)
        return false;
    for (const QChar ch : storedHash) {
        if (!ch.isDigit()
            && !(ch >= QLatin1Char('a') && ch <= QLatin1Char('f'))
            && !(ch >= QLatin1Char('A') && ch <= QLatin1Char('F')))
            return false;
    }
    return true;
}

bool isCurrentHash(const QString &storedHash)
{
    ParsedHash parsed;
    return parse(storedHash, &parsed);
}

bool verifyPassword(const QString &password, const QString &storedHash)
{
    if (isLegacyHash(storedHash))
        return constantTimeEquals(legacyDigest(password), storedHash.toLower().toLatin1());

    ParsedHash parsed;
    if (!parse(storedHash, &parsed))
        return false;

    QByteArray digest;
    if (!derive(legacyDigest(password), parsed.salt, parsed.rounds, &digest))
        return false;
    return constantTimeEquals(digest, parsed.digest);
}

void simulateVerification(const QString &password)
{
    // Același cost ca verifyPassword pentru un hash curent; rezultatul nu contează.
    static const QByteArray dummySalt(saltSize, '\0');
    QByteArray digest;
    derive(legacyDigest(password), dummySalt, iterations, &digest);
}

}
