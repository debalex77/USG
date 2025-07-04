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

        // 3. sincronizarea orderEcho
        syncOrder(dbConn_sync, dbConn_local);

        // 4. sincronizarea Raportului
        syncReport(dbConn_sync, dbConn_local);

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

int docSyncWorker::getLastNumberDoc(const QString name_table, QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);
    QString strQry = QString(R"(
        SELECT
            numberDoc
        FROM
            %1
        ORDER BY
            id
        DESC LIMIT 1;
    )").arg(name_table);
    qry.prepare(strQry);
    if (qry.exec() && qry.next()) {
        return qry.value(0).toInt();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ultimul numar documentului din tabela"
                                 << name_table;
        return 0;
    }
}

void docSyncWorker::setIDdoctor(QSqlDatabase &dbConn_sync)
{
    QSqlQuery qry(dbConn_sync);
    qry.prepare(R"(
        SELECT
            id
        FROM
            doctors
        WHERE
            name = ? AND
            fName = ?
        LIMIT 1
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
    if (m_datesSync.id_doctor <= 0 || m_datesSync.id_patient <= 0)
        return;

    if (! dbConn_sync.transaction()) {
        qCritical(logCritical()) << "[SYNC] Nu s-a putut porni tranzactia:"
                                 << dbConn_sync.lastError().text();
        return;
    } else {
        qInfo(logInfo()) << "[SYNC] Initierea tranzactiei pentru documentul 'Comanda ecografica' nr."
                         << m_data.nr_order;
    }

    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT * FROM orderEcho WHERE id = ?
    )");
    qry.addBindValue(m_data.id_order);

    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

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
        qry_sync.addBindValue(getLastNumberDoc("orderEcho", dbConn_sync) + 1);
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

        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea orderEcho:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes documentul 'Comanda ecografica' nr."
                             << qry.value(rec.indexOf("numberDoc")).toString();

            m_datesSync.id_order = qry_sync.lastInsertId().toInt();
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Nu s-a găsit Comanda ecografica cu id ="
                                 << m_data.id_order << "sau exec() a eșuat:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
        return;
    }

    // inserarea orderEchoTable
    qry.prepare(R"(
        SELECT
            *
        FROM
            orderEchoTable
        WHERE
            id_orderEcho = ?
    )");
    qry.addBindValue(m_data.id_order);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO orderEchoTable (
                deletionMark,
                id_orderEcho,
                cod,
                name,
                price)
            VALUES(?, ?, ?, ?, ?, ?)
        )");
        qry_sync.addBindValue(0);
        qry_sync.addBindValue(qry.value(rec.indexOf("id_orderEcho")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cod")));
        qry_sync.addBindValue(qry.value(rec.indexOf("name")));
        qry_sync.addBindValue(qry.value(rec.indexOf("price")));

        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea orderEchoTable:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes a tabelei documentului 'Comanda ecografica' nr."
                             << qry.value(rec.indexOf("numberDoc")).toString();
        }

        if (! dbConn_sync.commit()) {
            qCritical(logCritical()) << "[SYNC] Commit eșuat:"
                                     << dbConn_sync.lastError().text();
            dbConn_sync.rollback();
        } else {
            qInfo(logInfo()) << "[SYNC] Tranzacție finalizată cu succes pentru documentul 'Comanda ecografica' nr."
                             << m_data.nr_order;
        }
    }

}

void docSyncWorker::syncReport(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    if (m_datesSync.id_doctor <= 0 ||
        m_datesSync.id_patient <= 0 ||
        m_datesSync.id_order <= 0)
        return;

    if (! dbConn_sync.transaction()) {
        qCritical(logCritical()) << "[SYNC] Nu s-a putut porni tranzactia pentru Raport ecografic nr."
                                 << m_data.nr_report << ":"
                                 << dbConn_sync.lastError().text();
        return;
    } else {
        qInfo(logInfo()) << "[SYNC] Initierea tranzactiei pentru documentul 'Raport ecografic' nr."
                         << m_data.nr_report;
    }

    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);


    // raport local
    qry.prepare(R"(
        SELECT
            *
        FROM
            reportEcho
        WHERE
            id = ? AND
            id_orderEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    qry.addBindValue(m_data.id_order);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO reportEcho (
                deletionMark,
                numberDoc,
                dateDoc,
                id_pacients,
                id_orderEcho,
                t_organs_internal,
                t_urinary_system,
                t_prostate,
                t_gynecology,
                t_breast,
                t_thyroid,
                t_gestation0,
                t_gestation1,
                t_gestation2,
                t_gestation3,
                id_users,
                concluzion,
                comment,
                attachedImages)
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");
        qry_sync.addBindValue(0);
        qry_sync.addBindValue(getLastNumberDoc("reportEcho", dbConn_sync) + 1);
        qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
        qry_sync.addBindValue(m_datesSync.id_patient);
        qry_sync.addBindValue(m_data.id_order);
        qry_sync.addBindValue(qry.value(rec.indexOf("t_organs_internal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_urinary_system")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_prostate")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_gynecology")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_breast")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_thyroid")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_gestation0")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_gestation1")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_gestation2")));
        qry_sync.addBindValue(qry.value(rec.indexOf("t_gestation3")));
        qry_sync.addBindValue(qry.value(rec.indexOf("id_users")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
        qry_sync.addBindValue(qry.value(rec.indexOf("attachedImages")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea reportEcho:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes a documentului 'Raport ecografic' nr."
                             << qry.value(rec.indexOf("numberDoc")).toString();
        }
    }

    if (! dbConn_sync.commit()) {
        qCritical(logCritical()) << "[SYNC] Commit eșuat:"
                                 << dbConn_sync.lastError().text();
        dbConn_sync.rollback();
    } else {
        qInfo(logInfo()) << "[SYNC] Tranzacție finalizată cu succes pentru documentul 'Raport ecografic' nr."
                         << m_data.nr_report;
    }
}
