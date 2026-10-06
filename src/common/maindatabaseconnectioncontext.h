#ifndef MAINDATABASECONNECTIONCONTEXT_H
#define MAINDATABASECONNECTIONCONTEXT_H

#include <QReadWriteLock>
#include <QString>

enum class MainDatabaseBackend
{
    None,
    MariaDb,
    SQLite
};

struct MainDatabaseConnectionData
{
    MainDatabaseBackend backend = MainDatabaseBackend::None;

    // MariaDB
    QString hostName;
    QString databaseName;
    int port = 3306;
    QString connectionOptions;
    QString userName;
    QString password;

    // SQLite
    QString sqliteDatabaseName;
    QString sqliteDatabasePath;
    QString imageDatabasePath;
    bool sqliteEncrypted = false;
    // Runtime only: never persisted in the profile.
    QString sqliteKey;

    friend bool operator==(const MainDatabaseConnectionData &,
                           const MainDatabaseConnectionData &) = default;
};

class MainDatabaseConnectionContext final
{
public:
    static MainDatabaseConnectionContext &instance();

    [[nodiscard("MainDatabaseConnectionContext::instance().data() - verifica corectitudinea datelor")]]
    MainDatabaseConnectionData data() const;

    [[nodiscard("MainDatabaseConnectionContext::instance().backend() - verifica date backand-lui")]]
    MainDatabaseBackend backend() const;

    [[nodiscard]]
    bool isMariaDb() const;

    [[nodiscard]]
    bool isSqlite() const;

    void setData(const MainDatabaseConnectionData &data);
    void clear();

private:
    MainDatabaseConnectionContext() = default;

    mutable QReadWriteLock m_lock;
    MainDatabaseConnectionData m_data;
};

#endif // MAINDATABASECONNECTIONCONTEXT_H
