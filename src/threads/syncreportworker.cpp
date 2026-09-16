#include "syncreportworker.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>

#include <common/globals.h>
#include <data/loggingcategories.h>
#include <threads/databaseprovider.h>

namespace {
const QStringList reportChildTables = {
    QStringLiteral("tableLiver"), QStringLiteral("tableCholecist"),
    QStringLiteral("tablePancreas"), QStringLiteral("tableSpleen"),
    QStringLiteral("tableIntestinalLoop"), QStringLiteral("tableKidney"),
    QStringLiteral("tableBladder"), QStringLiteral("tableProstate"),
    QStringLiteral("tableGynecology"), QStringLiteral("tableBreast"),
    QStringLiteral("tableThyroid"), QStringLiteral("tableGestation0"),
    QStringLiteral("tableGestation1"), QStringLiteral("tableGestation2"),
    QStringLiteral("tableGestation2_biometry"),
    QStringLiteral("tableGestation2_cranium"),
    QStringLiteral("tableGestation2_SNC"),
    QStringLiteral("tableGestation2_heart"),
    QStringLiteral("tableGestation2_thorax"),
    QStringLiteral("tableGestation2_abdomen"),
    QStringLiteral("tableGestation2_urinarySystem"),
    QStringLiteral("tableGestation2_other"),
    QStringLiteral("tableGestation2_doppler"),
    QStringLiteral("tableSofTissuesLymphNodes")
};
}

SyncReportWorker::SyncReportWorker(int localReportId, QObject *parent)
    : QObject(parent), m_localReportId(localReportId)
{}

void SyncReportWorker::process()
{
    if (m_localReportId <= 0) {
        emit syncError(tr("ID-ul raportului local este invalid."));
        emit finished();
        return;
    }

    const quintptr threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    const QString cloudConnection = QStringLiteral("report_sync_%1").arg(threadId);
    const QString localConnection = QStringLiteral("report_local_%1").arg(threadId);
    const QString imageConnection = QStringLiteral("report_images_%1").arg(threadId);
    DatabaseProvider provider;

    qInfo(logInfo()).noquote()
        << QStringLiteral("[SYNC SyncReportWorker] Începe sincronizarea raportului. id_local=%1")
               .arg(m_localReportId);

    bool ok = true;
    {
        QSqlDatabase cloudDb = provider.getDatabaseSyncThread(cloudConnection);
        QSqlDatabase localDb = provider.getDatabaseThread(localConnection,
                                                           globals().thisMySQL,
                                                           QStringLiteral("[SYNC]"));
        QSqlDatabase imageDb;
        if (!cloudDb.isOpen() || !localDb.isOpen()) {
            ok = false;
            m_lastError = tr("Nu pot deschide conexiunea locală sau cloud.");
        }
        if (ok && localDb.tables(QSql::Tables).contains(
                      QStringLiteral("reportVideo"), Qt::CaseInsensitive))
            ok = ensureRemoteReportVideoSchema(cloudDb);

        // DDL must run before the report transaction (MariaDB commits DDL implicitly).
        if (ok && !cloudDb.record(QStringLiteral("tableGestation1")).contains(
                      QStringLiteral("multiplePregnancy"))) {
            QSqlQuery schema(cloudDb);
            if (!schema.exec(QStringLiteral("ALTER TABLE tableGestation1 ADD COLUMN "
                                            "multiplePregnancy INTEGER NOT NULL DEFAULT 0"))
                && !cloudDb.record(QStringLiteral("tableGestation1")).contains(
                    QStringLiteral("multiplePregnancy"))) {
                ok = false;
                m_lastError = tr("Actualizarea schemei Gestation1 în cloud a eșuat: %1")
                                  .arg(schema.lastError().text());
            }
        }
        if (ok && !cloudDb.transaction()) {
            ok = false;
            m_lastError = tr("Nu pot porni tranzacția cloud: %1")
                              .arg(cloudDb.lastError().text());
        }

        if (ok) {
            emit syncProgress(tr("Sincronizare antet raport..."));
            ok = syncReportHeader(cloudDb, localDb);
        }

        for (const QString &table : reportChildTables) {
            if (!ok)
                break;
            emit syncProgress(tr("Sincronizare raport: %1...").arg(table));
            ok = replaceChildTable(cloudDb, localDb, table);
        }

        // reportVideo was introduced after some historical databases. Its absence
        // means that the report simply has no videos to synchronize.
        if (ok && localDb.tables(QSql::Tables).contains(QStringLiteral("reportVideo"),
                                                         Qt::CaseInsensitive)) {
            emit syncProgress(tr("Sincronizare raport: reportVideo..."));
            ok = replaceChildTable(cloudDb, localDb, QStringLiteral("reportVideo"));
        }

        if (ok) {
            QSqlDatabase imageSource = localDb;
            if (globals().thisSqlite) {
                imageDb = provider.getDatabaseImagesThread(imageConnection);
                if (!imageDb.isOpen()) {
                    ok = false;
                    m_lastError = tr("Nu pot deschide baza locală cu imaginile raportului.");
                } else {
                    imageSource = imageDb;
                }
            }

            if (ok && imageSource.tables(QSql::Tables).contains(
                          QStringLiteral("imagesReports"), Qt::CaseInsensitive)) {
                emit syncProgress(tr("Sincronizare imagini raport..."));
                ok = replaceChildTable(cloudDb, imageSource,
                                       QStringLiteral("imagesReports"));
            }
        }

        if (ok && !cloudDb.commit()) {
            ok = false;
            m_lastError = tr("Commit-ul raportului cloud a eșuat: %1")
                              .arg(cloudDb.lastError().text());
        }
        if (!ok && cloudDb.isOpen())
            cloudDb.rollback();

        if (ok) {
            qInfo(logInfo()).noquote()
                << QStringLiteral("[SYNC SyncReportWorker] Raport sincronizat cu succes. "
                                  "id_local=%1 | id_cloud=%2")
                       .arg(m_localReportId).arg(m_remoteReportId);
        } else {
            qCritical(logCritical()).noquote()
                << QStringLiteral("[SYNC SyncReportWorker] Sincronizarea raportului a eșuat: %1")
                       .arg(m_lastError);
            emit syncError(m_lastError);
        }

        cloudDb.close();
        localDb.close();
        if (imageDb.isValid())
            imageDb.close();
    }
    provider.removeDatabaseThread(cloudConnection, QStringLiteral("[SYNC]"));
    provider.removeDatabaseThread(localConnection, QStringLiteral("[SYNC]"));
    if (QSqlDatabase::contains(imageConnection))
        provider.removeDatabaseThread(imageConnection, QStringLiteral("[SYNC]"));
    emit finished();
}

