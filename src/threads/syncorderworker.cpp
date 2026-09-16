#include "syncorderworker.h"

SyncOrderWorker::SyncOrderWorker(DatabaseProvider *provider,
                                 PatientDataStructure dataPatient,
                                 OrderDataStructure dataOrder,
                                 QObject *parent)
    : QObject{parent}
    , m_db(provider)
    , m_dataPatient(dataPatient)
    , m_dataOrder(dataOrder)
{}

void SyncOrderWorker::process()
{
    if (m_dataPatient.id <= 0 || m_dataOrder.id <= 0) {
        m_lastError = QStringLiteral("ID pacient sau comandă invalid.");
        qCritical(logCritical()).noquote()
            << QStringLiteral("[SYNC %1] %2").arg(metaObject()->className(), m_lastError);
        emit syncError(m_lastError);
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

    const QString conn_sync = QStringLiteral("order_sync_%1")
                                  .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    const QString conn_local = QStringLiteral("order_local_%1")
                                   .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    qInfo(logInfo()).noquote()
        << QStringLiteral("[SYNC %1] Începe sincronizarea comenzii. id_local=%2")
               .arg(metaObject()->className()).arg(m_dataOrder.id);

    {
        QSqlDatabase dbSync = m_db->getDatabaseSyncThread(conn_sync);
        QSqlDatabase dbLocal = m_db->getDatabaseThread(conn_local,
                                                        globals().thisMySQL,
                                                        "[SYNC]");
        bool ok = dbSync.isOpen() && dbLocal.isOpen();
        if (!ok) {
            m_lastError = QStringLiteral("Nu pot deschide conexiunea locală sau cloud.");
        } else if (!dbSync.transaction()) {
            ok = false;
            m_lastError = QStringLiteral("Nu pot porni tranzacția cloud: %1")
                              .arg(dbSync.lastError().text());
        }

        if (ok) {
            emit syncProgress(tr("Sincronizare pacient..."));
            ok = syncPatientData(dbSync, dbLocal);
        }
        if (ok) {
            emit syncProgress(tr("Sincronizare comandă..."));
            ok = syncOrderData(dbSync, dbLocal);
        }
        if (ok) {
            emit syncProgress(tr("Sincronizare investigații..."));
            ok = syncOrderTableData(dbSync, dbLocal);
        }

        if (ok && !dbSync.commit()) {
            ok = false;
            m_lastError = QStringLiteral("COMMIT cloud eșuat: %1")
                              .arg(dbSync.lastError().text());
        }
        if (!ok)
            dbSync.rollback();

        if (ok) {
            qInfo(logInfo()).noquote()
                << QStringLiteral("[SYNC %1] Comanda a fost sincronizată cu succes. id_local=%2 | id_cloud=%3")
                       .arg(metaObject()->className()).arg(m_dataOrder.id).arg(m_remoteOrderId);
            emit syncProgress(tr("Sincronizare finalizată cu succes"));
        } else {
            qCritical(logCritical()).noquote()
                << QStringLiteral("[SYNC %1] Sincronizarea comenzii a eșuat: %2")
                       .arg(metaObject()->className(), m_lastError);
            emit syncError(m_lastError);
        }

        dbSync.close();
        dbLocal.close();
    }

    m_db->removeDatabaseThread(conn_sync, "[SYNC]");
    m_db->removeDatabaseThread(conn_local, "[SYNC]");

    emit finished();
}

bool SyncOrderWorker::syncPatientData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_dataPatient.id <= 0)
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
    qryLocal.bindValue(":id", m_dataPatient.id);

    if (!qryLocal.exec()) {
        m_lastError = QStringLiteral("Citirea pacientului local a eșuat: %1")
                          .arg(qryLocal.lastError().text());
        return false;
    }

    if (!qryLocal.next()) {
        m_lastError = QStringLiteral("Pacientul local cu ID %1 nu a fost găsit.")
                          .arg(m_dataPatient.id);
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

bool SyncOrderWorker::syncOrderData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_dataOrder.id <= 0)
        return false;

    // Citim datele comenzii din SQLite
    QSqlQuery qryLocal(dbLocal);
    qryLocal.prepare("SELECT * FROM orderEcho WHERE id = :id");
    qryLocal.bindValue(":id", m_dataOrder.id);

    if (!qryLocal.exec()) {
        m_lastError = QStringLiteral("Citirea comenzii locale a eșuat: %1")
                          .arg(qryLocal.lastError().text());
        return false;
    }

    if (!qryLocal.next()) {
        m_lastError = QStringLiteral("Comanda locală cu ID %1 nu a fost găsită.")
                          .arg(m_dataOrder.id);
        return false;
    }

    QVariantMap orderData;
    QSqlRecord rec = qryLocal.record();
    for (int i = 0; i < rec.count(); ++i) {
        if (rec.fieldName(i) != QStringLiteral("docYear"))
            orderData[rec.fieldName(i)] = qryLocal.value(i);
    }

    if (!resolveRemoteForeignKeys(dbSync, dbLocal, orderData, &m_lastError))
        return false;

    if (!insertOrUpdateRecord(dbSync, "orderEcho", orderData)) {
        if (m_lastError.isEmpty())
            m_lastError = QStringLiteral("Inserarea/actualizarea antetului comenzii a eșuat.");
        return false;
    }

    QSqlQuery remoteIdQuery(dbSync);
    remoteIdQuery.prepare(QStringLiteral("SELECT id FROM orderEcho WHERE uuid = ? LIMIT 1"));
    remoteIdQuery.addBindValue(orderData.value(QStringLiteral("uuid")));
    if (!remoteIdQuery.exec() || !remoteIdQuery.next()) {
        m_lastError = QStringLiteral("Nu s-a putut determina ID-ul cloud al comenzii: %1")
                          .arg(remoteIdQuery.lastError().text());
        return false;
    }
    m_remoteOrderId = remoteIdQuery.value(0).toInt();
    return m_remoteOrderId > 0;
}

