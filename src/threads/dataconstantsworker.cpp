#include "dataconstantsworker.h"

DataConstantsWorker::DataConstantsWorker(DatabaseProvider *provider, GeneralData &data, QObject *parent)
    : QObject{parent}, m_data(data), m_db(provider)
{}

void DataConstantsWorker::process()
{
    const QString connName = QStringLiteral("connection_%1")
                                .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    bool success = true;
    { // Conexiunea trăieşte DOAR în acest bloc

        QSqlDatabase dbConn = m_db->getDatabaseThread(connName, m_data.thisMySQL);
        if (! dbConn.isOpen() &&
            ! dbConn.open()) {
            qCritical() << QStringLiteral("[THREAD %1] Nu pot deschide conexiunea DB:")
                               .arg(this->metaObject()->className())
                        << dbConn.lastError().text();
            success = false;
        } else {
            // constante
            QSqlQuery qry(dbConn);
            qry.prepare("SELECT * FROM constants WHERE id_users = ?");
            qry.addBindValue(m_data.id_user);
            if (! qry.exec()) {
                success = false;
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
                success = false;
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
                success = false;
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
            globals().cloud_configured = false;
            globals().cloud_srv_exist = false;
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
                success = false;
                qCritical(logCritical()) << QStringLiteral("[THREAD %1] Eroare exec SELECT(cloudServer):")
                                                .arg(this->metaObject()->className())
                                         << qry.lastError().text();
            } else {
                while (qry.next()) {
                    globals().cloud_configured = true;
                    QSqlRecord rec = qry.record();
                    globals().cloud_host          = qry.value(rec.indexOf("hostName")).toString();
                    globals().cloud_nameBase      = qry.value(rec.indexOf("databaseName")).toString();
                    globals().cloud_port          = qry.value(rec.indexOf("port")).toString();
                    globals().cloud_optionConnect = qry.value(rec.indexOf("connectionOption")).toString();
                    globals().cloud_user          = qry.value(rec.indexOf("username")).toString();

                    const QByteArray payload = CryptoManager::fromBase64(
                        qry.value(rec.indexOf("password")).toString());
                    CryptoManager::EncryptedData encrypted;
                    if (payload.size() > 16) {
                        encrypted.cipherText = payload.first(payload.size() - 16);
                        encrypted.tag = payload.last(16);
                        encrypted.iv = CryptoManager::fromBase64(
                            qry.value(rec.indexOf("iv")).toString());

                        const QByteArray userHash = QByteArray::fromHex(
                            qry.value(rec.indexOf("hashUser")).toString().toUtf8());
                        const int cloudOrganizationId =
                            qry.value(rec.indexOf("id_organizations")).toInt();
                        const QByteArray realKey = CryptoManager::deriveCloudKey(
                            userHash, cloudOrganizationId);
                        bool decrypted = false;
                        if (realKey.size() == 32) {
                            globals().cloud_passwd = CryptoManager::decryptText(
                                encrypted, realKey, &decrypted);
                        }
                        globals().cloud_srv_exist = decrypted;
                        if (!decrypted) {
                            qWarning(logWarning())
                                << "[THREAD] Parola cloud nu a putut fi decriptată.";
                        } else {
                            qInfo(logInfo())
                                << "[THREAD] Actualizate variabile globale pentru sincronizare cu serverul.";
                        }
                    } else {
                        globals().cloud_passwd.clear();
                        globals().cloud_srv_exist = false;
                        qWarning(logWarning())
                            << "[THREAD] Configurația cloud nu conține o parolă criptată validă.";
                    }
                }
            }

            dbConn.close();
        }
    } // <- destructor QSqlDatabase

    m_db->removeDatabaseThread(connName);

    emit finished(success);
}