bool SyncReportWorker::ensureRemoteReportVideoSchema(QSqlDatabase &cloudDb)
{
    const QString table = QStringLiteral("reportVideo");
    if (!cloudDb.tables(QSql::Tables).contains(table, Qt::CaseInsensitive)) {
        m_lastError = tr("Tabela reportVideo lipsește din baza cloud.");
        return false;
    }

    QSqlRecord record = cloudDb.record(table);
    QSqlQuery query(cloudDb);
    if (!record.contains(QStringLiteral("id_reportEcho"))) {
        if (!query.exec(QStringLiteral(
                "ALTER TABLE reportVideo ADD COLUMN id_reportEcho BIGINT UNSIGNED NULL"))) {
            m_lastError = tr("Coloana reportVideo.id_reportEcho nu a putut fi adăugată: %1")
                              .arg(query.lastError().text());
            return false;
        }
        qInfo(logInfo()) << "[SYNC SyncReportWorker] Coloana cloud reportVideo.id_reportEcho a fost adăugată.";
        record = cloudDb.record(table);
    }

    if (!record.contains(QStringLiteral("uuid"))) {
        if (!query.exec(QStringLiteral(
                "ALTER TABLE reportVideo ADD COLUMN uuid BINARY(16) NULL"))) {
            m_lastError = tr("Coloana reportVideo.uuid nu a putut fi adăugată: %1")
                              .arg(query.lastError().text());
            return false;
        }
        qInfo(logInfo()) << "[SYNC SyncReportWorker] Coloana cloud reportVideo.uuid a fost adăugată.";
    }

    if (!query.exec(QStringLiteral(
            "UPDATE reportVideo SET uuid=UNHEX(REPLACE(UUID(), '-', '')) "
            "WHERE uuid IS NULL OR OCTET_LENGTH(uuid) <> 16"))) {
        m_lastError = tr("UUID-urile istorice din reportVideo nu au putut fi completate: %1")
                          .arg(query.lastError().text());
        return false;
    }

    return true;
}