bool SyncOrderWorker::syncOrderTableData(QSqlDatabase &dbSync, QSqlDatabase &dbLocal)
{
    if (m_dataOrder.id <= 0)
        return false;

    // Citim toate rândurile comenzii din SQLite
    QSqlQuery qryLocal(dbLocal);
    qryLocal.prepare("SELECT * FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho");
    qryLocal.bindValue(":id_orderEcho", m_dataOrder.id);

    if (!qryLocal.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare citire detalii comandă SQLite:" << qryLocal.lastError().text();
        return false;
    }

    // Ștergem rândurile vechi din MariaDB
    QSqlQuery qryDeleteSync(dbSync);
    qryDeleteSync.prepare("DELETE FROM orderEchoTable WHERE id_orderEcho = ?");
    qryDeleteSync.addBindValue(m_remoteOrderId);

    if (!qryDeleteSync.exec()) {
        qCritical(logCritical()).noquote()
        << "Eroare ștergere detalii comandă MariaDB:" << qryDeleteSync.lastError().text();
        return false;
    }
        // Inserăm rândurile noi din SQLite în MariaDB
    while (qryLocal.next()) {
        QSqlQuery insertRow(dbSync);
        insertRow.prepare(QStringLiteral(
            "INSERT INTO orderEchoTable "
            "(deletionMark, id_orderEcho, cod, name, price) VALUES (?, ?, ?, ?, ?)"));
        insertRow.addBindValue(qryLocal.value(QStringLiteral("deletionMark")));
        insertRow.addBindValue(m_remoteOrderId);
        insertRow.addBindValue(qryLocal.value(QStringLiteral("cod")));
        insertRow.addBindValue(qryLocal.value(QStringLiteral("name")));
        insertRow.addBindValue(qryLocal.value(QStringLiteral("price")));
        if (!insertRow.exec()) {
            m_lastError = QStringLiteral("Inserarea liniei comenzii în cloud a eșuat: %1")
                              .arg(insertRow.lastError().text());
            return false;
        }
    }

    return true;
}

bool SyncOrderWorker::resolveRemoteForeignKeys(QSqlDatabase &dbSync,
                                               QSqlDatabase &dbLocal,
                                               QVariantMap &orderData,
                                               QString *error)
{
    struct ForeignKeyMap {
        const char *field;
        const char *table;
        bool required;
    };
    static const ForeignKeyMap mappings[] = {
        {"id_organizations", "organizations", true},
        {"id_contracts", "contracts", true},
        {"id_typesPrices", "typesPrices", true},
        {"id_doctors", "doctors", false},
        {"id_doctors_execute", "doctors", false},
        {"id_nurses", "nurses", false},
        {"patient_id", "patients", true},
        {"id_users", "users", true}
    };

    for (const ForeignKeyMap &mapping : mappings) {
        const QString field = QString::fromLatin1(mapping.field);
        const QVariant localValue = orderData.value(field);
        if (localValue.isNull() || localValue.toInt() <= 0) {
            if (mapping.required) {
                if (error)
                    *error = QStringLiteral("Cheia obligatorie %1 este invalidă.").arg(field);
                return false;
            }
            orderData[field] = QVariant();
            continue;
        }
        const int remoteId = remoteIdForLocalId(dbSync, dbLocal,
                                                 QString::fromLatin1(mapping.table),
                                                 localValue.toInt(), mapping.required, error);
        if (remoteId <= 0 && mapping.required)
            return false;
        orderData[field] = remoteId > 0 ? QVariant(remoteId) : QVariant();
    }
    return true;
}

