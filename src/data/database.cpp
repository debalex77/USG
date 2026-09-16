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

/********************************************************************
 **
 ** https://onlinetools.com/utf8/convert-utf8-to-octal
 **
 ** \304\203  - ă
 ** \303\242  - â
 ** \310\230  - Ș
 ** \310\231  - ș
 ** \310\233  = ț
 ** \303\256  = î
 **
 ********************************************************************/

#include "database.h"
#include "legacysettingscodec.h"
#include <QDomDocument>
#include <QThread>
#include <QUuid>
#include <QSqlDriver>

DataBase::DataBase(QObject *parent) : QObject(parent)
{
}

DataBase::~DataBase()
{
}

// *******************************************************************
// ******************* CONCTAREA LA BAZA DE DATE *********************

bool DataBase::connectToDataBase()
{
    if (globals().connectionMade == "MySQL"){
        return openDataBase();
    } else {
        if (QFile(globals().sqlitePathBase).exists()) {
            return openDataBase();
        } else {
            return restoreDataDase();
        }
    }
    return false;
}

bool DataBase::createConnectBaseSqlite(QString &txtMessage)
{
    return createConnectBaseSqlite(globals().sqliteNameBase,
                                   globals().sqlitePathBase,
                                   globals().firstLaunch,
                                   txtMessage);
}

bool DataBase::createConnectBaseSqlite(const QString &databaseName,
                                       const QString &databasePath,
                                       bool initializeSchema,
                                       QString &txtMessage)
{
    if (QFileInfo::exists(databasePath)) {
        txtMessage = tr("Baza de date <b>\"%1\"</b> deja este creata !!! <br>%2")
                         .arg(databaseName, databasePath);
        return true;
    }

    const QString connectionName = QStringLiteral("create_sqlite_%1")
                                       .arg(reinterpret_cast<quintptr>(this));
    bool created = false;
    QString errorText;
    {
        QSqlDatabase newDatabase = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                                              connectionName);
        newDatabase.setHostName(databaseName);
        newDatabase.setDatabaseName(databasePath);
        created = newDatabase.open();
        if (created && initializeSchema)
            created = DataBaseCommon::createAllTablesSqlite(newDatabase);
        if (!created)
            errorText = newDatabase.lastError().text();
        newDatabase.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (created) {
        txtMessage = tr("Baza de date \"<b>%1</b>\" este creata cu succes.")
                         .arg(databaseName);
        return true;
    }

    txtMessage = tr("Baza de date \"<b>%1</b>\" nu este creata. <br>"
                    "Verificati corectitudinea localizarii fisierului cu baza de date "
                    "sau adresati-va administratorului aplicatiei.<br>%2")
                     .arg(databaseName, errorText);
    return false;
}

QSqlDatabase DataBase::getDatabase()
{
    return db.database();
}

QSqlDatabase DataBase::getDatabaseThread(const QString threadConnectionName, const bool thisMySQL)
{
    qInfo(logInfo()) << "Se initiaza 'Thread' connection: " << threadConnectionName;
    if (! QSqlDatabase::contains(threadConnectionName)) {
        if (thisMySQL) {
            QSqlDatabase db_thread = QSqlDatabase::addDatabase("QMYSQL", threadConnectionName);
            db_thread.setHostName(globals().mySQLhost);
            db_thread.setDatabaseName(globals().mySQLnameBase);
            db_thread.setPort(globals().mySQLport.toInt());
            db_thread.setConnectOptions(globals().mySQLoptionConnect);
            db_thread.setUserName(globals().mySQLuser);
            db_thread.setPassword(globals().mySQLpasswdUser);
            if (! db_thread.open()) {
                qWarning(logWarning()) << "Eroare <thread> la deschiderea bazei de date(MYSQL):"
                                       << db_thread.lastError().text();
            }
            return db_thread;
        } else {
            QSqlDatabase db_thread = QSqlDatabase::addDatabase("QSQLITE", threadConnectionName);
            db_thread.setHostName(globals().sqliteNameBase);
            db_thread.setDatabaseName(globals().sqlitePathBase);
            if (! db_thread.open()) {
                qWarning(logWarning()) << "Eroare <thread> la deschiderea bazei de date(sqlite):"
                                       << db_thread.lastError().text();
            }
            return db_thread;
        }
    }
    return QSqlDatabase::database(threadConnectionName);
}

QSqlDatabase DataBase::getDatabaseCloudThread(const QString threadConnectionName)
{
    qInfo(logInfo()) << "Se initiaza 'Cloud Thread' connection: "
                     << threadConnectionName;
    if (! QSqlDatabase::contains(threadConnectionName)) {
        QSqlDatabase db_thread = QSqlDatabase::addDatabase("QMYSQL", threadConnectionName);
        db_thread.setHostName(globals().cloud_host);
        db_thread.setDatabaseName(globals().cloud_nameBase);
        db_thread.setPort(globals().cloud_port.toInt());
        db_thread.setConnectOptions(globals().cloud_optionConnect);
        db_thread.setUserName(globals().cloud_user);
        db_thread.setPassword(globals().cloud_passwd);
        if (! db_thread.open()) {
            qWarning(logWarning()) << "Eroare <cloud thread> la deschiderea bazei de date(MYSQL):"
                                   << db_thread.lastError().text();
        }
        return db_thread;
    }
    return QSqlDatabase::database(threadConnectionName);
}

QSqlDatabase DataBase::getDatabaseImageThread(const QString threadConnectionName)
{
    qInfo(logInfo()) << "Se initiaza 'Thread' connection 'Image': "
                     << threadConnectionName;
    if (! QSqlDatabase::contains(threadConnectionName)) {

        QSqlDatabase db_threadImage = QSqlDatabase::addDatabase("QSQLITE", threadConnectionName);
        db_threadImage.setHostName("db_image");
        db_threadImage.setDatabaseName(globals().pathImageBaseAppSettings);
        if (! db_threadImage.open()) {
            qWarning(logWarning()) << "Eroare la deschiderea bazei de date în thread:"
                                   << db_threadImage.lastError().text();
        }
        return db_threadImage;
    }
    return QSqlDatabase::database(threadConnectionName);
}

void DataBase::removeDatabaseThread(const QString threadConnectionName)
{
    qInfo(logInfo()) << "Initierea eliminarii 'Thread' connection:"
                     << threadConnectionName;

    if (QSqlDatabase::contains(threadConnectionName)) {
        {
            // Preluăm conexiunea cu open = false, pentru a NU o redeschide din greșeală.
            QSqlDatabase tempDb = QSqlDatabase::database(threadConnectionName, false);
            // qDebug() << "tempDb.isOpen() =" << tempDb.isOpen();
            if (tempDb.isValid() && tempDb.isOpen()) {
                tempDb.close();
            }
        }

        if (QSqlDatabase::contains(threadConnectionName)) {
            // qInfo(logInfo()) << "Lista conexiuni inainte de eliminare:" << QSqlDatabase::connectionNames();

            // info de eliminare a conexiunei
            qInfo(logInfo()) << "Se inchide conexiunea"
                             << threadConnectionName
                             << "isOpen="
                             << QSqlDatabase::database(threadConnectionName, false).isOpen();

            QSqlDatabase::removeDatabase(threadConnectionName);

            // qInfo(logInfo()) << "Lista conexiuni dupa eliminare 'Thread' connection:" << QSqlDatabase::connectionNames();
        }
    }

    if (! QSqlDatabase::contains(threadConnectionName))
        qInfo(logInfo()) << "Connection 'Thread' -" << threadConnectionName
                         << " a fost eliminata cu succes.";
}

QSqlDatabase DataBase::getDatabaseImage()
{
    return db_image.database("db_image");
}

int DataBase::getNextNumberDoc(const QString &docName, int year, QString *err)
{
    if (err)
        err->clear();

    QSqlDatabase db = getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        if (err) *err = tr("Conexiunea la baza de date nu este deschisă.");
        return -1;
    }

    if (docName != QStringLiteral("orderEcho")
        && docName != QStringLiteral("reportEcho")) {
        if (err) *err = tr("Tip de document nesuportat pentru numerotare: %1").arg(docName);
        return -1;
    }

    if (year < 1900 || year > 9999) {
        if (err) *err = tr("An invalid pentru numerotarea documentului: %1").arg(year);
        return -1;
    }

    // IMPORTANT:
    // Nu pornim aici tranzacție dacă funcția este apelată dintr-o tranzacție deja pornită în onSave().
    // Deci funcția presupune că apelantul controlează tranzacția.

    QSqlQuery q(db);

    if (globals().thisMySQL) {
        if (!q.prepare(R"(
            INSERT INTO doc_sequences(name, year, value)
            VALUES (?, ?, 0)
            ON DUPLICATE KEY UPDATE value = value
        )")) {
            if (err) *err = q.lastError().text();
            return -1;
        }
        q.addBindValue(docName);
        q.addBindValue(year);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (!q.prepare(R"(
            UPDATE doc_sequences
            SET value = LAST_INSERT_ID(value + 1)
            WHERE name = ? AND year = ?
        )")) {
            if (err) *err = q.lastError().text();
            return -1;
        }
        q.addBindValue(docName);
        q.addBindValue(year);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (q.numRowsAffected() != 1) {
            if (err) *err = tr("Secvența documentului nu a fost actualizată.");
            return -1;
        }

        if (!q.exec("SELECT LAST_INSERT_ID()")) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (!q.next()) {
            if (err) *err = tr("Nu s-a putut citi următorul număr de document.");
            return -1;
        }

        bool numberOk = false;
        const int number = q.value(0).toInt(&numberOk);
        if (!numberOk || number <= 0) {
            if (err) *err = tr("Numărul documentului returnat nu este valid.");
            return -1;
        }

        return number;
    }

    if (globals().thisSqlite) {
        if (!q.prepare(R"(
            INSERT OR IGNORE INTO doc_sequences(name, year, value)
            VALUES (?, ?, 0)
        )")) {
            if (err) *err = q.lastError().text();
            return -1;
        }
        q.addBindValue(docName);
        q.addBindValue(year);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (!q.prepare(R"(
            UPDATE doc_sequences
            SET value = value + 1
            WHERE name = ? AND year = ?
        )")) {
            if (err) *err = q.lastError().text();
            return -1;
        }
        q.addBindValue(docName);
        q.addBindValue(year);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (q.numRowsAffected() != 1) {
            if (err) *err = tr("Secvența documentului nu a fost actualizată.");
            return -1;
        }

        if (!q.prepare(R"(
            SELECT value
            FROM doc_sequences
            WHERE name = ? AND year = ?
        )")) {
            if (err) *err = q.lastError().text();
            return -1;
        }
        q.addBindValue(docName);
        q.addBindValue(year);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return -1;
        }

        if (!q.next()) {
            if (err) *err = tr("Nu s-a putut citi următorul număr de document.");
            return -1;
        }

        bool numberOk = false;
        const int number = q.value(0).toInt(&numberOk);
        if (!numberOk || number <= 0) {
            if (err) *err = tr("Numărul documentului returnat nu este valid.");
            return -1;
        }

        return number;
    }

    if (err) *err = tr("Tip de bază de date nesuportat.");
    return -1;
}

QString DataBase::formatDatabaseDate(const QString &rawDate)
{
    // Formatul intern din baza de date:
    // SQLite -> "yyyy-MM-dd HH:mm:ss" (cu spatiu)
    // MySQL  -> "yyyy-MM-ddTHH:mm:ss" (cu 'T')
    const QString format = globals().thisMySQL
                           ? "yyyy-MM-ddTHH:mm:ss"     // MySQL = ISO cu 'T'
                           : "yyyy-MM-dd HH:mm:ss";    // SQLite = spatiu

    QDateTime dt = QDateTime::fromString(rawDate, format);
    return dt.isValid()
           ? dt.toString("dd.MM.yyyy HH:mm:ss")
           : rawDate;
}

