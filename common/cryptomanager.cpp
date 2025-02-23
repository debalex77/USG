#include "cryptomanager.h"

QByteArray CryptoManager::generateRandomKey()
{
    QByteArray key(32, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(key.data()), key.size() / sizeof(quint32));
    return key;
}

QByteArray CryptoManager::generateRandomIV()
{
    QByteArray iv(16, 0); // IV trebuie să aibă exact 16 octeți
    if (RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size()) != 1) {
        qDebug() << "Eroare: RAND_bytes a eșuat!";
    }
    return iv;
}

QByteArray CryptoManager::encryptPassword(const QString &password, const QByteArray &key, QByteArray &iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QByteArray();

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.constData(), (const unsigned char*)iv.constData()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray encrypted(password.toUtf8().size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);
    int len = 0, encryptedLen = 0;

    if (EVP_EncryptUpdate(ctx, (unsigned char*)encrypted.data(), &len, (const unsigned char*)password.toUtf8().constData(), password.toUtf8().size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    encryptedLen += len;

    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)encrypted.data() + encryptedLen, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    encryptedLen += len;
    encrypted.resize(encryptedLen);

    EVP_CIPHER_CTX_free(ctx);

    return encrypted.toBase64();
}

QByteArray CryptoManager::decryptPassword(const QByteArray &encryptedPassword, const QByteArray &key, const QByteArray &iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QByteArray();

    QByteArray encryptedData = QByteArray::fromBase64(encryptedPassword);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.constData(), (const unsigned char*)iv.constData()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray decrypted(encryptedData.size(), 0);
    int len = 0, decryptedLen = 0;

    if (EVP_DecryptUpdate(ctx, (unsigned char*)decrypted.data(), &len, (const unsigned char*)encryptedData.constData(), encryptedData.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    decryptedLen += len;

    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)decrypted.data() + decryptedLen, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    decryptedLen += len;
    decrypted.resize(decryptedLen);

    EVP_CIPHER_CTX_free(ctx);
    return decrypted;
}

bool CryptoManager::encryptEmailDataToFile(const QString &filePath, const QString &email, const QString &username, const QString &password, const QByteArray &key)
{
    if (key.size() != 32) {
        qWarning(logWarning()) << "Eroare: cheia trebuie să aibă exact 32 de octeți!";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning(logWarning()) << "Eroare la deschiderea fișierului pentru criptare!";
        return false;
    }

    QString data = email + "\n" + username + "\n" + password;
    QByteArray arr = data.toUtf8();
    arr = arr.leftJustified(16 * ((arr.size() / 16) + 1), '\0');  // Aliniere la 16 bytes

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning(logWarning()) << "Eroare: Nu s-a putut crea contextul EVP!";
        return false;
    }

    QByteArray iv(16, 0);
    if (RAND_bytes((unsigned char*)iv.data(), iv.size()) != 1) {
        qWarning(logWarning()) << "Eroare: RAND_bytes a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.constData(), (const unsigned char*)iv.constData()) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_EncryptInit_ex a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QByteArray encrypted(arr.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);
    int len = 0, encryptedLen = 0;

    if (EVP_EncryptUpdate(ctx, (unsigned char*)encrypted.data(), &len, (const unsigned char*)arr.constData(), arr.size()) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_EncryptUpdate a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    encryptedLen += len;

    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)encrypted.data() + encryptedLen, &len) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_EncryptFinal_ex a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    encryptedLen += len;
    encrypted.resize(encryptedLen);

    EVP_CIPHER_CTX_free(ctx);

    file.write(iv.toBase64() + "\n" + encrypted.toBase64());  // Salvăm IV și datele criptate
    file.close();

    return true;
}

bool CryptoManager::decryptEmailDataFromFile(const QString &filePath, QString &email, QString &username, QString &password, const QByteArray &key)
{
    if (key.size() != 32) {
        qWarning(logWarning()) << "Eroare: cheia trebuie să aibă exact 32 de octeți!";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << "Eroare la deschiderea fișierului pentru decriptare!";
        return false;
    }

    QList<QByteArray> fileData = file.readAll().split('\n');
    if (fileData.size() < 2) {
        qWarning(logWarning()) << "Eroare: format invalid al fișierului criptat!";
        return false;
    }

    QByteArray iv = QByteArray::fromBase64(fileData[0]);
    QByteArray encrypted = QByteArray::fromBase64(fileData[1]);
    file.close();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning(logWarning()) << "Eroare: Nu s-a putut crea contextul EVP!";
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.constData(), (const unsigned char*)iv.constData()) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_DecryptInit_ex a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QByteArray decrypted(encrypted.size(), 0);
    int len = 0, decryptedLen = 0;

    if (EVP_DecryptUpdate(ctx, (unsigned char*)decrypted.data(), &len, (const unsigned char*)encrypted.constData(), encrypted.size()) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_DecryptUpdate a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    decryptedLen += len;

    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)decrypted.data() + decryptedLen, &len) != 1) {
        qWarning(logWarning()) << "Eroare: EVP_DecryptFinal_ex a eșuat!";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    decryptedLen += len;
    decrypted.resize(decryptedLen);

    EVP_CIPHER_CTX_free(ctx);

    QStringList lines = QString::fromUtf8(decrypted).trimmed().split("\n");
    if (lines.size() >= 3) {
        email    = lines[0];
        username = lines[1];
        password = lines[2];
        return true;
    }

    return false;
}
