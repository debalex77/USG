#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <iostream>
#include "infrastructure/database/sqliteconnection.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid() || !QSqlDatabase::isDriverAvailable("QSQLCIPHER")) return 1;
    MainDatabaseConnectionData config;
    MainDatabaseConnectionContext::instance().setData(config);
    auto plain = QSqlDatabase::addDatabase("QSQLITE", "plain");
    plain.setDatabaseName(directory.filePath("plain.db"));
    if (!SqliteConnection::open(plain)) return 2;
    QSqlQuery plainQuery(plain);
    if (!plainQuery.exec("CREATE TABLE sample (id INTEGER PRIMARY KEY)")) return 3;
    plain.close();

    config.sqliteEncrypted = true;
    config.sqliteKey = QStringLiteral("test-only-secret");
    MainDatabaseConnectionContext::instance().setData(config);
    auto cipher = QSqlDatabase::addDatabase(SqliteConnection::driver(), "cipher");
    const QString path = directory.filePath("encrypted.db");
    cipher.setDatabaseName(path);
    QString error;
    if (!SqliteConnection::open(cipher, &error)) return 4;
    QSqlQuery query(cipher);
    if (!query.exec("PRAGMA foreign_keys") || !query.next() || query.value(0).toInt() != 1) return 5;
    query.finish();
    if (!query.exec("CREATE TABLE sample (id INTEGER PRIMARY KEY)")) return 6;
    query.finish();
    cipher.close();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.read(16) == QByteArray("SQLite format 3\0", 16)) return 7;
    file.close();
    if (!SqliteConnection::open(cipher, &error)) return 8;
    cipher.close();
    config.sqliteKey = QStringLiteral("wrong-secret");
    MainDatabaseConnectionContext::instance().setData(config);
    if (SqliteConnection::open(cipher, &error) || cipher.isOpen() || error.contains(config.sqliteKey)) return 9;
    config.sqliteKey.clear();
    MainDatabaseConnectionContext::instance().setData(config);
    if (SqliteConnection::open(cipher, &error)) return 10;
    config.sqliteKey = QStringLiteral("test-only-secret");
    MainDatabaseConnectionContext::instance().setData(config);
    cipher.setDatabaseName(plain.databaseName());
    if (SqliteConnection::open(cipher, &error)) return 11;
    std::cout << "PASS: plain SQLite, encrypted file, reopen, foreign keys, wrong/missing key, plaintext rejection\n";
    return 0;
}
