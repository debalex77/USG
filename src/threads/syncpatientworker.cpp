#include "syncpatientworker.h"

SyncPatientWorker::SyncPatientWorker(DatabaseProvider *provider,
                                     PatientDataStructure data,
                                     QObject *parent)
    : QObject{parent}
    , m_db(provider)
    , m_data(data)
{}

void SyncPatientWorker::process()
{
    if (!m_db) {
        const QString error = QStringLiteral("[SYNC %1] DatabaseProvider este nullptr.")
                                  .arg(metaObject()->className());
        qCritical(logCritical()).noquote() << error;
        emit syncError(error);
        emit finished();
        return;
    }

    if (m_data.id <= 0) {
        const QString error = QStringLiteral("[SYNC %1] ID pacient invalid: %2")
                                  .arg(metaObject()->className())
                                  .arg(m_data.id);
        qWarning(logWarning()).noquote() << error;
        emit syncError(error);
        emit finished();
        return;
    }

    if (!isUuidValid()) {
        const QString error = QStringLiteral("[SYNC %1] UUID pacient invalid. id=%2")
                                  .arg(metaObject()->className())
                                  .arg(m_data.id);
        qCritical(logCritical()).noquote() << error;
        emit syncError(error);
        emit finished();
        return;
    }

    qInfo(logInfo()).noquote()
        << QStringLiteral("[SYNC %1] Începe sincronizarea pacientului. id_local=%2 | uuid=%3")
               .arg(metaObject()->className())
               .arg(m_data.id)
               .arg(uuidToHex());

    const QString connSync = QStringLiteral("sync_conn_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    QString syncErrorText;

    { // bloc pentru distrugerea lui dbConn_sync înainte de removeDatabaseThread
        QSqlDatabase dbConn_sync = m_db->getDatabaseSyncThread(connSync);

        if (!dbConn_sync.isOpen() && !dbConn_sync.open()) {
            syncErrorText = QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB: %2")
                                .arg(metaObject()->className(),
                                     dbConn_sync.lastError().text());
            qCritical(logCritical()).noquote() << syncErrorText;

            dbConn_sync.close();
        } else {
            bool ok = true;

            if (!dbConn_sync.transaction()) {
                syncErrorText = QStringLiteral("[SYNC %1] Nu pot porni tranzacția DB: %2")
                                    .arg(metaObject()->className(),
                                         dbConn_sync.lastError().text());
                qCritical(logCritical()).noquote() << syncErrorText;
                ok = false;
            }

            if (ok) {
                bool existsQueryOk = false;
                const bool exists = patientExistsInDatabase(dbConn_sync,
                                                            &existsQueryOk,
                                                            &syncErrorText);

                if (!existsQueryOk) {
                    ok = false;
                } else {
                    ok = exists ? updatePatient(dbConn_sync, &syncErrorText)
                                : insertPatient(dbConn_sync, &syncErrorText);
                }
            }

            if (ok) {
                if (!dbConn_sync.commit()) {
                    syncErrorText = QStringLiteral("[SYNC %1] Eroare la COMMIT: %2")
                                        .arg(metaObject()->className(),
                                             dbConn_sync.lastError().text());
                    qCritical(logCritical()).noquote() << syncErrorText;
                    if (!dbConn_sync.rollback())
                        qCritical(logCritical()).noquote()
                        << QStringLiteral("[SYNC %1] Eroare și la ROLLBACK: %2")
                               .arg(metaObject()->className(), dbConn_sync.lastError().text());
                } else {
                    qInfo(logInfo()).noquote()
                        << QStringLiteral("[SYNC %1] Pacient sincronizat cu succes. id_local=%2 | uuid=%3")
                               .arg(metaObject()->className())
                               .arg(m_data.id)
                               .arg(uuidToHex());
                }
            } else {
                if (!dbConn_sync.rollback())
                    qCritical(logCritical()).noquote()
                    << QStringLiteral("[SYNC %1] Eroare la ROLLBACK: %2")
                           .arg(metaObject()->className(), dbConn_sync.lastError().text());
            }

            dbConn_sync.close();
        }
    }

    m_db->removeDatabaseThread(connSync, "[SYNC]");
    if (!syncErrorText.isEmpty())
        emit syncError(syncErrorText);
    emit finished();
}

bool SyncPatientWorker::isUuidValid() const
{
    if (m_data.uuid.isNull())
        return false;

    const QByteArray bytes = m_data.uuid.toRfc4122();
    return bytes.size() == 16;
}

QString SyncPatientWorker::uuidToHex() const
{
    if (m_data.uuid.isNull())
        return QStringLiteral("<null uuid>");

    return QString::fromLatin1(m_data.uuid.toRfc4122().toHex());
}

void SyncPatientWorker::logQueryError(const QString &context, const QSqlQuery &q) const
{
    qCritical(logCritical()).noquote()
    << QStringLiteral("[SYNC %1] %2 failed. error=%3 | native=%4 | uuid=%5")
            .arg(metaObject()->className(),
                 context,
                 q.lastError().text(),
                 q.lastError().nativeErrorCode(),
                 uuidToHex());
}

void SyncPatientWorker::bindPatientFields(QSqlQuery &q)
{
    q.bindValue(":deletion_mark",  m_data.deletionMark);
    q.bindValue(":idnp",          m_data.idnp);
    q.bindValue(":name",          m_data.name);
    q.bindValue(":first_name",         m_data.firstName);
    q.bindValue(":middle_name",         m_data.middleName.isEmpty() ? QVariant() : m_data.middleName);
    q.bindValue(":medical_policy", m_data.medicalPolicy);
    q.bindValue(":birthday",      m_data.birthday.toString("yyyy-MM-dd"));
    q.bindValue(":address",       m_data.address);
    q.bindValue(":telephone",     m_data.phone);
    q.bindValue(":email",         m_data.email);
    q.bindValue(":comment",       m_data.comment);
    q.bindValue(":uuid",          m_data.uuid.toRfc4122());
}

bool SyncPatientWorker::patientExistsInDatabase(QSqlDatabase &dbConn,
                                                bool *ok,
                                                QString *error)
{
    if (ok)
        *ok = false;

    QSqlQuery q(dbConn);
    q.prepare(R"(
        SELECT
            id
        FROM
            patients
        WHERE
            uuid = :uuid
        LIMIT 1
    )");

    q.bindValue(":uuid", m_data.uuid.toRfc4122());

    if (!q.exec()) {
        if (error)
            *error = QStringLiteral("Verificare existență pacient: %1")
                         .arg(q.lastError().text());
        logQueryError(QStringLiteral("patientExistsInDatabase"), q);
        return false;
    }

    if (ok)
        *ok = true;

    return q.next();
}

bool SyncPatientWorker::updatePatient(QSqlDatabase &dbConn, QString *error)
{
    QSqlQuery q(dbConn);
    q.prepare(R"(
        UPDATE patients SET
            deletion_mark = ?, idnp = ?, last_name = ?, first_name = ?,
            middle_name = ?, medical_policy = ?, birthday = ?, address = ?,
            telephone = ?, email = ?, comment = ?
        WHERE uuid = ?
    )");
    q.addBindValue(m_data.deletionMark);
    q.addBindValue(m_data.idnp.isEmpty() ? QVariant() : m_data.idnp);
    q.addBindValue(m_data.name);
    q.addBindValue(m_data.firstName);
    q.addBindValue(m_data.middleName.isEmpty() ? QVariant() : m_data.middleName);
    q.addBindValue(m_data.medicalPolicy.isEmpty() ? QVariant() : m_data.medicalPolicy);
    q.addBindValue(m_data.birthday.toString("yyyy-MM-dd"));
    q.addBindValue(m_data.address.isEmpty() ? QVariant() : m_data.address);
    q.addBindValue(m_data.phone.isEmpty() ? QVariant() : m_data.phone);
    q.addBindValue(m_data.email.isEmpty() ? QVariant() : m_data.email);
    q.addBindValue(m_data.comment.isEmpty() ? QVariant() : m_data.comment);
    q.addBindValue(m_data.uuid.toRfc4122());

    if (!q.exec()) {
        if (error)
            *error = QStringLiteral("Actualizare pacient cloud: %1")
                         .arg(q.lastError().text());
        logQueryError(QStringLiteral("updatePatient"), q);
        return false;
    }

    if (q.numRowsAffected() <= 0) {
        qWarning(logWarning()).noquote()
        << QStringLiteral("[SYNC %1] UPDATE fără rând afectat. uuid=%2")
                .arg(metaObject()->className(), uuidToHex());
    }

    return true;
}

bool SyncPatientWorker::insertPatient(QSqlDatabase &dbConn, QString *error)
{
    QSqlQuery q(dbConn);
    const bool prepared = q.prepare(R"(
        INSERT INTO `patients` (
            `deletion_mark`, `idnp`, `last_name`, `first_name`, `middle_name`, `medical_policy`,
            `birthday`, `address`, `telephone`, `email`, `comment`, `uuid`
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
        ))");
    if (!prepared) {
        if (error)
            *error = QStringLiteral("Pregătire inserare pacient cloud: %1")
                         .arg(q.lastError().text());
        logQueryError(QStringLiteral("prepare insertPatient"), q);
        return false;
    }
    q.addBindValue(m_data.deletionMark);
    q.addBindValue(m_data.idnp.isEmpty() ? QVariant() : m_data.idnp);
    q.addBindValue(m_data.name);
    q.addBindValue(m_data.firstName);
    q.addBindValue(m_data.middleName.isEmpty() ? QVariant() : m_data.middleName);
    q.addBindValue(m_data.medicalPolicy.isEmpty() ? QVariant() : m_data.medicalPolicy);
    q.addBindValue(m_data.birthday.toString("yyyy-MM-dd"));
    q.addBindValue(m_data.address.isEmpty() ? QVariant() : m_data.address);
    q.addBindValue(m_data.phone.isEmpty() ? QVariant() : m_data.phone);
    q.addBindValue(m_data.email.isEmpty() ? QVariant() : m_data.email);
    q.addBindValue(m_data.comment.isEmpty() ? QVariant() : m_data.comment);
    q.addBindValue(m_data.uuid.toRfc4122());

    if (!q.exec()) {
        if (error)
            *error = QStringLiteral("Inserare pacient cloud: %1")
                         .arg(q.lastError().text());
        logQueryError(QStringLiteral("insertPatient"), q);
        return false;
    }

    return true;
}
