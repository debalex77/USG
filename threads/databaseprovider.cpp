#include "databaseprovider.h"

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject{parent}
{}

QSqlDatabase DatabaseProvider::getDatabaseThread(const QString &connectionName, bool mysql)
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
            qWarning(logWarning()) << this->metaObject()->className()
                                   << "[getDatabaseThread()]"
                                   << "[THREAD] Eroare la deschiderea bazei de date(MYSQL):"
                                   << db.lastError().text();
        } else {
            qInfo(logInfo()) << "[THREAD] realizata conexiunea -" << connectionName;
        }
    } else {
        db.setHostName(globals().sqliteNameBase);
        db.setDatabaseName(globals().sqlitePathBase);
        if (! db.open()) {
            qWarning(logWarning()) << this->metaObject()->className()
                                   << "[getDatabaseThread()]"
                                   << "[THREAD] Eroare la deschiderea bazei de date(sqlite):"
                                   << db.lastError().text();
        } else {
            qInfo(logInfo()) << "[THREAD] realizata conexiunea -" << connectionName;
        }
    }

    return db;
}

void DatabaseProvider::removeDatabaseThread(const QString &connectionName)
{
    QSqlDatabase::removeDatabase(connectionName);
    if (QSqlDatabase::contains(connectionName))
        qWarning(logWarning()) << "[THREAD] eroare la eliminare conexiunei -" << connectionName;
    else
        qInfo(logInfo()) << "[THREAD] eliminarea cu succes conexiunei -" << connectionName;
}
