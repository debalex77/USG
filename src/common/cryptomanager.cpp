#include "cryptomanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QByteArray CryptoManager::generateRandomBytes(int size)
{
    if (size <= 0) {
        qWarning(logWarning()) << "CryptoManager::generateRandomBytes - marime invalida:" << size;
        return QByteArray();
    }

    QByteArray data(size, 0);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(data.data()), data.size()) != 1) {
        qWarning(logWarning()) << "CryptoManager::generateRandomBytes - RAND_bytes a esuat";
        return QByteArray();
    }

    return data;
}

QByteArray CryptoManager::generateRandomIV(int size)
{
    return generateRandomBytes(size);
}

QString CryptoManager::toBase64(const QByteArray &data)
{
    return QString::fromUtf8(data.toBase64());
}

QByteArray CryptoManager::fromBase64(const QString &data)
{
    return QByteArray::fromBase64(data.toUtf8());
}

QString CryptoManager::localKeyPartFilePath(int idOrganization)
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (baseDir.isEmpty())
        baseDir = QDir::homePath() + "/.config/USG";

    return baseDir + QString("/crypto/org_%1.keypart").arg(idOrganization);
}

QByteArray CryptoManager::loadOrCreateLocalKeyPart(int idOrganization, bool *ok)
{
    if (ok)
        *ok = false;

    const QString filePath = localKeyPartFilePath(idOrganization);
    QFile file(filePath);

    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning(logWarning()) << "CryptoManager::loadOrCreateLocalKeyPart - nu pot deschide:"
                                   << filePath;
            return QByteArray();
        }

        const QByteArray encoded = file.readAll().trimmed();
        file.close();

        const QByteArray keyPart2 = QByteArray::fromBase64(encoded);
        if (keyPart2.size() != 32) {
            qWarning(logWarning()) << "CryptoManager::loadOrCreateLocalKeyPart - cheia locala invalida";
            return QByteArray();
        }

        if (ok)
            *ok = true;

        return keyPart2;
    }

    const QByteArray keyPart2 = generateRandomBytes(32);
    if (keyPart2.size() != 32) {
        qWarning(logWarning()) << "CryptoManager::loadOrCreateLocalKeyPart - nu s-a putut genera cheia locala";
        return QByteArray();
    }

    QFileInfo fi(filePath);
    QDir dir(fi.absolutePath());
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning(logWarning()) << "CryptoManager::loadOrCreateLocalKeyPart - nu pot crea directorul:"
                               << fi.absolutePath();
        return QByteArray();
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning(logWarning()) << "CryptoManager::loadOrCreateLocalKeyPart - nu pot salva:"
                               << filePath;
        return QByteArray();
    }

    file.write(keyPart2.toBase64());
    file.write("\n");
    file.close();

#ifdef Q_OS_LINUX
    QFile::setPermissions(filePath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif

    if (ok)
        *ok = true;

    return keyPart2;
}

