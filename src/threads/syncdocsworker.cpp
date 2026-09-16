#include "syncdocsworker.h"

SyncDocsWorker::SyncDocsWorker(DatabaseProvider *provider,
                               QObject *parent)
    : QObject{parent}
    , m_db(provider)
{}

void SyncDocsWorker::process()
{
    if (m_data.id_order == 0 ||
        m_data.id_report == 0 ||
        m_data.id_patient == 0 ||
        m_data.nameConnection.isEmpty()) {
        qCritical() << QStringLiteral("[SYNC %1] Nu sunt determinate datele pu sincronizare !!!.")
        .arg(this->metaObject()->className());
        emit finished();
        return;
    }

    if (!m_db) {
        qCritical(logCritical()).noquote()
        << QStringLiteral("[SYNC %1] DatabaseProvider este nullptr.")
                .arg(metaObject()->className());
        emit finished();
        return;
    }

    const QString conn_sync = QStringLiteral("connection_sync_%1")
                                  .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    const QString conn_local = QStringLiteral("connection_local_%1")
                                   .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    const QString conn_img = QStringLiteral("connection_img_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    QSqlDatabase dbConn_sync;
    QSqlDatabase dbConn_local;
    QSqlDatabase dbConnImg;

    {
        // efectuam conectarea la bd (sync)
        dbConn_sync = m_db->getDatabaseSyncThread(conn_sync);
        if (! dbConn_sync.isOpen() &&
            ! dbConn_sync.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (sync):")
            .arg(this->metaObject()->className())
                << dbConn_sync.lastError().text();
            dbConn_sync.close();
            emit finished();
            return;
        }

        // efectuam conectarea la bd (local)
        dbConn_local = m_db->getDatabaseThread(conn_local, globals().thisMySQL, "[SYNC]");
        if (! dbConn_local.isOpen() &&
            ! dbConn_local.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (local):")
            .arg(this->metaObject()->className())
                << dbConn_local.lastError().text();
            dbConn_local.close();
            emit finished();
            return;
        }

        // efectuam conectarea la bd (db_image)
        dbConnImg = m_db->getDatabaseImagesThread(conn_img);
        if (! dbConnImg.isOpen() &&
            ! dbConnImg.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (db_image):")
            .arg(this->metaObject()->className())
                << dbConnImg.lastError().text();
            dbConnImg.close();
            emit finished();
            return;
        }
    }

    try {
        // 1. Sincronizare date pacient
        emit syncProgress(tr("Sincronizare date pacient..."));
        if (!syncPatientData(dbConn_sync, dbConn_local)) {
            throw std::runtime_error("Sincronizare date pacient eșuată");
        }

        // 2. Sincronizare date comandă
        emit syncProgress(tr("Sincronizare date comandă..."));
        if (!syncOrderData(dbConn_sync, dbConn_local)) {
            throw std::runtime_error("Sincronizare date comandă eșuată");
        }

        // 3. Sincronizare detalii comandă
        emit syncProgress(tr("Sincronizare detalii comandă..."));
        if (!syncOrderTableData(dbConn_sync, dbConn_local)) {
            throw std::runtime_error("Sincronizare detalii comandă eșuată");
        }

        // 4. Sincronizare imagini
        emit syncProgress(tr("Sincronizare imagini..."));
        if (!syncAttachedImages(dbConn_sync, dbConnImg)) {
            throw std::runtime_error("Sincronizare imagini eșuată");
        }

        emit syncProgress(tr("Sincronizare finalizată cu succes"));
        qInfo(logInfo()) << QStringLiteral("[SYNC] Sincronizare comandă ecografică ID=%1 finalizată cu succes")
                                .arg(m_data.id_order);

    } catch (const std::exception &e) {

        qCritical(logCritical()) << QStringLiteral("[SYNC] Eroare sincronizare: %1").arg(e.what());
        emit syncError(QString::fromStdString(e.what()));
    }

    // Curățare conexiuni
    if (m_db->containConnection(conn_sync))
        m_db->removeDatabaseThread(conn_sync, "[SYNC]");
    if (m_db->containConnection(conn_local))
        m_db->removeDatabaseThread(conn_local, "[SYNC]");
    if (m_db->containConnection(conn_img))
        m_db->removeDatabaseThread(conn_img, "[SYNC]");

    dbConn_sync.close();
    dbConn_local.close();
    dbConnImg.close();

    emit finished();
}

bool SyncDocsWorker::syncPatientData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_data.id_patient <= 0)
        return true;

    // Citim datele pacientului din SQLite (local)
    QSqlQuery qryLocal(dbLocal);
    qryLocal.prepare(R"(
        SELECT
            *
        FROM
            patients
        WHERE
            id = :id
        )");
    qryLocal.bindValue(":id", m_data.id_patient);

    if (!qryLocal.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare citire pacient SQLite:" << qryLocal.lastError().text();
        return false;
    }

    if (!qryLocal.next()) {
        qCritical(logCritical()) << "Pacientul cu ID" << m_data.id_patient << "nu este găsit în SQLite";
        return false;
    }

    // Construim map-ul cu datele pacientului
    QVariantMap patientData;
    QSqlRecord rec = qryLocal.record();
    for (int i = 0; i < rec.count(); ++i) {
        patientData[rec.fieldName(i)] = qryLocal.value(i);
    }

    // Sincronizare în MariaDB
    return insertOrUpdateRecord(dbSync, "patients", patientData);
}

