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
    const QString connName_img = QStringLiteral("connection_img_%1")
                                       .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if (m_data.id_order <= 0 ||
        m_data.id_report <= 0 ||
        m_data.id_patient <= 0)
        return;

    {
        // efectuam conectarea la bd (sync) int-un alt thread
        QSqlDatabase dbConn_sync = m_db->getDatabaseSyncThread(connName_sync);
        if (! dbConn_sync.isOpen() &&
            ! dbConn_sync.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (sync):")
                               .arg(this->metaObject()->className())
                        << dbConn_sync.lastError().text();
        }

        // efectuam conectarea la bd (sync) int-un alt thread
        QSqlDatabase dbConn_local = m_db->getDatabaseThread(connName_local, m_data.thisMySQL, "[SYNC]");
        if (! dbConn_local.isOpen() &&
            ! dbConn_local.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (local):")
                               .arg(this->metaObject()->className())
                        << dbConn_local.lastError().text();
        }

        // efectuam conectarea la bd (db_image) intr-un alt thread
        QSqlDatabase dbConnImg = m_db->getDatabaseImagesThread(connName_img);
        if (! dbConnImg.isOpen() &&
            ! dbConnImg.open()) {
            qCritical() << QStringLiteral("[SYNC %1] Nu pot deschide conexiunea DB (db_image):")
                               .arg(this->metaObject()->className())
                        << dbConnImg.lastError().text();
        }

        // 2. determinam ID pacientului din BD (sync)
        setIDpacient(dbConn_sync, dbConn_local);

        existreportDoc(dbConn_sync);

        // 3. sincronizarea orderEcho
        syncOrder(dbConn_sync, dbConn_local);

        // 4. sincronizarea Raportului
        syncReport(dbConn_sync, dbConn_local);

        // 5. sincronizam imaginile
        syncImages(dbConn_sync, dbConnImg);

        // inchidem conexiunea cu BD
        if (dbConn_sync.isOpen())
            dbConn_sync.close();
        if (dbConn_local.isOpen())
            dbConn_local.close();
    }

    if (m_db->containConnection(connName_sync))
        m_db->removeDatabaseThread(connName_sync, "[SYNC]");
    if (m_db->containConnection(connName_local))
        m_db->removeDatabaseThread(connName_local, "[SYNC]");
    if (m_db->containConnection(connName_img))
        m_db->removeDatabaseThread(connName_img, "[SYNC]");

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

int docSyncWorker::getIDdoctor(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local, const int m_ID)
{
    if (m_ID <= 0)
        return 0;

    QString name_doc  = nullptr;
    QString fname_doc = nullptr;

    QSqlQuery qry_local(dbConn_local);
    qry_local.prepare(R"(SELECT * FROM doctors WHERE id = ?)");
    qry_local.addBindValue(m_ID);
    if (qry_local.exec() && qry_local.next()) {
        QSqlRecord rec = qry_local.record();
        name_doc  = qry_local.value(rec.indexOf("name")).toString();
        fname_doc = qry_local.value(rec.indexOf("fName")).toString();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinate datele doctorului din BD (local):"
                                 << qry_local.lastError().text();
        return 0;
    }


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
    qry.addBindValue(name_doc);
    qry.addBindValue(fname_doc);
    if (qry.exec() && qry.next()) {
        return qry.value(0).toInt();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ID doctorului din BD (sync)";
        return 0;
    }
}

void docSyncWorker::setIDpacient(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry_local(dbConn_local);
    qry_local.prepare(R"(SELECT * FROM pacients WHERE id = ?)");
    qry_local.addBindValue(m_data.id_patient);
    if (qry_local.exec() && qry_local.next()) {
        QSqlRecord rec = qry_local.record();
        m_data.name_parient     = qry_local.value(rec.indexOf("name")).toString();
        m_data.fname_patient    = qry_local.value(rec.indexOf("fName")).toString();
        m_data.idnp_patient     = qry_local.value(rec.indexOf("IDNP")).toString();
        m_data.birthday_patient = qry_local.value(rec.indexOf("birthday")).toDate();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinate datele pacientului din BD (local):"
                                 << qry_local.lastError().text();
        return;
    }


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
    qry.addBindValue(m_data.name_parient);
    qry.addBindValue(m_data.fname_patient);
    qry.addBindValue(m_data.birthday_patient);
    if (qry.exec() && qry.next()) {
        m_datesSync.id_patient = qry.value(0).toInt();
    } else {
        qCritical(logCritical()) << "[SYNC] Nu este determinat ID pacientului din BD (sync)";
    }
}

bool docSyncWorker::existreportDoc(QSqlDatabase &dbConn_sync)
{
    QSqlQuery qry(dbConn_sync);
    qry.prepare(R"(
        SELECT
            reportEcho.id           AS id_report,
            reportEcho.id_orderEcho AS id_order,
            reportEcho.numberDoc    AS nr_report,
            reportEcho.id_pacients  AS id_patient,
            orderEcho.numberDoc     AS nr_order,
            orderEcho.id_doctors    AS id_doctor,
            orderEcho.id_doctors_execute AS id_doctor_execut
        FROM
            reportEcho
        INNER JOIN
            orderEcho ON orderEcho.id = reportEcho.id_orderEcho
        WHERE
            reportEcho.dateDoc = ? AND
            reportEcho.id_pacients = ?
        )");
    qry.addBindValue(m_data.dateTime_doc);
    qry.addBindValue(m_datesSync.id_patient);
    if (qry.exec() && qry.next()) {
        QSqlRecord rec = qry.record();
        m_datesSync.existDoc  = true;
        m_datesSync.id_report = qry.value(rec.indexOf("id_report")).toInt();
        m_datesSync.id_order  = qry.value(rec.indexOf("id_order")).toInt();
        m_datesSync.nr_report = qry.value(rec.indexOf("nr_report")).toString();
        m_datesSync.nr_order  = qry.value(rec.indexOf("nr_order")).toString();
        m_datesSync.id_doctor = qry.value(rec.indexOf("id_doctor")).toInt();
        m_datesSync.id_patient = qry.value(rec.indexOf("id_patient")).toInt();
        m_datesSync.id_doctor_execut = qry.value(rec.indexOf("id_doctor_execut")).toInt();
        return true;
    }
    return false;
}

