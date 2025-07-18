#include "patientdatasaverworker.h"

PatientDataSaverWorker::PatientDataSaverWorker(DatabaseProvider *provider,
                                               DatesCatPatient &data,
                                               QObject *parent)
    : QObject{parent}, m_db(provider), m_data(data)
{}

// **********************************************************************************
// --- functiile principale de inserare si actualizare

void PatientDataSaverWorker::processInsert()
{
    // 1. definim denumirea conexiunei
    const QString connName = QStringLiteral("connection_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    // 2. anuntam variabile locale
    bool patientExist = false; // daca exista pacient
    QStringList err;           // lista cu erori

    QVariantMap map;
    map["fullName"]  = m_data.name + " " + m_data.fname;
    map["birthday"]  = m_data.birthday;
    map["idnp"]      = m_data.idnp;
    map["id"]        = m_data.id;

    { // Conexiunea trăieşte DOAR în acest bloc

        // 3. conectarea la BD in thread
        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                               .arg(this->metaObject()->className())
                        << dbConn.lastError().text();
            emit finished(map);
            return;
        }

        // 4. verificam daca exista pacientul in bd, daca nu exista inseram datele pacientului
        if (patientExistInDB(dbConn)) {
            qInfo(logInfo()) << "[THREAD] Pacientul " + m_data.name + " " + m_data.fname + " este determinat in bd."
                             << "Inserarea nu a fost efectuata.";
            patientExist = true;
        } else {
            patientDataInsertInDB(dbConn, err);
        }

        // 5. inchidem conexiunea cu bd
        dbConn.close();

    } // <- destructor QSqlDatabase

    // 6. distrugem conexiunea cu bd
    m_db->removeDatabaseThread(connName);

    // 7. emitem signal in dependenta de conditii
    if (patientExist) {
        emit finishedPatientExistInBD(map);
    } else {
        if (err.isEmpty())
            emit finished(map);
        else
            emit finishedError(err);
    }
}

void PatientDataSaverWorker::processUpdate()
{
    // 1. definim denumirea conexiunei
    const QString connName = QStringLiteral("connection_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    // 2. anuntam variabile locale
    QStringList err; // lista cu erori

    QVariantMap map;
    map["fullName"]  = m_data.name + " " + m_data.fname;
    map["birthday"]  = m_data.birthday;
    map["idnp"]      = m_data.idnp;
    map["id"]        = m_data.id;

    { // Conexiunea trăieşte DOAR în acest bloc

        // 3. conectarea la BD in thread
        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
            .arg(this->metaObject()->className())
                << dbConn.lastError().text();
            emit finished(map);
            return;
        }

        // 4. actualizam date
        patientDataUpdate(dbConn, err);

        // 5. inchidem conexiunea cu bd
        dbConn.close();
    } // <- destructor QSqlDatabase

    // 6. distrugem conexiunea cu bd
    m_db->removeDatabaseThread(connName);

    // 7. emitem signal in dependenta de conditii
    if (err.isEmpty())
        emit finished(map);
    else
        emit finishedError(err);
}

// **********************************************************************************
// --- procesare de inserare si actualizare

bool PatientDataSaverWorker::patientExistInDB(QSqlDatabase &dbConn)
{
    QSqlQuery qry(dbConn);

    if (! m_data.idnp.isEmpty()) {  // 1. daca este idnp - (principal)

        qry.prepare(R"(
            SELECT 1 FROM pacients
            WHERE IDNP = ? LIMIT 1
        )");
        qry.addBindValue(m_data.idnp);

    } else {  // 2. daca nu este idnp (cautarea dupa: nume, prenume, data nasterii)

        QSqlQuery qry_no_idnp(dbConn);
        qry_no_idnp.prepare(R"(
            SELECT 1 FROM pacients
            WHERE name = ? AND fName = ? AND birthday = ? LIMIT 1
        )");
        qry_no_idnp.addBindValue(m_data.name);
        qry_no_idnp.addBindValue(m_data.fname);
        qry_no_idnp.addBindValue(m_data.birthday);

    }

    return qry.exec() && qry.next();
}

bool PatientDataSaverWorker::patientDataInsertInDB(QSqlDatabase &dbConn, QStringList &err)
{
    dbConn.transaction();

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        INSERT INTO pacients (
            id,
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
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?)
    )");
    qry.addBindValue(m_data.id);
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
        dbConn.rollback();
        err << "[THREAD] Eroare de inserare datelor pacientului - "
            << m_data.name + " " + m_data.fname + " :"
            << qry.lastError().text();
        qCritical(logCritical()) << err;
        return false;
    } else {
        if (dbConn.commit() == false) {
            dbConn.rollback();
            qCritical() << QStringLiteral("[THREAD %1] Commit-ul pentru inserarea în tabela 'pacients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn.lastError().text());
            return false;
        } else {
            qInfo(logInfo()) << QStringLiteral("[THREAD] Inserarea cu succes a datelor pacientului - %1")
                                    .arg(m_data.name + " " + m_data.fname);
            return true;
        }

    }
}

void PatientDataSaverWorker::patientDataUpdate(QSqlDatabase &dbConn, QStringList &err)
{
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
    qry.addBindValue(m_data.id);
    if (! qry.exec()) {
        dbConn.rollback();
        err << "[THREAD] Eroare de actualizare a datelor pacientului - "
            << m_data.name + " " + m_data.fname + " :"
            << qry.lastError().text();
        qCritical(logCritical()) << err;
    } else {
        if (dbConn.commit() == false) {
            dbConn.rollback();
            qCritical() << QStringLiteral("[THREAD %1] Commit-ul pentru actualizarea datelor in tabela 'pacients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn.lastError().text());
        } else {
            qInfo(logInfo()) << QStringLiteral("[THREAD] Actualizarea cu succes a datelor pacientului - %1")
                                    .arg(m_data.name + " " + m_data.fname);
        }
    }
}