bool SyncDocsWorker::syncOrderData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_data.id_order <= 0)
        return false;

    // Citim datele comenzii din SQLite
    QSqlQuery qryLocal(dbLocal);
    qryLocal.prepare("SELECT * FROM orderEcho WHERE id = :id");
    qryLocal.bindValue(":id", m_data.id_order);

    if (!qryLocal.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare citire comandă SQLite:" << qryLocal.lastError().text();
        return false;
    }

    if (!qryLocal.next()) {
        qCritical(logCritical()) << "Comanda cu ID" << m_data.id_order << "nu găsită în SQLite";
        return false;
    }

    QVariantMap orderData;
    QSqlRecord rec = qryLocal.record();
    for (int i = 0; i < rec.count(); ++i) {
        orderData[rec.fieldName(i)] = qryLocal.value(i);
    }

    // Sincronizare în MariaDB
    return insertOrUpdateRecord(dbSync, "orderEcho", orderData);
}

bool SyncDocsWorker::syncOrderTableData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_data.id_order <= 0)
        return false;

    // Citim toate rândurile comenzii din SQLite
    QSqlQuery qryLocal(dbLocal);
    qryLocal.prepare("SELECT * FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho");
    qryLocal.bindValue(":id_orderEcho", m_data.id_order);

    if (!qryLocal.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare citire detalii comandă SQLite:" << qryLocal.lastError().text();
        return false;
    }

    // Ștergem rândurile vechi din MariaDB
    QSqlQuery qryDeleteSync(dbSync);
    qryDeleteSync.prepare("DELETE FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho");
    qryDeleteSync.bindValue(":id_orderEcho", m_data.id_order);

    if (!qryDeleteSync.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare ștergere detalii comandă MariaDB:" << qryDeleteSync.lastError().text();
        return false;
    }
    // Inserăm rândurile noi din SQLite în MariaDB
    while (qryLocal.next()) {
        QVariantMap rowData;
        QSqlRecord rec = qryLocal.record();
        for (int i = 0; i < rec.count(); ++i) {
            rowData[rec.fieldName(i)] = qryLocal.value(i);
        }

        if (!insertOrUpdateRecord(dbSync, "orderEchoTable", rowData)) {
            qCritical(logCritical()) << "Eroare sincronizare rând comandă ID:" << qryLocal.value("id");
            return false;
        }
    }

    return true;
}