void docSyncWorker::syncOrder(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    if (m_datesSync.id_patient <= 0)
        return;

    if (! dbConn_sync.transaction()) {
        qCritical(logCritical()) << "[SYNC] Nu s-a putut porni tranzactia:"
                                 << dbConn_sync.lastError().text();
        return;
    } else {
        m_datesSync.nr_order = QString::number(getLastNumberDoc("orderEcho", dbConn_sync) + 1); // determ.nr_order
        qInfo(logInfo()) << "[SYNC] Initierea tranzactiei pentru documentul 'Comanda ecografica' nr."
                         << m_datesSync.nr_order;
    }

    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT * FROM orderEcho WHERE id = ?
    )");
    qry.addBindValue(m_data.id_order);

    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // insert
        if (! m_datesSync.existDoc) {

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
            qry_sync.addBindValue(m_datesSync.nr_order);
            qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_organizations")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_contracts")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_typesPrices")));

            int id_doc = getIDdoctor(dbConn_sync, dbConn_local, qry.value(rec.indexOf("id_doctors")).toInt());
            int id_doc_execut = getIDdoctor(dbConn_sync, dbConn_local, qry.value(rec.indexOf("id_doctors_execute")).toInt());
            qry_sync.addBindValue(id_doc <= 0 ? QVariant() : id_doc);
            qry_sync.addBindValue(id_doc_execut <= 0 ? QVariant() : id_doc_execut);

            qry_sync.addBindValue((m_datesSync.id_nurse <= 0) ? QVariant() : m_datesSync.id_nurse);
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(qry.value(rec.indexOf("id_users")));
            qry_sync.addBindValue(qry.value(rec.indexOf("sum")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
            qry_sync.addBindValue(qry.value(rec.indexOf("cardPayment")));
            qry_sync.addBindValue(qry.value(rec.indexOf("attachedImages")));

        } else {

            qry_sync.prepare(R"(
                UPDATE orderEcho SET
                    deletionMark     = ?,
                    numberDoc        = ?,
                    dateDoc          = ?,
                    id_organizations = ?,
                    id_contracts     = ?,
                    id_typesPrices   = ?,
                    id_doctors       = ?,
                    id_doctors_execute = ?,
                    id_nurses      = ?,
                    id_pacients    = ?,
                    id_users       = ?,
                    sum            = ?,
                    comment        = ?,
                    cardPayment    = ?,
                    attachedImages = ?
                WHERE
                    id = ?;
            )");
            qry_sync.addBindValue(qry.value(rec.indexOf("deletionMark")));
            qry_sync.addBindValue(m_datesSync.nr_order);
            qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_organizations")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_contracts")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_typesPrices")));
            qry_sync.addBindValue(m_datesSync.id_doctor <= 0 ? QVariant() : m_datesSync.id_doctor);
            qry_sync.addBindValue(m_datesSync.id_doctor_execut <= 0 ? QVariant() : m_datesSync.id_doctor_execut);
            qry_sync.addBindValue((m_datesSync.id_nurse <= 0) ? QVariant() : m_datesSync.id_nurse);
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(qry.value(rec.indexOf("id_users")));
            qry_sync.addBindValue(qry.value(rec.indexOf("sum")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
            qry_sync.addBindValue(qry.value(rec.indexOf("cardPayment")));
            qry_sync.addBindValue(qry.value(rec.indexOf("attachedImages")));
            qry_sync.addBindValue(m_datesSync.id_order);
        }

        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea orderEcho:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes documentul 'Comanda ecografica' nr."
                             << m_datesSync.nr_order;

            if (! m_datesSync.existDoc)
                m_datesSync.id_order = qry_sync.lastInsertId().toInt(); // determ.id_order-lui
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
        SELECT * FROM orderEchoTable WHERE id_orderEcho = ?
    )");
    qry.addBindValue(m_data.id_order);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord rec = qry.record();

            if (! m_datesSync.existDoc) {

                qry_sync.prepare(R"(
                    INSERT INTO orderEchoTable (
                        deletionMark, id_orderEcho, cod, name, price)
                    VALUES(?,?,?,?,?)
                )");
                qry_sync.addBindValue(0);
                qry_sync.addBindValue(m_datesSync.id_order);
                qry_sync.addBindValue(qry.value(rec.indexOf("cod")));
                qry_sync.addBindValue(qry.value(rec.indexOf("name")));
                qry_sync.addBindValue(qry.value(rec.indexOf("price")));

            } else {

                qry_sync.prepare(R"(
                    UPDATE orderEchoTable SET
                        deletionMark = ?, cod = ?, name = ?, price = ?
                    WHERE
                        id_orderEcho = ?
                )");
                qry_sync.addBindValue(0);
                qry_sync.addBindValue(qry.value(rec.indexOf("cod")));
                qry_sync.addBindValue(qry.value(rec.indexOf("name")));
                qry_sync.addBindValue(qry.value(rec.indexOf("price")));
                qry_sync.addBindValue(m_datesSync.id_order);

            }

            if (! qry_sync.exec()) {
                qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea orderEchoTable:"
                                         << qry_sync.lastError().text();
                dbConn_sync.rollback();
                return;
            } else {
                qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes tabelei documentul 'Comanda ecografica' nr."
                                 << m_datesSync.nr_order;
            }
        }
    }

    if (! dbConn_sync.commit()) {
        qCritical(logCritical()) << "[SYNC] Commit eșuat:"
                                 << dbConn_sync.lastError().text();
        dbConn_sync.rollback();
        return;
    } else {
        qInfo(logInfo()) << "[SYNC] Tranzacție finalizată cu succes pentru documentul 'Comanda ecografica' nr."
                         << m_datesSync.nr_order;
    }

}