bool DataBase::insertIntoTable(const QString class_name,
                               const QString name_table,
                               const QVariantMap &values,
                               QStringList &err,
                               QVariant *insertedId)
{
    // verificam daca sunt transmise date
    if (values.isEmpty()) {
        err << class_name
            << "[insertIntoTable]: "
            << "Table: " + name_table
            << "Nu s-au furnizat date pentru inserare.";
        return false;
    }

    // anuntam variabile necesare
    QStringList columns;
    QStringList placeholders;
    QList<QVariant> orderedValues;

    // Construim coloanele și valorile în ordine
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        columns << it.key();
        placeholders << "?";               // folosit cu addBindValue
        orderedValues.append(it.value());  // păstrăm ordinea
    }

    // textul solicitarii
    QString queryStr = QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(name_table,
                                columns.join(", "),
                                placeholders.join(", "));

    // prepararea solicitarii
    QSqlQuery qry;
    if (! qry.prepare(queryStr)) {
        err << class_name
            << "[insertIntoTable]: "
            << "Table: " + name_table
            << "Eroare la pregătirea INSERT: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    // Asociere valori - addBindValue
    for (const auto &val : orderedValues) {
        qry.addBindValue(val);
    }

    // executarea solicitarii
    if (! qry.exec()) {
        err << class_name
            << "[insertIntoTable]: "
            << "Table: " + name_table
            << "Eroare la execuția INSERT: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    // returnam ID inserat daca e solicitat
    if (insertedId != nullptr)
        *insertedId = qry.lastInsertId();

    return true;
}

bool DataBase::updateTable(const QString class_name,
                           const QString name_table,
                           const QVariantMap &values,
                           const QMap<QString, QVariant> &where_conditions,
                           QStringList &err)
{
    if (values.isEmpty()) {
        err << class_name
            << "[updateTable]: "
            << "Table: " + name_table
            << "Nu s-au furnizat date pentru actualizare.";
        return false;
    }

    if (where_conditions.isEmpty()) {
        err << class_name
            << "[updateTable]: "
            << "Table: " + name_table
            << "Condiția WHERE este goală. Actualizare interzisă.";
        return false;
    }

    QStringList set_clauses;
    QList<QVariant> bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        set_clauses << QString("%1 = ?").arg(it.key());
        bindValues << it.value();
    }

    QStringList where_clauses;
    for (auto it = where_conditions.constBegin(); it != where_conditions.constEnd(); ++it) {
        where_clauses << QString("%1 = ?").arg(it.key());
        bindValues << it.value();
    }

    QString queryStr = QString("UPDATE %1 SET %2 WHERE %3")
                           .arg(name_table,
                                set_clauses.join(", "),
                                where_clauses.join(" AND "));

    QSqlQuery qry;
    if (! qry.prepare(queryStr)) {
        err << class_name
            << "[updateTable]: "
            << "Table: " + name_table
            << "Eroare la pregătirea UPDATE: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    for (int i = 0; i < bindValues.size(); ++i) {
        qry.addBindValue(bindValues.at(i));
    }

    if (! qry.exec()) {
        err << class_name
            << "[updateTable]: "
            << "Table: " + name_table
            << "Eroare la execuția UPDATE: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    return true;

}

QVariantMap DataBase::selectSingleRow(const QString class_name,
                                      const QString name_table,
                                      const QVariantMap &values,
                                      const QMap<QString,
                                      QVariant> &where_conditions,
                                      QStringList &err)
{
    QVariantMap result;

    if (values.isEmpty()) {
        err << class_name
            << "[selectSingleRow]: "
            << "Nu s-au specificat coloane pentru SELECT.";
        return result;
    }

    if (name_table.trimmed().isEmpty()) {
        err << class_name
            << "[selectSingleRow]: "
            << "Numele tabelului este gol.";
        return result;
    }

    // construim sectiile selectate
    QString columnStr;
    QStringList select_columns;

    if (values.size() == 1 && values.contains("*")) {
        columnStr = "*";
    } else {
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            select_columns << it.key();
        }
        columnStr = select_columns.join(", ");
    }

    QString queryStr = QString("SELECT %1 FROM %2").arg(columnStr, name_table);

    QList<QVariant> bindValues;

    // construim conditiile
    if (! where_conditions.isEmpty()) {
        QStringList where_clauses;
        for (auto it = where_conditions.constBegin(); it != where_conditions.constEnd(); ++it) {
            where_clauses << QString("%1 = ?").arg(it.key());
            bindValues << it.value();
        }
        queryStr += " WHERE " + where_clauses.join(" AND ");
    }

    // LIMIT 1, pentru siguranță
    queryStr += " LIMIT 1";

    QSqlQuery qry;
    if (! qry.prepare(queryStr)) {
        err << class_name
            << "[selectSingleRow]: "
            << "Eroare la pregătirea SELECT: "
            << qry.lastError().text();
        return result;
    }

    for (int i = 0; i < bindValues.size(); ++i) {
        qry.addBindValue(bindValues.at(i));
    }

    // executam
    if (! qry.exec()) {
        err << class_name
            << "[selectSingleRow]: "
            << "Eroare la execuția SELECT: "
            << qry.lastError().text();
        return result;
    }

    if (qry.next()) {
        if (columnStr == "*") {
            QSqlRecord record = qry.record();
            for (int i = 0; i < record.count(); ++i) {
                result[record.fieldName(i)] = qry.value(i);
            }
        } else {
            for (int i = 0; i < select_columns.size(); ++i) {
                const QString &col = select_columns.at(i);
                result[col] = qry.value(col);
            }
        }
    }

    return result;
}

QVariantMap DataBase::selectJoinConstantsUserPreferencesByUserId(const int id_user)
{
    QVariantMap result;

    if (id_user == -1 || id_user == 0)
        return result;

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            c.*,
            u.*
        FROM
            userPreferences AS u
        LEFT JOIN
            constants AS c ON c.id_users = u.id_users
        WHERE
            u.id_users = ?
    )");
    qry.addBindValue(id_user);
    if (qry.exec()) {
        if (qry.next()) {
            QSqlRecord rec = qry.record();
            for (int i = 0; i < rec.count(); ++i) {
                result.insert(rec.fieldName(i), qry.value(i));
            }
        }
    } else {
        qWarning(logWarning()) << this->metaObject()->className()
                               << "[selectJoinConstantsUserPreferencesByUserId]:"
                               << "Eroare SELECT:" << qry.lastError().text();
    }

    return result;
}

bool DataBase::deleteFromTable(const QString class_name,
                               const QString name_table,
                               const QMap<QString, QVariant> &where_conditions,
                               QStringList &err)
{
    if (name_table.trimmed().isEmpty()) {
        err << class_name
            << "[deleteFromTable]: "
            << "Table: " + name_table
            << "Numele tabelului este gol.";
        return false;
    }

    if (where_conditions.isEmpty()) {
        err << class_name
            << "[deleteFromTable]: "
            << "Table: " + name_table
            << "Condiția WHERE este goală — ștergere blocată pentru siguranță.";
        return false;
    }

    QStringList where_clauses;
    QList<QVariant> bindValues;

    for (auto it = where_conditions.constBegin(); it != where_conditions.constEnd(); ++it) {
        where_clauses << QString("%1 = ?").arg(it.key());
        bindValues << it.value();
    }

    QString queryStr = QString("DELETE FROM %1 WHERE %2")
                           .arg(name_table,
                                where_clauses.join(" AND "));

    QSqlQuery qry;
    if (! qry.prepare(queryStr)) {
        err << class_name
            << "[deleteFromTable]: "
            << "Table: " + name_table
            << "Eroare la pregătirea DELETE: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    for (int i = 0; i < bindValues.size(); ++i) {
        qry.addBindValue(bindValues.at(i));
    }

    if (! qry.exec()) {
        err << class_name
            << "[deleteFromTable]: "
            << "Table: " + name_table
            << "Eroare la execuția DELETE: "
            << qry.lastError().text()
            << "Query: " + queryStr;
        return false;
    }

    return true;
}

bool DataBase::deleteDataFromTable(const QString name_table, const QString deletionCondition, const QString valueCondition)
{
    QSqlQuery qry;
    QString str_qry;
    if (deletionCondition == nullptr)
        str_qry = QString("DELETE FROM %1;").arg(name_table);
    else
        str_qry = QString("DELETE FROM %1 WHERE %2 = '%3';").arg(name_table, deletionCondition, valueCondition);

    qry.prepare(str_qry);
    if (qry.exec())
        return true;

    return false;
}

QString DataBase::getTextSQL(const QString &resourcePath)
{
    return db_common.getTextQryFromResource(resourcePath);
}

bool DataBase::execPreparedFromFile(QSqlDatabase database,
                                    const QString &sqlPath,
                                    const QVector<QVariant> &binds,
                                    QString *err)
{
    return db_common.execPreparedFromFile(database, sqlPath, binds, err);
}

bool DataBase::execPreparedFromFileReturnID(QSqlDatabase database,
                                            const QString &sqlPath,
                                            const QVector<QVariant> &binds,
                                            QVariant *lastInsertId,
                                            QString *err)
{
    return db_common.execPreparedFromFileReturnID(database, sqlPath, binds, lastInsertId, err);
}

void DataBase::setModelQuery(QSqlQueryModel &model, QSqlDatabase db, QString sql, const QVariantList &binds, QVariantMap other)
{
    // pu typesPrices
    bool noncomercial = false;
    if (other.contains("noncomercial"))
        noncomercial = other["noncomercial"].toBool();

    // schimbam valorile necesare
    if (globals().thisSqlite) {

        // :/sql/queries_print/tablePatientByID.sql
        sql.replace("%fullName%",
                    "patients.last_name || ' ' || patients.first_name AS fullName");
        sql.replace("%birthday%",
                    "strftime('%d.%m.%Y', patients.birthday) AS birthday");

        // :/sql/queries_print/orderTable.sql
        sql.replace("%price%",
                    noncomercial
                        ? "'0-00'"
                        : "printf('%.2f', tab.price)");
    }
    else { // MySQL / MariaDB

        // :/sql/queries_print/tablePatientByID.sql
        sql.replace("%fullName%",
                    "CONCAT(patients.last_name,' ',patients.first_name) AS fullName");
        sql.replace("%birthday%",
                    "DATE_FORMAT(patients.birthday,'%d.%m.%Y') AS birthday");

        // :/sql/queries_print/orderTable.sql
        sql.replace("%price%",
                    noncomercial
                        ? "'0-00'"
                        : "FORMAT(tab.price, 2)");
    }

    QSqlQuery qry(db);

    if (!qry.prepare(sql)) {
        qDebug() << "Prepare error:" << qry.lastError().text();
        qDebug() << sql;
        return;
    }

    // adaugarea parametrilor în ordinea aparitiei
    for (const QVariant &v : binds)
        qry.addBindValue(v);

    if (!qry.exec()) {
        qDebug() << "Exec error:" << qry.lastError().text();
        qDebug() << qry.lastQuery();
        qDebug() << "Values:" << qry.boundValues();
        return;
    }

    model.setQuery(std::move(qry));
}

bool DataBase::deleteDocByID(const QString nameTable, const int id)
{
    QSqlQuery query;

    query.prepare(QStringLiteral("DELETE FROM %1 WHERE id = ?")
                      .arg(nameTable));
    query.addBindValue(id);

    if (!query.exec()) {
        qCritical(logCritical()).noquote()
        << "Delete error:"
        << query.lastError().text();
        return false;
    }

    return true;
}

// *******************************************************************
// *************** CREAREA TABELELOR, OBIECTELOR *********************

bool DataBase::creatingTables()
{
    if (globals().thisSqlite)
        return db_common.createAllTablesSqlite();
    if (globals().thisMySQL)
        return db_common.createAllTablesMariaDB();

    // ensureUUIDEveryWhereSqlite(getDatabase());
    // createUniqueUUIDIndexes(getDatabase());
    return false;
}

