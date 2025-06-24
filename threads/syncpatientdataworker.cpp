#include "syncpatientdataworker.h"

SyncPatientDataWorker::SyncPatientDataWorker(DatabaseProvider *provider,
                                             DatesCatPatient &data,
                                             QObject *parent)
    : QObject{parent}, m_db(provider), m_data(data)
{
    map_data["id_cloud_patient"] = 0;
}

void SyncPatientDataWorker::process()
{
    // 1. verificam daca este determinat ID
    if (m_data.id <= 0)
        return;

    // 2. definim denumirea conecxiunilor
    const QString connCloud = QStringLiteral("cloud_sync_conn_%1")
                                  .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    bool exist_patient = false;

    { // bloc pentru dbConn

        // 3. conectarea principala la BD in thread
        QSqlDatabase dbConn_cloud = m_db->getDatabaseSyncThread(connCloud, true);
        if (! dbConn_cloud.isOpen() &&
            ! dbConn_cloud.open()) {

            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB:")
            .arg(this->metaObject()->className())
                << dbConn_cloud.lastError().text();

        } else {

            // 4. verificam daca exista pacient cu acelasi nume, prenume, data nasterii
            if (patientExistInBD(dbConn_cloud)) {
                qWarning(logWarning()) << QStringLiteral("[SYNC] Pacientul '%1' din %2 deja exista in baza de date 'cloud'.")
                                              .arg(m_data.name + " " + m_data.fname,
                                                   m_data.birthday.toString("dd.MM.yyyy"));
                patientDataUpdate(dbConn_cloud);
                exist_patient = true;
            }

            // 5. sincronizarea de date
            if (! exist_patient)
                pacientDataInsert(dbConn_cloud);

            // 6. inchidem conexiunile cu bd
            dbConn_cloud.close();
        }

    } // distrugerea blocului dbConn

    // 7. distrugem conexiunea cu bd
    m_db->removeDatabaseThread(connCloud, "[SYNC]");

    // 8. emitem signal
    emit finished();
}

bool SyncPatientDataWorker::patientExistInBD(QSqlDatabase &dbConn)
{
    bool exist = false;

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        SELECT
            id
        FROM
            pacients
        WHERE
            IDNP = ?
        OR
            (name = ? AND
            fName = ? AND
            birthday = ?)
        LIMIT 1
    )");
    qry.addBindValue(m_data.idnp);
    qry.addBindValue(m_data.name);
    qry.addBindValue(m_data.fname);
    qry.addBindValue(m_data.birthday);

    if (qry.exec() && qry.next()) {
        exist = true;
        map_data["id_cloud_patient"] = qry.value(0).toInt();
    }

    return exist;
}

void SyncPatientDataWorker::patientDataUpdate(QSqlDatabase &dbConn)
{

    if (map_data.value("id_cloud_patient").toInt() <= 0) {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ID pacientului. Actualizarea datelor nu este posibila.";
        return;
    }

    dbConn.transaction();

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        UPDATE pacients SET
            deletionMark  = ?,
            IDNP          = ?,
            name          = ?,
            fName         = ?,
            mName         = ?,
            medicalPolicy = ?,
            birthday      = ?,
            address       = ?,
            telephone     = ?,
            email         = ?,
            comment       = ?
        WHERE
            id = ?
    )");
    qry.addBindValue(0);
    qry.addBindValue(m_data.idnp.isEmpty() ? QVariant() : m_data.idnp);
    qry.addBindValue(m_data.name);
    qry.addBindValue(m_data.fname);
    qry.addBindValue(QVariant());
    qry.addBindValue(m_data.medicalPolicy.isEmpty() ? QVariant() : m_data.medicalPolicy);
    qry.addBindValue(m_data.birthday);
    qry.addBindValue(m_data.address.isEmpty() ? QVariant() : m_data.address);
    qry.addBindValue(m_data.phone.isEmpty() ? QVariant() : m_data.phone);
    qry.addBindValue(m_data.email.isEmpty() ? QVariant() : m_data.email);
    qry.addBindValue(m_data.comment.isEmpty() ? QVariant() : m_data.comment);
    qry.addBindValue(map_data.value("id_cloud_patient").toInt()); // !!! ATENTIE indicam ID din bd cloud
    if (! qry.exec()) {
        dbConn.rollback();
        qCritical(logCritical()) << QStringLiteral("[SYNC %1] Eroare de actualizare a datelor pacientului %2: %3")
                                        .arg(this->metaObject()->className(),
                                             m_data.name + " " + m_data.fname,
                                             qry.lastError().text());
    } else {
        if (dbConn.commit() == false) {
            dbConn.rollback();
            qCritical() << QStringLiteral("[SYNC %1] Commit-ul pentru actualizarea datelor in tabela 'pacients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn.lastError().text());
        } else {
            qInfo(logInfo()) << QStringLiteral("[SYNC] Actualizarea cu succes a datelor pacientului - %1")
                                    .arg(m_data.name + " " + m_data.fname);
        }
    }
}

void SyncPatientDataWorker::pacientDataInsert(QSqlDatabase &dbConn_cloud)
{
    dbConn_cloud.transaction();

    QSqlQuery qry(dbConn_cloud);
    qry.prepare(R"(
        INSERT INTO pacients (
            deletionMark,
            IDNP,
            name,
            fName,
            mName,
            medicalPolicy,
            birthday,
            address,
            telephone,
            email,
            comment)
        VALUES (?,?,?,?,?,?,?,?,?,?,?)
        )");
    // id nu adaugam
    qry.addBindValue(0);
    qry.addBindValue((m_data.idnp.isEmpty()) ? QVariant() : m_data.idnp);
    qry.addBindValue(m_data.name);
    qry.addBindValue(m_data.fname);
    qry.addBindValue(QVariant());
    qry.addBindValue((m_data.medicalPolicy.isEmpty()) ? QVariant() : m_data.medicalPolicy);
    qry.addBindValue(m_data.birthday); //???
    qry.addBindValue((m_data.address.isEmpty()) ? QVariant() : m_data.address);
    qry.addBindValue((m_data.phone.isEmpty()) ? QVariant() : m_data.phone);
    qry.addBindValue((m_data.email.isEmpty()) ? QVariant() : m_data.email);
    qry.addBindValue((m_data.comment.isEmpty()) ? QVariant() : m_data.comment);

    if (! qry.exec()) {
        dbConn_cloud.rollback();
        qCritical(logCritical()) << QStringLiteral("[SYNC %1] Inserarea datelor pacientului %2 a eșuat: %3")
                                        .arg(this->metaObject()->className(),
                                             m_data.name + " " + m_data.fname,
                                             qry.lastError().text());
    } else {
        if (dbConn_cloud.commit() == false) {
            dbConn_cloud.rollback();
            qCritical() << QStringLiteral("[SYNC %1] Commit-ul pentru inserarea în tabela 'pacients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn_cloud.lastError().text());
        } else {
            qInfo(logInfo()) << QStringLiteral("[SYNC] Pacientul '%1' a fost sincronizat cu succes în cloud.")
                                    .arg(m_data.name + " " + m_data.fname);
        }
    }
}