void docSyncWorker::syncReport(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    if (m_datesSync.id_patient <= 0 ||
        m_datesSync.id_order <= 0)
        return;

    if (! dbConn_sync.transaction()) {
        qCritical(logCritical()) << "[SYNC] Nu s-a putut porni tranzactia pentru Raport ecografic nr."
                                 << m_data.nr_report << ":"
                                 << dbConn_sync.lastError().text();
        return;
    } else {
        m_datesSync.nr_report = QString::number(getLastNumberDoc("reportEcho", dbConn_sync) + 1);
        qInfo(logInfo()) << "[SYNC] Initierea tranzactiei pentru documentul 'Raport ecografic' nr."
                         << m_datesSync.nr_report;
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

        if (! m_datesSync.existDoc) {

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
            qry_sync.addBindValue(m_datesSync.nr_report);
            qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(m_datesSync.id_order);
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

        } else {

            qry_sync.prepare(R"(
                UPDATE reportEcho SET
                    deletionMark = ?,
                    numberDoc    = ?,
                    dateDoc      = ?,
                    id_pacients  = ?,
                    id_orderEcho      = ?,
                    t_organs_internal = ?,
                    t_urinary_system  = ?,
                    t_prostate        = ?,
                    t_gynecology      = ?,
                    t_breast          = ?,
                    t_thyroid         = ?,
                    t_gestation0 = ?,
                    t_gestation1 = ?,
                    t_gestation2 = ?,
                    t_gestation3 = ?,
                    id_users     = ?,
                    concluzion   = ?,
                    comment      = ?,
                    attachedImages = ?
                WHERE
                    id = ?
            )");
            qry_sync.addBindValue(0);
            qry_sync.addBindValue(m_datesSync.nr_report);
            qry_sync.addBindValue(qry.value(rec.indexOf("dateDoc")));
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(m_datesSync.id_order);
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
            qry_sync.addBindValue(m_datesSync.id_report);

        }

        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea reportEcho:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizat cu succes a documentului 'Raport ecografic' nr."
                             << qry.value(rec.indexOf("numberDoc")).toString();

            m_datesSync.id_report = qry_sync.lastInsertId().toInt();

            // org.interne
            if (qry.value(rec.indexOf("t_organs_internal")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertOrgansInternal(dbConn_sync, dbConn_local);
                else
                    syncUpdateOrgansInternal(dbConn_sync, dbConn_local);
            }

            // s.urinar
            if (qry.value(rec.indexOf("t_urinary_system")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertUrinarySystem(dbConn_sync, dbConn_local);
                else
                    syncUpdateUrinarySystem(dbConn_sync, dbConn_local);
            }

            // prostata
            if (qry.value(rec.indexOf("t_prostate")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertProstate(dbConn_sync, dbConn_local);
                else
                    syncUpdateProstate(dbConn_sync, dbConn_local);
            }

            // ginecologia
            if (qry.value(rec.indexOf("t_gynecology")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertGynecology(dbConn_sync, dbConn_local);
                else
                    syncUpdateGynecology(dbConn_sync, dbConn_local);
            }

            // gl.mamare
            if (qry.value(rec.indexOf("t_breast")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertBreast(dbConn_sync, dbConn_local);
                else
                    syncUpdateBreast(dbConn_sync, dbConn_local);
            }

            // gl.tiroida
            if (qry.value(rec.indexOf("t_thyroid")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertThyroid(dbConn_sync, dbConn_local);
                else
                    syncUpdateThyroid(dbConn_sync, dbConn_local);
            }

            // gestation0
            if (qry.value(rec.indexOf("t_gestation0")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertGestation0(dbConn_sync, dbConn_local);
                else
                    syncUpdateGestation0(dbConn_sync, dbConn_local);
            }

            // gestation1
            if (qry.value(rec.indexOf("t_gestation1")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertGestation1(dbConn_sync, dbConn_local);
                else
                    syncUpdateGestation1(dbConn_sync, dbConn_local);
            }

            // gestation2
            if (qry.value(rec.indexOf("t_gestation2")).toBool()) {
                if (! m_datesSync.existDoc)
                    syncInsertGestation2(dbConn_sync, dbConn_local);
                else
                    syncUpdateGestation2(dbConn_sync, dbConn_local);
            }

            // imaginile
            syncImages(dbConn_sync, dbConn_local);
        }
    }

    if (! dbConn_sync.commit()) {
        qCritical(logCritical()) << "[SYNC] Commit eșuat:"
                                 << dbConn_sync.lastError().text();
        dbConn_sync.rollback();
    } else {
        qInfo(logInfo()) << "[SYNC] Tranzacție finalizată cu succes pentru documentul 'Raport ecografic' nr."
                         << m_datesSync.nr_report;
    }
}

void docSyncWorker::syncInsertOrgansInternal(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            l.id,
            l.id_reportEcho,
            l.[left]         AS liver_left_lobe,
            l.[right]        AS liver_right_lobe,
            l.contur         AS liver_contur,
            l.parenchim      AS liver_parenchim,
            l.ecogenity      AS liver_ecogenity,
            l.formations     AS liver_formations,
            l.ductsIntrahepatic AS liver_ductsIntrahepatic,
            l.porta          AS liver_porta,
            l.lienalis       AS liver_lienalis,
            l.concluzion     AS liver_concluzion,
            l.recommendation AS liver_recommendation,
            c.form           AS cholecist_form,
            c.dimens         AS cholecist_dimens,
            c.walls          AS cholecist_walls,
            c.choledoc       AS cholecist_choledoc,
            c.formations     AS cholecist_formations,
            p.cefal          AS pancreas_cefal,
            p.corp           AS pancreas_corp,
            p.tail           AS pancreas_tail,
            p.texture        AS pancreas_texture,
            p.ecogency       AS pancreas_ecogency,
            p.formations     AS pancreas_formations,
            s.dimens     AS spleen_dimens,
            s.contur     AS spleen_contur,
            s.parenchim  AS spleen_parenchim,
            s.formations AS spleen_formations,
            i.formations AS intestinal_formation
        FROM
            tableLiver AS l
        LEFT JOIN
            tableCholecist AS c ON l.id_reportEcho = c.id_reportEcho
        LEFT JOIN
            tablePancreas AS p ON l.id_reportEcho = p.id_reportEcho
        LEFT JOIN
            tableSpleen AS s ON l.id_reportEcho = s.id_reportEcho
        LEFT JOIN
            tableIntestinalLoop AS i ON l.id_reportEcho = i.id_reportEcho
        WHERE
            l.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // liver
        qry_sync.prepare(R"(
            INSERT INTO tableLiver (
                id_reportEcho, `left`, `right`, contur, parenchim, ecogenity,
                formations, ductsIntrahepatic, porta, lienalis, concluzion, recommendation)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_left_lobe")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_right_lobe")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_contur")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_parenchim")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_ecogenity")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_ductsIntrahepatic")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_porta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_lienalis")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_recommendation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableLiver:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableLiver' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // cholecist
        qry_sync.prepare(R"(
            INSERT INTO tableCholecist(
                id_reportEcho, form, dimens, walls, choledoc, formations)
            VALUES(?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_form")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_walls")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_choledoc")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableCholecist:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableCholecist' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // pancreas
        qry_sync.prepare(R"(
            INSERT INTO tablePancreas (
                id_reportEcho, cefal, corp, tail, texture, ecogency, formations)
            VALUES(?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_cefal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_corp")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_tail")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_texture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_ecogency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tablePancreas:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tablePancreas' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // spleen
        qry_sync.prepare(R"(
            INSERT INTO tableSpleen (
                id_reportEcho, dimens, contur, parenchim, formations)
            VALUES(?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_contur")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_parenchim")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableSpleen:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableSpleen' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // intestinal loop
        qry_sync.prepare(R"(
            INSERT INTO tableIntestinalLoop (
                id_reportEcho, formations)
            VALUES(?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("intestinal_formation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableIntestinalLoop:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableIntestinalLoop' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea organelor interne:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateOrgansInternal(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            l.id,
            l.id_reportEcho,
            l.[left]         AS liver_left_lobe,
            l.[right]        AS liver_right_lobe,
            l.contur         AS liver_contur,
            l.parenchim      AS liver_parenchim,
            l.ecogenity      AS liver_ecogenity,
            l.formations     AS liver_formations,
            l.ductsIntrahepatic AS liver_ductsIntrahepatic,
            l.porta          AS liver_porta,
            l.lienalis       AS liver_lienalis,
            l.concluzion     AS liver_concluzion,
            l.recommendation AS liver_recommendation,
            c.form           AS cholecist_form,
            c.dimens         AS cholecist_dimens,
            c.walls          AS cholecist_walls,
            c.choledoc       AS cholecist_choledoc,
            c.formations     AS cholecist_formations,
            p.cefal          AS pancreas_cefal,
            p.corp           AS pancreas_corp,
            p.tail           AS pancreas_tail,
            p.texture        AS pancreas_texture,
            p.ecogency       AS pancreas_ecogency,
            p.formations     AS pancreas_formations,
            s.dimens     AS spleen_dimens,
            s.contur     AS spleen_contur,
            s.parenchim  AS spleen_parenchim,
            s.formations AS spleen_formations,
            i.formations AS intestinal_formation
        FROM
            tableLiver AS l
        LEFT JOIN
            tableCholecist AS c ON l.id_reportEcho = c.id_reportEcho
        LEFT JOIN
            tablePancreas AS p ON l.id_reportEcho = p.id_reportEcho
        LEFT JOIN
            tableSpleen AS s ON l.id_reportEcho = s.id_reportEcho
        LEFT JOIN
            tableIntestinalLoop AS i ON l.id_reportEcho = i.id_reportEcho
        WHERE
            l.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // liver
        qry_sync.prepare(R"(
            UPDATE tableLiver SET
                `left` = ?, `right` = ?, contur = ?, parenchim = ?, ecogenity = ?,
                formations = ?, ductsIntrahepatic = ?, porta = ?, lienalis = ?, concluzion = ?, recommendation = ?
            WHERE
                id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_left_lobe")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_right_lobe")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_contur")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_parenchim")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_ecogenity")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_ductsIntrahepatic")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_porta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_lienalis")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liver_recommendation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableLiver:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableLiver' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // cholecist
        qry_sync.prepare(R"(
            UPDATE tableCholecist SET
                form = ?, dimens = ?, walls = ?, choledoc = ?, formations = ?
            WHERE
                id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_form")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_walls")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_choledoc")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableCholecist:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableCholecist' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // pancreas
        qry_sync.prepare(R"(
            UPDATE tablePancreas SET
                cefal = ?, corp = ?, tail = ?, texture = ?, ecogency = ?, formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_cefal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_corp")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_tail")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_texture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_ecogency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pancreas_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tablePancreas:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tablePancreas' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // spleen
        qry_sync.prepare(R"(
            UPDATE tableSpleen SET
                dimens = ?, contur = ?, parenchim = ?, formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_contur")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_parenchim")));
        qry_sync.addBindValue(qry.value(rec.indexOf("spleen_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableSpleen:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableSpleen' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // intestinal loop
        qry_sync.prepare(R"(
            UPDATE tableIntestinalLoop SET
                formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("intestinal_formation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableIntestinalLoop:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableIntestinalLoop' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea organelor interne:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertUrinarySystem(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            k.id,
            k.id_reportEcho,
            k.contour_right    AS kidney_contour_right,
            k.contour_left     AS kidney_contour_left,
            k.dimens_right     AS kidney_dimens_right,
            k.dimens_left      AS kidney_dimens_left,
            k.corticomed_right AS kidney_corticomed_right,
            k.corticomed_left  AS kidney_corticomed_left,
            k.pielocaliceal_right AS kidney_pielocaliceal_right,
            k.pielocaliceal_left  AS kidney_pielocaliceal_left,
            k.formations            AS kidney_formations,
            k.suprarenal_formations AS kidney_suprarenal_formations,
            k.concluzion            AS kidney_concluzion,
            k.recommendation        AS kidney_recommendation,
            b.volum                 AS bladder_volum,
            b.walls                 AS bladder_walls,
            b.formations            AS bladder_formations
        FROM
            tableKidney k
        LEFT JOIN
            tableBladder AS b ON k.id_reportEcho = b.id_reportEcho
        WHERE
            k.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // rinichi
        qry_sync.prepare(R"(
            INSERT INTO tableKidney (
                id_reportEcho, dimens_right, dimens_left, corticomed_right, corticomed_left, pielocaliceal_right, pielocaliceal_left,
                formations, concluzion, recommendation, contour_right, contour_left, suprarenal_formations)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_dimens_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_dimens_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_corticomed_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_corticomed_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_pielocaliceal_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_pielocaliceal_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_contour_right")).toString());
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_contour_left")).toString());
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_suprarenal_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableKidney:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableKidney' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // v.urinara
        qry_sync.prepare(R"(
            INSERT INTO tableBladder (
                id_reportEcho, volum, walls, formations)
            VALUES(?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_walls")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableBladder:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableBladder' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea s.urinar:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }

}

void docSyncWorker::syncUpdateUrinarySystem(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            k.id,
            k.id_reportEcho,
            k.contour_right    AS kidney_contour_right,
            k.contour_left     AS kidney_contour_left,
            k.dimens_right     AS kidney_dimens_right,
            k.dimens_left      AS kidney_dimens_left,
            k.corticomed_right AS kidney_corticomed_right,
            k.corticomed_left  AS kidney_corticomed_left,
            k.pielocaliceal_right AS kidney_pielocaliceal_right,
            k.pielocaliceal_left  AS kidney_pielocaliceal_left,
            k.formations            AS kidney_formations,
            k.suprarenal_formations AS kidney_suprarenal_formations,
            k.concluzion            AS kidney_concluzion,
            k.recommendation        AS kidney_recommendation,
            b.volum                 AS bladder_volum,
            b.walls                 AS bladder_walls,
            b.formations            AS bladder_formations
        FROM
            tableKidney k
        LEFT JOIN
            tableBladder AS b ON k.id_reportEcho = b.id_reportEcho
        WHERE
            k.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // rinichi
        qry_sync.prepare(R"(
            UPDATE tableKidney SET
                dimens_right = ?, dimens_left = ?, corticomed_right = ?, corticomed_left = ?, pielocaliceal_right = ?, pielocaliceal_left = ?,
                formations = ?, concluzion = ?, recommendation = ?, contour_right = ?, contour_left = ?, suprarenal_formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_dimens_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_dimens_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_corticomed_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_corticomed_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_pielocaliceal_right")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_pielocaliceal_left")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_contour_right")).toString());
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_contour_left")).toString());
        qry_sync.addBindValue(qry.value(rec.indexOf("kidney_suprarenal_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableKidney:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableKidney' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // v.urinara
        qry_sync.prepare(R"(
            UPDATE tableBladder SET
                volum = ?, walls = ?, formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_walls")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableBladder:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableBladder' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea s.urinar:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertProstate(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableProstate WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableProstate (
                id_reportEcho, dimens, volume, ecostructure, contour,
                ecogency, formations, transrectal, concluzion, recommendation)
            VALUES(?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("volume")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("contour")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecogency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("transrectal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableProstate:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableProstate' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.prostata:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateProstate(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableProstate WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            UPDATE tableProstate SET
                dimens = ?, volume = ?, ecostructure = ?, contour = ?,
                ecogency = ?, formations = ?, transrectal = ?, concluzion = ?, recommendation = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("volume")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("contour")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecogency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("transrectal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableProstate:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableProstate' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.prostata:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertGynecology(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGynecology WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableGynecology (
                id_reportEcho, transvaginal, dateMenstruation, antecedent, uterus_dimens,
                uterus_pozition, uterus_ecostructure, uterus_formations, ecou_dimens,
                ecou_ecostructure, cervix_dimens, cervix_ecostructure, douglas, plex_venos,
                ovary_right_dimens, ovary_left_dimens, ovary_right_volum, ovary_left_volum,
                ovary_right_follicle, ovary_left_follicle, ovary_right_formations, ovary_left_formations,
                concluzion, recommendation, junctional_zone, junctional_zone_description, cervical_canal,
                cervical_canal_formations, fallopian_tubes, fallopian_tubes_formations)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("transvaginal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("dateMenstruation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_pozition")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecou_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecou_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("douglas")));
        qry_sync.addBindValue(qry.value(rec.indexOf("plex_venos")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_follicle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_follicle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("junctional_zone")));
        qry_sync.addBindValue(qry.value(rec.indexOf("junctional_zone_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervical_canal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervical_canal_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fallopian_tubes")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fallopian_tubes_formations")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableGynecology:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableGynecology' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea ginecologia:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateGynecology(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGynecology WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            UPDATE tableGynecology SET
                transvaginal = ?, dateMenstruation = ?, antecedent = ?, uterus_dimens = ?,
                uterus_pozition = ?, uterus_ecostructure = ?, uterus_formations = ?, ecou_dimens = ?,
                ecou_ecostructure = ?, cervix_dimens = ?, cervix_ecostructure = ?, douglas = ?, plex_venos = ?,
                ovary_right_dimens = ?, ovary_left_dimens = ?, ovary_right_volum = ?, ovary_left_volum = ?,
                ovary_right_follicle = ?, ovary_left_follicle = ?, ovary_right_formations = ?, ovary_left_formations = ?,
                concluzion, recommendation = ?, junctional_zone = ?, junctional_zone_description = ?, cervical_canal = ?,
                cervical_canal_formations = ?, fallopian_tubes = ?, fallopian_tubes_formations = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("transvaginal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("dateMenstruation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_pozition")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterus_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecou_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ecou_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("douglas")));
        qry_sync.addBindValue(qry.value(rec.indexOf("plex_venos")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_follicle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_follicle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_right_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary_left_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("junctional_zone")));
        qry_sync.addBindValue(qry.value(rec.indexOf("junctional_zone_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervical_canal")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervical_canal_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fallopian_tubes")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fallopian_tubes_formations")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGynecology:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGynecology' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea ginecologia:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertBreast(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableBreast WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableBreast (
                id_reportEcho, breast_right_ecostrcture, breast_right_duct, breast_right_ligament,
                breast_right_formations, breast_right_ganglions, breast_left_ecostrcture, breast_left_duct,
                breast_left_ligament, breast_left_formations, breast_left_ganglions, concluzion, recommendation)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ecostrcture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_duct")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ligament")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ecostrcture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_duct")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ligament")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableBreast:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableBreast' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.mamare:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateBreast(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableBreast WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            UPDATE tableBreast SET
                breast_right_ecostrcture = ?, breast_right_duct = ?, breast_right_ligament = ?,
                breast_right_formations = ?, breast_right_ganglions = ?, breast_left_ecostrcture = ?, breast_left_duct = ?,
                breast_left_ligament = ?, breast_left_formations = ?, breast_left_ganglions = ?, concluzion = ?, recommendation = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ecostrcture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_duct")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ligament")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_right_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ecostrcture")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_duct")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ligament")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("breast_left_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableBreast:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableBreast' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.mamare:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertThyroid(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableThyroid WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableThyroid (
                id_reportEcho, thyroid_right_dimens, thyroid_right_volum, thyroid_left_dimens,
                thyroid_left_volum, thyroid_istm, thyroid_ecostructure, thyroid_formations,
                thyroid_ganglions, concluzion, recommendation)
            VALUES(?,?,?,?,?,?,?,?,?,?,?);
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_right_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_right_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_left_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_left_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_istm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableThyroid:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableThyroid' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.tiroida:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateThyroid(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableThyroid WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            UPDATE tableThyroid SET
                thyroid_right_dimens = ?, thyroid_right_volum = ?, thyroid_left_dimens = ?,
                thyroid_left_volum = ?, thyroid_istm = ?, thyroid_ecostructure = ?, thyroid_formations = ?,
                thyroid_ganglions = ?, concluzion = ?, recommendation = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_right_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_right_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_left_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_left_volum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_istm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_ecostructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_formations")));
        qry_sync.addBindValue(qry.value(rec.indexOf("thyroid_ganglions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableThyroid:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableThyroid' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea gl.tiroida:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertGestation0(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGestation0 WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableGestation0 (
                id_reportEcho, view_examination, antecedent, gestation_age,
                GS, GS_age, CRL, CRL_age, BCF, liquid_amniotic, miometer,
                cervix, ovary, concluzion, recommendation, lmp)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("GS")));
        qry_sync.addBindValue(qry.value(rec.indexOf("GS_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BCF")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liquid_amniotic")));
        qry_sync.addBindValue(qry.value(rec.indexOf("miometer")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("lmp")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableGestation0:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableGestation0' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina pana la 11 sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateGestation0(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGestation0 WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            UPDATE tableGestation0 SET
                view_examination = ?, antecedent = ?, gestation_age = ?,
                GS = ?, GS_age = ?, CRL = ?, CRL_age = ?, BCF = ?, liquid_amniotic = ?, miometer = ?,
                cervix = ?, ovary = ?, concluzion = ?, recommendation = ?, lmp = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("GS")));
        qry_sync.addBindValue(qry.value(rec.indexOf("GS_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BCF")));
        qry_sync.addBindValue(qry.value(rec.indexOf("liquid_amniotic")));
        qry_sync.addBindValue(qry.value(rec.indexOf("miometer")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("lmp")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation0:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation0' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina pana la 11 sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertGestation1(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGestation1 WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            INSERT INTO tableGestation1 (
                id_reportEcho, view_examination, antecedent, gestation_age,
                CRL, CRL_age, BPD, BPD_age, NT, NT_percent, BN, BN_percent,
                BCF, FL, FL_age, callote_cranium, plex_choroid, vertebral_column,
                stomach, bladder, diaphragm, abdominal_wall, location_placenta,
                sac_vitelin, amniotic_liquid, miometer, cervix, ovary, concluzion, recommendation, lmp)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, ?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("NT")));
        qry_sync.addBindValue(qry.value(rec.indexOf("NT_percent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BN")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BN_percent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BCF")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("callote_cranium")));
        qry_sync.addBindValue(qry.value(rec.indexOf("plex_choroid")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebral_column")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder")));
        qry_sync.addBindValue(qry.value(rec.indexOf("diaphragm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominal_wall")));
        qry_sync.addBindValue(qry.value(rec.indexOf("location_placenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("sac_vitelin")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amniotic_liquid")));
        qry_sync.addBindValue(qry.value(rec.indexOf("miometer")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("lmp")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (insertia) tabelei tableGestation1:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (insertia) cu succes a tabelei 'tableGestation1' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina 11-14 sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateGestation1(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(SELECT * FROM tableGestation1 WHERE id_reportEcho = ?)");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        qry_sync.prepare(R"(
            Update tableGestation1 SET
                view_examination = ?, antecedent = ?, gestation_age = ?,
                CRL = ?, CRL_age = ?, BPD = ?, BPD_age = ?, NT = ?, NT_percent = ?, BN = ?, BN_percent = ?,
                BCF = ?, FL = ?, FL_age = ?, callote_cranium = ?, plex_choroid = ?, vertebral_column = ?,
                stomach = ?, bladder = ?, diaphragm = ?, abdominal_wall = ?, location_placenta = ?,
                sac_vitelin = ?, amniotic_liquid = ?, miometer = ?, cervix = ?, ovary, concluzion = ?, recommendation = ?, lmp = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("CRL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("NT")));
        qry_sync.addBindValue(qry.value(rec.indexOf("NT_percent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BN")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BN_percent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BCF")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("callote_cranium")));
        qry_sync.addBindValue(qry.value(rec.indexOf("plex_choroid")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebral_column")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder")));
        qry_sync.addBindValue(qry.value(rec.indexOf("diaphragm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominal_wall")));
        qry_sync.addBindValue(qry.value(rec.indexOf("location_placenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("sac_vitelin")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amniotic_liquid")));
        qry_sync.addBindValue(qry.value(rec.indexOf("miometer")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ovary")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("lmp")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation1:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation1' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina 11-14 sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncInsertGestation2(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            ges.id AS ges_id,
            ges.id_reportEcho,
            ges.gestation_age,
            ges.trimestru,
            ges.dateMenstruation,
            ges.view_examination,
            ges.single_multiple_pregnancy,
            ges.single_multiple_pregnancy_description,
            ges.antecedent,
            ges.comment,
            ges.concluzion,
            ges.recommendation,
            bio.BPD,
            bio.BPD_age,
            bio.HC,
            bio.HC_age,
            bio.AC,
            bio.AC_age,
            bio.FL,
            bio.FL_age,
            bio.FetusCorresponds,
            cr.calloteCranium,
            cr.facialeProfile,
            cr.nasalBones,
            cr.nasalBones_dimens,
            cr.eyeball,
            cr.eyeball_desciption,
            cr.nasolabialTriangle,
            cr.nasolabialTriangle_description,
            cr.nasalFold,
            snc.hemispheres,
            snc.fissureSilvius,
            snc.corpCalos,
            snc.ventricularSystem,
            snc.ventricularSystem_description,
            snc.cavityPellucidSeptum,
            snc.choroidalPlex,
            snc.choroidalPlex_description,
            snc.cerebellum,
            snc.cerebellum_description,
            snc.vertebralColumn,
            snc.vertebralColumn_description,
            hrt.`position`,
            hrt.heartBeat,
            hrt.heartBeat_frequency,
            hrt.heartBeat_rhythm,
            hrt.pericordialCollections,
            hrt.planPatruCamere,
            hrt.planPatruCamere_description,
            hrt.ventricularEjectionPathLeft,
            hrt.ventricularEjectionPathLeft_description,
            hrt.ventricularEjectionPathRight,
            hrt.ventricularEjectionPathRight_description,
            hrt.intersectionVesselMagistral,
            hrt.intersectionVesselMagistral_description,
            hrt.planTreiVase,
            hrt.planTreiVase_description,
            hrt.archAorta,
            hrt.planBicav,
            th.pulmonaryAreas,
            th.pulmonaryAreas_description,
            th.pleuralCollections,
            th.diaphragm,
            abd.abdominalWall,
            abd.abdominalCollections,
            abd.stomach,
            abd.stomach_description,
            abd.abdominalOrgans,
            abd.cholecist,
            abd.cholecist_description,
            abd.intestine,
            abd.intestine_description,
            us.kidneys,
            us.kidneys_descriptions,
            us.ureter,
            us.ureter_descriptions,
            us.bladder,
            oth.externalGenitalOrgans,
            oth.externalGenitalOrgans_aspect,
            oth.extremities,
            oth.extremities_descriptions,
            oth.fetusMass,
            oth.placenta,
            oth.placentaLocalization,
            oth.placentaDegreeMaturation,
            oth.placentaDepth,
            oth.placentaStructure,
            oth.placentaStructure_descriptions,
            oth.umbilicalCordon,
            oth.umbilicalCordon_description,
            oth.insertionPlacenta,
            oth.amnioticIndex,
            oth.amnioticIndexAspect,
            oth.amnioticBedDepth,
            oth.cervix,
            oth.cervix_description,
            dop.ombilic_PI,
            dop.ombilic_RI,
            dop.ombilic_SD,
            dop.ombilic_flux,
            dop.cerebral_PI,
            dop.cerebral_RI,
            dop.cerebral_SD,
            dop.cerebral_flux,
            dop.uterRight_PI,
            dop.uterRight_RI,
            dop.uterRight_SD,
            dop.uterRight_flux,
            dop.uterLeft_PI,
            dop.uterLeft_RI,
            dop.uterLeft_SD,
            dop.uterLeft_flux,
            dop.ductVenos
        FROM
            tableGestation2 AS ges
        LEFT JOIN
            tableGestation2_biometry AS bio ON bio.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_cranium AS cr ON cr.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_SNC AS snc ON snc.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_heart AS hrt ON hrt.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_thorax AS th ON th.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_abdomen AS abd ON abd.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_urinarySystem AS us ON us.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_other AS oth ON oth.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_doppler AS dop ON dop.id_reportEcho = ges.id_reportEcho
        WHERE
            ges.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // ges2
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2 (
                id_reportEcho, gestation_age, trimestru, dateMenstruation, view_examination,
                single_multiple_pregnancy, single_multiple_pregnancy_description, antecedent,
                comment, concluzion, recommendation)
            VALUES(?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("trimestru")));
        qry_sync.addBindValue(qry.value(rec.indexOf("dateMenstruation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("single_multiple_pregnancy")));
        qry_sync.addBindValue(qry.value(rec.indexOf("single_multiple_pregnancy_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // biometria
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_biometry (
                id_reportEcho, BPD, BPD_age, HC, HC_age,
                AC, AC_age, FL, FL_age, FetusCorresponds)
            VALUES(?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("HC")));
        qry_sync.addBindValue(qry.value(rec.indexOf("HC_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("AC")));
        qry_sync.addBindValue(qry.value(rec.indexOf("AC_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FetusCorresponds")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_biometry:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_biometry' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // cranium
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_cranium (
                id_reportEcho, calloteCranium, facialeProfile, nasalBones, nasalBones_dimens,
                eyeball, eyeball_desciption, nasolabialTriangle, nasolabialTriangle_description, nasalFold)
            VALUES(?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("calloteCranium")));
        qry_sync.addBindValue(qry.value(rec.indexOf("facialeProfile")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalBones")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalBones_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("eyeball")));
        qry_sync.addBindValue(qry.value(rec.indexOf("eyeball_desciption")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasolabialTriangle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasolabialTriangle_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalFold")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_cranium:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_cranium' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // snc
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_SNC (
                id_reportEcho, hemispheres, fissureSilvius, corpCalos, ventricularSystem,
                ventricularSystem_description, cavityPellucidSeptum, choroidalPlex,
                choroidalPlex_description, cerebellum, cerebellum_description, vertebralColumn,
                vertebralColumn_description)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("hemispheres")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fissureSilvius")));
        qry_sync.addBindValue(qry.value(rec.indexOf("corpCalos")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularSystem")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularSystem_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cavityPellucidSeptum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("choroidalPlex")));
        qry_sync.addBindValue(qry.value(rec.indexOf("choroidalPlex_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebellum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebellum_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebralColumn")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebralColumn_description")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_SNC:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_SNC' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // heart
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_heart (
                id_reportEcho, `position`, heartBeat, heartBeat_frequency, heartBeat_rhythm,
                pericordialCollections, planPatruCamere, planPatruCamere_description,
                ventricularEjectionPathLeft, ventricularEjectionPathLeft_description,
                ventricularEjectionPathRight, ventricularEjectionPathRight_description,
                intersectionVesselMagistral, intersectionVesselMagistral_description,
                planTreiVase, planTreiVase_description, archAorta, planBicav)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("position")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat_frequency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat_rhythm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pericordialCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planPatruCamere")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planPatruCamere_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathLeft")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathLeft_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathRight")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathRight_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intersectionVesselMagistral")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intersectionVesselMagistral_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planTreiVase")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planTreiVase_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("archAorta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planBicav")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_heart:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_heart' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // thorax
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_thorax (
                id_reportEcho, pulmonaryAreas, pulmonaryAreas_description, pleuralCollections, diaphragm)
            VALUES(?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("pulmonaryAreas")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pulmonaryAreas_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pleuralCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("diaphragm")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_thorax:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_thorax' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // abdomen
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_abdomen (
                id_reportEcho, abdominalWall, abdominalCollections, stomach, stomach_description,
                abdominalOrgans, cholecist, cholecist_description, intestine, intestine_description)
            VALUES(?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalWall")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalOrgans")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intestine")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intestine_description")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_abdomen:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_abdomen' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // urinary system
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_urinarySystem (
                id_reportEcho, kidneys, kidneys_descriptions, ureter, ureter_descriptions, bladder)
            VALUES(?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("kidneys")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidneys_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ureter")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ureter_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_urinarySystem:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_urinarySystem' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // other
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_other (
                id_reportEcho, externalGenitalOrgans, externalGenitalOrgans_aspect, extremities, extremities_descriptions,
                fetusMass, placenta, placentaLocalization, placentaDegreeMaturation, placentaDepth, placentaStructure,
                placentaStructure_descriptions, umbilicalCordon, umbilicalCordon_description, insertionPlacenta, amnioticIndex,
                amnioticIndexAspect, amnioticBedDepth, cervix, cervix_description)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("externalGenitalOrgans")));
        qry_sync.addBindValue(qry.value(rec.indexOf("externalGenitalOrgans_aspect")));
        qry_sync.addBindValue(qry.value(rec.indexOf("extremities")));
        qry_sync.addBindValue(qry.value(rec.indexOf("extremities_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fetusMass")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaLocalization")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaDegreeMaturation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaDepth")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaStructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaStructure_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("umbilicalCordon")));
        qry_sync.addBindValue(qry.value(rec.indexOf("umbilicalCordon_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("insertionPlacenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticIndex")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticIndexAspect")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticBedDepth")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_description")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_other:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_other' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // doppler
        qry_sync.prepare(R"(
            INSERT INTO tableGestation2_doppler (
                id_reportEcho, ombilic_PI, ombilic_RI, ombilic_SD, ombilic_flux, cerebral_PI, cerebral_RI,
                cerebral_SD, cerebral_flux, uterRight_PI, uterRight_RI, uterRight_SD, uterRight_flux,
                uterLeft_PI, uterLeft_RI, uterLeft_SD, uterLeft_flux, ductVenos)
            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        qry_sync.addBindValue(m_datesSync.id_report);
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ductVenos")));
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei tableGestation2_doppler:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'tableGestation2_doppler' a documentului nr."
                             << m_datesSync.nr_report;
        }


    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina II si III sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncUpdateGestation2(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConn_local)
{
    QSqlQuery qry(dbConn_local);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT
            ges.id AS ges_id,
            ges.id_reportEcho,
            ges.gestation_age,
            ges.trimestru,
            ges.dateMenstruation,
            ges.view_examination,
            ges.single_multiple_pregnancy,
            ges.single_multiple_pregnancy_description,
            ges.antecedent,
            ges.comment,
            ges.concluzion,
            ges.recommendation,
            bio.BPD,
            bio.BPD_age,
            bio.HC,
            bio.HC_age,
            bio.AC,
            bio.AC_age,
            bio.FL,
            bio.FL_age,
            bio.FetusCorresponds,
            cr.calloteCranium,
            cr.facialeProfile,
            cr.nasalBones,
            cr.nasalBones_dimens,
            cr.eyeball,
            cr.eyeball_desciption,
            cr.nasolabialTriangle,
            cr.nasolabialTriangle_description,
            cr.nasalFold,
            snc.hemispheres,
            snc.fissureSilvius,
            snc.corpCalos,
            snc.ventricularSystem,
            snc.ventricularSystem_description,
            snc.cavityPellucidSeptum,
            snc.choroidalPlex,
            snc.choroidalPlex_description,
            snc.cerebellum,
            snc.cerebellum_description,
            snc.vertebralColumn,
            snc.vertebralColumn_description,
            hrt.`position`,
            hrt.heartBeat,
            hrt.heartBeat_frequency,
            hrt.heartBeat_rhythm,
            hrt.pericordialCollections,
            hrt.planPatruCamere,
            hrt.planPatruCamere_description,
            hrt.ventricularEjectionPathLeft,
            hrt.ventricularEjectionPathLeft_description,
            hrt.ventricularEjectionPathRight,
            hrt.ventricularEjectionPathRight_description,
            hrt.intersectionVesselMagistral,
            hrt.intersectionVesselMagistral_description,
            hrt.planTreiVase,
            hrt.planTreiVase_description,
            hrt.archAorta,
            hrt.planBicav,
            th.pulmonaryAreas,
            th.pulmonaryAreas_description,
            th.pleuralCollections,
            th.diaphragm,
            abd.abdominalWall,
            abd.abdominalCollections,
            abd.stomach,
            abd.stomach_description,
            abd.abdominalOrgans,
            abd.cholecist,
            abd.cholecist_description,
            abd.intestine,
            abd.intestine_description,
            us.kidneys,
            us.kidneys_descriptions,
            us.ureter,
            us.ureter_descriptions,
            us.bladder,
            oth.externalGenitalOrgans,
            oth.externalGenitalOrgans_aspect,
            oth.extremities,
            oth.extremities_descriptions,
            oth.fetusMass,
            oth.placenta,
            oth.placentaLocalization,
            oth.placentaDegreeMaturation,
            oth.placentaDepth,
            oth.placentaStructure,
            oth.placentaStructure_descriptions,
            oth.umbilicalCordon,
            oth.umbilicalCordon_description,
            oth.insertionPlacenta,
            oth.amnioticIndex,
            oth.amnioticIndexAspect,
            oth.amnioticBedDepth,
            oth.cervix,
            oth.cervix_description,
            dop.ombilic_PI,
            dop.ombilic_RI,
            dop.ombilic_SD,
            dop.ombilic_flux,
            dop.cerebral_PI,
            dop.cerebral_RI,
            dop.cerebral_SD,
            dop.cerebral_flux,
            dop.uterRight_PI,
            dop.uterRight_RI,
            dop.uterRight_SD,
            dop.uterRight_flux,
            dop.uterLeft_PI,
            dop.uterLeft_RI,
            dop.uterLeft_SD,
            dop.uterLeft_flux,
            dop.ductVenos
        FROM
            tableGestation2 AS ges
        LEFT JOIN
            tableGestation2_biometry AS bio ON bio.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_cranium AS cr ON cr.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_SNC AS snc ON snc.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_heart AS hrt ON hrt.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_thorax AS th ON th.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_abdomen AS abd ON abd.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_urinarySystem AS us ON us.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_other AS oth ON oth.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_doppler AS dop ON dop.id_reportEcho = ges.id_reportEcho
        WHERE
            ges.id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // ges2
        qry_sync.prepare(R"(
            UPDATE tableGestation2 SET
                gestation_age = ?, trimestru = ?, dateMenstruation = ?, view_examination = ?,
                single_multiple_pregnancy = ?, single_multiple_pregnancy_description = ?, antecedent = ?,
                comment = ?, concluzion = ?, recommendation = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("gestation_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("trimestru")));
        qry_sync.addBindValue(qry.value(rec.indexOf("dateMenstruation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("view_examination")));
        qry_sync.addBindValue(qry.value(rec.indexOf("single_multiple_pregnancy")));
        qry_sync.addBindValue(qry.value(rec.indexOf("single_multiple_pregnancy_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("antecedent")));
        qry_sync.addBindValue(qry.value(rec.indexOf("comment")));
        qry_sync.addBindValue(qry.value(rec.indexOf("concluzion")));
        qry_sync.addBindValue(qry.value(rec.indexOf("recommendation")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // biometria
        qry_sync.prepare(R"(
            UPDATE tableGestation2_biometry SET
                BPD = ?, BPD_age = ?, HC = ?, HC_age = ?,
                AC = ?, AC_age = ?, FL = ?, FL_age = ?, FetusCorresponds = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("BPD_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("HC")));
        qry_sync.addBindValue(qry.value(rec.indexOf("HC_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("AC")));
        qry_sync.addBindValue(qry.value(rec.indexOf("AC_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FL_age")));
        qry_sync.addBindValue(qry.value(rec.indexOf("FetusCorresponds")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_biometry:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_biometry' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // cranium
        qry_sync.prepare(R"(
            UPDATE tableGestation2_cranium SET
                calloteCranium = ?, facialeProfile = ?, nasalBones = ?, nasalBones_dimens = ?,
                eyeball = ?, eyeball_desciption = ?, nasolabialTriangle = ?, nasolabialTriangle_description = ?, nasalFold = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("calloteCranium")));
        qry_sync.addBindValue(qry.value(rec.indexOf("facialeProfile")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalBones")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalBones_dimens")));
        qry_sync.addBindValue(qry.value(rec.indexOf("eyeball")));
        qry_sync.addBindValue(qry.value(rec.indexOf("eyeball_desciption")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasolabialTriangle")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasolabialTriangle_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("nasalFold")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_cranium:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_cranium' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // snc
        qry_sync.prepare(R"(
            UPDATE tableGestation2_SNC SET
                hemispheres = ?, fissureSilvius = ?, corpCalos = ?, ventricularSystem = ?,
                ventricularSystem_description = ?, cavityPellucidSeptum = ?, choroidalPlex = ?,
                choroidalPlex_description = ?, cerebellum = ?, cerebellum_description = ?, vertebralColumn = ?,
                vertebralColumn_description = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("hemispheres")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fissureSilvius")));
        qry_sync.addBindValue(qry.value(rec.indexOf("corpCalos")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularSystem")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularSystem_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cavityPellucidSeptum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("choroidalPlex")));
        qry_sync.addBindValue(qry.value(rec.indexOf("choroidalPlex_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebellum")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebellum_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebralColumn")));
        qry_sync.addBindValue(qry.value(rec.indexOf("vertebralColumn_description")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_SNC:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_SNC' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // heart
        qry_sync.prepare(R"(
            UPDATE tableGestation2_heart SET
                `position` = ?, heartBeat = ?, heartBeat_frequency = ?, heartBeat_rhythm = ?,
                pericordialCollections = ?, planPatruCamere = ?, planPatruCamere_description = ?,
                ventricularEjectionPathLeft = ?, ventricularEjectionPathLeft_description = ?,
                ventricularEjectionPathRight = ?, ventricularEjectionPathRight_description = ?,
                intersectionVesselMagistral = ?, intersectionVesselMagistral_description = ?,
                planTreiVase = ?, planTreiVase_description = ?, archAorta = ?, planBicav = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("position")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat_frequency")));
        qry_sync.addBindValue(qry.value(rec.indexOf("heartBeat_rhythm")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pericordialCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planPatruCamere")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planPatruCamere_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathLeft")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathLeft_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathRight")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ventricularEjectionPathRight_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intersectionVesselMagistral")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intersectionVesselMagistral_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planTreiVase")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planTreiVase_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("archAorta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("planBicav")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_heart:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_heart' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // thorax
        qry_sync.prepare(R"(
            UPDATE tableGestation2_thorax SET
                pulmonaryAreas = ?, pulmonaryAreas_description = ?, pleuralCollections = ?, diaphragm = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("pulmonaryAreas")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pulmonaryAreas_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("pleuralCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("diaphragm")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_thorax:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_thorax' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // abdomen
        qry_sync.prepare(R"(
            UPDATE tableGestation2_abdomen SET
                abdominalWall = ?, abdominalCollections = ?, stomach = ?, stomach_description = ?,
                abdominalOrgans = ?, cholecist = ?, cholecist_description = ?, intestine = ?, intestine_description = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalWall")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalCollections")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach")));
        qry_sync.addBindValue(qry.value(rec.indexOf("stomach_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("abdominalOrgans")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cholecist_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intestine")));
        qry_sync.addBindValue(qry.value(rec.indexOf("intestine_description")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_abdomen:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_abdomen' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // urinary system
        qry_sync.prepare(R"(
            UPDATE tableGestation2_urinarySystem SET
                kidneys = ?, kidneys_descriptions = ?, ureter = ?, ureter_descriptions = ?, bladder = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("kidneys")));
        qry_sync.addBindValue(qry.value(rec.indexOf("kidneys_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ureter")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ureter_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("bladder")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_urinarySystem:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_urinarySystem' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // other
        qry_sync.prepare(R"(
            UPDATE tableGestation2_other SET
                externalGenitalOrgans = ?, externalGenitalOrgans_aspect = ?, extremities = ?, extremities_descriptions = ?,
                fetusMass = ?, placenta = ?, placentaLocalization = ?, placentaDegreeMaturation = ?, placentaDepth = ?, placentaStructure = ?,
                placentaStructure_descriptions = ?, umbilicalCordon = ?, umbilicalCordon_description = ?, insertionPlacenta = ?, amnioticIndex = ?,
                amnioticIndexAspect = ?, amnioticBedDepth = ?, cervix = ?, cervix_description = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("externalGenitalOrgans")));
        qry_sync.addBindValue(qry.value(rec.indexOf("externalGenitalOrgans_aspect")));
        qry_sync.addBindValue(qry.value(rec.indexOf("extremities")));
        qry_sync.addBindValue(qry.value(rec.indexOf("extremities_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("fetusMass")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaLocalization")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaDegreeMaturation")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaDepth")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaStructure")));
        qry_sync.addBindValue(qry.value(rec.indexOf("placentaStructure_descriptions")));
        qry_sync.addBindValue(qry.value(rec.indexOf("umbilicalCordon")));
        qry_sync.addBindValue(qry.value(rec.indexOf("umbilicalCordon_description")));
        qry_sync.addBindValue(qry.value(rec.indexOf("insertionPlacenta")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticIndex")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticIndexAspect")));
        qry_sync.addBindValue(qry.value(rec.indexOf("amnioticBedDepth")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cervix_description")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_other:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_other' a documentului nr."
                             << m_datesSync.nr_report;
        }

        // doppler
        qry_sync.prepare(R"(
            UPDATE tableGestation2_doppler SET
                ombilic_PI = ?, ombilic_RI = ?, ombilic_SD = ?, ombilic_flux = ?, cerebral_PI = ?, cerebral_RI = ?,
                cerebral_SD = ?, cerebral_flux = ?, uterRight_PI = ?, uterRight_RI = ?, uterRight_SD = ?, uterRight_flux = ?,
                uterLeft_PI = ?, uterLeft_RI = ?, uterLeft_SD = ?, uterLeft_flux = ?, ductVenos = ?
            WHERE id_reportEcho = ?
        )");
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ombilic_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("cerebral_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterRight_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_PI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_RI")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_SD")));
        qry_sync.addBindValue(qry.value(rec.indexOf("uterLeft_flux")));
        qry_sync.addBindValue(qry.value(rec.indexOf("ductVenos")));
        qry_sync.addBindValue(m_datesSync.id_report);
        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea (actualizarea) tabelei tableGestation2_doppler:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea (actualizarea) cu succes a tabelei 'tableGestation2_doppler' a documentului nr."
                             << m_datesSync.nr_report;
        }


    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea sarcina II si III sapt.:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }
}

void docSyncWorker::syncImages(QSqlDatabase &dbConn_sync, QSqlDatabase &dbConnImg)
{
    if (m_data.id_report <= 0 ||
        m_data.id_order <= 0 ||
        m_datesSync.id_report <= 0)
        return;

    bool existImage = false;
    QSqlQuery qry_image(dbConn_sync);
    qry_image.prepare(R"(
        SELECT COUNT(id) FROM imagesReports WHERE id_reportEcho = ?
    )");
    qry_image.addBindValue(m_datesSync.id_report);
    if (qry_image.exec() && qry_image.next())
        existImage = (qry_image.value(0).toInt() > 0);

    if (! dbConn_sync.transaction()) {
        qCritical(logCritical()) << "[SYNC] Nu s-a putut porni tranzactia:"
                                 << dbConn_sync.lastError().text();
        return;
    } else {
        m_datesSync.nr_order = QString::number(getLastNumberDoc("orderEcho", dbConn_sync) + 1); // determ.nr_order
        qInfo(logInfo()) << "[SYNC] Initierea tranzactiei pentru documentul (IMAGE) 'Comanda ecografica' nr."
                         << m_datesSync.nr_order;
    }

    QSqlQuery qry(dbConnImg);
    QSqlQuery qry_sync(dbConn_sync);

    qry.prepare(R"(
        SELECT * FROM imagesReports WHERE id_orderEcho = ? AND id_reportEcho = ?
    )");
    qry.addBindValue(m_data.id_order);
    qry.addBindValue(m_data.id_report);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // insert
        if (! existImage) {

            qry_sync.prepare(R"(
                INSERT INTO imagesReports (
                    id_reportEcho,
                    id_orderEcho,
                    id_patients,
                    image_1,
                    image_2,
                    image_3,
                    image_4,
                    image_5,
                    comment_1,
                    comment_2,
                    comment_3,
                    comment_4,
                    comment_5,
                    id_user)
                VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);
            )");
            qry_sync.addBindValue(m_datesSync.id_report);
            qry_sync.addBindValue(m_datesSync.id_order);
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(qry.value(rec.indexOf("image_1")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_2")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_3")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_4")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_5")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_1")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_2")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_3")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_4")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_5")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_user")));

        } else {

            qry_sync.prepare(R"(
                UPDATE imagesReports SET
                    id_orderEcho  = ?,
                    id_patients   = ?,
                    image_1       = ?,
                    image_2       = ?,
                    image_3       = ?,
                    image_4       = ?,
                    image_5       = ?,
                    comment_1     = ?,
                    comment_2     = ?,
                    comment_3     = ?,
                    comment_4     = ?,
                    comment_5     = ?,
                    id_user       = ?
                WHERE
                    id_reportEcho = ?
            )");
            qry_sync.addBindValue(m_datesSync.id_order);
            qry_sync.addBindValue(m_datesSync.id_patient);
            qry_sync.addBindValue(qry.value(rec.indexOf("image_1")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_2")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_3")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_4")));
            qry_sync.addBindValue(qry.value(rec.indexOf("image_5")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_1")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_2")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_3")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_4")));
            qry_sync.addBindValue(qry.value(rec.indexOf("comment_5")));
            qry_sync.addBindValue(qry.value(rec.indexOf("id_user")));
            qry_sync.addBindValue(m_datesSync.id_report);

        }

        if (! qry_sync.exec()) {
            qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea tabelei imagesReports:"
                                     << qry_sync.lastError().text();
            dbConn_sync.rollback();
            return;
        } else {
            qInfo(logInfo()) << "[SYNC] Sincronizarea cu succes a tabelei 'imagesReports' a documentului nr."
                             << m_datesSync.nr_report;
        }

    } else {
        qCritical(logCritical()) << "[SYNC] Eroare la sincronizarea a imaginelor:"
                                 << qry.lastError().text();
        dbConn_sync.rollback();
    }

    if (! dbConn_sync.commit()) {
        qCritical(logCritical()) << "[SYNC] Commit eșuat:"
                                 << dbConn_sync.lastError().text();
        dbConn_sync.rollback();
        return;
    } else {
        qInfo(logInfo()) << "[SYNC] Tranzacție finalizată cu succes pentru documentul (IMAGE) 'Comanda ecografica' nr."
                         << m_datesSync.nr_order;
    }
}