bool DataBase::creatingTables_DbImage()
{
    DataBaseCommon db_common;
    if (! db_common.createTableDBImageSqlite(getDatabaseImage(),
                                       ":/sql/sqlite/tables/image_reports.sql",
                                       "image_reports (DB_Image)"))
    {
        qCritical(logCritical()) << tr("Eroare la crearea tabelei 'imagesReports' in baza de date 'DB_IMAGE'.");
        return false;
    }
    return true;
}

bool DataBase::verifyNewDatabaseSchema() const
{
    const QSqlDatabase currentDatabase = QSqlDatabase::database();
    if (!currentDatabase.isValid() || !currentDatabase.isOpen()) {
        qCritical(logCritical())
            << tr("Verificarea schemei a eșuat: baza principală nu este deschisă.");
        return false;
    }

    const QStringList requiredTables = {
        QStringLiteral("users"), QStringLiteral("doctors"),
        QStringLiteral("fullNameDoctors"), QStringLiteral("nurses"),
        QStringLiteral("fullNameNurses"), QStringLiteral("patients"),
        QStringLiteral("typesPrices"), QStringLiteral("organizations"),
        QStringLiteral("investigations"), QStringLiteral("investigationsGroup"),
        QStringLiteral("constants"), QStringLiteral("contracts"),
        QStringLiteral("pricings"), QStringLiteral("pricingsTable"),
        QStringLiteral("pricingsPresentation"),
        QStringLiteral("orderEcho"), QStringLiteral("orderEchoTable"),
        QStringLiteral("orderEchoPresentation"),
        QStringLiteral("reportEcho"), QStringLiteral("reportEchoPresentation"),
        QStringLiteral("reportVideo"), QStringLiteral("tableLiver"),
        QStringLiteral("tableCholecist"), QStringLiteral("tablePancreas"),
        QStringLiteral("tableSpleen"), QStringLiteral("tableIntestinalLoop"),
        QStringLiteral("tableKidney"), QStringLiteral("tableBladder"),
        QStringLiteral("tableProstate"), QStringLiteral("tableGynecology"),
        QStringLiteral("tableBreast"), QStringLiteral("tableThyroid"),
        QStringLiteral("tableGestation0"), QStringLiteral("tableGestation1"),
        QStringLiteral("tableGestation2"),
        QStringLiteral("tableGestation2_biometry"),
        QStringLiteral("tableGestation2_cranium"),
        QStringLiteral("tableGestation2_SNC"),
        QStringLiteral("tableGestation2_heart"),
        QStringLiteral("tableGestation2_thorax"),
        QStringLiteral("tableGestation2_abdomen"),
        QStringLiteral("tableGestation2_urinarySystem"),
        QStringLiteral("tableGestation2_other"),
        QStringLiteral("tableGestation2_doppler"),
        QStringLiteral("tableSofTissuesLymphNodes"),
        QStringLiteral("normograms"), QStringLiteral("settingsUsers"),
        QStringLiteral("userPreferences"),
        QStringLiteral("patientAppointments"),
        QStringLiteral("patientAppointmentInvestigations"),
        QStringLiteral("conclusionTemplates"),
        QStringLiteral("formationsSystemTemplates"),
        QStringLiteral("onlineAccount"), QStringLiteral("cloudServer"),
        QStringLiteral("cryptoSplitKey"),
        QStringLiteral("doc_sequences")
    };
    const QStringList requiredViews = {
        QStringLiteral("v_users_combo_active"),
        QStringLiteral("v_doctors_active"),
        QStringLiteral("v_nurses_active"),
        QStringLiteral("v_patients_completer_active"),
        QStringLiteral("v_types_prices_active"),
        QStringLiteral("v_organizations_active"),
        QStringLiteral("v_contracts_listView_active")
    };

    const QStringList existingTables = currentDatabase.tables(QSql::Tables);
    const QStringList existingViews = currentDatabase.tables(QSql::Views);
    QStringList missingObjects;
    for (const QString &table : requiredTables) {
        if (!existingTables.contains(table, Qt::CaseInsensitive))
            missingObjects.append(tr("tabela %1").arg(table));
    }
    for (const QString &view : requiredViews) {
        if (!existingViews.contains(view, Qt::CaseInsensitive))
            missingObjects.append(tr("view-ul %1").arg(view));
    }
    if (globals().thisMySQL
        && !existingTables.contains(QStringLiteral("imagesReports"),
                                    Qt::CaseInsensitive)) {
        missingObjects.append(tr("tabela imagesReports"));
    }

    if (!missingObjects.isEmpty()) {
        qCritical(logCritical())
            << tr("Schema bazei noi este incompletă. Lipsesc: %1")
                   .arg(missingObjects.join(QStringLiteral(", ")));
        return false;
    }

    const QMap<QString, QStringList> requiredColumns = {
        {QStringLiteral("patients"),
         {QStringLiteral("deletion_mark"), QStringLiteral("idnp"),
          QStringLiteral("last_name"), QStringLiteral("first_name"),
          QStringLiteral("middle_name"), QStringLiteral("medical_policy"),
          QStringLiteral("uuid")}},
        {QStringLiteral("investigations"),
         {QStringLiteral("owner"), QStringLiteral("uuid")}},
        {QStringLiteral("orderEcho"),
         {QStringLiteral("patient_id"), QStringLiteral("docYear"),
          QStringLiteral("uuid")}},
        {QStringLiteral("reportEcho"),
         {QStringLiteral("patient_id"), QStringLiteral("docYear"),
          QStringLiteral("uuid")}},
        {QStringLiteral("reportVideo"), {QStringLiteral("uuid")}},
        {QStringLiteral("patientAppointments"),
         {QStringLiteral("patient_id"), QStringLiteral("investigation_id")}},
        {QStringLiteral("patientAppointmentInvestigations"),
         {QStringLiteral("appointment_id"), QStringLiteral("investigation_id"),
          QStringLiteral("position")}},
        {QStringLiteral("conclusionTemplates"), {QStringLiteral("uuid")}},
        {QStringLiteral("formationsSystemTemplates"), {QStringLiteral("uuid")}}
    };
    for (auto table = requiredColumns.cbegin(); table != requiredColumns.cend(); ++table) {
        const QSqlRecord record = currentDatabase.record(table.key());
        for (const QString &column : table.value()) {
            if (!record.contains(column)) {
                qCritical(logCritical())
                    << tr("Schema bazei noi este incompletă: lipsește %1.%2.")
                           .arg(table.key(), column);
                return false;
            }
        }
    }

    for (const QString &view : requiredViews) {
        QSqlQuery viewQuery(currentDatabase);
        if (!viewQuery.exec(QStringLiteral("SELECT * FROM `%1` LIMIT 0").arg(view))) {
            qCritical(logCritical())
                << tr("View-ul %1 există, dar nu poate fi executat: %2")
                       .arg(view, viewQuery.lastError().text());
            return false;
        }
    }

    const QStringList requiredTriggers = {
        QStringLiteral("create_full_name_doctor"),
        QStringLiteral("update_full_name_doctor"),
        QStringLiteral("create_full_name_nurse"),
        QStringLiteral("update_full_name_nurse"),
        QStringLiteral("create_Pricings_presentation"),
        QStringLiteral("update_Pricings_presentation"),
        QStringLiteral("create_orderDoc_presentation"),
        QStringLiteral("update_orderDoc_presentation"),
        QStringLiteral("create_reportEcho_presentation"),
        QStringLiteral("update_reportEcho_presentation")
    };
    QStringList existingTriggers;
    QSqlQuery triggerQuery(currentDatabase);
    if (globals().thisSqlite) {
        if (!triggerQuery.exec(QStringLiteral(
                "SELECT name FROM sqlite_master WHERE type='trigger'"))) {
            qCritical(logCritical()) << tr("Trigger-ele SQLite nu pot fi verificate:")
                                     << triggerQuery.lastError().text();
            return false;
        }
        while (triggerQuery.next())
            existingTriggers.append(triggerQuery.value(0).toString());
    } else {
        if (!triggerQuery.exec(QStringLiteral("SHOW TRIGGERS"))) {
            qCritical(logCritical()) << tr("Trigger-ele MariaDB nu pot fi verificate:")
                                     << triggerQuery.lastError().text();
            return false;
        }
        while (triggerQuery.next())
            existingTriggers.append(triggerQuery.value(0).toString());
    }
    for (const QString &trigger : requiredTriggers) {
        if (!existingTriggers.contains(trigger, Qt::CaseInsensitive)) {
            qCritical(logCritical())
                << tr("Schema bazei noi este incompletă: lipsește trigger-ul %1.")
                       .arg(trigger);
            return false;
        }
    }

    if (globals().thisSqlite) {
        const QSqlDatabase imageDatabase =
            QSqlDatabase::database(QStringLiteral("db_image"), false);
        if (!imageDatabase.isValid() || !imageDatabase.isOpen()
            || !imageDatabase.tables(QSql::Tables).contains(
                QStringLiteral("imagesReports"), Qt::CaseInsensitive)) {
            qCritical(logCritical())
                << tr("Schema bazei noi este incompletă: lipsește db_image.imagesReports.");
            return false;
        }
        const QSqlRecord imageRecord = imageDatabase.record(QStringLiteral("imagesReports"));
        for (const QString &column : {QStringLiteral("patient_id"),
                                      QStringLiteral("uuid")}) {
            if (!imageRecord.contains(column)) {
                qCritical(logCritical())
                    << tr("Schema db_image este incompletă: lipsește imagesReports.%1.")
                           .arg(column);
                return false;
            }
        }
    } else {
        const QSqlRecord imageRecord = currentDatabase.record(QStringLiteral("imagesReports"));
        for (const QString &column : {QStringLiteral("patient_id"),
                                      QStringLiteral("uuid")}) {
            if (!imageRecord.contains(column)) {
                qCritical(logCritical())
                    << tr("Schema MariaDB este incompletă: lipsește imagesReports.%1.")
                           .arg(column);
                return false;
            }
        }
    }

    return true;
}

