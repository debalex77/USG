#include "patientsaverworker.h"

#include <limits>

PatientSaverWorker::PatientSaverWorker(DatabaseProvider *provider,
                                       const PatientDataStructure &patientData,
                                       QObject *parent)
    : QObject{parent}
    , m_provider(provider)
    , m_patientData(patientData)
{}

void PatientSaverWorker::processInsert()
{
    if (!m_provider) {
        emit finishedError({QStringLiteral("[THREAD] DatabaseProvider este nullptr.")});
        return;
    }

    const QString connName = QStringLiteral("connection_%1")
    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    bool patientExist = false;
    QStringList err;

    {
        QSqlDatabase dbConn = m_provider->getDatabaseThread(connName, globals().thisMySQL);

        if (!dbConn.isOpen() && !dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
            .arg(this->metaObject()->className())
                << dbConn.lastError().text();

            err.append(dbConn.lastError().text());
        } else {
            bool existsQueryOk = false;
            QString existsQueryError;
            const bool patientExists = patientExistInDB(dbConn,
                                                        &existsQueryOk,
                                                        &existsQueryError);

            if (!existsQueryOk) {
                err.append(QStringLiteral("[THREAD] Verificarea existenței pacientului a eșuat: %1")
                               .arg(existsQueryError));
            } else if (patientExists) {
                qInfo(logInfo()) << "[THREAD] Pacientul "
                                        + m_patientData.name + " " + m_patientData.firstName
                                        + " este determinat in bd. Inserarea nu a fost efectuata.";
                patientExist = true;
            } else {
                patientDataInsertInDB(dbConn, err);
            }
        }
        dbConn.close();
    }

    m_provider->removeDatabaseThread(connName);

    if (!err.isEmpty()) {
        emit finishedError(err);
        return;
    }

    if (patientExist) {
        emit finishedPatientExistInBD(m_patientData);
        return;
    }

    emit finished(m_patientData);
}

void PatientSaverWorker::processUpdate()
{
    if (!m_provider) {
        emit finishedError({QStringLiteral("[THREAD] DatabaseProvider este nullptr.")});
        return;
    }

    const QString connName = QStringLiteral("connection_%1")
    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    QStringList err;

    {
        QSqlDatabase dbConn = m_provider->getDatabaseThread(connName, globals().thisMySQL);

        if (!dbConn.isOpen() && !dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
            .arg(this->metaObject()->className())
                << dbConn.lastError().text();

            err.append(dbConn.lastError().text());
        } else {
            patientDataUpdate(dbConn, err);
        }

        dbConn.close();
    }

    m_provider->removeDatabaseThread(connName);

    if (!err.isEmpty()) {
        emit finishedError(err);
        return;
    }

    emit finished(m_patientData);
}

bool PatientSaverWorker::loadPatientUuidById(QSqlDatabase &dbConn, QUuid &uuidOut)
{
    if (m_patientData.id <= 0)
        return false;

    QSqlQuery q(dbConn);
    q.prepare(R"(
        SELECT uuid
        FROM patients
        WHERE id = :id
        LIMIT 1
    )");
    q.bindValue(":id", m_patientData.id);

    if (!q.exec())
        return false;

    if (!q.next())
        return false;

    const QByteArray uuidBytes = q.value(0).toByteArray();
    if (uuidBytes.size() != 16)
        return false;

    const QUuid uuid = QUuid::fromRfc4122(uuidBytes);
    if (uuid.isNull())
        return false;

    uuidOut = uuid;
    return true;
}

bool PatientSaverWorker::patientExistInDB(QSqlDatabase &dbConn,
                                          bool *queryOk,
                                          QString *queryError)
{
    if (queryOk)
        *queryOk = false;
    if (queryError)
        queryError->clear();

    QSqlQuery q(dbConn);

    if (! m_patientData.idnp.isEmpty()) {  // 1. daca este idnp - (principal)

        q.prepare(R"(
            SELECT 1 FROM patients
            WHERE idnp = :idnp LIMIT 1
        )");
        q.bindValue(":idnp", m_patientData.idnp);

    } else {  // 2. daca nu este idnp (cautarea dupa: nume, prenume, data nasterii)

        q.prepare(R"(
            SELECT 1 FROM patients
            WHERE last_name = :name AND first_name = :firstName AND birthday = :birthday LIMIT 1
        )");
        q.bindValue(":name",     m_patientData.name);
        q.bindValue(":firstName",    m_patientData.firstName);
        q.bindValue(":birthday", m_patientData.birthday.toString("yyyy-MM-dd"));

    }

    if (!q.exec()) {
        if (queryError)
            *queryError = q.lastError().text();
        qCritical(logCritical()).noquote()
        << QStringLiteral("[THREAD %1] Verificarea pacientului a eșuat: %2")
               .arg(metaObject()->className(), q.lastError().text());
        return false;
    }

    if (queryOk)
        *queryOk = true;

    return q.next();
}