bool CryptoManager::loadOrCreateDbKeyPart(QSqlDatabase &db,
                                          int idOrganization,
                                          QByteArray *keyPart1,
                                          QString *error)
{
    if (error)
        error->clear();

    if (!keyPart1) {
        if (error)
            *error = "Pointer keyPart1 este null.";
        return false;
    }

    // Unele baze actualizate la 4.1.0 au fost create înainte ca tabela
    // pentru cheia separată să fie inclusă în schema inițială.
    QSqlQuery ensureTable(db);
    const bool mysql = db.driverName().compare(QStringLiteral("QMYSQL"),
                                                Qt::CaseInsensitive) == 0;
    const QString createTableSql = mysql
        ? QStringLiteral(
              "CREATE TABLE IF NOT EXISTS cryptoSplitKey ("
              "id_organizations BIGINT UNSIGNED NOT NULL PRIMARY KEY, "
              "key_part1 VARCHAR(128) NOT NULL)")
        : QStringLiteral(
              "CREATE TABLE IF NOT EXISTS cryptoSplitKey ("
              "id_organizations INTEGER PRIMARY KEY, "
              "key_part1 TEXT NOT NULL)");
    if (!ensureTable.exec(createTableSql)) {
        if (error)
            *error = ensureTable.lastError().text();
        return false;
    }

    QSqlQuery qry(db);
    qry.prepare(R"(
        SELECT key_part1
        FROM cryptoSplitKey
        WHERE id_organizations = ?
    )");
    qry.addBindValue(idOrganization);

    if (!qry.exec()) {
        if (error)
            *error = qry.lastError().text();
        return false;
    }

    if (qry.next()) {
        const QByteArray loaded = QByteArray::fromBase64(qry.value(0).toString().toUtf8());
        if (loaded.size() != 32) {
            if (error)
                *error = "key_part1 din BD este invalida.";
            return false;
        }

        *keyPart1 = loaded;
        return true;
    }

    const QByteArray newKeyPart1 = generateRandomBytes(32);
    if (newKeyPart1.size() != 32) {
        if (error)
            *error = "Nu s-a putut genera key_part1.";
        return false;
    }

    QSqlQuery ins(db);
    ins.prepare(R"(
        INSERT INTO cryptoSplitKey
        (
            id_organizations,
            key_part1
        )
        VALUES
        (
            ?, ?
        )
    )");
    ins.addBindValue(idOrganization);
    ins.addBindValue(QString::fromUtf8(newKeyPart1.toBase64()));

    if (!ins.exec()) {
        if (error)
            *error = ins.lastError().text();
        return false;
    }

    *keyPart1 = newKeyPart1;
    return true;
}

QByteArray CryptoManager::deriveRealKey(const QByteArray &keyPart1,
                                        const QByteArray &keyPart2)
{
    if (keyPart1.size() != 32 || keyPart2.size() != 32) {
        qWarning(logWarning()) << "CryptoManager::deriveRealKey - parti de cheie invalide";
        return QByteArray();
    }

    return QCryptographicHash::hash(keyPart1 + keyPart2, QCryptographicHash::Sha256);
}

QByteArray CryptoManager::deriveCloudKey(const QByteArray &userHash,
                                         int idOrganization)
{
    if (userHash.isEmpty() || idOrganization <= 0)
        return QByteArray();

    QByteArray material("USG/cloud-password/v1\0", 22);
    material.append(userHash);
    material.append('\0');
    material.append(QByteArray::number(idOrganization));
    return QCryptographicHash::hash(material, QCryptographicHash::Sha256);
}

bool CryptoManager::loadOrCreateSplitKey(QSqlDatabase &db,
                                         int idOrganization,
                                         QByteArray *realKey,
                                         QString *error)
{
    if (error)
        error->clear();

    if (!realKey) {
        if (error)
            *error = "Pointer realKey este null.";
        return false;
    }

    QByteArray keyPart1;
    if (!loadOrCreateDbKeyPart(db, idOrganization, &keyPart1, error))
        return false;

    bool okLocal = false;
    const QByteArray keyPart2 = loadOrCreateLocalKeyPart(idOrganization, &okLocal);
    if (!okLocal || keyPart2.size() != 32) {
        if (error)
            *error = "Nu s-a putut incarca/genereaza key_part2 local.";
        return false;
    }

    const QByteArray key = deriveRealKey(keyPart1, keyPart2);
    if (!isValidAes256Key(key)) {
        if (error)
            *error = "Cheia finala derivata este invalida.";
        return false;
    }

    *realKey = key;
    return true;
}

bool CryptoManager::isValidAes256Key(const QByteArray &key)
{
    return key.size() == 32;
}

