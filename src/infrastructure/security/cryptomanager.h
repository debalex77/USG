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

#include <core/loggingcategories.h>

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
                                      QString *error = nullptr,
                                      bool *created = nullptr);

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

    // logAuthFailure = false: eșecul autentificării (cheie greșită) nu se
    // jurnalizează – pentru încercări cu mai multe chei.
    static QByteArray decryptText(const EncryptedData &data,
                                  const QByteArray &realKey,
                                  bool *ok = nullptr,
                                  bool logAuthFailure = true);

    // Parola serverului cloud (cloudServer.password = base64(cipher + tag),
    // cloudServer.iv = base64(iv)). De la 4.2.7 cheia este cea împărțită a
    // organizației; cheia veche, derivată din users.hash (SHA-256 hex), este
    // încercată doar cât baza nu este încă migrată.
    static bool decryptCloudPassword(QSqlDatabase &db,
                                     int idOrganization,
                                     const QString &passwordBase64,
                                     const QString &ivBase64,
                                     const QString &legacyUserHash,
                                     QString *plainText,
                                     bool *usedLegacyKey = nullptr,
                                     QString *error = nullptr);

private:
    static bool isValidAes256Key(const QByteArray &key);
};

#endif // CRYPTOMANAGER_H
