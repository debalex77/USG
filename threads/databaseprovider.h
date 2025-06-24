#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <data/globals.h>
#include <data/loggingcategories.h>

class DatabaseProvider : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseProvider(QObject *parent = nullptr);

    QSqlDatabase getDatabaseThread(const QString &connectionName, bool mysql, QString prefixConn = "[THREAD]");
    QSqlDatabase getDatabaseImagesThread(const QString &connectionName);
    QSqlDatabase getDatabaseSyncThread(const QString &connectionName = "cloud_sync_conn", bool mysql = true);

    bool containConnection(const QString &connectionName);
    void removeDatabaseThread(const QString &connectionName, QString prefixConn = "[THREAD]");

};

#endif // DATABASEPROVIDER_H