int SyncOrderWorker::remoteIdForLocalId(QSqlDatabase &dbSync,
                                        QSqlDatabase &dbLocal,
                                        const QString &table,
                                        int localId,
                                        bool required,
                                        QString *error)
{
    QSqlQuery localQuery(dbLocal);
    localQuery.prepare(QStringLiteral("SELECT uuid FROM `%1` WHERE id = ? LIMIT 1").arg(table));
    localQuery.addBindValue(localId);
    if (!localQuery.exec() || !localQuery.next()) {
        if (required && error)
            *error = QStringLiteral("Nu s-a găsit %1 local, id=%2: %3")
                         .arg(table).arg(localId).arg(localQuery.lastError().text());
        return 0;
    }

    const QByteArray uuid = localQuery.value(0).toByteArray();
    if (uuid.size() != 16) {
        if (required && error)
            *error = QStringLiteral("UUID invalid pentru %1 local, id=%2.")
                         .arg(table).arg(localId);
        return 0;
    }

    const auto reconcileRemoteUuid = [&](int remoteId,
                                         const QString &matchedBy) -> int {
        QSqlQuery reconcileQuery(dbSync);
        reconcileQuery.prepare(
            QStringLiteral("UPDATE `%1` SET uuid = ? WHERE id = ?").arg(table));
        reconcileQuery.addBindValue(uuid, QSql::Binary);
        reconcileQuery.addBindValue(remoteId);
        if (!reconcileQuery.exec()) {
            if (error) {
                *error = QStringLiteral(
                    "Reconcilierea UUID pentru %1 a eșuat (id local=%2, id cloud=%3): %4")
                             .arg(table).arg(localId).arg(remoteId)
                             .arg(reconcileQuery.lastError().text());
            }
            return 0;
        }

        qInfo(logInfo()).noquote()
            << QStringLiteral("[SYNC %1] UUID reconciliat pentru %2: "
                              "id_local=%3 | id_cloud=%4 | identificare=%5.")
                   .arg(metaObject()->className(), table)
                   .arg(localId).arg(remoteId).arg(matchedBy);
        return remoteId;
    };

    QSqlQuery remoteQuery(dbSync);
    remoteQuery.prepare(QStringLiteral("SELECT id FROM `%1` WHERE uuid = ? LIMIT 1").arg(table));
    remoteQuery.addBindValue(uuid);
    if (!remoteQuery.exec()) {
        if (required && error)
            *error = QStringLiteral("Căutarea %1 în cloud după UUID a eșuat (id local=%2): %3")
                         .arg(table).arg(localId).arg(remoteQuery.lastError().text());
        return 0;
    }
    if (remoteQuery.next())
        return remoteQuery.value(0).toInt();

    // Pentru organizații, identificatorul fiscal este mai stabil decât ID-ul
    // numeric între două baze. Coloana se numește încă IDNP în schema curentă,
    // dar reprezintă IDNO și va putea fi redenumită într-o migrare ulterioară.
    if (table == QStringLiteral("organizations")) {
        QSqlQuery localIdnoQuery(dbLocal);
        localIdnoQuery.prepare(QStringLiteral(
            "SELECT IDNP FROM organizations WHERE id = ? LIMIT 1"));
        localIdnoQuery.addBindValue(localId);
        if (!localIdnoQuery.exec()) {
            if (required && error)
                *error = QStringLiteral("Citirea IDNO al organizației locale a eșuat: %1")
                             .arg(localIdnoQuery.lastError().text());
            return 0;
        }

        if (localIdnoQuery.next()) {
            const QString idno = localIdnoQuery.value(0).toString().trimmed();
            if (!idno.isEmpty()) {
                QSqlQuery remoteIdnoQuery(dbSync);
                remoteIdnoQuery.prepare(QStringLiteral(
                    "SELECT id FROM organizations WHERE IDNP = ? LIMIT 2"));
                remoteIdnoQuery.addBindValue(idno);
                if (!remoteIdnoQuery.exec()) {
                    if (required && error)
                        *error = QStringLiteral("Căutarea organizației în cloud după IDNO a eșuat: %1")
                                     .arg(remoteIdnoQuery.lastError().text());
                    return 0;
                }

                if (remoteIdnoQuery.next()) {
                    const int idByIdno = remoteIdnoQuery.value(0).toInt();
                    if (!remoteIdnoQuery.next()) {
                        qWarning(logWarning()).noquote()
                            << QStringLiteral("[SYNC %1] UUID diferit pentru organizations id_local=%2; "
                                              "organizația a fost identificată în cloud după IDNO.")
                                   .arg(metaObject()->className()).arg(localId);
                        return reconcileRemoteUuid(idByIdno, QStringLiteral("IDNO"));
                    }
                    qWarning(logWarning()).noquote()
                        << QStringLiteral("[SYNC %1] IDNO duplicat în cloud pentru organizations id_local=%2; "
                                          "se încearcă maparea istorică după ID.")
                               .arg(metaObject()->className()).arg(localId);
                }
            }
        }
    }

    // Bazele istorice SQLite/MariaDB puteau fi migrate separat, caz în care
    // aceleași înregistrări au primit UUID-uri diferite. În schema veche,
    // sincronizarea cataloagelor se baza pe ID-uri identice; păstrăm această
    // compatibilitate numai când rândul cu același ID există în cloud.
    QSqlQuery legacyIdQuery(dbSync);
    legacyIdQuery.prepare(QStringLiteral("SELECT id FROM `%1` WHERE id = ? LIMIT 1").arg(table));
    legacyIdQuery.addBindValue(localId);
    if (!legacyIdQuery.exec()) {
        if (required && error)
            *error = QStringLiteral("Căutarea compatibilă a %1 în cloud a eșuat (id local=%2): %3")
                         .arg(table).arg(localId).arg(legacyIdQuery.lastError().text());
        return 0;
    }
    if (legacyIdQuery.next()) {
        qWarning(logWarning()).noquote()
            << QStringLiteral("[SYNC %1] UUID diferit pentru %2 id_local=%3; "
                              "înregistrarea este identificată după ID-ul istoric.")
                   .arg(metaObject()->className(), table).arg(localId);
        return reconcileRemoteUuid(legacyIdQuery.value(0).toInt(),
                                   QStringLiteral("ID istoric"));
    }

    if (required && error)
        *error = QStringLiteral("Nu s-a găsit %1 în cloud nici după UUID, nici după ID (id local=%2).")
                     .arg(table).arg(localId);
    return 0;
}