int SyncReportWorker::remoteIdByUuid(QSqlDatabase &cloudDb,
                                     QSqlDatabase &localDb,
                                     const QString &table,
                                     int localId)
{
    if (localId <= 0)
        return 0;
    QSqlQuery local(localDb);
    local.prepare(QStringLiteral("SELECT uuid FROM `%1` WHERE id=? LIMIT 1").arg(table));
    local.addBindValue(localId);
    if (!local.exec() || !local.next()) {
        m_lastError = tr("Nu s-a găsit %1 local, id=%2: %3")
                          .arg(table).arg(localId).arg(local.lastError().text());
        return 0;
    }
    const QByteArray uuid = local.value(0).toByteArray();
    if (uuid.size() != 16) {
        m_lastError = tr("UUID invalid pentru %1 local, id=%2.").arg(table).arg(localId);
        return 0;
    }
    QSqlQuery cloud(cloudDb);
    cloud.prepare(QStringLiteral("SELECT id FROM `%1` WHERE uuid=? LIMIT 1").arg(table));
    cloud.addBindValue(uuid, QSql::Binary);
    if (!cloud.exec() || !cloud.next()) {
        m_lastError = tr("Nu s-a găsit %1 în cloud după UUID, id local=%2: %3")
                          .arg(table).arg(localId).arg(cloud.lastError().text());
        return 0;
    }
    return cloud.value(0).toInt();
}

bool SyncReportWorker::syncReportHeader(QSqlDatabase &cloudDb, QSqlDatabase &localDb)
{
    QSqlQuery local(localDb);
    local.prepare(QStringLiteral("SELECT * FROM reportEcho WHERE id=? LIMIT 1"));
    local.addBindValue(m_localReportId);
    if (!local.exec() || !local.next()) {
        m_lastError = tr("Raportul local nu a fost găsit: %1").arg(local.lastError().text());
        return false;
    }

    QSqlRecord record = local.record();
    QVariantMap data;
    for (int column = 0; column < record.count(); ++column) {
        const QString name = record.fieldName(column);
        if (name != QStringLiteral("id") && name != QStringLiteral("docYear"))
            data.insert(name, local.value(column));
    }

    m_localPatientId = data.value(QStringLiteral("patient_id")).toInt();
    m_localOrderId = data.value(QStringLiteral("id_orderEcho")).toInt();
    const int localUserId = data.value(QStringLiteral("id_users")).toInt();
    m_remotePatientId = remoteIdByUuid(cloudDb, localDb, QStringLiteral("patients"),
                                       m_localPatientId);
    if (m_remotePatientId <= 0)
        return false;
    m_remoteOrderId = remoteIdByUuid(cloudDb, localDb, QStringLiteral("orderEcho"),
                                     m_localOrderId);
    if (m_remoteOrderId <= 0)
        return false;
    m_remoteUserId = remoteIdByUuid(cloudDb, localDb, QStringLiteral("users"),
                                    localUserId);
    if (m_remoteUserId <= 0)
        return false;

    data[QStringLiteral("patient_id")] = m_remotePatientId;
    data[QStringLiteral("id_orderEcho")] = m_remoteOrderId;
    data[QStringLiteral("id_users")] = m_remoteUserId;
    if (!writeByUuid(cloudDb, QStringLiteral("reportEcho"), data))
        return false;

    QSqlQuery remote(cloudDb);
    remote.prepare(QStringLiteral("SELECT id FROM reportEcho WHERE uuid=? LIMIT 1"));
    remote.addBindValue(data.value(QStringLiteral("uuid")), QSql::Binary);
    if (!remote.exec() || !remote.next()) {
        m_lastError = tr("Nu s-a putut determina ID-ul cloud al raportului: %1")
                          .arg(remote.lastError().text());
        return false;
    }
    m_remoteReportId = remote.value(0).toInt();
    if (m_remoteReportId <= 0)
        return false;

    QSqlQuery updateOrder(cloudDb);
    updateOrder.prepare(QStringLiteral(
        "UPDATE orderEcho SET attachedImages=? WHERE id=?"));
    updateOrder.addBindValue(data.value(QStringLiteral("attachedImages")));
    updateOrder.addBindValue(m_remoteOrderId);
    if (!updateOrder.exec()) {
        m_lastError = tr("Actualizarea indicatorului de atașamente al comenzii cloud a eșuat: %1")
                          .arg(updateOrder.lastError().text());
        return false;
    }
    return true;
}