void DataBase::loadInvestigationFromXml()
{
    int count_num = 107; // vezi resource.qrc - investig.xml
    int progress = 0;

    QDomDocument investigXML;
    QFile xmlFile(":/xmls/investig.xml");
    if (!xmlFile.open(QIODevice::ReadOnly )){
        qWarning(logWarning()) << tr("Fisierul ':/xmls/investig.xml' nu a fost citit !!!");
    }
    investigXML.setContent(&xmlFile);
    xmlFile.close();

    QDomElement root = investigXML.documentElement();
    QDomElement node = root.firstChild().toElement();

    while(node.isNull() == false) {
        if(node.tagName() == "entry"){
            while(!node.isNull()){
                QString cod  = node.attribute("cod", "cod");
                QString name = node.attribute("name", "name");

                QSqlQuery qry;
                qry.prepare(R"(
                    INSERT INTO investigations (
                        id,deletionMark,cod,name,`use`)
                    VALUES ( ?, ?, ?, ?, ?);
                )");
                qry.addBindValue(getLastIdForTable("investigations") + 1);
                qry.addBindValue(0);
                qry.addBindValue(cod);
                qry.addBindValue(name);
                qry.addBindValue(1);
                if (qry.exec())
                    qInfo(logInfo()) << tr("Investigatia '%1' este introdusa in baza de date cu codul '%2'.")
                                        .arg(name, cod);
                else
                    qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'investigations': %1")
                                              .arg(qry.lastError().text());

                ++progress;
                emit updateProgress(count_num, progress);

                node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }

    emit finishedProgress(tr("Au fost încărcate %1 investigatii.").arg(QString::number(count_num)));

}

bool DataBase::updateInvestigationFromXML_2024()
{
    int progress = 0;

    // Deschidem fișierul XML
    QFile file(":/xmls/investig_2024.xml");
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << "Nu se poate deschide fișierul XML:" << file.errorString();
        return false;
    }

    // Citim și parsează XML-ul
    QDomDocument doc;
    if (! doc.setContent(&file)) {
        qWarning(logWarning()) << "Eroare la parsarea fișierului XML ':/xmls/investig_2024.xml' ";
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "list_investigation") {
        qWarning(logWarning()) << "Tagul rădăcină al fișierului XML este incorect.";
        return false;
    }

    QSqlDatabase currentDatabase = getDatabase();
    if (!currentDatabase.isValid() || !currentDatabase.isOpen()) {
        qWarning(logWarning()) << "Baza de date nu este deschisă pentru actualizarea investigațiilor.";
        return false;
    }
    const QSqlRecord investigationRecord =
        currentDatabase.record(QStringLiteral("investigations"));
    const bool hasOwner = investigationRecord.contains(QStringLiteral("owner"));
    const bool hasUuid = investigationRecord.contains(QStringLiteral("uuid"));

    if (!currentDatabase.transaction()) {
        qWarning(logWarning())
            << "Nu s-a putut porni tranzacția pentru actualizarea investigațiilor:"
            << currentDatabase.lastError().text();
        return false;
    }
    const auto rollback = [&currentDatabase]() {
        if (!currentDatabase.rollback())
            qCritical(logCritical())
                << "Rollback-ul actualizării investigațiilor a eșuat:"
                << currentDatabase.lastError().text();
        return false;
    };

    // Marcăm toate înregistrările existente ca `use=0`.
    QSqlQuery query(currentDatabase);
    if (! query.exec("UPDATE investigations SET `use` = 0;")) {
        qWarning(logWarning()) << "Nu a fost gasita nici o investigatiei: "
                               << query.lastError().text();
        return rollback();
    }

    // Parcurgem intrările din XML
    QDomNodeList entries = root.elementsByTagName("entry");
    for (int i = 0; i < entries.count(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.isNull())
            return rollback();

        const QString cod  = entry.attribute("cod");
        const QString name = entry.attribute("name");
        const int owner    = entry.attribute("owner").toInt();
        QVariant ownerValue;
        if (hasOwner && owner > 0
            && currentDatabase.tables(QSql::Tables).contains(
                QStringLiteral("investigationsGroup"), Qt::CaseInsensitive)) {
            QSqlQuery ownerQuery(currentDatabase);
            ownerQuery.prepare(QStringLiteral(
                "SELECT id FROM investigationsGroup WHERE id=?"));
            ownerQuery.addBindValue(owner);
            if (!ownerQuery.exec()) {
                qWarning(logWarning()) << "Eroare la verificarea grupului investigației:"
                                       << ownerQuery.lastError().text();
                return rollback();
            }
            if (ownerQuery.next())
                ownerValue = owner;
        }

        // Verificăm dacă codul există deja în baza de date
        query.prepare("SELECT id FROM investigations WHERE cod = ?;");
        query.addBindValue(cod);
        if (! query.exec()) {
            qWarning(logWarning()) << "Eroare la interogarea codului: "
                                   << query.lastError().text();
            return rollback();
        }

        if (query.next()) {
            // Codul există -> Actualizăm
            query.prepare(hasOwner
                              ? QStringLiteral(
                                    "UPDATE investigations SET name=?, `use`=1, owner=? "
                                    "WHERE cod=?")
                              : QStringLiteral(
                                    "UPDATE investigations SET name=?, `use`=1 WHERE cod=?"));
            query.addBindValue(name);
            if (hasOwner)
                query.addBindValue(ownerValue);
            query.addBindValue(cod);
            if (! query.exec()) {
                qWarning(logWarning()) << "Eroare la actualizarea codului: "
                                       << query.lastError().text();
                return rollback();
            } else {
                qInfo(logInfo()) << "Investigatia cu codul "
                                 << cod << " a fost actualizata cu succes.";
            }
        } else {
            // Codul nu există -> Inserăm
            QStringList columns{QStringLiteral("deletionMark"), QStringLiteral("cod"),
                                QStringLiteral("name"), QStringLiteral("`use`")};
            if (hasOwner)
                columns.append(QStringLiteral("owner"));
            if (hasUuid)
                columns.append(QStringLiteral("uuid"));
            const QStringList placeholders(columns.size(), QStringLiteral("?"));
            query.prepare(QStringLiteral("INSERT INTO investigations (%1) VALUES (%2)")
                              .arg(columns.join(QLatin1Char(',')),
                                   placeholders.join(QLatin1Char(','))));
            query.addBindValue(0);
            query.addBindValue(cod);
            query.addBindValue(name);
            query.addBindValue(1);
            if (hasOwner)
                query.addBindValue(ownerValue);
            if (hasUuid)
                query.addBindValue(QUuid::createUuid().toRfc4122());
            if (! query.exec()) {
                qWarning(logWarning()) << "Eroare la inserarea codului: "
                                       << query.lastError().text();
                return rollback();
            } else {
                qInfo(logInfo()) << "A fost inserata investigatia noua cu codul "
                                 << cod << ".";
            }
        }

        ++progress;
        emit updateProgress(entries.count(), progress);
    }

    if (!currentDatabase.commit()) {
        qCritical(logCritical())
            << "Commit-ul actualizării investigațiilor a eșuat:"
            << currentDatabase.lastError().text();
        return rollback();
    }

    emit finishedProgress(tr("Actualizat clasificatorul \"Investigatii\" pe anul 2024."));

    qInfo(logInfo()) << "Actualizat clasificatorul \"Investigatii\" pe anul 2024.";
    return true;
}

bool DataBase::loadNormogramsFromXml()
{
    QDomDocument normogramsXml;
    QFile xmlFile(":/xmls/normograms.xml");
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << tr("Fisierul ':/xmls/normograms.xml' nu a fost citit !!!");
        return false;
    }
    const QDomDocument::ParseResult parseResult = normogramsXml.setContent(&xmlFile);
    if (!parseResult) {
        qWarning(logWarning())
            << tr("Fișierul normogramelor este invalid la linia %1, coloana %2: %3")
                   .arg(parseResult.errorLine)
                   .arg(parseResult.errorColumn)
                   .arg(parseResult.errorMessage);
        xmlFile.close();
        return false;
    }
    xmlFile.close();

    const QDomNodeList entries = normogramsXml.elementsByTagName(QStringLiteral("entry"));
    if (entries.isEmpty()) {
        qWarning(logWarning()) << tr("Fișierul normogramelor nu conține înregistrări.");
        return false;
    }

    QSqlDatabase currentDatabase = getDatabase();
    if (!currentDatabase.isValid() || !currentDatabase.isOpen()) {
        qWarning(logWarning()) << tr("Baza de date nu este deschisă pentru încărcarea normogramelor.");
        return false;
    }

    QSqlQuery countQuery(currentDatabase);
    if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM normograms"))
        || !countQuery.next()) {
        qWarning(logWarning()) << tr("Nu s-a putut verifica tabela normograms: %1")
                                      .arg(countQuery.lastError().text());
        return false;
    }

    const int existingCount = countQuery.value(0).toInt();
    if (existingCount == entries.count())
        return true; // reluarea sigură după o inițializare deja finalizată
    if (existingCount != 0) {
        qWarning(logWarning())
            << tr("Tabela normograms este inițializată parțial: %1 din %2 înregistrări.")
                   .arg(existingCount)
                   .arg(entries.count());
        return false;
    }

    if (!currentDatabase.transaction()) {
        qWarning(logWarning()) << tr("Nu s-a putut porni tranzacția pentru normograme: %1")
                                      .arg(currentDatabase.lastError().text());
        return false;
    }

    QSqlQuery insertQuery(currentDatabase);
    if (!insertQuery.prepare(R"(
            INSERT INTO normograms (
                `name`, `crl`, `5_centile`, `50_centile`, `95_centile`)
            VALUES (?, ?, ?, ?, ?)
        )")) {
        qWarning(logWarning()) << tr("Nu s-a putut pregăti inserarea normogramelor: %1")
                                      .arg(insertQuery.lastError().text());
        currentDatabase.rollback();
        return false;
    }

    for (int index = 0; index < entries.count(); ++index) {
        const QDomElement entry = entries.at(index).toElement();
        insertQuery.bindValue(0, entry.attribute(QStringLiteral("name")));
        insertQuery.bindValue(1, entry.attribute(QStringLiteral("crl")));
        insertQuery.bindValue(2, entry.attribute(QStringLiteral("_5_centile")));
        insertQuery.bindValue(3, entry.attribute(QStringLiteral("_50_centile")));
        insertQuery.bindValue(4, entry.attribute(QStringLiteral("_95_centile")));
        if (!insertQuery.exec()) {
            qWarning(logWarning()) << tr("Eroare la inserarea normogramei %1: %2")
                                          .arg(index + 1)
                                          .arg(insertQuery.lastError().text());
            currentDatabase.rollback();
            return false;
        }
        emit updateProgress(entries.count(), index + 1);
    }

    if (!currentDatabase.commit()) {
        qWarning(logWarning()) << tr("Salvarea normogramelor a eșuat: %1")
                                      .arg(currentDatabase.lastError().text());
        currentDatabase.rollback();
        return false;
    }

    emit finishedProgress(tr("Au fost încărcate %1 elemente ale normogramelor.")
                              .arg(entries.count()));
    return true;
}

// *******************************************************************
// ********* INSERAREA, ACTUALIZAREA SETARILOR DIN TABELE ************

void DataBase::insertDataForTabletypesPrices()
{
    QSqlQuery qry;
    int m_id = getLastIdForTable("typesPrices") + 1;

    /** preturi comerciale */
    qry.prepare(R"(
        INSERT INTO typesPrices (
            id, uuid, deletionMark, name, discount, noncomercial)
        VALUES ( ?, ?, ?, ?, ?, ?);
    )");
    qry.addBindValue(m_id);
    QUuid uuid = QUuid::createUuid();
    qry.addBindValue(uuid.toRfc4122());
    qry.addBindValue(0);
    qry.addBindValue(tr("Pre\310\233uri comerciale"));
    qry.addBindValue(0);
    if (globals().thisMySQL)
        qry.addBindValue(false);
    else
        qry.addBindValue(0);
    if (qry.exec())
        qInfo(logInfo()) << tr("In baza de date este introdus tipul pretului 'Preturi comerciale'.");
    else
        qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'typesPrices': %1").arg(qry.lastError().text());

    qry.clear();

    /** preturi CNAM */
    qry.prepare(R"(
        INSERT INTO typesPrices (
            id, uuid, deletionMark, name, discount, noncomercial)
        VALUES ( ?, ?, ?, ?, ?, ?);
    )");
    qry.addBindValue(m_id + 1);
    QUuid uuid2 = QUuid::createUuid();
    qry.addBindValue(uuid2.toRfc4122());
    qry.addBindValue(0);
    qry.addBindValue(tr("Pre\310\233uri CNAM"));
    qry.addBindValue(0);
    if (globals().thisMySQL)
        qry.addBindValue(true);
    else
        qry.addBindValue(1);
    if (qry.exec())
        qInfo(logInfo()) << tr("In baza de date este introdus tipul pretului 'Preturi CNAM'.");
    else
        qWarning(logWarning()) << tr("Eroare la inserare a datelor in tabela 'typesPrices': %1").arg(qry.lastError().text());
}