bool SyncOrderWorker::insertOrUpdateRecord(QSqlDatabase &dbTarget,
                                           const QString &table,
                                           const QVariantMap &data)
{
    // Extract UUID from data
    const QByteArray uuid = data.value("uuid").toByteArray();
    if (uuid.size() != 16) {
        m_lastError = QStringLiteral("UUID invalid pentru tabela %1.").arg(table);
        return false;
    }

    if (checkIfRecordExists(dbTarget, table, uuid)) {
        // UPDATE
        QStringList updateFields;
        QSqlQuery qryUpdate(dbTarget);
        QString updateQuery = QString("UPDATE %1 SET ").arg(table);

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.key() != "id" && it.key() != "uuid") {
                updateFields.append(QString("`%1` = :%1").arg(it.key()));
            }
        }

        updateQuery.append(updateFields.join(", "));
        updateQuery.append(" WHERE uuid = :uuid");
        if (!qryUpdate.prepare(updateQuery)) {
            m_lastError = QStringLiteral("Pregătirea UPDATE pentru %1 a eșuat: %2")
                              .arg(table, qryUpdate.lastError().text());
            return false;
        }
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.key() != "id" && it.key() != "uuid")
                qryUpdate.bindValue(QStringLiteral(":") + it.key(), it.value());
        }
        qryUpdate.bindValue(":uuid", uuid, QSql::Binary);

        if (!qryUpdate.exec()) {
            m_lastError = QStringLiteral("UPDATE %1 a eșuat: %2")
                              .arg(table, qryUpdate.lastError().text());
            return false;
        }
    } else {
        // INSERT
        QSqlQuery qryInsert(dbTarget);
        QString insertQuery = QString("INSERT INTO %1 (").arg(table);
        QStringList fields;
        QStringList placeholders;

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.key() == QStringLiteral("id"))
                continue;
            fields.append(it.key());
            placeholders.append(QString(":%1").arg(it.key()));
        }

        insertQuery.append(fields.join(", "));
        insertQuery.append(") VALUES (");
        insertQuery.append(placeholders.join(", "));
        insertQuery.append(")");
        if (!qryInsert.prepare(insertQuery)) {
            m_lastError = QStringLiteral("Pregătirea INSERT pentru %1 a eșuat: %2")
                              .arg(table, qryInsert.lastError().text());
            return false;
        }
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.key() == QStringLiteral("id"))
                continue;
            qryInsert.bindValue(QStringLiteral(":") + it.key(), it.value(),
                                it.key() == QStringLiteral("uuid") ? QSql::Binary : QSql::In);
        }

        if (!qryInsert.exec()) {
            m_lastError = QStringLiteral("INSERT %1 a eșuat: %2")
                              .arg(table, qryInsert.lastError().text());
            return false;
        }
    }

    return true;
}

bool SyncOrderWorker::checkIfRecordExists(QSqlDatabase &db,
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