bool SyncReportWorker::replaceChildTable(QSqlDatabase &cloudDb,
                                         QSqlDatabase &localDb,
                                         const QString &table)
{
    if (!localDb.tables(QSql::Tables).contains(table, Qt::CaseInsensitive)
        || !cloudDb.tables(QSql::Tables).contains(table, Qt::CaseInsensitive)) {
        m_lastError = tr("Tabela raportului %1 lipsește din baza locală sau cloud.").arg(table);
        return false;
    }

    QSqlQuery remove(cloudDb);
    remove.prepare(QStringLiteral("DELETE FROM `%1` WHERE id_reportEcho=?").arg(table));
    remove.addBindValue(m_remoteReportId);
    if (!remove.exec()) {
        m_lastError = tr("Ștergerea datelor vechi din %1 a eșuat: %2")
                          .arg(table, remove.lastError().text());
        return false;
    }

    QSqlQuery local(localDb);
    local.prepare(QStringLiteral("SELECT * FROM `%1` WHERE id_reportEcho=? ORDER BY id").arg(table));
    local.addBindValue(m_localReportId);
    if (!local.exec()) {
        m_lastError = tr("Citirea datelor locale din %1 a eșuat: %2")
                          .arg(table, local.lastError().text());
        return false;
    }

    while (local.next()) {
        const QSqlRecord record = local.record();
        QVariantMap data;
        for (int column = 0; column < record.count(); ++column) {
            const QString name = record.fieldName(column);
            if (name != QStringLiteral("id"))
                data.insert(name, local.value(column));
        }
        data[QStringLiteral("id_reportEcho")] = m_remoteReportId;
        if (data.contains(QStringLiteral("id_orderEcho")))
            data[QStringLiteral("id_orderEcho")] = m_remoteOrderId;
        if (data.contains(QStringLiteral("patient_id")))
            data[QStringLiteral("patient_id")] = m_remotePatientId;
        if (data.contains(QStringLiteral("id_users")))
            data[QStringLiteral("id_users")] = m_remoteUserId;
        if (data.contains(QStringLiteral("id_user")))
            data[QStringLiteral("id_user")] = m_remoteUserId;
        if (!insertMap(cloudDb, table, data))
            return false;
    }
    return true;
}

bool SyncReportWorker::writeByUuid(QSqlDatabase &cloudDb,
                                   const QString &table,
                                   const QVariantMap &data)
{
    const QByteArray uuid = data.value(QStringLiteral("uuid")).toByteArray();
    if (uuid.size() != 16) {
        m_lastError = tr("UUID invalid pentru %1.").arg(table);
        return false;
    }
    QSqlQuery exists(cloudDb);
    exists.prepare(QStringLiteral("SELECT 1 FROM `%1` WHERE uuid=? LIMIT 1").arg(table));
    exists.addBindValue(uuid, QSql::Binary);
    if (!exists.exec()) {
        m_lastError = tr("Verificarea raportului cloud a eșuat: %1")
                          .arg(exists.lastError().text());
        return false;
    }
    if (!exists.next())
        return insertMap(cloudDb, table, data);

    QStringList assignments;
    QVariantList values;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        if (it.key() == QStringLiteral("uuid"))
            continue;
        assignments << QStringLiteral("`%1`=?").arg(it.key());
        values << it.value();
    }
    QSqlQuery update(cloudDb);
    if (!update.prepare(QStringLiteral("UPDATE `%1` SET %2 WHERE uuid=?")
                            .arg(table, assignments.join(QStringLiteral(", "))))) {
        m_lastError = tr("Pregătirea actualizării %1 a eșuat: %2")
                          .arg(table, update.lastError().text());
        return false;
    }
    for (const QVariant &value : values)
        update.addBindValue(value);
    update.addBindValue(uuid, QSql::Binary);
    if (!update.exec()) {
        m_lastError = tr("Actualizarea %1 a eșuat: %2").arg(table, update.lastError().text());
        return false;
    }
    return true;
}

bool SyncReportWorker::insertMap(QSqlDatabase &db,
                                 const QString &table,
                                 const QVariantMap &data)
{
    QStringList columns;
    QStringList placeholders;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        columns << QStringLiteral("`%1`").arg(it.key());
        placeholders << QStringLiteral("?");
    }
    QSqlQuery insert(db);
    if (!insert.prepare(QStringLiteral("INSERT INTO `%1` (%2) VALUES (%3)")
                            .arg(table, columns.join(QStringLiteral(", ")),
                                 placeholders.join(QStringLiteral(", "))))) {
        m_lastError = tr("Pregătirea inserării în %1 a eșuat: %2")
                          .arg(table, insert.lastError().text());
        return false;
    }
    for (auto it = data.cbegin(); it != data.cend(); ++it)
        insert.addBindValue(it.value(),
                            it.key() == QStringLiteral("uuid") ? QSql::Binary : QSql::In);
    if (!insert.exec()) {
        m_lastError = tr("Inserarea în %1 a eșuat: %2").arg(table, insert.lastError().text());
        return false;
    }
    return true;
}
