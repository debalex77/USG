#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QByteArray>
#include <QString>
#include <QSqlDatabase>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <data/loggingcategories.h>

#if defined(Q_OS_WIN)
#include <3rdparty/openssl/include/openssl/evp.h>
#include <3rdparty/openssl/include/openssl/rand.h>
#elif defined(Q_OS_LINUX)
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

class CryptoManager
{
public:
    struct EncryptedData
    {
        QByteArray cipherText; // raw
        QByteArray iv;         // raw
        QByteArray tag;        // raw

        bool isValid() const
        {
            return !cipherText.isEmpty() && !iv.isEmpty() && !tag.isEmpty();
        }
    };

    static QByteArray generateRandomBytes(int size);
    static QByteArray generateRandomIV(int size = 12);

    static QString toBase64(const QByteArray &data);
    static QByteArray fromBase64(const QString &data);

    static QString localKeyPartFilePath(int idOrganization);

    static QByteArray loadOrCreateLocalKeyPart(int idOrganization, bool *ok = nullptr);

    static bool loadOrCreateDbKeyPart(QSqlDatabase &db,
                                      int idOrganization,
                                      QByteArray *keyPart1,
                                      QString *error = nullptr);

    static QByteArray deriveRealKey(const QByteArray &keyPart1,
                                    const QByteArray &keyPart2);

    static QByteArray deriveCloudKey(const QByteArray &userHash,
                                     int idOrganization);

    static bool loadOrCreateSplitKey(QSqlDatabase &db,
                                     int idOrganization,
                                     QByteArray *realKey,
                                     QString *error = nullptr);

    static EncryptedData encryptText(const QString &plainText,
                                     const QByteArray &realKey);

    static QByteArray decryptText(const EncryptedData &data,
                                  const QByteArray &realKey,
                                  bool *ok = nullptr);

private:
    static bool isValidAes256Key(const QByteArray &key);
};

#endif // CRYPTOMANAGER_H
