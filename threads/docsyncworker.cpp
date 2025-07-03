#include "docsyncworker.h"

docSyncWorker::docSyncWorker(DatabaseProvider *provider, DatesDocsOrderReportSync &data, QObject *parent)
    : QObject{parent}, m_db(provider), m_data(data)
{}

void docSyncWorker::process()
{
    const QString connName_sync = QStringLiteral("connection_sync_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    const QString connName_local = QStringLiteral("connection_local_%1")
                                      .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if (m_data.id_order <= 0 ||
        m_data.id_report <= 0 ||
        m_data.id_patient <= 0)
        return;

    {
        // efectuam conectarea la bd (sync) int-un alt thread
        QSqlDatabase dbConn_sync = m_db->getDatabaseThread(connName_sync, m_data.thisMySQL);
        if (! dbConn_sync.isOpen() &&
            ! dbConn_sync.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (sync):")
                               .arg(this->metaObject()->className())
                        << dbConn_sync.lastError().text();
        }

        // efectuam conectarea la bd (sync) int-un alt thread
        QSqlDatabase dbConn_local = m_db->getDatabaseThread(connName_local, m_data.thisMySQL);
        if (! dbConn_local.isOpen() &&
            ! dbConn_local.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (local):")
                               .arg(this->metaObject()->className())
                        << dbConn_local.lastError().text();
        }

        // 1. determinam ID doctorului care a trimis pacientul din BD (sync)
        setIDdoctor(dbConn_sync);

        // 2. determinam ID pacientului din BD (sync)
        setIDpacient(dbConn_sync);

        // inchidem conexiunea cu BD
        if (dbConn_sync.isOpen())
            dbConn_sync.close();
        if (dbConn_local.isOpen())
            dbConn_local.close();
    }

    if (m_db->containConnection(connName_sync))
        m_db->removeDatabaseThread(connName_sync);
    if (m_db->containConnection(connName_local))
        m_db->removeDatabaseThread(connName_local);

    emit finished();
}

void docSyncWorker::setIDdoctor(QSqlDatabase &dbConn_sync)
{
    QSqlQuery qry(dbConn_sync);
    qry.prepare(R"(
        SELECT id FROM doctors WHERE name = ? AND fName = ? LIMIT 1
    )");
    qry.addBindValue(m_data.name_doctor);
    qry.addBindValue(m_data.fname_doctor);
    if (qry.exec() && qry.next()) {
        m_datesSync.id_doctor = qry.value(0).toInt();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ID doctorului din BD (sync)";
    }
}

void docSyncWorker::setIDpacient(QSqlDatabase &dbConn_sync)
{
    QSqlQuery qry(dbConn_sync);
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
    qry.addBindValue(m_data.idnp_patient);
    qry.addBindValue(m_data.name_doctor);
    qry.addBindValue(m_data.fname_doctor);
    if (qry.exec() && qry.next()) {
        m_datesSync.id_patient = qry.value(0).toInt();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ID pacientului din BD (sync)";
    }
}

void docSyncWorker::syncOrder(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    qry.prepare(R"(
        SELECT * FROM orderEcho WHERE id = ?
    )");
    qry.addBindValue(m_data.id_order);

    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        QSqlQuery qry_sync(dbConn_sync);
        qry_sync.prepare(R"(
            INSERT INTO orderEcho (
                deletionMark,
                numberDoc,
                dateDoc,
                id_organizations,
                id_contracts,
                id_typesPrices,
                id_doctors,
                id_doctors_execute,
                id_nurses,
                id_pacients,
                id_users,
                sum,
                comment,
                cardPayment,
                attachedImages)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");

        qry_sync.addBindValue(qry.value(rec.indexOf("deletionMark")));
        qry_sync.addBindValue(qry.value(rec.indexOf("numberDoc")));      // problema !!!!!
        qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
        qry_sync.addBindValue(qry.value(rec.indexOf("id_organizations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("id_contracts")));
        qry_sync.addBindValue(qry.value(rec.indexOf("id_typesPrices")));
        qry_sync.addBindValue(m_datesSync.id_doctor);
        qry_sync.addBindValue(qry.value(rec.indexOf("id_doctors_execute")));
        qry_sync.addBindValue(m_datesSync.id_nurse);
        qry_sync.addBindValue(m_datesSync.id_patient);
        qry_sync.addBindValue(qry.value(rec.indexOf("id_users")));
        qry_sync.addBindValue(qry.value(rec.indexOf("sum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cardPayment")));
        qry_sync.addBindValue(qry.value(rec.indexOf("attachedImages")));

        if (!qry_sync.exec()) {
            qWarning(logWarning()) << "[SYNC] Eroare la sincronizarea orderEcho:"
                                   << qry_sync.lastError().text();
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes documentul 'Comanda ecografica' nr."
                             << qry.value(rec.indexOf("numberDoc")).toString();
        }

    } else {
        qWarning(logWarning()) << "[SYNC] Nu s-a găsit comanda cu id ="
                               << m_data.id_order << "sau exec() a eșuat:"
                               << qry.lastError().text();
    }
}
