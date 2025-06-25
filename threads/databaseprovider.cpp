#include "databaseprovider.h"

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject{parent}
{}

QSqlDatabase DatabaseProvider::getDatabaseThread(const QString &connectionName, bool mysql, QString prefixConn)
{
    if (QSqlDatabase::contains(connectionName))
        return QSqlDatabase::database(connectionName);

    const QString driver = mysql ? QStringLiteral("QMYSQL")
                                 : QStringLiteral("QSQLITE");

    QSqlDatabase db = QSqlDatabase::addDatabase(driver, connectionName);
    if (mysql) {
        db.setHostName(globals().mySQLhost);
        db.setDatabaseName(globals().mySQLnameBase);
        db.setPort(globals().mySQLport.toInt());
        db.setConnectOptions(globals().mySQLoptionConnect);
        db.setUserName(globals().mySQLuser);
        db.setPassword(globals().mySQLpasswdUser);
        if (! db.open()) {
            qCritical(logCritical()) << this->metaObject()->className()
                                     << "[getDatabaseThread()]"
                                     << prefixConn + " Eroare la deschiderea bazei de date(MYSQL):"
                                     << db.lastError().text();
        } else {
            qInfo(logInfo()) << prefixConn + " realizata conexiunea -"
                             << connectionName;
        }
    } else {
        db.setHostName(globals().sqliteNameBase);
        db.setDatabaseName(globals().sqlitePathBase);
        if (! db.open()) {
            qCritical(logCritical()) << this->metaObject()->className()
                                     << "[getDatabaseThread()]"
                                     << prefixConn + " Eroare la deschiderea bazei de date(sqlite):"
                                     << db.lastError().text();
        } else {
            qInfo(logInfo()) << prefixConn + " realizata conexiunea -"
                             << connectionName;
        }
    }

    return db;
}

QSqlDatabase DatabaseProvider::getDatabaseImagesThread(const QString &connectionName)
{
    if (QSqlDatabase::contains(connectionName))
        return QSqlDatabase::database(connectionName);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setHostName("db_image");
    db.setDatabaseName(globals().pathImageBaseAppSettings);
    if (! db.open()) {
        qWarning(logWarning()) << this->metaObject()->className()
                               << "[getDatabaseThread()]"
                               << "[THREAD] Eroare la deschiderea bazei de date(db_image 'sqlite'):"
                               << db.lastError().text();
    } else {
        qInfo(logInfo()) << "[THREAD] realizata conexiunea (db_image) -" << connectionName;
    }
    return db;
}

QSqlDatabase DatabaseProvider::getDatabaseSyncThread(const QString &connectionName, bool mysql)
{
    if (! mysql)
        return QSqlDatabase();

    if (QSqlDatabase::contains(connectionName))
        return QSqlDatabase::database(connectionName);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    db.setHostName(globals().cloud_host);
    db.setDatabaseName(globals().cloud_nameBase);
    db.setPort(globals().cloud_port.toInt());
    db.setConnectOptions(globals().cloud_optionConnect);
    db.setUserName(globals().cloud_user);
    db.setPassword(globals().cloud_passwd);
    if (! db.open()) {
        qWarning(logWarning()) << this->metaObject()->className()
                               << "[getDatabaseThread()]"
                               << "[SYNC] Eroare la deschiderea bazei de date(db_image 'sqlite'):"
                               << db.lastError().text();
    } else {
        qInfo(logInfo()) << "[SYNC] realizata conexiunea -" << connectionName;
    }
    return db;
}

bool DatabaseProvider::containConnection(const QString &connectionName)
{
    return QSqlDatabase::contains(connectionName);
}

void DatabaseProvider::removeDatabaseThread(const QString &connectionName, QString prefixConn)
{
    QSqlDatabase::removeDatabase(connectionName);
    if (QSqlDatabase::contains(connectionName))
        qWarning(logWarning()) << QStringLiteral("%1 eroare la eliminare conexiunei - %2")
            .arg(prefixConn, connectionName);
    else
        qInfo(logInfo()) << QStringLiteral("%1 eliminarea cu succes conexiunei - %2")
            .arg(prefixConn, connectionName);
}
