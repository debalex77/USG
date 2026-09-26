/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 *****************************************************************************/

#include "profilesecretcodec.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace ProfileSecretCodec {

namespace {

constexpr auto prefix = "aes-gcm-v1:";
constexpr auto keyFileName = "profile.key";
constexpr auto additionalData = "USG/profile-password/v1";
constexpr int keySize = 32;
constexpr int ivSize = 12;
constexpr int tagSize = 16;

QString keyPathForProfile(const QString &settingsPath)
{
    return QFileInfo(settingsPath).absoluteDir().filePath(
        QStringLiteral("crypto/") + QLatin1String(keyFileName));
}

bool decodeCanonicalBase64(const QString &encoded, QByteArray *decoded)
{
    if (!decoded)
        return false;

    const QByteArray source = encoded.toLatin1();
    *decoded = QByteArray::fromBase64(source, QByteArray::AbortOnBase64DecodingErrors);
    return !decoded->isNull() && decoded->toBase64() == source;
}

bool loadKey(const QString &settingsPath, bool createIfMissing,
             QByteArray *key, QString *error)
{
    if (!key) {
        if (error)
            *error = QStringLiteral("Destinația cheii este nulă.");
        return false;
    }

    const QString keyPath = keyPathForProfile(settingsPath);
    QFile keyFile(keyPath);
    if (keyFile.exists()) {
        if (!keyFile.open(QIODevice::ReadOnly)) {
            if (error)
                *error = QStringLiteral("Cheia profilului nu poate fi citită: %1").arg(keyPath);
            return false;
        }

        QByteArray decoded;
        const QByteArray encoded = keyFile.readAll().trimmed();
        if (!decodeCanonicalBase64(QString::fromLatin1(encoded), &decoded)
            || decoded.size() != keySize) {
            if (error)
                *error = QStringLiteral("Cheia profilului este invalidă: %1").arg(keyPath);
            return false;
        }

        *key = decoded;
        return true;
    }

    if (!createIfMissing) {
        if (error)
            *error = QStringLiteral("Cheia profilului lipsește: %1").arg(keyPath);
        return false;
    }

    QByteArray generated(keySize, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char *>(generated.data()), generated.size()) != 1) {
        if (error)
            *error = QStringLiteral("Nu s-a putut genera cheia profilului.");
        return false;
    }

    const QFileInfo keyInfo(keyPath);
    QDir keyDirectory = keyInfo.absoluteDir();
    if (!keyDirectory.exists() && !keyDirectory.mkpath(QStringLiteral("."))) {
        if (error)
            *error = QStringLiteral("Directorul cheii profilului nu poate fi creat: %1")
                         .arg(keyDirectory.absolutePath());
        return false;
    }

    QFile output(keyPath);
    if (!output.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
        if (error)
            *error = QStringLiteral("Cheia profilului nu poate fi creată fără suprascriere: %1")
                         .arg(keyPath);
        return false;
    }

#if defined(Q_OS_UNIX)
    if (!output.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        output.close();
        QFile::remove(keyPath);
        if (error)
            *error = QStringLiteral("Permisiunile cheii profilului nu pot fi securizate: %1")
                         .arg(keyPath);
        return false;
    }
#endif

    const QByteArray encoded = generated.toBase64() + '\n';
    if (output.write(encoded) != encoded.size() || !output.flush()) {
        output.close();
        QFile::remove(keyPath);
        if (error)
            *error = QStringLiteral("Salvarea cheii profilului a eșuat: %1").arg(keyPath);
        return false;
    }
    output.close();

    *key = generated;
    return true;
}

}

bool isEncrypted(const QString &value)
{
    return value.startsWith(QLatin1String(prefix));
}