bool PatientSaverWorker::patientDataInsertInDB(QSqlDatabase &dbConn, QStringList &err)
{
    if (!dbConn.transaction()) {
        err.append(QStringLiteral("[THREAD] Nu s-a putut porni tranzacția pentru inserarea pacientului: %1")
                       .arg(dbConn.lastError().text()));
        return false;
    }

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        INSERT INTO patients (
            deletion_mark,
            idnp,
            last_name,
            first_name,
            middle_name,
            medical_policy,
            birthday,
            address,
            telephone,
            email,
            comment,
            uuid)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?)
    )");
    qry.addBindValue(m_patientData.deletionMark);
    qry.addBindValue((m_patientData.idnp.isEmpty()) ? QVariant() : m_patientData.idnp);
    qry.addBindValue(m_patientData.name);
    qry.addBindValue(m_patientData.firstName);
    qry.addBindValue(m_patientData.middleName.isEmpty() ? QVariant() : m_patientData.middleName);
    qry.addBindValue((m_patientData.medicalPolicy.isEmpty()) ? QVariant() : m_patientData.medicalPolicy);
    qry.addBindValue(m_patientData.birthday.toString("yyyy-MM-dd"));
    qry.addBindValue((m_patientData.address.isEmpty()) ? QVariant() : m_patientData.address);
    qry.addBindValue((m_patientData.phone.isEmpty()) ? QVariant() : m_patientData.phone);
    qry.addBindValue((m_patientData.email.isEmpty()) ? QVariant() : m_patientData.email);
    qry.addBindValue((m_patientData.comment.isEmpty()) ? QVariant() : m_patientData.comment);

    QUuid uuid = QUuid::createUuid();
    qry.addBindValue(uuid.toRfc4122());

    if (! qry.exec()) {
        dbConn.rollback();
        err << "[THREAD] Eroare de inserare datelor pacientului - "
            << m_patientData.name + " " + m_patientData.firstName + " :"
            << qry.lastError().text();
        qCritical(logCritical()) << err;
        return false;
    } else {
        // QMYSQL nu garantează păstrarea lastInsertId() după COMMIT.
        // Capturăm ID-ul cât timp rezultatul INSERT-ului este încă activ.
        const qint64 insertedPatientId = qry.lastInsertId().toLongLong();
        if (insertedPatientId <= 0 || insertedPatientId > std::numeric_limits<int>::max()) {
            dbConn.rollback();
            err.append(QStringLiteral("[THREAD] ID invalid returnat după inserarea pacientului: %1")
                           .arg(insertedPatientId));
            return false;
        }

        if (dbConn.commit() == false) {
            dbConn.rollback();
            qCritical() << QStringLiteral("[THREAD %1] Commit-ul pentru inserarea în tabela 'patients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn.lastError().text());
            err.append(dbConn.lastError().text());
            return false;
        } else {
            m_patientData.id = static_cast<int>(insertedPatientId);
            m_patientData.uuid = uuid;

            qInfo(logInfo()) << QStringLiteral("[THREAD] Inserarea cu succes a datelor pacientului - %1")
            .arg(m_patientData.name + " " + m_patientData.firstName);
            return true;
        }

    }
}

bool PatientSaverWorker::patientDataUpdate(QSqlDatabase &dbConn, QStringList &err)
{
    // setam UUID
    if (m_patientData.uuid.isNull()) {
        QUuid loadedUuid;
        if (!loadPatientUuidById(dbConn, loadedUuid)) {
            err << QStringLiteral("[THREAD] Nu s-a putut determina UUID-ul pacientului pentru id=%1")
            .arg(m_patientData.id);
            return false;
        }
        m_patientData.uuid = loadedUuid;
    }

    if (!dbConn.transaction()) {
        err.append(QStringLiteral("[THREAD] Nu s-a putut porni tranzacția pentru actualizarea pacientului: %1")
                       .arg(dbConn.lastError().text()));
        return false;
    }

    QSqlQuery qry(dbConn);
    qry.prepare(R"(
        UPDATE patients SET
            deletion_mark  = ?,
            idnp          = ?,
            last_name          = ?,
            first_name         = ?,
            middle_name         = ?,
            medical_policy = ?,
            birthday      = ?,
            address       = ?,
            telephone     = ?,
            email         = ?,
            comment       = ?
        WHERE
            id = ?
    )");
    qry.addBindValue(m_patientData.deletionMark);
    qry.addBindValue(m_patientData.idnp.isEmpty() ? QVariant() : m_patientData.idnp);
    qry.addBindValue(m_patientData.name);
    qry.addBindValue(m_patientData.firstName);
    qry.addBindValue(m_patientData.middleName.isEmpty() ? QVariant() : m_patientData.middleName);
    qry.addBindValue(m_patientData.medicalPolicy.isEmpty() ? QVariant() : m_patientData.medicalPolicy);
    qry.addBindValue(m_patientData.birthday.toString("yyyy-MM-dd"));
    qry.addBindValue(m_patientData.address.isEmpty() ? QVariant() : m_patientData.address);
    qry.addBindValue(m_patientData.phone.isEmpty() ? QVariant() : m_patientData.phone);
    qry.addBindValue(m_patientData.email.isEmpty() ? QVariant() : m_patientData.email);
    qry.addBindValue(m_patientData.comment.isEmpty() ? QVariant() : m_patientData.comment);
    qry.addBindValue(m_patientData.id);
    if (! qry.exec()) {
        dbConn.rollback();
        err << "[THREAD] Eroare de actualizare a datelor pacientului - "
            << m_patientData.name + " " + m_patientData.firstName + " :"
            << qry.lastError().text();
        qCritical(logCritical()) << err;
        return false;
    } else {
        if (dbConn.commit() == false) {
            dbConn.rollback();
            qCritical() << QStringLiteral("[THREAD %1] Commit-ul pentru actualizarea datelor in tabela 'patients' a eșuat: %2")
                               .arg(this->metaObject()->className(),
                                    dbConn.lastError().text());
            err.append(dbConn.lastError().text());
            return false;
        } else {
            qInfo(logInfo()) << QStringLiteral("[THREAD] Actualizarea cu succes a datelor pacientului - %1")
            .arg(m_patientData.name + " " + m_patientData.firstName);
        }
    }
    return true;
}