CryptoManager::EncryptedData CryptoManager::encryptText(const QString &plainText,
                                                        const QByteArray &realKey)
{
    EncryptedData out;

    if (!isValidAes256Key(realKey)) {
        qWarning(logWarning()) << "CryptoManager::encryptText - cheia AES-256 este invalida";
        return out;
    }

    out.iv = generateRandomIV(12);
    if (out.iv.size() != 12) {
        qWarning(logWarning()) << "CryptoManager::encryptText - IV invalid";
        return EncryptedData{};
    }

    const QByteArray plain = plainText.toUtf8();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning(logWarning()) << "CryptoManager::encryptText - EVP_CIPHER_CTX_new a esuat";
        return EncryptedData{};
    }

    bool success = true;
    int len = 0;
    int cipherLen = 0;

    out.cipherText.resize(plain.size());
    out.tag.resize(16);

    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            success = false;
            break;
        }

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, out.iv.size(), nullptr) != 1) {
            success = false;
            break;
        }

        if (EVP_EncryptInit_ex(ctx,
                               nullptr,
                               nullptr,
                               reinterpret_cast<const unsigned char*>(realKey.constData()),
                               reinterpret_cast<const unsigned char*>(out.iv.constData())) != 1) {
            success = false;
            break;
        }

        if (!plain.isEmpty()) {
            if (EVP_EncryptUpdate(ctx,
                                  reinterpret_cast<unsigned char*>(out.cipherText.data()),
                                  &len,
                                  reinterpret_cast<const unsigned char*>(plain.constData()),
                                  plain.size()) != 1) {
                success = false;
                break;
            }
            cipherLen = len;
        }

        if (EVP_EncryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char*>(out.cipherText.data()) + cipherLen,
                                &len) != 1) {
            success = false;
            break;
        }
        cipherLen += len;
        out.cipherText.resize(cipherLen);

        if (EVP_CIPHER_CTX_ctrl(ctx,
                                EVP_CTRL_GCM_GET_TAG,
                                out.tag.size(),
                                out.tag.data()) != 1) {
            success = false;
            break;
        }

    } while (false);

    EVP_CIPHER_CTX_free(ctx);

    if (!success) {
        qWarning(logWarning()) << "CryptoManager::encryptText - criptarea a esuat";
        return EncryptedData{};
    }

    return out;
}

QByteArray CryptoManager::decryptText(const EncryptedData &data,
                                      const QByteArray &realKey,
                                      bool *ok)
{
    if (ok)
        *ok = false;

    if (!isValidAes256Key(realKey)) {
        qWarning(logWarning()) << "CryptoManager::decryptText - cheia AES-256 este invalida";
        return QByteArray();
    }

    if (!data.isValid()) {
        qWarning(logWarning()) << "CryptoManager::decryptText - date criptate incomplete";
        return QByteArray();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning(logWarning()) << "CryptoManager::decryptText - EVP_CIPHER_CTX_new a esuat";
        return QByteArray();
    }

    QByteArray plain(data.cipherText.size(), 0);
    int len = 0;
    int plainLen = 0;
    bool success = true;

    do {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            success = false;
            break;
        }

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, data.iv.size(), nullptr) != 1) {
            success = false;
            break;
        }

        if (EVP_DecryptInit_ex(ctx,
                               nullptr,
                               nullptr,
                               reinterpret_cast<const unsigned char*>(realKey.constData()),
                               reinterpret_cast<const unsigned char*>(data.iv.constData())) != 1) {
            success = false;
            break;
        }

        if (EVP_DecryptUpdate(ctx,
                              reinterpret_cast<unsigned char*>(plain.data()),
                              &len,
                              reinterpret_cast<const unsigned char*>(data.cipherText.constData()),
                              data.cipherText.size()) != 1) {
            success = false;
            break;
        }
        plainLen = len;

        if (EVP_CIPHER_CTX_ctrl(ctx,
                                EVP_CTRL_GCM_SET_TAG,
                                data.tag.size(),
                                const_cast<char*>(data.tag.constData())) != 1) {
            success = false;
            break;
        }

        if (EVP_DecryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char*>(plain.data()) + plainLen,
                                &len) != 1) {
            success = false;
            break;
        }

        plainLen += len;
        plain.resize(plainLen);

    } while (false);

    EVP_CIPHER_CTX_free(ctx);

    if (!success) {
        qWarning(logWarning()) << "CryptoManager::decryptText - decriptarea/autentificarea a esuat";
        return QByteArray();
    }

    if (ok)
        *ok = true;

    return plain;
}
