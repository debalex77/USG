#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSslSocket>
#include <QTemporaryDir>

// Same sequence as SqliteConnection::open(): PRAGMA key before schema access.
static bool openEncrypted(const QString &connection, const QString &file, const QString &key)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLCIPHER"), connection);
    db.setDatabaseName(file);
    if (!db.open())
        return false;
    QSqlQuery query(db);
    return query.exec(QStringLiteral("PRAGMA key = '%1'").arg(key))
        && query.exec(QStringLiteral("PRAGMA cipher_version")) && query.next()
        && !query.value(0).toString().isEmpty()
        && query.exec(QStringLiteral("SELECT count(*) FROM sqlite_master"));
}

static int checkSqlCipher()
{
    QTemporaryDir dir;
    if (!dir.isValid())
        return 10;
    const QString file = dir.filePath(QStringLiteral("check.sqlite3"));
    const QString key = QStringLiteral("usg-runtime-check");
    int result = 0;
    {
        if (!openEncrypted(QStringLiteral("create"), file, key)) {
            result = 11;
        } else {
            QSqlQuery query(QSqlDatabase::database(QStringLiteral("create")));
            if (!query.exec(QStringLiteral("CREATE TABLE t(id INTEGER PRIMARY KEY)"))
                || !query.exec(QStringLiteral("INSERT INTO t(id) VALUES (1)")))
                result = 12;
        }
        QSqlDatabase::database(QStringLiteral("create"), false).close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("create"));
    if (result != 0)
        return result;
    {
        if (!openEncrypted(QStringLiteral("reopen"), file, key)) {
            result = 13;
        } else {
            QSqlQuery query(QStringLiteral("SELECT id FROM t"),
                            QSqlDatabase::database(QStringLiteral("reopen")));
            if (!query.next() || query.value(0).toInt() != 1)
                result = 14;
        }
        QSqlDatabase::database(QStringLiteral("reopen"), false).close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("reopen"));
    if (result != 0)
        return result;
    {
        if (openEncrypted(QStringLiteral("wrong"), file, QStringLiteral("wrong-key")))
            result = 15;
        QSqlDatabase::database(QStringLiteral("wrong"), false).close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("wrong"));
    if (result != 0)
        return result;
    QFile raw(file);
    if (!raw.open(QIODevice::ReadOnly) || raw.read(16).startsWith("SQLite format 3"))
        return 16;
    return 0;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    // Only plugins from the deployed package may satisfy this check.
    QCoreApplication::setLibraryPaths({QCoreApplication::applicationDirPath()});
    for (const auto &driver : {"QMYSQL", "QSQLITE", "QSQLCIPHER"}) {
        const auto db = QSqlDatabase::addDatabase(driver, driver);
        if (!db.isValid()) {
            qCritical() << "Cannot load deployed SQL driver" << driver;
            return 1;
        }
    }
    if (!QSslSocket::supportsSsl()) {
        qCritical() << "No working deployed TLS backend";
        return 2;
    }
    if (const int code = checkSqlCipher(); code != 0) {
        qCritical() << "Deployed QSQLCIPHER failed the encrypted database check, code" << code;
        return code;
    }
    qInfo() << "SQL drivers, SQLCipher encryption and TLS loaded successfully";
    return 0;
}
