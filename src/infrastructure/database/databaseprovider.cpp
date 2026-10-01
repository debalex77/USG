/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "databaseprovider.h"
#include "common/cloudconnectioncontext.h"
#include "common/maindatabaseconnectioncontext.h"

DatabaseProvider::DatabaseProvider(QObject *parent)
    : QObject{parent}
{

}

QSqlDatabase DatabaseProvider::getDatabaseThread(const QString &connectionName, bool mysql, QString prefixConn)
{
    if (QSqlDatabase::contains(connectionName))
        return QSqlDatabase::database(connectionName);

    const QString driver = mysql ? QStringLiteral("QMYSQL")
                                 : QStringLiteral("QSQLITE");

    const MainDatabaseConnectionData connection =
        MainDatabaseConnectionContext::instance().data();
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, connectionName);
    if (mysql) {
        db.setHostName(connection.hostName);
        db.setDatabaseName(connection.databaseName);
        db.setPort(connection.port);
        db.setConnectOptions(connection.connectionOptions);
        db.setUserName(connection.userName);
        db.setPassword(connection.password);
        if (! db.open()) {
            qCritical(logCritical()).noquote()
                << prefixConn << this->metaObject()->className()
                << "[getDatabaseThread()] Eroare la deschiderea bazei de date(MYSQL):"
                << db.lastError().text();
        } else {
            qInfo(logInfo()).noquote() << prefixConn << "realizata conexiunea -"
                                       << connectionName;
        }
    } else {
        db.setHostName(connection.sqliteDatabaseName);
        db.setDatabaseName(connection.sqliteDatabasePath);
        if (! db.open()) {
            qCritical(logCritical()).noquote()
                << prefixConn << this->metaObject()->className()
                << "[getDatabaseThread()] Eroare la deschiderea bazei de date(sqlite):"
                << db.lastError().text();
        } else {
            QSqlQuery pragma(db);
            if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
                qCritical(logCritical()).noquote()
                    << prefixConn << "activarea foreign_keys pentru SQLite a eșuat:"
                    << pragma.lastError().text();
                db.close();
                return db;
            }
            qInfo(logInfo()).noquote() << prefixConn << "realizata conexiunea -"
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
    db.setDatabaseName(
        MainDatabaseConnectionContext::instance().data().imageDatabasePath);
    if (! db.open()) {
        qWarning(logWarning()).noquote()
            << "[THREAD]" << this->metaObject()->className()
            << "[getDatabaseImagesThread()] Eroare la deschiderea bazei de date(db_image 'sqlite'):"
            << db.lastError().text();
    } else {
        QSqlQuery pragma(db);
        if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
            qCritical(logCritical()).noquote()
                << "[THREAD] activarea foreign_keys pentru db_image a eșuat:"
                << pragma.lastError().text();
            db.close();
            return db;
        }
        qInfo(logInfo()).noquote() << "[THREAD] realizata conexiunea (db_image) -" << connectionName;
    }
    return db;
}

QSqlDatabase DatabaseProvider::getDatabaseSyncThread(const QString &connectionName)
{
    if (QSqlDatabase::contains(connectionName))
        return QSqlDatabase::database(connectionName);

    // Serverul de sincronizare este MariaDB indiferent de tipul bazei locale.
    const CloudConnectionData cloud = CloudConnectionContext::instance().data();
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    db.setHostName(cloud.hostName);
    db.setDatabaseName(cloud.databaseName);
    db.setPort(cloud.port);
    db.setConnectOptions(cloud.connectionOptions);
    db.setUserName(cloud.userName);
    db.setPassword(cloud.password);
    if (! db.open()) {
        qWarning(logWarning()).noquote()
            << "[SYNC]" << this->metaObject()->className()
            << "[getDatabaseSyncThread()] Eroare la deschiderea bazei de date cloud (MariaDB):"
            << db.lastError().text();
    } else {
        qInfo(logInfo()).noquote() << "[SYNC] realizata conexiunea -" << connectionName;
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
        qWarning(logWarning()).noquote() << QStringLiteral("%1 eroare la eliminare conexiunei - %2")
            .arg(prefixConn, connectionName);
    else
        qInfo(logInfo()).noquote() << QStringLiteral("%1 eliminarea cu succes conexiunei - %2")
            .arg(prefixConn, connectionName);
}
