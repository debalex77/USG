#ifndef SQLITE_CONNECTION_H
#define SQLITE_CONNECTION_H

#include "common/maindatabaseconnectioncontext.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>

namespace SqliteConnection {
inline bool isSqlite(const QSqlDatabase &db)
{
    return db.driverName().compare(QStringLiteral("QSQLITE"), Qt::CaseInsensitive) == 0
        || db.driverName().compare(QStringLiteral("QSQLCIPHER"), Qt::CaseInsensitive) == 0;
}
inline QString driver(const MainDatabaseConnectionData &config = MainDatabaseConnectionContext::instance().data())
{
    return config.sqliteEncrypted
        ? QStringLiteral("QSQLCIPHER") : QStringLiteral("QSQLITE");
}
inline bool open(QSqlDatabase &db, QString *error = nullptr,
                 const MainDatabaseConnectionData &config = MainDatabaseConnectionContext::instance().data())
{
    auto fail = [&](const QString &message) {
        if (error) *error = message;
        db.close();
        return false;
    };
    const bool encrypted = db.driverName().compare(QStringLiteral("QSQLCIPHER"),
                                                    Qt::CaseInsensitive) == 0;
    if (encrypted && config.sqliteKey.isEmpty())
        return fail(QObject::tr("SQLCipher: cheia lipsește. Setați USG_SQLCIPHER_KEY înainte de lansare."));
    if (!db.open()) return fail(db.lastError().text());
    QSqlQuery query(db);
    if (encrypted) {
        // Key before any schema access. Do not log this query or its errors.
        QString key = config.sqliteKey;
        key.replace(QLatin1Char('\''), QStringLiteral("''"));
        if (!query.exec(QStringLiteral("PRAGMA key = '%1'").arg(key)))
            return fail(QObject::tr("SQLCipher: cheia nu a putut fi aplicată."));
        if (!query.exec(QStringLiteral("PRAGMA cipher_version")) || !query.next()
            || query.value(0).toString().isEmpty())
            return fail(QObject::tr("Driverul nu oferă criptare SQLCipher."));
    }
    if (!query.exec(QStringLiteral("SELECT count(*) FROM sqlite_master")))
        return fail(encrypted ? QObject::tr("SQLCipher: cheie incorectă, format incompatibil sau bază deteriorată.")
                              : query.lastError().text());
    query.finish();
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys = ON")))
        return fail(query.lastError().text());
    return true;
}
}
#endif
