#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <data/loggingcategories.h>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QDebug>
#include <QRandomGenerator>

#if defined(Q_OS_WIN)
    #include <QStandardPaths>
    #include <3rdparty/openssl/include/openssl/evp.h>
    #include <3rdparty/openssl/include/openssl/rand.h>
#elif defined(Q_OS_LINUX)
    #include <openssl/evp.h>
    #include <openssl/rand.h>
#endif

class CryptoManager
{
public:

    // generarea parolei
    static QByteArray generateRandomKey();

    /************************************************************************************
     **
     ** O funcție criptografic sigură din OpenSSL:
     **  - IV este un set de biți aleatori folosiți pentru a face criptarea mai sigură
     **  - IV-ul trebuie să fie unic pentru fiecare criptare, dar nu trebuie să fie secret
     **  - IV-ul ajută la generarea unui text criptat diferit pentru fiecare sesiune
     **  - IV înseamnă Initialization Vector și previne atacurile asupra datelor criptate
     **  - Fără IV, textul criptat ar fi identic pentru aceleași date și cheie.
     **  - IV-ul trebuie să fie unic pentru fiecare criptare, dar poate fi stocat împreună cu datele criptate.
     **
     ************************************************************************************/
    static QByteArray generateRandomIV();

    // criptarea/decriptarea parolei
    static QByteArray encryptPassword(const QString &password, const QByteArray &key, QByteArray &iv);
    static QByteArray decryptPassword(const QByteArray &encryptedPassword, const QByteArray &key, const QByteArray &iv);

    // criptarea/decriptarea fisierului
    static bool encryptEmailDataToFile(const QString &filePath, const QString &email, const QString &username, const QString &password, const QByteArray &key);
    static bool decryptEmailDataFromFile(const QString &filePath, QString &email, QString &username, QString &password, const QByteArray &key);
};

#endif // CRYPTOMANAGER_H