bool SyncDocsWorker::syncAttachedImages(QSqlDatabase &dbSync, QSqlDatabase &dbImg)
{
    if (m_data.id_order <= 0)
        return true; // Nu sunt imagini, skip

    // Citim imaginile din baza de date imagini (SQLite)
    QSqlQuery qryImgLocal(dbImg);
    qryImgLocal.prepare(QStringLiteral(
        "SELECT id, id_orderEcho, imageData, dateCreated, description "
        "FROM orderImages WHERE id_orderEcho = :id_orderEcho"
        ));
    qryImgLocal.bindValue(":id_orderEcho", m_data.id_order);

    if (!qryImgLocal.exec()) {
        qWarning(logWarning()).noquote()
        << "Eroare citire imagini SQLite:" << qryImgLocal.lastError().text();
        return true; // Nu este critic, continuăm
    }

    // Ștergem imaginile vechi din MariaDB
    QSqlQuery qryDeleteImg(dbSync);
    qryDeleteImg.prepare("DELETE FROM orderImages WHERE id_orderEcho = :id_orderEcho");
    qryDeleteImg.bindValue(":id_orderEcho", m_data.id_order);
    if (!qryDeleteImg.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare ștergere imagini MariaDB:" << qryDeleteImg.lastError().text();
        return false;
    }

    // Inserăm imaginile noi din SQLite în MariaDB
    while (qryImgLocal.next()) {
        QVariantMap rowData;
        QSqlRecord rec = qryImgLocal.record();
        for (int i = 0; i < rec.count(); ++i) {
            rowData[rec.fieldName(i)] = qryImgLocal.value(i);
        }

        if (!insertOrUpdateRecord(dbSync, "orderImages", rowData)) {
            qCritical(logCritical()) << "Eroare sincronizare imagine ID:" << qryImgLocal.value("id");
            return false;
        }
    }

    return true;
}

bool SyncDocsWorker::checkIfRecordExists(QSqlDatabase &db,
                                         const QString &table,
                                         const QByteArray &uuid)
{
    QSqlQuery qry(db);
    QString query = QString("SELECT 1 FROM %1 WHERE uuid = :uuid LIMIT 1").arg(table);
    qry.prepare(query);
    qry.bindValue(":uuid", uuid);

    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare verificare înregistrare:" << qry.lastError().text();
        return false;
    }

    return qry.next();
}

bool SyncDocsWorker::insertOrUpdateRecord(QSqlDatabase &dbTarget,
                                          const QString &table,
                                          const QVariantMap &data)
{
    // Extract UUID from data
    const QByteArray uuid = data.value("uuid").toByteArray();
    if (uuid.isEmpty()) {
        qCritical(logCritical()) << "UUID lipseste pentru tabel:" << table;
        return false;
    }

    if (checkIfRecordExists(dbTarget, table, uuid)) {
        // UPDATE
        QStringList updateFields;
        QSqlQuery qryUpdate(dbTarget);
        QString updateQuery = QString("UPDATE %1 SET ").arg(table);

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.key() != "id" && it.key() != "uuid") {
                updateFields.append(QString("%1 = :%1").arg(it.key()));
                qryUpdate.addBindValue(it.value(), QSql::Binary);
            }
        }

        updateQuery.append(updateFields.join(", "));
        updateQuery.append(" WHERE uuid = :uuid");
        qryUpdate.addBindValue(uuid, QSql::Binary);
        qryUpdate.prepare(updateQuery);

        if (!qryUpdate.exec()) {
            qCritical(logCritical()).noquote()
            << "Eroare UPDATE:" << qryUpdate.lastError().text();
            return false;
        }
    } else {
        // INSERT
        QSqlQuery qryInsert(dbTarget);
        QString insertQuery = QString("INSERT INTO %1 (").arg(table);
        QStringList fields;
        QStringList placeholders;

        for (auto it = data.begin(); it != data.end(); ++it) {
            fields.append(it.key());
            placeholders.append(QString(":%1").arg(it.key()));

            // Handle BLOB/BINARY types
            if (it.key() == "uuid") {
                qryInsert.addBindValue(it.value(), QSql::Binary);
            } else {
                qryInsert.addBindValue(it.value());
            }
        }

        insertQuery.append(fields.join(", "));
        insertQuery.append(") VALUES (");
        insertQuery.append(placeholders.join(", "));
        insertQuery.append(")");
        qryInsert.prepare(insertQuery);

        if (!qryInsert.exec()) {
            qCritical(logCritical()).noquote()
            << "Eroare INSERT:" << qryInsert.lastError().text();
            return false;
        }
    }

    return true;
}