void DataBase::insertSetTableSettingsUsers()
{
    QSqlQuery qry;
    qry.prepare(R"(
        INSERT INTO userPreferences (
            id,
            id_users,
            versionApp,
            showQuestionCloseApp,
            showUserManual,
            showHistoryVersion,
            order_splitFullName,
            updateListDoc,
            showDesignerMenuPrint,
            checkNewVersionApp,
            databasesArchiving,
            showAsistantHelper,
            showDocumentsInSeparatWindow,
            minimizeAppToTray)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)
    )");
    qry.addBindValue(globals().idUserApp);
    qry.addBindValue(globals().idUserApp);
    // O bază creată de versiunea curentă nu trebuie introdusă imediat în
    // fluxul de migrare ca și cum versiunea schemei ar lipsi.
    qry.addBindValue(USG_VERSION_FULL);
    qry.addBindValue(1);  // showQuestionCloseApp
    qry.addBindValue(0);  // showUserManual
    qry.addBindValue(0);  // showHistoryVersion
    qry.addBindValue(0);  // order_splitFullName
    qry.addBindValue(0);  // updateListDoc
    qry.addBindValue(0);  // showDesignerMenuPrint
    qry.addBindValue(1);  // checkNewVersionApp
    qry.addBindValue(0);  // databasesArchiving
    qry.addBindValue(1);  // showAsistantHelper
    qry.addBindValue(0);  // showDocumentsInSeparatWindow
    qry.addBindValue(0);  // minimizeAppToTray

    if (qry.exec()) {
        globals().showQuestionCloseApp  = true;
        globals().showUserManual        = false;
        globals().order_splitFullName   = false;
        globals().showDesignerMenuPrint = false;
        globals().checkNewVersionApp    = true;
        globals().databasesArchiving    = false;
        globals().showAsistantHelper    = true;
        globals().showDocumentsInSeparatWindow = false;
        globals().minimizeAppToTray = false;
    } else {
        qCritical(logCritical()) << tr("Nu au fost inserate date initiale in tabela 'userPreferences' %1.")
                                        .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
    }
}

// *******************************************************************
// ****************** FUNCTIILE CU DOCUMENTE *************************

bool DataBase::execQuery(const QString strQuery)
{
    QSqlQuery qry;

    qry.prepare(strQuery);
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: Executarea solicitarii nereusita !!!").arg(metaObject()->className())
                               << qry.lastError().text();
        return false;
    }
}

bool DataBase::removeObjectById(const QString nameTable, const int _id)
{
    QSqlQuery qry;

    qry.prepare(QString("DELETE FROM %1 WHERE id = '%1';").arg(nameTable, QString::number(_id)));
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: Executarea eliminarii obiectului nereusita !!!").arg(metaObject()->className())
                               << qry.lastError().text();
        return false;
    }
}

int DataBase::getLastIdForTable(const QString nameTable) const
{
    int lastId = 0;
    QString strQryLastNrDoc;
    strQryLastNrDoc = QString("SELECT id FROM %1 ORDER BY id DESC LIMIT 1;").arg(nameTable);

    QMap<QString, QString> _items;
    if (getDataFromQueryByRecord(strQryLastNrDoc, _items)){
        if (_items.count() != 0){
            lastId = _items.constFind("id").value().toInt();
        }
    }
    return lastId;
}

int DataBase::getLastIdForTableByDatabase(const QString nameTable, QSqlDatabase name_database) const
{
    int last_id = 0;
    QSqlQuery qry(name_database);
    qry.prepare(QString("SELECT id FROM %1 ORDER BY id DESC LIMIT 1;").arg(nameTable));
    if (qry.exec()){
        qry.first();
        last_id = qry.value(0).toInt();
    } else {
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii 'getLastIdForTableByDatabase()': %1").arg(qry.lastError().text());
    }
    return last_id;
}

int DataBase::getLastNumberDoc(const QString nameTable) const
{
    QString lastNumberDoc;
    QString strQry = QString(R"(
        SELECT
            numberDoc
        FROM
            %1
        ORDER BY
            id
        DESC LIMIT 1;
    )").arg(nameTable);
    QMap<QString, QString> _items;
    if (getDataFromQueryByRecord(strQry, _items)){
        if (_items.count() != 0){
            lastNumberDoc = _items.constFind("numberDoc").value();
        }
    }
    if (lastNumberDoc.isEmpty())
        return 0;
    else
        return lastNumberDoc.toInt();
}

int DataBase::getLastIDPricings(const int id_organization, const int id_contract, const int id_typePrice) const
{
    QSqlQuery qry;
    qry.prepare(db_common.getTextQryFromResource(":/sql/queries/pricings_lastID.sql"));
    qry.addBindValue(id_organization);
    qry.addBindValue(id_contract);
    qry.addBindValue(id_typePrice);
    if (qry.exec() && qry.next()) {
        return qry.value(0).toInt();
    }

    return -1;
}

bool DataBase::getDataFromQuery(const QString strQuery, QMap<QString, QString> &items)
{
    QSqlQuery qry;
    qry.exec(strQuery);
    while (qry.next()){
        items.insert(qry.value(0).toString(),qry.value(1).toString());
    }

    return true;
}

bool DataBase::getDataFromQueryByRecord(const QString strQuery, QMap<QString, QString> &items) const
{
    QSqlQuery qry;
    qry.exec(strQuery);
    QSqlRecord rec = qry.record();

    while (qry.next()){
        for (int n = 0; n < rec.count(); n++) {
            items.insert(rec.fieldName(n), qry.value(n).toString());
        }
    }

    return true;
}

int DataBase::statusDeletionMarkObject(const QString nameTable, const int _id) const
{
    QSqlQuery qry;
    qry.prepare(QString("SELECT deletionMark FROM %1 WHERE id = ?;").arg(nameTable));
    qry.addBindValue(_id);
    if (qry.exec() && qry.next()){
        return qry.value(0).toInt();
    } else {
        return -1;
    }
}

bool DataBase::deletionMarkObject(const QString nameTable, const int _id)
{
    int statusDeletion;

    // determinam statut curent
    int currentStatus = statusDeletionMarkObject(nameTable, _id);
    if (currentStatus != -1){
        if (currentStatus == 0){
            statusDeletion = 1;
        } else {
            statusDeletion = 0;
        }
    } else {
        qWarning(logWarning()) << tr("%1 - deletionMarkObject(nameTable = %2, id = %3)")
                                  .arg(metaObject()->className(), nameTable, QString::number(_id))
                               << tr("Status 'deletionMark' = -1 : nu poate fi negativ !!!");
        return false;
    }
    // marcam elementul
    QSqlQuery qry;
    qry.prepare("UPDATE " + nameTable + " SET deletionMark = :deletionMark WHERE id = :id;");
    qry.bindValue(":deletionMark", statusDeletion);
    qry.bindValue(":id", _id);
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1 - deletionMarkObject(nameTable = %2, id = %3)")
                                  .arg(metaObject()->className(), nameTable, QString::number(_id))
                               << tr("Modificarea statusului 'deletionMark' a obiectului cu 'ID'=%1 nu este reusita. Erroarea:%2")
                                  .arg(QString::number(_id), qry.lastError().text());
        return false;
    }
}

bool DataBase::getObjectDataById(const QString &nameTable, const int _id, QMap<QString, QString> &items)
{
    QSqlQuery qry;
    qry.prepare(QString("SELECT * FROM %1 WHERE id = ?").arg(nameTable));
    qry.addBindValue(_id);

    if (! qry.exec()) {
        qWarning() << "Query exec failed:"
                   << qry.lastError().text();
        return false;
    }

    if (! qry.next()) {
        qWarning() << "No record found with id ="
                   << _id << "in table" << nameTable;
        return false;
    }

    QSqlRecord rec = qry.record();
    for (int n = 0; n < rec.count(); ++n) {
        items.insert(rec.fieldName(n), rec.value(n).toString());
    }

    return true;
}

bool DataBase::postDocument(const QString nameTable, QMap<QString, QString> &items)
{
    QString _key;        // nume colonitelor
    QString _value;      // valoarea
    QMapIterator<QString, QString> it(items);
    while (it.hasNext()) {   // determinam _key and _value
        it.next();
        _key = _key + "'" + it.key() + "',";         // ex: 'name', 'comment',
        _value = _value + "'" + it.value() + "',";
    }
    _key.resize(_key.size() - 1);      // eliminam ultimul simbol (,):
    _value.resize(_value.size() - 1);  // ex: 'name', 'comment'

    QSqlQuery qry;  // determinam id
    //    qry.prepare("SELECT max(id) FROM " + nameTable + ";");
    //    qry.exec();
    //    qry.next();
    //    int _id = qry.value(0).toInt() + 1;

    // cream obiect nou
    qry.prepare("INSERT INTO " + nameTable +
                "(" + _key + ")"
                             " VALUES(" + _value + ");");
    if (qry.exec()){
        return true;
    } else {
        qWarning(logWarning()) << tr("%1: postDocument(nameTable = %2): "
                                     "<br>Validarea documentului a esuat.")
                                  .arg(metaObject()->className(),
                                       nameTable);
        return false;
    }
}

// *******************************************************************
// ************** CREAREA TABELELOR LA PRIMA LANSARE *****************