bool encrypt(const QString &plainText, const QString &settingsPath,
             QString *encodedValue, QString *error)
{
    if (error)
        error->clear();
    if (!encodedValue || settingsPath.isEmpty()) {
        if (error)
            *error = QStringLiteral("Parametri invalizi pentru criptarea profilului.");
        return false;
    }

    if (plainText.isEmpty()) {
        encodedValue->clear();
        return true;
    }

    QByteArray key;
    if (!loadKey(settingsPath, true, &key, error))
        return false;

    QByteArray iv(ivSize, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char *>(iv.data()), iv.size()) != 1) {
        if (error)
            *error = QStringLiteral("Nu s-a putut genera IV-ul parolei profilului.");
        return false;
    }

    const QByteArray plain = plainText.toUtf8();
    QByteArray cipher(plain.size(), Qt::Uninitialized);
    QByteArray tag(tagSize, Qt::Uninitialized);
    EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
    if (!context) {
        if (error)
            *error = QStringLiteral("Inițializarea criptării profilului a eșuat.");
        return false;
    }

    bool ok = true;
    int length = 0;
    int cipherLength = 0;
    do {
        if (EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1
            || EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr) != 1
            || EVP_EncryptInit_ex(context, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(key.constData()),
                                  reinterpret_cast<const unsigned char *>(iv.constData())) != 1
            || EVP_EncryptUpdate(context, nullptr, &length,
                                 reinterpret_cast<const unsigned char *>(additionalData),
                                 int(qstrlen(additionalData))) != 1) {
            ok = false;
            break;
        }

        if (!plain.isEmpty()
            && EVP_EncryptUpdate(context,
                                 reinterpret_cast<unsigned char *>(cipher.data()), &length,
                                 reinterpret_cast<const unsigned char *>(plain.constData()),
                                 plain.size()) != 1) {
            ok = false;
            break;
        }
        cipherLength = plain.isEmpty() ? 0 : length;

        if (EVP_EncryptFinal_ex(context,
                                reinterpret_cast<unsigned char *>(cipher.data()) + cipherLength,
                                &length) != 1
            || EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG,
                                   tag.size(), tag.data()) != 1) {
            ok = false;
            break;
        }
        cipher.resize(cipherLength + length);
    } while (false);

    EVP_CIPHER_CTX_free(context);
    if (!ok) {
        if (error)
            *error = QStringLiteral("Criptarea parolei profilului a eșuat.");
        return false;
    }

    *encodedValue = QLatin1String(prefix)
                    + QString::fromLatin1(iv.toBase64()) + QLatin1Char(':')
                    + QString::fromLatin1(cipher.toBase64()) + QLatin1Char(':')
                    + QString::fromLatin1(tag.toBase64());
    return true;
}

bool decrypt(const QString &encodedValue, const QString &settingsPath,
             QString *plainText, QString *error)
{
    if (error)
        error->clear();
    if (!plainText || settingsPath.isEmpty() || !isEncrypted(encodedValue)) {
        if (error)
            *error = QStringLiteral("Format invalid pentru parola criptată a profilului.");
        return false;
    }

    const QString payload = encodedValue.mid(int(qstrlen(prefix)));
    const QStringList parts = payload.split(QLatin1Char(':'), Qt::KeepEmptyParts);
    QByteArray iv;
    QByteArray cipher;
    QByteArray tag;
    if (parts.size() != 3
        || !decodeCanonicalBase64(parts.at(0), &iv)
        || !decodeCanonicalBase64(parts.at(1), &cipher)
        || !decodeCanonicalBase64(parts.at(2), &tag)
        || iv.size() != ivSize || tag.size() != tagSize) {
        if (error)
            *error = QStringLiteral("Conținut invalid pentru parola criptată a profilului.");
        return false;
    }

    QByteArray key;
    if (!loadKey(settingsPath, false, &key, error))
        return false;

    QByteArray plain(cipher.size(), Qt::Uninitialized);
    EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
    if (!context) {
        if (error)
            *error = QStringLiteral("Inițializarea decriptării profilului a eșuat.");
        return false;
    }

    bool ok = true;
    int length = 0;
    int plainLength = 0;
    do {
        if (EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1
            || EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr) != 1
            || EVP_DecryptInit_ex(context, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(key.constData()),
                                  reinterpret_cast<const unsigned char *>(iv.constData())) != 1
            || EVP_DecryptUpdate(context, nullptr, &length,
                                 reinterpret_cast<const unsigned char *>(additionalData),
                                 int(qstrlen(additionalData))) != 1) {
            ok = false;
            break;
        }

        if (!cipher.isEmpty()
            && EVP_DecryptUpdate(context,
                                 reinterpret_cast<unsigned char *>(plain.data()), &length,
                                 reinterpret_cast<const unsigned char *>(cipher.constData()),
                                 cipher.size()) != 1) {
            ok = false;
            break;
        }
        plainLength = cipher.isEmpty() ? 0 : length;

        if (EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG,
                                tag.size(), tag.data()) != 1
            || EVP_DecryptFinal_ex(context,
                                   reinterpret_cast<unsigned char *>(plain.data()) + plainLength,
                                   &length) != 1) {
            ok = false;
            break;
        }
        plain.resize(plainLength + length);
    } while (false);

    EVP_CIPHER_CTX_free(context);
    if (!ok) {
        if (error)
            *error = QStringLiteral("Parola profilului nu poate fi decriptată sau a fost modificată.");
        return false;
    }

    *plainText = QString::fromUtf8(plain);
    return true;
}

}
