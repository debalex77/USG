#include "dataconstantsworker.h"

DataConstantsWorker::DataConstantsWorker(DatabaseProvider *provider, GeneralData &data, QObject *parent)
    : QObject{parent}, m_data(data), m_db(provider)
{}

void DataConstantsWorker::process()
{
    const QString connName = QStringLiteral("connection_%1")
                                .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    { // Conexiunea trăieşte DOAR în acest bloc

        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                               .arg(this->metaObject()->className())
                        << dbConn.lastError().text();
            emit finished();
            return;
        }

        // constante
        QSqlQuery qry(dbConn);
        qry.prepare("SELECT * FROM constants WHERE id_users = ?");
        qry.addBindValue(m_data.id_user);
        if (! qry.exec()) {
            qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(constants):")
                                            .arg(this->metaObject()->className())
                                     << qry.lastError().text();
        } else {
            while (qry.next()) {
                QSqlRecord rec = qry.record();
                globals().c_id_organizations = qry.value(rec.indexOf("id_organizations")).toInt();
                globals().c_id_doctor        = qry.value(rec.indexOf("id_doctors")).toInt();
                globals().c_id_nurse         = qry.value(rec.indexOf("id_nurses")).toInt();
                globals().c_brandUSG         = qry.value(rec.indexOf("brandUSG")).toString();
                globals().c_logo_byteArray   = QByteArray::fromBase64(qry.value(rec.indexOf("logo")).toString().toUtf8());
                qInfo(logInfo()) << "[THREAD] Actualizate variabile globale din tabela 'constants'";
            }
        }

        // datele organizatiei implicite
        qry.prepare(R"(
            SELECT
                name,address,telephone,email,stamp
            FROM
                organizations
            WHERE
                id = ?
        )");
        if (m_data.id_organization == -1)
            qry.addBindValue(globals().c_id_organizations);
        else
            qry.addBindValue(m_data.id_organization);
        if (! qry.exec()) {
            qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(organizations):")
                                            .arg(this->metaObject()->className())
                                     << qry.lastError().text();
        } else {
            while (qry.next()) {
                QSqlRecord rec = qry.record();
                globals().main_name_organization   = qry.value(rec.indexOf("name")).toString();
                globals().main_addres_organization = qry.value(rec.indexOf("address")).toString();
                globals().main_phone_organization  = qry.value(rec.indexOf("telephone")).toString();
                globals().main_email_organization  = qry.value(rec.indexOf("email")).toString();
                globals().main_stamp_organization  = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());
                qInfo(logInfo()) << "[THREAD] Actualizate variabile globale cu date a organizatiei implicite";
            }
        }

        // datele doctorului
        qry.prepare(R"(
            SELECT
                doctors.id,
                doctors.signature,
                doctors.stamp,
                fullNameDoctors.name AS fullName,
                fullNameDoctors.nameAbbreviated
            FROM
                doctors
            INNER JOIN
                fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
            WHERE
                doctors.deletionMark = 0 AND
                doctors.id = ?
        )");
        if (m_data.id_doctor == -1)
            qry.addBindValue(globals().c_id_doctor);
        else
            qry.addBindValue(m_data.id_doctor);
        if (! qry.exec()) {
            qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(doctors):")
                                            .arg(this->metaObject()->className())
                                     << qry.lastError().text();
        } else {
            while (qry.next()) {
                QSqlRecord rec = qry.record();
                globals().main_name_doctor           = qry.value(rec.indexOf("fullName")).toString();
                globals().main_name_abbreviat_doctor = qry.value(rec.indexOf("nameAbbreviated")).toString();
                globals().stamp_main_doctor          = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());
                globals().signature_main_doctor      = QByteArray::fromBase64(qry.value(rec.indexOf("signature")).toString().toUtf8());
                qInfo(logInfo()) << "[THREAD] Actualizate variabile globale cu date doctorului implicit";
            }
        }

        // datele conectarii la serverul pu syncronizare
        qry.prepare(R"(
            SELECT
                cloudServer.*,
                users.hash AS hashUser
            FROM
                cloudServer
            INNER JOIN
                users ON cloudServer.id_users = users.id
            WHERE
                cloudServer.id_organizations = ? AND
                cloudServer.id_users = ?
            )");
        qry.addBindValue(globals().c_id_organizations);
        qry.addBindValue(m_data.id_user);
        if (! qry.exec()) {
            qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(cloudServer):")
                                            .arg(this->metaObject()->className())
                                     << qry.lastError().text();
        } else {
            while (qry.next()) {
                QSqlRecord rec = qry.record();
                globals().cloud_host          = qry.value(rec.indexOf("hostName")).toString();
                globals().cloud_nameBase      = qry.value(rec.indexOf("databaseName")).toString();
                globals().cloud_port          = qry.value(rec.indexOf("port")).toString();
                globals().cloud_optionConnect = qry.value(rec.indexOf("connectionOption")).toString();
                globals().cloud_user          = qry.value(rec.indexOf("username")).toString();

                QByteArray hash_user         = QByteArray::fromHex(qry.value(rec.indexOf("hashUser")).toString().toUtf8());
                QByteArray iv                = QByteArray::fromBase64(qry.value(rec.indexOf("iv")).toString().toUtf8());
                QByteArray encryptedPassword = QByteArray::fromBase64(qry.value(rec.indexOf("password")).toString().toUtf8());
                QByteArray decryptedPassword = crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
                globals().cloud_passwd = decryptedPassword;
                qInfo(logInfo()) << "[THREAD] Actualizate variabile globale cu date pentru sincronizare cu serverul.";
            }
        }

        dbConn.close();

    } // <- destructor QSqlDatabase

    m_db->removeDatabaseThread(connName);

    emit finished();
}