bool DataBase::createIndexTables()
{
    QSqlQuery qry;

    if (qry.exec("CREATE INDEX idx_patients_name ON patients(last_name, first_name);"))
        qInfo(logInfo()) << "creat indexul 'idx_patients_name'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_patients_name'.";

    if (qry.exec("CREATE INDEX idx_patients_name_idnp ON patients(last_name, first_name, idnp);"))
        qInfo(logInfo()) << "creat indexul 'idx_patients_name_idnp'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_patients_name_idnp'.";

    //-----------------------------------------------------------------
    //---------------- pricing

    if (qry.exec("CREATE INDEX idx_dateDoc_pricing ON pricings(dateDoc);"))
        qInfo(logInfo()) << "creat indexul 'idx_dateDoc_pricing'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_dateDoc_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_organizations_pricing ON pricings(id_organizations);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_organizations_pricing'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_organizations_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_pricing ON pricings(id_contracts);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_contracts_pricing'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_contracts_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_users_pricing ON pricings(id_users);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_users_pricing'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_users_pricing'.";

    if (qry.exec("CREATE INDEX idx_id_typesPrices_pricing ON pricings(id_typesPrices);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_typesPrices_pricing'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_typesPrices_pricing'.";

    //-----------------------------------------------------------------
    //---------------- pricingsTable

    if (qry.exec("CREATE INDEX idx_id_pricings_pricingTable ON pricingsTable(id_pricings);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_pricings_pricingTable'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_pricings_pricingTable'.";

    //-----------------------------------------------------------------
    //---------------- orderEcho

    if (qry.exec("CREATE INDEX idx_id_organizations_order ON orderEcho(id_organizations);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_organizations_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_organizations_order'.";

    if (qry.exec("CREATE INDEX idx_id_contracts_order ON orderEcho(id_contracts);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_contracts_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_contracts_order'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_order ON orderEcho(patient_id);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_pacients_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_pacients_order'.";

    if (qry.exec("CREATE INDEX idx_id_doctors_order ON orderEcho(id_doctors);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_doctors_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_doctors_order'.";

    if (qry.exec("CREATE INDEX idx_id_users_order ON orderEcho(id_users);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_users_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_users_order'.";

    if (qry.exec("CREATE INDEX idx_dateDoc_order ON orderEcho(dateDoc);"))
        qInfo(logInfo()) << "creat indexul 'idx_dateDoc_order'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_dateDoc_order'.";

    //----------------------------------------------------------------
    //----------------- orderEchoTable

    if (qry.exec("CREATE INDEX idx_id_orderEcho_orderTable ON orderEchoTable(id_orderEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_orderEcho_orderTable'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_orderEcho_orderTable'.";

    //----------------------------------------------------------------
    //----------------- reportEcho

    if (qry.exec("CREATE INDEX idx_dateDoc_report ON reportEcho(dateDoc);"))
        qInfo(logInfo()) << "creat indexul 'idx_dateDoc_report'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_dateDoc_report'.";

    if (qry.exec("CREATE INDEX idx_id_pacients_report ON reportEcho(patient_id);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_pacients_report'.";
    else
        qCritical(logCritical()) << "Eroare : nu a fost creat indexul 'idx_id_pacients_report'.";

    if (qry.exec("CREATE INDEX idx_id_orderEcho_report ON reportEcho(id_orderEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_orderEcho_report'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_orderEcho_report'.";

    if (qry.exec("CREATE INDEX idx_id_users_report ON reportEcho(id_users);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_users_report'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_users_report'.";

    //---------------------------------------------------------------
    //----------------- tableCholecist

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cholecist ON tableCholecist(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_cholecist'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_cholecist'.";

    //---------------------------------------------------------------
    //----------------- tablePancreas

    if (qry.exec("CREATE INDEX idx_id_reportEcho_pancreas ON tablePancreas(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_pancreas'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_pancreas'.";

    //---------------------------------------------------------------
    //----------------- tableSpleen

    if (qry.exec("CREATE INDEX idx_id_reportEcho_spleen ON tableSpleen(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_spleen'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_spleen'.";


    //---------------------------------------------------------------
    //----------------- tableIntestinalLoop

    if (qry.exec("CREATE INDEX idx_id_reportEcho_intestin ON tableIntestinalLoop(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_intestin'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_intestin'.";

    //---------------------------------------------------------------
    //----------------- tableKidney

    if (qry.exec("CREATE INDEX idx_id_reportEcho_kidney ON tableKidney(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_kidney'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_kidney'.";

    //---------------------------------------------------------------
    //----------------- tableBladder

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bladder ON tableBladder(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_bladder'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_bladder'.";

    //---------------------------------------------------------------
    //----------------- tableProstate

    if (qry.exec("CREATE INDEX idx_id_reportEcho_prostate ON tableProstate(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_prostate'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_prostate'.";

    //---------------------------------------------------------------
    //----------------- tableGynecology

    if (qry.exec("CREATE INDEX idx_id_reportEcho_gynecology ON tableGynecology(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_gynecology'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_gynecology'.";

    //---------------------------------------------------------------
    //----------------- tableBreast

    if (qry.exec("CREATE INDEX idx_id_reportEcho_breast ON tableBreast(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_breast'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_breast'.";

    //---------------------------------------------------------------
    //----------------- tableThyroid

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thyroid ON tableThyroid(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_thyroid'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_thyroid'.";

    //---------------------------------------------------------------
    //----------------- tableGestation0

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges0 ON tableGestation0(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_ges0'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_ges0'.";

    //---------------------------------------------------------------
    //----------------- tableGestation1

    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges1 ON tableGestation1(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_ges1'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_ges1'.";

    //---------------------------------------------------------------
    //----------------- tables Gestation2
    if (qry.exec("CREATE INDEX idx_id_reportEcho_ges2 ON tableGestation2(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_ges2'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_ges2'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_bio ON tableGestation2_biometry(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_bio'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_bio'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_cr ON tableGestation2_cranium(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_cr'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_cr'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_snc ON tableGestation2_SNC(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_snc'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_snc'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_heart ON tableGestation2_heart(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_heart'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_heart'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_thorax ON tableGestation2_thorax(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_thorax'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_thorax'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_abd ON tableGestation2_abdomen(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_abd'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_abd'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_us ON tableGestation2_urinarySystem(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_us'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_us'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_other ON tableGestation2_other(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_other'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_other'.";

    if (qry.exec("CREATE INDEX idx_id_reportEcho_doppler ON tableGestation2_doppler(id_reportEcho);"))
        qInfo(logInfo()) << "creat indexul 'idx_id_reportEcho_doppler'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_reportEcho_doppler'.";

    //---------------------------------------------------------------
    //----------------- formationsSystemTemplates

    if (qry.exec("CREATE INDEX idx_name_typeSystem_formationsSystemTemplates ON formationsSystemTemplates(name, typeSystem);"))
        qInfo(logInfo()) << "creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";
    else
        qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_name_typeSystem_formationsSystemTemplates'.";

    //---------------------------------------------------------------
    //----------------- db_image

    if (globals().thisSqlite) {

        QSqlQuery qry_image(QSqlDatabase::database("db_image"));

        if (qry_image.exec("CREATE INDEX idx_id_documents_imagesReports ON imagesReports(id_reportEcho, id_orderEcho, patient_id, id_user);"))
            qInfo(logInfo()) << "creat indexul 'idx_id_documents_imagesReports'.";
        else
            qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_documents_imagesReports'.";

    } else {

        if (qry.exec("CREATE INDEX idx_id_documents_imagesReports ON imagesReports(id_reportEcho, id_orderEcho, patient_id, id_user);"))
            qInfo(logInfo()) << "creat indexul 'idx_id_documents_imagesReports'.";
        else
            qCritical(logCritical()) << "Eroare: nu a fost creat indexul 'idx_id_documents_imagesReports'.";

    }

    return true;
}

void DataBase::updateVariableFromTableSettingsUser()
{
    QSqlQuery qry(getDatabase());
    qry.prepare(R"(
        SELECT
            showQuestionCloseApp,
            showUserManual,
            showHistoryVersion,
            order_splitFullName,
            updateListDoc,
            showDesignerMenuPrint,
            checkNewVersionApp,
            databasesArchiving,
            showAsistantHelper,
            showDocumentsInSeparatWindow,
            minimizeAppToTray
        FROM
            userPreferences
        WHERE
            id_users = ?;
    )");
    qry.addBindValue(globals().idUserApp);
    if (!qry.exec()) {
        qWarning(logWarning()) << "Citirea preferințelor utilizatorului a eșuat:"
                               << qry.lastError().text();
        return;
    }

    if (!qry.next()) {
        qWarning(logWarning()) << "Nu există preferințe pentru utilizatorul cu id="
                               << globals().idUserApp;
        return;
    }

    const QSqlRecord rec = qry.record();
    globals().showQuestionCloseApp         = qry.value(rec.indexOf("showQuestionCloseApp")).toBool();
    globals().showUserManual               = qry.value(rec.indexOf("showUserManual")).toBool();
    globals().showHistoryVersion           = qry.value(rec.indexOf("showHistoryVersion")).toBool();
    globals().order_splitFullName          = qry.value(rec.indexOf("order_splitFullName")).toBool();
    globals().updateIntervalListDoc        = qry.value(rec.indexOf("updateListDoc")).toInt();
    globals().showDesignerMenuPrint        = qry.value(rec.indexOf("showDesignerMenuPrint")).toBool();
    globals().checkNewVersionApp           = qry.value(rec.indexOf("checkNewVersionApp")).toBool();
    globals().databasesArchiving           = qry.value(rec.indexOf("databasesArchiving")).toBool();
    globals().showAsistantHelper           = qry.value(rec.indexOf("showAsistantHelper")).toBool();
    globals().showDocumentsInSeparatWindow = qry.value(rec.indexOf("showDocumentsInSeparatWindow")).toBool();
    globals().minimizeAppToTray            = qry.value(rec.indexOf("minimizeAppToTray")).toBool();
}

// *******************************************************************
// ***************** FUNTIILE SUPLIMENTARE ***************************

bool DataBase::existColumnInTable(const QString nameTable, const QString nameColumn) const
{
    QSqlQuery qry;
    if (globals().thisSqlite) {
        QString queryStr = QString("PRAGMA table_info(%1);").arg(nameTable);
        if (qry.exec(queryStr)) {
            while (qry.next()) {
                if (qry.value(1).toString() == nameColumn) {
                    return true;
                }
            }
        }
    } else {
        QString queryStr = QString("SHOW COLUMNS FROM `%1` LIKE '%2';").arg(nameTable, nameColumn);
        if (qry.exec(queryStr) && qry.next()) {
            return true;  // Coloana există
        }
    }
    return false;
}

// *******************************************************************
// *************** SOLICITARI PU DOCUMENTE****************************

bool DataBase::existIdDocument(const QString nameTable, const QString name_condition, const QString value_condition, QSqlDatabase nameDatabase)
{
    bool exist_id = false;
    QSqlQuery qry(nameDatabase);
    qry.prepare(QString(R"(
        SELECT count(id) FROM %1 WHERE %2 = ?
    )").arg(nameTable, name_condition));
    qry.addBindValue(value_condition);
    if (qry.exec()){
        qry.next();
        if (qry.value(0).toInt() > 0)
            exist_id = true;
    } else {
        qWarning(logWarning()) << tr("Eroare determinarii existentii 'count(id)' a documentului cu 'id=%1' din baza de date 'DB_IMAGE':\n")
                                  .arg(value_condition) << qry.lastError().text();
    }

    return exist_id;
}

bool DataBase::existSubalternDocument(const QString nameTable, const QString name_condition, const QString value_condition, int &id_doc)
{
    QSqlQuery qry;
    qry.prepare(QString(R"(
        SELECT id  FROM %1 WHERE %1 = ?
    )").arg(nameTable, name_condition));
    qry.addBindValue(value_condition);
    if (qry.exec()){
        qry.next();
        if (qry.value(0).toInt() > 0){
            id_doc = qry.value(0).toInt();
            return id_doc > 0;
        } else {
            return false;
        }
    } else {
        qWarning(logWarning()) << tr("%1 - existSubalternDocument()").arg(metaObject()->className())
                               << tr("Solicitarea nereusita: %1").arg(qry.lastError().text());
        return false;
    }
}

QString DataBase::getQryFromTableConstantById(const int id_user) const
{
    return QString(R"(
            SELECT
                constants.id_organizations,
                organizations.IDNP,
                organizations.name,
                organizations.address,
                organizations.telephone,
                fullNameDoctors.nameAbbreviated AS doctor,
                organizations.email FROM constants
            INNER JOIN
                organizations ON constants.id_organizations = organizations.id
            INNER JOIN
                fullNameDoctors ON constants.id_doctors = fullNameDoctors.id_doctors
            WHERE
                constants.id_users = %1;
        )").arg(QString::number(id_user));
}

QString DataBase::getQryForTableOrgansInternalById(const int id_doc) const
{
    const bool isMySQL = globals().thisMySQL;

    const QString leftField  = isMySQL ? "tl.left"  : "tl.[left]";
    const QString rightField = isMySQL ? "tl.right" : "tl.[right]";

    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesOrgansInternal.sql");
    str.replace("%l_left%", leftField);
    str.replace("%l_right%", rightField);
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableUrinarySystemById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesUrinarySystem.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableProstateById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesProstate.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableGynecologyById(const int id_doc) const
{
    const bool isMySQL = globals().thisMySQL;

    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tableGynecology.sql");
    str.replace("%lmp%", isMySQL
                             ? "DATE_FORMAT(tg.dateMenstruation, '%d.%m.%Y')"
                             : "strftime('%d.%m.%Y', tg.dateMenstruation)");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableBreastById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesBreast.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableThyroidById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesThyroid.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableGestation0dById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesGestation0.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableGestation1dById(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesGestation1.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableGestation2(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesGestation2.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableLymphNodes(const int id_doc) const
{
    QString str = db_common.getTextQryFromResource(":/sql/queries_print/tablesLymphNodes.sql");
    str.replace("%id%", QString::number(id_doc));

    return str;
}

QString DataBase::getQryForTableOrderById(const int id_doc, const QString str_price) const
{
    QString str_qry;
    str_qry = QString(R"(
        SELECT
            orderEchoPresentation.docPresentationDate AS title_order,
            orderEchoTable.cod,
            orderEchoTable.name AS Investigation,
            %1 AS Price
        FROM
            orderEcho
        INNER JOIN
            orderEchoTable ON orderEcho.id = orderEchoTable.id_orderEcho
        INNER JOIN
            orderEchoPresentation ON orderEcho.id = orderEchoPresentation.id_orderEcho
        WHERE
            orderEcho.id = '%2' AND
            orderEcho.deletionMark = '2'
        ORDER BY
            orderEchoTable.cod;
    )").arg(str_price, QString::number(id_doc));

    return str_qry;
}

// *******************************************************************
// ****** ARHIVAREA BAZEI DE DATE SI A FISIERELOR APLICATIEI *********

QByteArray DataBase::fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }
    return QByteArray();
}

// *******************************************************************
// ************************* CRYPTAREA *******************************

QString DataBase::encode_string(const QString &str)
{
    return LegacySettingsCodec::encode(str);
}

QString DataBase::decode_string(const QString &str)
{
    return LegacySettingsCodec::decode(str);
}

QString DataBase::getVersionSQLite()
{
    QString str = "unknow";
    QSqlQuery qry;
    qry.exec("select sqlite_version();");
    while (qry.next()){
        str = qry.value(0).toString();
    }
    return str;
}

QString DataBase::getVersionMySQL()
{
    QString str = "unknow";
    QSqlQuery qry;
    qry.exec("SELECT VERSION();");
    while (qry.next()){
        str = qry.value(0).toString();
    }
    return str;
}

QString DataBase::getHTMLImageInfo()
{
    return QString("<img src=\"qrc:///img/info_x32.png\" alt=\"info\" width=\"20\" height=\"20\" style=\"vertical-align:middle; margin-right:5px;\" />");
}

QString DataBase::getHTMLImageWarning()
{
    return QString("<img src = \"qrc:///img/warning.png\" alt = \"info\" width=\"20\" height=\"20\" style=\"vertical-align:middle; margin-right:5px;\"  />");
}

QString DataBase::getStyleForButtonMessageBox()
{
    if (globals().isSystemThemeDark)
        return QString(R"(
            QPushButton
            {
                min-width: 60px;
                border: 1px solid rgba(255, 255, 255, 0.2);
                border-radius: 8px;
                background-color: #2b2b2b;
                color: #ffffff;
                font-size: 13px;
                padding: 4px 10px;
                min-width: 80px;
            }
            QPushButton:hover
            {
                background-color: #3b3b3b;
            }
            QPushButton:pressed
            {
                background-color: #4b4b4b;
            }
        )");
    else
        return QString(R"(
            QPushButton
            {
                min-width: 60px;
                border: 1px solid rgba(0, 0, 0, 0.2);
                border-radius: 8px;
                background-color: #f5f5f5;
                color: #000000;
                font-size: 13px;
                padding: 4px 10px;
                min-width: 80px;
            }
            QPushButton:hover
            {
                background-color: #e0e0e0;
            }
            QPushButton:pressed
            {
                background-color: #d0d0d0;
            }
        )");
}

QString DataBase::toolButtonStyleForText()
{
    if (globals().isSystemThemeDark)
        return QString(R"(
            QToolButton {
                border: 1px solid rgba(255, 255, 255, 0.2);
                border-radius: 8px;
                background-color: #2b2b2b;
                color: #ffffff;
                font-size: 13px;
                padding: 4px 4px;
            }

            QToolButton:hover {
                background-color: #3b3b3b;
                border: 1px solid rgba(255, 255, 255, 0.35);
            }

            QToolButton:pressed {
                background-color: #1e1e1e;
                border: 1px solid rgba(255, 255, 255, 0.5);
            }

            QToolButton:focus {
                outline: none;
                border: 1px solid #00baff;
                background-color: #343434;
            }

            QToolButton:disabled {
                background-color: #2b2b2b;
                color: rgba(255, 255, 255, 0.4);
                border: 1px solid rgba(255, 255, 255, 0.1);
            }
        )");
    else
        return QString(R"(
            QToolButton {
                border: 1px solid rgba(0, 0, 0, 0.1);
                border-radius: 8px;
                background-color: #f1f1f1;
                color: #000000;
                font-size: 13px;
                padding: 4px 4px;
            }

            QToolButton:hover {
                background-color: #e6e6e6;
            }

            QToolButton:pressed {
                background-color: #dcdcdc;
            }

            QToolButton:focus {
                outline: none;
                border: 2px solid #0078d4;
                background-color: #ffffff;
            }

            QToolButton:disabled {
                color: rgba(0, 0, 0, 0.35);
                border-color: rgba(0, 0, 0, 0.15);
                background-color: #f1f1f1;
            }

            QToolButton:disabled:hover,
            QToolButton:disabled:pressed {
                color: rgba(0, 0, 0, 0.35);
                border-color: rgba(0, 0, 0, 0.15);
                background-color: #f1f1f1;
            }
        )");
}

QString DataBase::toolButtonStyleForIcon()
{
    if (globals().isSystemThemeDark)
        return QString(R"(
            QToolButton
            {
                border: none;
                background-color: transparent;
                color: #ffffff;
                font-size: 14px;
            }
            QToolButton:hover
            {
                background-color: #3b3b3b;
                color: #000000;
            }
            QToolButton:pressed
            {
                background-color: #4b4b4b;
            }
        )");
    else
        return QString(R"(
            QToolButton
            {
                border: none;
                background-color: transparent;
                color: #4a4a4a;
                font-size: 14px;
            }
            QToolButton:hover
            {
                background-color: #f0f0f0;
                color: #000000;
            }
            QToolButton:pressed
            {
                background-color: #dcdcdc;
            }
        )");
}

QByteArray DataBase::getHashUserApp()
{
    QSqlQuery qry;
    qry.prepare("SELECT hash FROM users WHERE id = ?");
    qry.addBindValue(globals().idUserApp);
    if (qry.exec() && qry.next()) {
        return QByteArray::fromHex(qry.value(0).toString().toUtf8());
    } else {
        return QByteArray();
    }
}

void DataBase::ensureUUIDs()
{
    const QString patientTable = getDatabase().tables(QSql::Tables)
                                         .contains(QStringLiteral("patients"), Qt::CaseInsensitive)
                                     ? QStringLiteral("patients")
                                     : QStringLiteral("pacients");
    QStringList rootTablesWithUUID = {
        "contracts",
        "doctors",
        "investigations",
        "investigationsGroup",
        "conclusionTemplates",
        "formationsSystemTemplates",
        "nurses",
        "orderEcho",
        "organizations",
        patientTable,
        "pricings",
        "reportEcho",
        "typesPrices",
        "users"
    };

    if (globals().thisMySQL)
        rootTablesWithUUID << "imagesReports";

    getDatabase().transaction();

    for (qsizetype tableIndex = 0; tableIndex < rootTablesWithUUID.size(); ++tableIndex) {
        const QString &table = rootTablesWithUUID.at(tableIndex);

        emit uuidProgress(static_cast<int>(tableIndex),
                          static_cast<int>(rootTablesWithUUID.size()),
                          tr("UUID: se verifică tabela %1...").arg(table));

        /* ===============================
         * 1️ verificam daca exista coloana UUID
         * =============================== */
        bool hasUuid = false;
        {
            QSqlQuery q(getDatabase());

            if (globals().thisSqlite) {
                q.exec(QString("PRAGMA table_info(%1)").arg(table));
                while (q.next()) {
                    if (q.value("name").toString() == "uuid") {
                        hasUuid = true;
                        break;
                    }
                }
            }
            else if (globals().thisMySQL) {
                q.prepare(R"(
                    SELECT 1
                    FROM information_schema.COLUMNS
                    WHERE TABLE_SCHEMA = DATABASE()
                      AND TABLE_NAME = ?
                      AND COLUMN_NAME = 'uuid'
                )");
                q.addBindValue(table);
                q.exec();
                hasUuid = q.next();
            }
        }

        /* ===============================
         * 2️ adaugam coloana UUID daca lipseste
         * =============================== */
        if (!hasUuid) {
            QSqlQuery alter(getDatabase());
            QString sql;

            if (globals().thisSqlite) {
                sql = QString("ALTER TABLE %1 ADD COLUMN uuid BLOB").arg(table);
            }
            else if (globals().thisMySQL) {
                sql = QString("ALTER TABLE %1 ADD COLUMN uuid BINARY(16)").arg(table);
            }

            if (!alter.exec(sql)) {
                qWarning() << "Failed to add uuid column:"
                           << table << alter.lastError();
                continue;
            }

            qDebug() << "uuid column added:" << table;
        }

        /* ===============================
         * 3️ completam UUID unde este NULL
         * =============================== */
        QSqlQuery select(getDatabase());
        select.exec(
            QString("SELECT id FROM %1 WHERE uuid IS NULL").arg(table)
            );

        QList<qint64> ids;
        while (select.next()) {
            ids << select.value(0).toLongLong();
        }

        if (ids.isEmpty())
            continue;

        QSqlQuery update(getDatabase());
        update.prepare(
            QString("UPDATE %1 SET uuid = ? WHERE id = ?").arg(table)
            );

        qsizetype processed = 0;
        for (qint64 id : std::as_const(ids)) {
            update.addBindValue(
                QUuid::createUuid().toRfc4122()   // RFC4122 only
                );
            update.addBindValue(id);

            if (!update.exec()) {
                qWarning() << "UUID update failed:"
                           << table << "id=" << id
                           << update.lastError();
            }

            ++processed;
            if (processed % 100 == 0 || processed == ids.size()) {
                emit uuidProgress(static_cast<int>(processed),
                                  static_cast<int>(ids.size()),
                                  tr("UUID: %2 din %3 înregistrări procesate în %4...")
                                      .arg(processed)
                                      .arg(ids.size())
                                      .arg(table));
            }
        }

        qDebug() << "UUID filled:" << table << ids.count();
    }

    getDatabase().commit();
}

void DataBase::ensureIndexUUIDs()
{
    const QString patientTable = getDatabase().tables(QSql::Tables)
                                         .contains(QStringLiteral("patients"), Qt::CaseInsensitive)
                                     ? QStringLiteral("patients")
                                     : QStringLiteral("pacients");
    QStringList rootTablesWithUUID = {
        "contracts",
        "doctors",
        "investigations",
        "investigationsGroup",
        "conclusionTemplates",
        "formationsSystemTemplates",
        "nurses",
        "orderEcho",
        "organizations",
        patientTable,
        "pricings",
        "reportEcho",
        "typesPrices",
        "users"
    };

    if (globals().thisMySQL)
        rootTablesWithUUID << "imagesReports";

    auto db = getDatabase();
    db.transaction();

    for (qsizetype tableIndex = 0; tableIndex < rootTablesWithUUID.size(); ++tableIndex) {
        const QString &table = rootTablesWithUUID.at(tableIndex);

        emit uuidProgress(static_cast<int>(tableIndex),
                          static_cast<int>(rootTablesWithUUID.size()),
                          tr("UUID: se verifică indexul unic pentru %1...").arg(table));

        /* 1. verificam daca exista coloana UUID */
        bool hasUuid = false;
        {
            QSqlQuery q(db);

            if (globals().thisSqlite) {
                if (!q.exec(QString("PRAGMA table_info(%1)").arg(table))) {
                    qWarning() << "PRAGMA table_info failed:" << table << q.lastError();
                    continue;
                }
                while (q.next()) {
                    if (q.value("name").toString() == "uuid") {
                        hasUuid = true;
                        break;
                    }
                }
            } else if (globals().thisMySQL) {
                q.prepare(R"(
                    SELECT 1
                    FROM information_schema.COLUMNS
                    WHERE TABLE_SCHEMA = DATABASE()
                      AND TABLE_NAME = ?
                      AND COLUMN_NAME = 'uuid'
                    LIMIT 1
                )");
                q.addBindValue(table);
                if (!q.exec()) {
                    qWarning() << "COLUMNS check failed:" << table << q.lastError();
                    continue;
                }
                hasUuid = q.next();
            }
        }

        if (!hasUuid) {
            qDebug() << "Skip (no uuid column):" << table;
            continue;
        }

        /* 2. verificam daca exista deja UNIQUE pe UUID */
        bool hasUniqueOnUuid = false;
        {
            QSqlQuery q(db);

            if (globals().thisSqlite) {
                // pragme: index_list + index_info
                if (!q.exec(QString("PRAGMA index_list(%1)").arg(table))) {
                    qWarning() << "PRAGMA index_list failed:" << table << q.lastError();
                    continue;
                }

                while (q.next()) {
                    const bool isUnique = q.value("unique").toInt() == 1;
                    const QString idxName = q.value("name").toString();
                    if (!isUnique)
                        continue;

                    QSqlQuery qi(db);
                    if (!qi.exec(QString("PRAGMA index_info(%1)").arg(idxName))) {
                        continue;
                    }

                    // verificam daca indexul unic e exact pe coloana UUID
                    // (acceptam si index compus care include UUID? -> aici cerem exact UUID)
                    int colCount = 0;
                    bool containsUuid = false;
                    while (qi.next()) {
                        ++colCount;
                        if (qi.value("name").toString() == "uuid")
                            containsUuid = true;
                    }

                    if (containsUuid && colCount == 1) {
                        hasUniqueOnUuid = true;
                        break;
                    }
                }
            } else if (globals().thisMySQL) {
                // orice UNIQUE care are coloana UUID
                q.prepare(R"(
                    SELECT 1
                    FROM information_schema.STATISTICS
                    WHERE TABLE_SCHEMA = DATABASE()
                      AND TABLE_NAME = ?
                      AND COLUMN_NAME = 'uuid'
                      AND NON_UNIQUE = 0
                    LIMIT 1
                )");
                q.addBindValue(table);
                if (!q.exec()) {
                    qWarning() << "STATISTICS check failed:"
                               << table
                               << q.lastError();
                    continue;
                }
                hasUniqueOnUuid = q.next();
            }
        }

        if (hasUniqueOnUuid) {
            qDebug() << "Unique(uuid) already exists:" << table;
            continue;
        }

        /* 3. cream UNIQUE pe UUID */
        {
            QSqlQuery alter(db);
            QString sql;

            if (globals().thisSqlite) {
                sql = QString("CREATE UNIQUE INDEX IF NOT EXISTS uq_%1_uuid ON %1(uuid)").arg(table);
            } else if (globals().thisMySQL) {
                // numele îl setam, dar daca exista alt nume deja, noi oricum nu ajungem aici (am verificat mai sus)
                sql = QString("ALTER TABLE %1 ADD UNIQUE KEY uq_%1_uuid (uuid)").arg(table);
            }

            if (!alter.exec(sql)) {
                qWarning() << "Failed to create unique(uuid):" << table << alter.lastError();
                continue;
            }

            qDebug() << "Created unique(uuid):" << table;
        }

        emit uuidProgress(static_cast<int>(tableIndex + 1),
                          static_cast<int>(rootTablesWithUUID.size()),
                          tr("UUID: indexurile verificate pentru %1 din %2 tabele.")
                              .arg(tableIndex + 1)
                              .arg(rootTablesWithUUID.size()));
    }

    db.commit();
}

// *******************************************************************
// ******** FUNCTIILE PRIVATE DE CONECTARE LA BD *********************

bool DataBase::openDataBase()
{
    if (globals().connectionMade == "MySQL"){

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(globals().mySQLhost);         // localhost
        db.setDatabaseName(globals().mySQLnameBase); // USGdb
        db.setPort(globals().mySQLport.toInt());
        db.setConnectOptions(globals().mySQLoptionConnect);
        db.setUserName(globals().mySQLuser);
        db.setPassword(globals().mySQLpasswdUser);
        if (db.open()){
            qInfo(logInfo()) << "Conectarea cu baza de date MYSQL instalata cu succes";
        } else {
            qWarning(logWarning()) << "Conectarea cu baza de date MYSQL lipseste: " + db.lastError().text();
            return false;
        }

    } else {

        //----------------------------------------------------------------------------------------------------
        // baza de date implicita
        if (globals().sqlitePathBase.isEmpty() || globals().sqlitePathBase.isNull()){
            qCritical(logCritical()) << tr("Nu este indicata variabila globala 'sqlitePathBase'.");
            return false;
        }
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setHostName(globals().sqliteNameBase);
        db.setDatabaseName(globals().sqlitePathBase);
        if(db.open()){
            qInfo(logInfo()) << "";
            qInfo(logInfo()) << "=~=~=~=~=~=~=~=~=~~=~=~=~=~= LANSARE NOUA =~=~=~=~=~=~=~=~~=~=~=~=~=~=~=~=~=";
            qInfo(logInfo()) << tr("Conectarea la baza de date '%1' este instalata cu succes.").arg(globals().sqliteNameBase);
            if (! enableForeignKeys())
                qWarning(logWarning()) << "Nu a fost activata suportul cheii externe";
        } else {
            qWarning(logWarning()) << tr("Conectarea la baza de date '%1' nu a fost instalata.").arg(globals().sqliteNameBase)
                                   << db.lastError().text();
            return false;
        }

        //----------------------------------------------------------------------------------------------------
        // baza de date image
        if (globals().pathImageBaseAppSettings.isEmpty() ||
            globals().pathImageBaseAppSettings.isNull()){
            qWarning(logWarning()) << tr("Nu este indicata variabila globala 'pathImageBaseAppSettings'.");

        } else {
            // if (db_image.isOpen())
            //     return true;

            db_image = QSqlDatabase::addDatabase("QSQLITE", "db_image");
            db_image.setHostName("db_image");
            db_image.setDatabaseName(globals().pathImageBaseAppSettings);
            if (db_image.open()){
                qInfo(logInfo()) << tr("Conectarea la baza de date 'db_image' este instalata cu succes.");

                // db_image is a separate SQLite file and has no independent
                // application-version marker. Repair its UUID schema
                // idempotently even when the main database is already 4.1.0.
                if (db_image.tables(QSql::Tables).contains(
                        QStringLiteral("imagesReports"), Qt::CaseInsensitive)) {
                    QSqlQuery imageQuery(db_image);
                    QSqlRecord imageRecord = db_image.record(QStringLiteral("imagesReports"));
                    if (!imageRecord.contains(QStringLiteral("patient_id"))) {
                        // Some historical db_image files contain copied views
                        // which refer to tables that only exist in the main DB.
                        // SQLite validates those broken views during ALTER TABLE
                        // and otherwise refuses an unrelated column rename.
                        QSqlQuery viewsQuery(db_image);
                        if (viewsQuery.exec(QStringLiteral(
                                "SELECT name FROM sqlite_master WHERE type='view'"))) {
                            QStringList invalidViews;
                            while (viewsQuery.next()) {
                                const QString viewName = viewsQuery.value(0).toString();
                                QString quotedView = viewName;
                                quotedView.replace(QLatin1Char('"'), QStringLiteral("\"\""));
                                QSqlQuery validateView(db_image);
                                if (!validateView.exec(QStringLiteral(
                                        "SELECT 1 FROM \"%1\" LIMIT 0").arg(quotedView)))
                                    invalidViews.append(viewName);
                            }
                            for (const QString &viewName : std::as_const(invalidViews)) {
                                QString quotedView = viewName;
                                quotedView.replace(QLatin1Char('"'), QStringLiteral("\"\""));
                                if (!imageQuery.exec(QStringLiteral("DROP VIEW \"%1\"")
                                                         .arg(quotedView))) {
                                    qCritical(logCritical())
                                        << tr("Nu s-a putut elimina view-ul invalid %1 din db_image:")
                                               .arg(viewName)
                                        << imageQuery.lastError().text();
                                    return false;
                                }
                                qInfo(logInfo())
                                    << tr("View invalid eliminat din db_image: %1.").arg(viewName);
                            }
                        }

                        const QString oldPatientColumn =
                            imageRecord.contains(QStringLiteral("id_patients"))
                                ? QStringLiteral("id_patients")
                                : (imageRecord.contains(QStringLiteral("id_pacients"))
                                       ? QStringLiteral("id_pacients") : QString{});
                        if (oldPatientColumn.isEmpty()
                            || !imageQuery.exec(QStringLiteral(
                                "ALTER TABLE imagesReports RENAME COLUMN `%1` TO patient_id")
                                                    .arg(oldPatientColumn))) {
                            qCritical(logCritical())
                                << tr("Nu s-a putut actualiza coloana pacientului în db_image.imagesReports:")
                                << imageQuery.lastError().text();
                            return false;
                        }
                        qInfo(logInfo())
                            << tr("Coloana %1 a fost redenumită în patient_id în db_image.imagesReports.")
                                   .arg(oldPatientColumn);
                        imageRecord = db_image.record(QStringLiteral("imagesReports"));
                    }

                    if (!imageRecord.contains(QStringLiteral("uuid"))) {
                        if (!imageQuery.exec(QStringLiteral(
                                "ALTER TABLE imagesReports ADD COLUMN uuid BLOB"))) {
                            qCritical(logCritical())
                                << tr("Nu s-a putut adăuga UUID în db_image.imagesReports:")
                                << imageQuery.lastError().text();
                            return false;
                        }
                        qInfo(logInfo()) << tr("Coloana UUID a fost adăugată în db_image.imagesReports.");
                    }

                    if (!imageQuery.exec(QStringLiteral(
                            "SELECT id FROM imagesReports "
                            "WHERE uuid IS NULL OR length(uuid) <> 16"))) {
                        qCritical(logCritical())
                            << tr("Nu s-au putut verifica UUID-urile imaginilor:")
                            << imageQuery.lastError().text();
                        return false;
                    }
                    QList<qint64> imageIds;
                    while (imageQuery.next())
                        imageIds.append(imageQuery.value(0).toLongLong());

                    if (!db_image.transaction()) {
                        qCritical(logCritical())
                            << tr("Nu s-a putut porni tranzacția UUID pentru db_image:")
                            << db_image.lastError().text();
                        return false;
                    }
                    QSqlQuery updateImage(db_image);
                    updateImage.prepare(QStringLiteral(
                        "UPDATE imagesReports SET uuid=? WHERE id=?"));
                    bool imageUuidOk = true;
                    for (qint64 imageId : std::as_const(imageIds)) {
                        updateImage.bindValue(0, QUuid::createUuid().toRfc4122(), QSql::Binary);
                        updateImage.bindValue(1, imageId);
                        if (!updateImage.exec()) {
                            imageUuidOk = false;
                            qCritical(logCritical())
                                << tr("Nu s-a putut genera UUID pentru imaginea id=%1:")
                                       .arg(imageId)
                                << updateImage.lastError().text();
                            break;
                        }
                    }
                    if (imageUuidOk) {
                        imageUuidOk = imageQuery.exec(QStringLiteral(
                            "CREATE UNIQUE INDEX IF NOT EXISTS uq_imagesReports_uuid "
                            "ON imagesReports(uuid)"));
                        if (!imageUuidOk)
                            qCritical(logCritical())
                                << tr("Nu s-a putut crea indexul UUID pentru imagini:")
                                << imageQuery.lastError().text();
                    }
                    if (!imageUuidOk || !db_image.commit()) {
                        db_image.rollback();
                        return false;
                    }
                    if (!imageIds.isEmpty())
                        qInfo(logInfo())
                            << tr("UUID generate pentru imaginile existente: %1.")
                                   .arg(imageIds.size());
                }
            } else {
                qWarning(logWarning()) << tr("Conectarea la baza de date 'db_image' nu a fost instalata.")
                                       << db.lastError().text();
                return false;
            }
        }

    }

    // reportVideo was added after some databases had already reached their
    // recorded application version. CREATE IF NOT EXISTS keeps this repair
    // safe for both existing and new SQLite/MariaDB installations.
    if (db.isOpen()
        && !db.tables(QSql::Tables).contains(QStringLiteral("reportVideo"),
                                              Qt::CaseInsensitive)) {
        const QString reportVideoResource = globals().thisSqlite
                                                ? QStringLiteral(":/sql/sqlite/tables/report_video.sql")
                                                : QStringLiteral(":/sql/mariadb/tables/report_video.sql");
        if (!db_common.execFileBatch(reportVideoResource,
                                     QStringLiteral("report_video repair"))) {
            qCritical(logCritical())
                << tr("Tabela reportVideo lipsește și nu a putut fi creată.");
            return false;
        }
        qInfo(logInfo()) << tr("Tabela lipsă reportVideo a fost creată.");
    }
    return true;
}

bool DataBase::restoreDataDase()
{
    if (this->openDataBase())
        return true;
    else
        return false;
}

void DataBase::closeDataBase()
{
    db.close();
}

bool DataBase::enableForeignKeys()
{
    QSqlQuery qry;
    qry.prepare("PRAGMA foreign_keys = ON;");
    if(! qry.exec()){
        qCritical(logCritical()) << "Eroare la activare a cheilor externe (foreign_keys).";
        qCritical(logCritical()) << qry.lastError().text();
        return false;
    } else {
        qInfo(logInfo()) << "Cheile externe(foreign_keys) au fost activate cu succes.";
        return true;
    }
    return false;
}
