/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "handlerfunctionthread.h"

#include <QThread>

HandlerFunctionThread::HandlerFunctionThread(QObject *parent)
    : QObject{parent}
{
    db = new DataBase(this);
}

HandlerFunctionThread::~HandlerFunctionThread()
{
    delete db;
}

// **********************************************************************************
// --- functiile pentru setarea variabilelor locale

void HandlerFunctionThread::setRequiredVariabile(const int id_mainUser,
                                                 const int id_mainOrganization,
                                                 const int id_mainDoctor, const bool thisMySQL)
{
    m_id_user = id_mainUser;
    m_id_mainDoctor = id_mainDoctor;
    m_id_mainOrganization = id_mainOrganization;
    m_thisMySQL = thisMySQL;
}

void HandlerFunctionThread::setRequiredVariableForCatPatient(const int patient_id,
                                                             const QString patient_name,
                                                             const QString patient_fname,
                                                             const QString patient_idnp,
                                                             const QString patient_medicalPolicy,
                                                             const QDate patient_birthday,
                                                             const QString patient_address,
                                                             const QString patient_email,
                                                             const QString patient_telephone,
                                                             const QString patient_comment,
                                                             const bool thisMySQL)
{
    cat_patient_id = patient_id;
    cat_patient_name = patient_name;
    cat_patient_fname = patient_fname;
    cat_patient_idnp = patient_idnp;
    cat_patient_medicalPolicy = patient_medicalPolicy;
    cat_patient_birthday = patient_birthday;
    cat_patient_address = patient_address;
    cat_patient_email = patient_email;
    cat_patient_telephone = patient_telephone;
    cat_patient_comment = patient_comment;
    m_thisMySQL = thisMySQL;
}

void HandlerFunctionThread::setRequiredVariableForExportDocuments(const bool thisMySQL,
                                                                  const int id_order,
                                                                  const int id_report,
                                                                  const QString unitMeasure,
                                                                  const QByteArray logo_byteArray,
                                                                  const QByteArray stamp_main_organization,
                                                                  const QByteArray stamp_main_doctor,
                                                                  const QByteArray signature_main_doctor,
                                                                  const QString pathTemplatesDocs,
                                                                  const QString filePDF)
{
    m_thisMySQL = thisMySQL;
    m_id_order = id_order;
    m_id_report = id_report;
    m_unitMeasure = unitMeasure;
    m_logo_byteArray = logo_byteArray;
    m_stamp_main_organization = stamp_main_organization;
    m_stamp_main_doctor = stamp_main_doctor;
    m_signature_main_doctor = signature_main_doctor;
    m_pathTemplatesDocs = pathTemplatesDocs;
    m_filePDF = filePDF;
}

void HandlerFunctionThread::setTagsDocumentReport(const bool organs_internal,
                                                  const bool urinary_system,
                                                  const bool prostate,
                                                  const bool gynecology,
                                                  const bool breast,
                                                  const bool thyroide,
                                                  const bool gestation0,
                                                  const bool gestation1,
                                                  const bool gestation2,
                                                  const bool gestation3)
{
    m_organs_internal = organs_internal;
    m_urinary_system = urinary_system;
    m_prostate = prostate;
    m_gynecology = gynecology;
    m_breast = breast;
    m_thyroide = thyroide;
    m_gestation0 = gestation0;
    m_gestation1 = gestation1;
    m_gestation2 = gestation2;
    m_gestation3 = gestation3;
}

// **********************************************************************************
// --- procesarea SLOT-lor publice

void HandlerFunctionThread::setDataConstants()
{
    const QString threadConnectionName = QString("connection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    {
        QSqlDatabase dbConnection = db->getDatabaseThread(threadConnectionName, m_thisMySQL);
        {
            QSqlQuery qry(dbConnection);
            qry.prepare(R"(SELECT * FROM constants WHERE id_users = ?)");
            qry.addBindValue(m_id_user);
            if (! qry.exec()) {
                qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    globals().c_id_organizations = qry.value(rec.indexOf("id_organizations")).toInt();
                    globals().c_id_doctor = qry.value(rec.indexOf("id_doctors")).toInt();
                    globals().c_id_nurse = qry.value(rec.indexOf("id_nurses")).toInt();
                    globals().c_brandUSG = qry.value(rec.indexOf("brandUSG")).toString();
                    globals().c_logo_byteArray = QByteArray::fromBase64(qry.value(rec.indexOf("logo")).toString().toUtf8());
                }
            }

            qry.prepare(R"(
                SELECT
                    name,address,telephone,email,stamp
                FROM
                    organizations WHERE id = ?
            )");
            if (m_id_mainOrganization == -1)
                qry.addBindValue(globals().c_id_organizations);
            else
                qry.addBindValue(m_id_mainOrganization);
            if (! qry.exec()) {
                qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    globals().main_name_organization = qry.value(rec.indexOf("name")).toString();
                    globals().main_addres_organization = qry.value(rec.indexOf("address")).toString();
                    globals().main_phone_organization = qry.value(rec.indexOf("telephone")).toString();
                    globals().main_email_organization = qry.value(rec.indexOf("email")).toString();
                    globals().main_stamp_organization = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());
                }
            }

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
            if (m_id_mainDoctor == -1)
                qry.addBindValue(globals().c_id_doctor);
            else
                qry.addBindValue(m_id_mainDoctor);
            if (! qry.exec()) {
                qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
            } else {
                while (qry.next()) {
                    QSqlRecord rec = qry.record();
                    globals().main_name_doctor           = qry.value(rec.indexOf("fullName")).toString();
                    globals().main_name_abbreviat_doctor = qry.value(rec.indexOf("nameAbbreviated")).toString();
                    globals().stamp_main_doctor     = QByteArray::fromBase64(qry.value(rec.indexOf("stamp")).toString().toUtf8());
                    globals().signature_main_doctor = QByteArray::fromBase64(qry.value(rec.indexOf("signature")).toString().toUtf8());
                }
            }

            QSqlQuery qry_cloud(dbConnection);
            qry_cloud.prepare(R"(
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
            qry_cloud.addBindValue(globals().c_id_organizations);
            qry_cloud.addBindValue(m_id_user);
            if (! qry_cloud.exec()) {
                qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
            } else {
                while (qry_cloud.next()) {
                    QSqlRecord rec = qry_cloud.record();
                    globals().cloud_host          = qry_cloud.value(rec.indexOf("hostName")).toString();
                    globals().cloud_nameBase      = qry_cloud.value(rec.indexOf("databaseName")).toString();
                    globals().cloud_port          = qry_cloud.value(rec.indexOf("port")).toString();
                    globals().cloud_optionConnect = qry_cloud.value(rec.indexOf("connectionOption")).toString();
                    globals().cloud_user          = qry_cloud.value(rec.indexOf("username")).toString();

                    QByteArray hash_user         = QByteArray::fromHex(qry_cloud.value(rec.indexOf("hashUser")).toString().toUtf8());
                    QByteArray iv                = QByteArray::fromBase64(qry_cloud.value(rec.indexOf("iv")).toString().toUtf8());
                    QByteArray encryptedPassword = QByteArray::fromBase64(qry_cloud.value(rec.indexOf("password")).toString().toUtf8());
                    QByteArray decryptedPassword = crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
                    globals().cloud_passwd = decryptedPassword;
                }
            }

        }  // qry se destruge aici

        emit finishSetDataConstants();

        if (dbConnection.isOpen()) {
            dbConnection.close();
        }

    } // dbConnection se destruge aici

    db->removeDatabaseThread(threadConnectionName);

    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

void HandlerFunctionThread::saveDataPatient()
{
    const QString threadConnectionName = QString("connection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    { // -- bloc pentru dbConnection
        QSqlDatabase dbConnection = db->getDatabaseThread(threadConnectionName, m_thisMySQL);

        { // -- bloc pentru QSqlQuery

            if (! patientExistsInDB(dbConnection)){
                if (insertDataPatientInDB(dbConnection)){
                    emit finishInsertDataCatPatient(true, cat_patient_id);
                    if (! m_thisMySQL) {
                        syncWithMySQL(cat_patient_id);  // sincronizare în cloud
                    }
                } else {
                    emit finishInsertDataCatPatient(false, cat_patient_id);
                }
            } else {
                emit finishExistPatientInBD(cat_patient_name,
                                            cat_patient_fname,
                                            cat_patient_birthday,
                                            cat_patient_idnp);
            }

        } // -- distrugerea QSqlQuery

        if (dbConnection.isOpen()) {
            dbConnection.close();
        }
    } // -- distrugerea obiectului dbConnection

    db->removeDatabaseThread(threadConnectionName);

    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

void HandlerFunctionThread::syncWithMySQL(int localPatientId)
{
    // 1. verificam daca este completat tabela 'cloudServer'
    if (globals().cloud_host.isEmpty() ||
        globals().cloud_nameBase.isEmpty() ||
        globals().cloud_user.isEmpty() ||
        globals().cloud_passwd.isEmpty())
        return;

    // 2. verificam daca ID e valid
    if (localPatientId <= 0)
        return;

    // 3. efectuam conectarea la BD MySQL
    QSqlDatabase dbCloud = QSqlDatabase::addDatabase("QMYSQL", "cloud_sync_conn");
    dbCloud.setHostName(globals().cloud_host); // completează din globals sau cloudServer
    dbCloud.setDatabaseName(globals().cloud_nameBase);
    dbCloud.setPort(globals().cloud_port.toInt());
    dbCloud.setUserName(globals().cloud_user);
    dbCloud.setPassword(globals().cloud_passwd);
    dbCloud.setConnectOptions(globals().cloud_optionConnect);
    if (! dbCloud.open()) {
        qWarning(logWarning()) << "[SYNC] Conectarea la MySQL (cloud) a eșuat: " << dbCloud.lastError().text();
        return;
    }

    // 4. efectuam selectarea din bd locala
    QSqlQuery qrySelect(QSqlDatabase::database("connection_" + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()))));
    qrySelect.prepare("SELECT * FROM pacients WHERE id = ?");
    qrySelect.addBindValue(localPatientId);
    if (! qrySelect.exec() || ! qrySelect.next()) {
        qWarning(logWarning()) << QStringLiteral("[SYNC] Pacientul '%1' nu a fost găsit în baza locală: ")
                                      .arg(cat_patient_name + " " + cat_patient_fname)
                               << qrySelect.lastError().text();
        dbCloud.close();
        QSqlDatabase::removeDatabase("cloud_sync_conn");
        return;
    }

    // 5. verificam daca exista pacient cu acelasi IDNP
    QSqlQuery qryIDNP_cloud(dbCloud);
    qryIDNP_cloud.prepare(R"(
        SELECT 1 FROM pacients WHERE IDNP = ? LIMIT 1
    )");
    qryIDNP_cloud.addBindValue(cat_patient_idnp);
    if (qryIDNP_cloud.exec() && qryIDNP_cloud.next()) {
        qWarning(logWarning()) << QStringLiteral("[SYNC] Pacientul cu IDNP:%1 deja exista in baza de date 'cloud'.")
                                      .arg(cat_patient_idnp);
        return;
    }

    // 6. verificam daca exista pacient cu acelasi nume, prenume
    QSqlQuery qryName_cloud(dbCloud);
    qryName_cloud.prepare(R"(
        SELECT 1 FROM pacients
        WHERE name = ? AND fName = ? AND birthday = ? LIMIT 1
    )");
    qryName_cloud.addBindValue(cat_patient_name);
    qryName_cloud.addBindValue(cat_patient_fname);
    qryName_cloud.addBindValue(cat_patient_birthday);
    if (qryName_cloud.exec() && qryName_cloud.next()) {
        qWarning(logWarning()) << QStringLiteral("[SYNC] Pacientul '%1' din %2 deja exista in baza de date 'cloud'.")
                                      .arg(cat_patient_name + " " + cat_patient_fname,
                                           cat_patient_birthday.toString("dd.MM.yyyy"));
        return;
    }

    // 7. sincronizam datele cu MySQL
    QSqlQuery qryCloud(dbCloud);
    qryCloud.prepare(R"(
        INSERT INTO pacients (
            deletionMark, IDNP, name, fName, mName,
            medicalPolicy, birthday, address, telephone, email, comment)
        VALUES (:deletionMark, :IDNP, :name, :fName, :mName,
            :medicalPolicy, :birthday, :address, :telephone, :email, :comment)
        ON DUPLICATE KEY UPDATE
            deletionMark = VALUES(deletionMark), IDNP = VALUES(IDNP), name = VALUES(name), fName = VALUES(fName), mName = VALUES(mName),
            medicalPolicy = VALUES(medicalPolicy), birthday = VALUES(birthday), address = VALUES(address), telephone = VALUES(telephone),
            email = VALUES(email), comment = VALUES(comment)
    )");
    // id nu adaugam
    qryCloud.bindValue(":deletionMark", qrySelect.value("deletionMark"));
    qryCloud.bindValue(":IDNP", qrySelect.value("IDNP"));
    qryCloud.bindValue(":name", qrySelect.value("name"));
    qryCloud.bindValue(":fName", qrySelect.value("fName"));
    qryCloud.bindValue(":mName", qrySelect.value("mName"));
    qryCloud.bindValue(":medicalPolicy", qrySelect.value("medicalPolicy"));
    qryCloud.bindValue(":birthday", qrySelect.value("birthday"));
    qryCloud.bindValue(":address", qrySelect.value("address"));
    qryCloud.bindValue(":telephone", qrySelect.value("telephone"));
    qryCloud.bindValue(":email", qrySelect.value("email"));
    qryCloud.bindValue(":comment", qrySelect.value("comment"));

    if (! qryCloud.exec()) {
        qWarning(logWarning()) << QStringLiteral("[SYNC] Inserarea datelor pacientului '%1' în MySQL (cloud) a eșuat: ")
                                      .arg(cat_patient_name + " " + cat_patient_fname)
                               << qryCloud.lastError().text();
    } else {
        qInfo(logInfo()) << QStringLiteral("[SYNC] Pacientul '%1' a fost sincronizat cu succes în cloud.")
                                .arg(cat_patient_name + " " + cat_patient_fname);
    }

    // 8. inchidem bd cloud si eliminam conectarea
    dbCloud.close();
    QSqlDatabase::removeDatabase("cloud_sync_conn");
}

void HandlerFunctionThread::updateDataPatientInDB()
{
    const QString threadConnectionName = QString("connection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    { // -- bloc pentru dbConnection
        QSqlDatabase dbConnection = db->getDatabaseThread(threadConnectionName, m_thisMySQL);

        { // -- bloc pentru QSqlQuery
            QSqlQuery qry(dbConnection);
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
                WHERE id = ?
            )");
            qry.addBindValue(0);
            qry.addBindValue((cat_patient_idnp.isEmpty()) ? QVariant() : cat_patient_idnp);
            qry.addBindValue(cat_patient_name);
            qry.addBindValue(cat_patient_fname);
            qry.addBindValue(QVariant());
            qry.addBindValue((cat_patient_medicalPolicy.isEmpty()) ? QVariant() : cat_patient_medicalPolicy);
            qry.addBindValue(cat_patient_birthday); //???
            qry.addBindValue((cat_patient_address.isEmpty()) ? QVariant() : cat_patient_address);
            qry.addBindValue((cat_patient_telephone.isEmpty()) ? QVariant() : cat_patient_telephone);
            qry.addBindValue((cat_patient_email.isEmpty()) ? QVariant() : cat_patient_email);
            qry.addBindValue((cat_patient_comment.isEmpty()) ? QVariant() : cat_patient_comment);
            qry.addBindValue(cat_patient_id);
            if (qry.exec()){
                emit finishUpdateDataCatPatient(true);
            }
        } // -- distrugerea QSqlQuery

        if (dbConnection.isOpen()) {
            dbConnection.close();
        }
    } // -- distrugerea obiectului dbConnection

    db->removeDatabaseThread(threadConnectionName);

    emit finishUpdateDataCatPatient(true);

    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

void HandlerFunctionThread::exportDocumentsToPDF()
{
    // 📌 1 Generam denumirile conexiunilor in afara blocului
    const QString threadConnectionName = QString("connection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    const QString threadImageConnectionName = QString("connection_image_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    if (m_thisMySQL)
        Q_UNUSED(threadImageConnectionName);

    // 💡
    /*--------------------------------------------------------
     * cream un bloc pentru distrugerea obectelor:
     *  - dbConnection
     *  - dbImageConnection
     * ... QSqlDatabase::database(name) returnează un obiect
     * pentru distrugerea folosim blocuri {}
     *-------------------------------------------------------*/
    {
        QSqlDatabase dbConnection;
        QSqlDatabase dbImageConnection;
        dbConnection = db->getDatabaseThread(threadConnectionName, m_thisMySQL);
        if (! m_thisMySQL) {
            dbImageConnection = db->getDatabaseImageThread(threadImageConnectionName);
        }

        exportDocumentOrder(dbConnection);
        exportDocumentReport(dbConnection);
        if (m_thisMySQL)
            exportDocumentImages(dbConnection);
        else
            exportDocumentImages(dbImageConnection);

        if (dbConnection.isOpen()) {
            dbConnection.close();
        }

        if (! m_thisMySQL && dbImageConnection.isOpen()) {
            dbImageConnection.close();
        }
    } // 💡 la nivel acesta se distruge: dbConnection & dbImageConnection

    db->removeDatabaseThread(threadConnectionName);
    if (! m_thisMySQL)
        db->removeDatabaseThread(threadImageConnectionName);

    // emitem signal de exportare cu succes
    emit finishExportDocumenstToPDF(data_agentEmail);

    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

bool HandlerFunctionThread::patientExistsInDB(QSqlDatabase dbConnection)
{
    QSqlQuery qry(dbConnection);

    if (! cat_patient_idnp.isEmpty()) {  // 1. daca este idnp - (principal)

        qry.prepare(R"(
            SELECT 1 FROM pacients
            WHERE IDNP = ? LIMIT 1
        )");
        qry.addBindValue(cat_patient_idnp);

    } else {  // 2. daca nu este idnp (cautarea dupa: nume, prenume, data nasterii)

        QSqlQuery qry_no_idnp(dbConnection);
        qry_no_idnp.prepare(R"(
            SELECT 1 FROM pacients
            WHERE name = ? AND fName = ? AND birthday = ? LIMIT 1
        )");
        qry_no_idnp.addBindValue(cat_patient_name);
        qry_no_idnp.addBindValue(cat_patient_fname);
        qry_no_idnp.addBindValue(cat_patient_birthday);

    }

    return qry.exec() && qry.next();
}

bool HandlerFunctionThread::insertDataPatientInDB(QSqlDatabase dbConnection)
{
    QSqlQuery qry(dbConnection);
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
    qry.addBindValue(cat_patient_id);
    qry.addBindValue(0);
    qry.addBindValue((cat_patient_idnp.isEmpty()) ? QVariant() : cat_patient_idnp);
    qry.addBindValue(cat_patient_name);
    qry.addBindValue(cat_patient_fname);
    qry.addBindValue(QVariant());
    qry.addBindValue((cat_patient_medicalPolicy.isEmpty()) ? QVariant() : cat_patient_medicalPolicy);
    qry.addBindValue(cat_patient_birthday); //???
    qry.addBindValue((cat_patient_address.isEmpty()) ? QVariant() : cat_patient_address);
    qry.addBindValue((cat_patient_telephone.isEmpty()) ? QVariant() : cat_patient_telephone);
    qry.addBindValue((cat_patient_email.isEmpty()) ? QVariant() : cat_patient_email);
    qry.addBindValue((cat_patient_comment.isEmpty()) ? QVariant() : cat_patient_comment);
    return qry.exec();
}

// **********************************************************************************
// --- procesarea exportului fisierelor

void HandlerFunctionThread::setModelImgForPrint()
{
    // 1. logotip
    QPixmap pix_logo = QPixmap();
    QStandardItem *img_item_logo = new QStandardItem();
    if (! m_logo_byteArray.isEmpty() && pix_logo.loadFromData(m_logo_byteArray)) {
        img_item_logo->setData(pix_logo.scaled(300,50, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_logo = 1;
    }

    // 2. stampila organizatiei
    QPixmap pix_stamp_organization = QPixmap();
    QStandardItem* img_item_stamp_organization = new QStandardItem();
    if (! m_stamp_main_organization.isEmpty() && pix_stamp_organization.loadFromData(m_stamp_main_organization)) {
        img_item_stamp_organization->setData(pix_stamp_organization.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stamp_organization = 1;
    }

    // 3. stampila doctorului
    QPixmap pix_stamp_doctor = QPixmap();
    QStandardItem* img_item_stamp_doctor = new QStandardItem();
    if (! m_stamp_main_doctor.isEmpty() && pix_stamp_doctor.loadFromData(m_stamp_main_doctor)) {
        img_item_stamp_doctor->setData(pix_stamp_doctor.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stamp_doctor = 1;
    }

    // 4. semnatura doctorului
    QPixmap pix_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    if (! m_signature_main_doctor.isEmpty() && pix_signature.loadFromData(m_signature_main_doctor)) {
        img_item_signature->setData(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_signature = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp_organization);
    items_img.append(img_item_stamp_doctor);
    items_img.append(img_item_signature);

    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(4);
    model_img->appendRow(items_img);
}

void HandlerFunctionThread::exportDocumentOrder(QSqlDatabase dbConnection)
{
    if (m_id_order == -1) {
        emit errorExportDocs("Nu este determinat ID documentului 'OrderEcho' pentru exportare in PDF !!!");
        return;
    }

    ExportData data;
    bool noncomercial_price = false;
    int sum_order = 0;

    {
        //-----------------------------------------------------------------------------
        // 📌 1 setam variabila 'noncomercial_price'
        QSqlQuery qry(dbConnection);
        qry.prepare("SELECT "
                    "  orderEcho.id_typesPrices, "
                    "  orderEcho.id_pacients, "
                    "  orderEcho.sum, "
                    "  orderEcho.numberDoc  AS nr_order, "
                    "  orderEcho.dateDoc    AS dateInvestigation, "
                    "  reportEcho.id        AS id_report, "
                    "  reportEcho.numberDoc AS nr_report, "
                    "  typesPrices.noncomercial,"
                    "  fullNamePacients.name AS name_patient, "
                    "  fullNameDoctors.nameAbbreviated AS doctore_execute, "
                    "  pacients.email AS emailTo "
                    "FROM "
                    "  orderEcho "
                    "INNER JOIN "
                    "  typesPrices ON typesPrices.id = orderEcho.id_typesPrices "
                    "INNER JOIN "
                    "  reportEcho ON reportEcho.id_orderEcho = orderEcho.id "
                    "INNER JOIN "
                    "  fullNamePacients ON fullNamePacients.id_pacients = orderEcho.id_pacients "
                    "INNER JOIN "
                    "  fullNameDoctors ON fullNameDoctors.id_doctors = orderEcho.id_doctors_execute "
                    "INNER JOIN "
                    "  pacients ON pacients.id = orderEcho.id_pacients "
                    "WHERE "
                    "  orderEcho.id = ?;");
        qry.addBindValue(m_id_order);
        if (! qry.exec()) {
            emit errorExportDocs("SQL Error: " + qry.lastError().text());
            return;
        } else {
            while (qry.next()) {
                QSqlRecord rec = qry.record();
                noncomercial_price = qry.value(rec.indexOf("noncomercial")).toBool();
                m_id_patient = qry.value(rec.indexOf("id_pacients")).toInt();
                sum_order    = qry.value(rec.indexOf("sum")).toInt();
                m_id_report  = qry.value(rec.indexOf("id_report")).toInt();
                m_nr_order   = qry.value(rec.indexOf("nr_order")).toInt();
                m_nr_report  = qry.value(rec.indexOf("nr_report")).toInt();

                // introducem date in structura pentru transmiterea
                data.nr_order     = qry.value(rec.indexOf("nr_order")).toString();
                data.nr_report    = qry.value(rec.indexOf("nr_report")).toString();
                data.emailTo      = qry.value(rec.indexOf("emailTo")).toString();
                data.name_patient = qry.value(rec.indexOf("name_patient")).toString();
                data.name_doctor_execute   = qry.value(rec.indexOf("doctore_execute")).toString();
                data.str_dateInvestigation = qry.value(rec.indexOf("dateInvestigation")).toString();
            }
        }

        // introducem structura in vector
        data_agentEmail.append(data);

        //-----------------------------------------------------------------------------
        // 📌 2 setam modelul 'model_img' - logotipul, stampila organizatiei,
        // semnatura doctorului si stampila doctorului
        setModelImgForPrint();

        //-----------------------------------------------------------------------------
        // 📌 3 alocam memoria
        m_report = new LimeReport::ReportEngine(this);
        m_report->dataManager()->addModel("table_img", model_img, true);

        //-----------------------------------------------------------------------------
        // 📌 4 setam model pentru organizatia
        model_organization = new QSqlQueryModel(this);
        QSqlQuery qry_organiation(dbConnection);
        qry_organiation.prepare("SELECT constants.id_organizations, "
                                "  organizations.IDNP, "
                                "  organizations.name, "
                                "  organizations.address, "
                                "  organizations.telephone, "
                                "  fullNameDoctors.nameAbbreviated AS doctor,"
                                "  organizations.email FROM constants "
                                "INNER JOIN "
                                "  organizations ON constants.id_organizations = organizations.id "
                                "INNER JOIN "
                                "  fullNameDoctors ON constants.id_doctors = fullNameDoctors.id_doctors "
                                "WHERE "
                                "  constants.id_users = ?;");
        qry_organiation.addBindValue(m_id_user);
        if (! qry_organiation.exec()) {
            model_organization->deleteLater();
            m_report->deleteLater();
            emit errorExportDocs("SQL Error: " + qry_organiation.lastError().text());
            return;
        }
        model_organization->setQuery(std::move(qry_organiation));

        //-----------------------------------------------------------------------------
        // 📌 5 setam model pentru pacient
        model_patient = new QSqlQueryModel(this);
        QSqlQuery qry_patient(dbConnection);
        if (m_thisMySQL)
            qry_patient.prepare("SELECT CONCAT(pacients.name,' ', pacients.fName) AS FullName,"
                                "  CONCAT(SUBSTRING(pacients.birthday, 9, 2) ,'.', SUBSTRING(pacients.birthday, 6, 2) ,'.', SUBSTRING(pacients.birthday, 1, 4)) AS birthday,"
                                "  pacients.IDNP,"
                                "  pacients.medicalPolicy,"
                                "  pacients.address "
                                "FROM "
                                "  pacients "
                                "WHERE "
                                "  id = ?;");
        else
            qry_patient.prepare("SELECT pacients.name ||' '|| pacients.fName AS FullName,"
                                "  substr(pacients.birthday, 9, 2) ||'.'|| substr(pacients.birthday, 6, 2) ||'.'|| substr(pacients.birthday, 1, 4) AS birthday,"
                                "  pacients.IDNP,"
                                "  pacients.medicalPolicy,"
                                "  pacients.address "
                                "FROM "
                                "  pacients "
                                "WHERE "
                                "  id = ?;");
        qry_patient.addBindValue(m_id_patient);
        if (! qry_patient.exec()) {
            model_organization->deleteLater();
            model_patient->deleteLater();
            m_report->deleteLater();
            emit errorExportDocs("SQL Error: " + qry_patient.lastError().text());
            return;
        }
        model_patient->setQuery(std::move(qry_patient));

        //-----------------------------------------------------------------------------
        // 📌 6 setam model pentru tabela documentului
        model_table = new QSqlQueryModel(this);
        QSqlQuery qry_table(dbConnection);
        QString str_price;
        if (m_thisMySQL)
            str_price = (noncomercial_price) ? "'0-00'" : "orderEchoTable.price";
        else
            str_price = (noncomercial_price) ? "'0-00'" : "orderEchoTable.price ||'0'";
        qry_table.prepare(db->getQryForTableOrderById(m_id_order, (noncomercial_price) ? "'0-00'" : str_price));
        if (! qry_table.exec()) {
            model_organization->deleteLater();
            model_patient->deleteLater();
            model_table->deleteLater();
            m_report->deleteLater();
            emit errorExportDocs("SQL Error: " + qry_table.lastError().text());
            return;
        }
        model_table->setQuery(std::move(qry_table));

        double _num = 0;
        _num = (noncomercial_price) ? 0 : sum_order; // daca 'noncomercial_price' -> nu aratam suma

        // *************************************************************************************
        // 📌 7 transmitem variabile si modelurile generatorului de rapoarte
        m_report->dataManager()->clearUserVariables();
        m_report->dataManager()->setReportVariable("sume_total", QString("%1").arg(_num, 0, 'f', 2));
        m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
        m_report->dataManager()->setReportVariable("v_exist_stamp", exist_stamp_organization);
        m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
        m_report->dataManager()->setReportVariable("v_exist_signature", exist_signature);

        m_report->dataManager()->addModel("main_organization", model_organization, false);
        m_report->dataManager()->addModel("table_pacient", model_patient, false);
        m_report->dataManager()->addModel("table_table", model_table, false);
        m_report->setShowProgressDialog(true);

        // *************************************************************************************
        // 📌 8 verificam daca este fisierul de tipar
        QDir dir;
        if (! QFile(dir.toNativeSeparators(m_pathTemplatesDocs + "/Order.lrxml")).exists()){
            model_img->deleteLater();
            model_organization->deleteLater();
            model_patient->deleteLater();
            model_table->deleteLater();
            m_report->deleteLater();
            emit errorExportDocs("Nu a fost gasit fisierul formai de tipar !!!");
            return;
        }

        // *************************************************************************************
        // 📌 9 exportam fgisierul in formatul PDF
        m_report->loadFromFile(dir.toNativeSeparators(m_pathTemplatesDocs + "/Order.lrxml"));
        m_report->printToPDF(m_filePDF + "/Comanda_ecografica_nr_" + QString::number(m_nr_order) + ".pdf");
        emit setTextInfo("S-a descarcat documentul 'Comanda ecografica'...");

        model_table->deleteLater();
    }

    // *************************************************************************************
    // 📌 10 elibiram memoria
    // model_img->deleteLater();
    // model_organization->deleteLater();
    // model_patient->deleteLater();

    m_report->deleteLater();

}

void HandlerFunctionThread::exportDocumentReport(QSqlDatabase dbConnection)
{
    if (m_id_report == -1) {
        emit errorExportDocs("Nu este determinat ID documentului 'ReportEcho' pentru exportare in PDF !!!");
        return;
    }

    {
        m_report = new LimeReport::ReportEngine(this);
        QSqlQueryModel *modelOrgansInternal = new QSqlQueryModel();
        QSqlQueryModel *modelUrinarySystem  = new QSqlQueryModel();
        QSqlQueryModel *modelProstate       = new QSqlQueryModel();
        QSqlQueryModel *modelGynecology     = new QSqlQueryModel();
        QSqlQueryModel *modelBreast         = new QSqlQueryModel();
        QSqlQueryModel *modelThyroid        = new QSqlQueryModel();
        QSqlQueryModel *modelGestationO     = new QSqlQueryModel();
        QSqlQueryModel *modelGestation1     = new QSqlQueryModel();
        QSqlQueryModel *modelGestation2     = new QSqlQueryModel();

        QSqlQuery qry(dbConnection);
        {
            qry.prepare("SELECT "
                        "  t_organs_internal,"
                        "  t_urinary_system,"
                        "  t_prostate,"
                        "  t_gynecology,"
                        "  t_breast,"
                        "  t_thyroid,"
                        "  t_gestation0,"
                        "  t_gestation1,"
                        "  t_gestation2,"
                        "  t_gestation3 "
                        " FROM "
                        "  reportEcho "
                        "WHERE "
                        "  id_orderEcho = ?;");
            qry.addBindValue(m_id_order);
            if (! qry.exec()) {
                // eliberam memoria
                model_img->deleteLater();
                model_organization->deleteLater();
                model_patient->deleteLater();
                model_table->deleteLater();
                modelOrgansInternal->deleteLater();
                modelUrinarySystem->deleteLater();
                modelProstate->deleteLater();
                modelGynecology->deleteLater();
                modelBreast->deleteLater();
                modelThyroid->deleteLater();
                modelGestationO->deleteLater();
                modelGestation1->deleteLater();
                modelGestation2->deleteLater();
                m_report->deleteLater();
                emit errorExportDocs("SQL Error: " + qry.lastError().text());
                return;
            } else {
                if (qry.next()) {
                    QSqlRecord rec = qry.record();
                    m_organs_internal = qry.value(rec.indexOf("t_organs_internal")).toBool();
                    m_urinary_system  = qry.value(rec.indexOf("t_urinary_system")).toBool();
                    m_prostate   = qry.value(rec.indexOf("t_prostate")).toBool();
                    m_gynecology = qry.value(rec.indexOf("t_gynecology")).toBool();
                    m_breast     = qry.value(rec.indexOf("t_breast")).toBool();
                    m_thyroide   = qry.value(rec.indexOf("t_thyroid")).toBool();
                    m_gestation0 = qry.value(rec.indexOf("t_gestation0")).toBool();
                    m_gestation1 = qry.value(rec.indexOf("t_gestation1")).toBool();
                    m_gestation2 = qry.value(rec.indexOf("t_gestation2")).toBool();
                    m_gestation3 = qry.value(rec.indexOf("t_gestation3")).toBool();
                }
            }

            m_report->dataManager()->addModel("table_logo", model_img, true);
            m_report->dataManager()->addModel("main_organization", model_organization, false);
            m_report->dataManager()->addModel("table_patient", model_patient, false);
            m_report->dataManager()->clearUserVariables();
            m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
            m_report->dataManager()->setReportVariable("v_export_pdf", 1);
            m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
            m_report->dataManager()->setReportVariable("v_exist_signature_doctor", exist_signature);
            m_report->dataManager()->setReportVariable("unitMeasure", (m_unitMeasure == "milimetru") ? "mm" : "cm");

            // complex
            if (m_organs_internal && m_urinary_system){

                QSqlQuery qry_organsInternal(dbConnection);
                qry_organsInternal.prepare(db->getQryForTableOrgansInternalById(m_id_report));
                if (qry_organsInternal.exec()) {
                    modelOrgansInternal->setQuery(std::move(qry_organsInternal));

                }
                QSqlQuery qry_urinarySystem(dbConnection);
                qry_urinarySystem.prepare(db->getQryForTableUrinarySystemById(m_id_report));
                if (qry_urinarySystem.exec()){
                    modelUrinarySystem->setQuery(std::move(qry_urinarySystem));
                }
                m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);
                m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);
                m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Complex.lrxml")){

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();
                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Complex.lrxml' !!!");

                    return;
                }

                // export
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_complex.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - complex ...");
            }

            // organs internal
            if (m_organs_internal && !m_urinary_system){

                // extragem datele si introducem in model + setam variabile
                m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
                QSqlQuery qry_organsInternal(dbConnection);
                qry_organsInternal.prepare(db->getQryForTableOrgansInternalById(m_id_report));
                if (qry_organsInternal.exec()) {
                    modelOrgansInternal->setQuery(std::move(qry_organsInternal));

                }
                m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Organs internal.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Organs internal.lrxml' !!!");
                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // export
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_organe_interne.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - organe interne ...");
            }

            // urinary system
            if (m_urinary_system && !m_organs_internal){

                // extragem datele si introducem in model + setam variabile
                m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
                QSqlQuery qry_urinarySystem(dbConnection);
                qry_urinarySystem.prepare(db->getQryForTableUrinarySystemById(m_id_report));
                if (qry_urinarySystem.exec()){
                    modelUrinarySystem->setQuery(std::move(qry_urinarySystem));
                }
                m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Urinary system.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Urinary system.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // export
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_sistemul_urinar.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - s.urinar ...");
            }

            // prostate
            if (m_prostate){

                // extragem datele si introducem in model + setam variabile
                QSqlQuery qry_supliment(dbConnection);
                qry_supliment.prepare("SELECT transrectal FROM tableProstate WHERE id_reportEcho = ?;");
                qry_supliment.addBindValue(m_id_report);
                if (qry_supliment.exec() && qry_supliment.next()){
                    m_report->dataManager()->setReportVariable("method_examination", (qry_supliment.value(0).toInt() == 1) ? "transrectal" : "transabdominal");
                } else {
                    m_report->dataManager()->setReportVariable("method_examination", "transabdominal");
                }
                m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");

                QSqlQuery qry_prostate(dbConnection);
                qry_prostate.prepare(db->getQryForTableProstateById(m_id_report));
                if (qry_prostate.exec()){
                    modelProstate->setQuery(std::move(qry_prostate));
                }
                m_report->dataManager()->addModel("table_prostate", modelProstate, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Prostate.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Prostate.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_prostata.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - prostata ...");
            }

            // gynecology
            if (m_gynecology){

                // extragem datele si introducem in model + setam variabile
                QSqlQuery qry_ginecSupliment(dbConnection);
                qry_ginecSupliment.prepare("SELECT transvaginal FROM tableGynecology WHERE id_reportEcho = ?;");
                qry_ginecSupliment.addBindValue(m_id_report);
                if (qry_ginecSupliment.exec() && qry_ginecSupliment.next()){
                    m_report->dataManager()->setReportVariable("method_examination", (qry_ginecSupliment.value(0).toInt() == 1) ? "transvaginal" : "transabdominal");
                } else {
                    m_report->dataManager()->setReportVariable("method_examination", "transabdominal");
                }
                m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");

                QSqlQuery qry_ginecology(dbConnection);
                qry_ginecology.prepare(db->getQryForTableGynecologyById(m_id_report));
                if (qry_ginecology.exec()){
                    modelGynecology->setQuery(std::move(qry_ginecology));
                }
                m_report->dataManager()->addModel("table_gynecology", modelGynecology, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Gynecology.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Gynecology.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_ginecologie.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic - ginecologia'...");
            }

            // breast
            if (m_breast){

                // extragem datele si introducem in model + setam variabile
                QSqlQuery qry_breast(dbConnection);
                qry_breast.prepare(db->getQryForTableBreastById(m_id_report));
                if (qry_breast.exec()){
                    modelBreast->setQuery(std::move(qry_breast));
                }
                m_report->dataManager()->addModel("table_breast", modelBreast, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Breast.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Breast.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_glandele_mamare.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - gl.mamare ...");
            }

            // thyroid
            if (m_thyroide){

                // extragem datele si introducem in model + setam variabile
                m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
                QSqlQuery qry_thyroid(dbConnection);
                qry_thyroid.prepare(db->getQryForTableThyroidById(m_id_report));
                if (qry_thyroid.exec()){
                    modelThyroid->setQuery(std::move(qry_thyroid));
                }
                m_report->dataManager()->addModel("table_thyroid", modelThyroid, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Thyroid.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Thyroid.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_tiroida.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - tiroida ...");
            }

            // gestation0
            if (m_gestation0){

                // extragem datele si introducem in model + setam variabile
                QSqlQuery qry_supGestation0(dbConnection);
                qry_supGestation0.prepare("SELECT view_examination FROM tableGestation0 WHERE id_reportEcho = ?;");
                qry_supGestation0.addBindValue(m_id_report);
                if (qry_supGestation0.exec() && qry_supGestation0.next()){
                    m_report->dataManager()->setReportVariable("ivestigation_view", (qry_supGestation0.value(0).toInt() == 1));
                } else {
                    m_report->dataManager()->setReportVariable("ivestigation_view", 0);
                }

                QSqlQuery qry_gestation0(dbConnection);
                qry_gestation0.prepare(db->getQryForTableGestation0dById(m_id_report));
                if (qry_gestation0.exec()){
                    modelGestationO->setQuery(std::move(qry_gestation0));
                }
                m_report->dataManager()->addModel("table_gestation0", modelGestationO, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Gestation0.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Gestation0.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_sarcina0.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina pana la 11 sapt. ...");
            }

            // gestation1
            if (m_gestation1){

                // extragem datele si introducem in model + setam variabile
                QSqlQuery qry_supGestation1(dbConnection);
                qry_supGestation1.prepare("SELECT view_examination FROM tableGestation1 WHERE id_reportEcho = ?;");
                qry_supGestation1.addBindValue(m_id_report);
                if (qry_supGestation1.exec() && qry_supGestation1.next()){
                    m_report->dataManager()->setReportVariable("ivestigation_view", (qry_supGestation1.value(0).toInt() == 1));
                } else {
                    m_report->dataManager()->setReportVariable("ivestigation_view", 0);
                }

                QSqlQuery qry_gestation1(dbConnection);
                qry_gestation1.prepare(db->getQryForTableGestation1dById(m_id_report));
                if (qry_gestation1.exec()){
                    modelGestation1->setQuery(std::move(qry_gestation1));
                }
                m_report->dataManager()->addModel("table_gestation1", modelGestation1, false);

                // completam sablonul
                if (! m_report->loadFromFile(m_pathTemplatesDocs + "/Gestation1.lrxml")){

                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Gestation1.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentarea preview sau designer
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_sarcina1.pdf");
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina trim.I ...");
            }

            // gestation2
            if (m_gestation2){

                QSqlQuery qry_gestation2(dbConnection);
                qry_gestation2.prepare(db->getQryForTableGestation2(m_id_report));
                if (qry_gestation2.exec()){
                    modelGestation2->setQuery(std::move(qry_gestation2));
                }
                m_report->dataManager()->addModel("table_gestation2", modelGestation2, false);

                int trim = -1;
                QSqlQuery qry_supGestation2(dbConnection);
                qry_supGestation2.prepare("SELECT trimestru FROM tableGestation2 WHERE id_reportEcho = ?;");
                qry_supGestation2.addBindValue(m_id_report);
                if (qry_supGestation2.exec() && qry_supGestation2.next()){
                    trim = qry_supGestation2.value(0).toInt();
                } else {
                    trim = 2;
                }

                // sablonul
                QString pathToFileGestation = (trim == 2) ? m_pathTemplatesDocs + "/Gestation2.lrxml" : m_pathTemplatesDocs + "/Gestation3.lrxml";
                if (! m_report->loadFromFile(pathToFileGestation)){
                    // emitem ca este eroarea
                    emit errorExportDocs("Nu a fost gasit forma de tipar 'Gestation2.lrxml' !!!");

                    // eliberam memoria
                    model_img->deleteLater();
                    model_organization->deleteLater();
                    model_patient->deleteLater();
                    model_table->deleteLater();
                    modelOrgansInternal->deleteLater();
                    modelUrinarySystem->deleteLater();
                    modelProstate->deleteLater();
                    modelGynecology->deleteLater();
                    modelBreast->deleteLater();
                    modelThyroid->deleteLater();
                    modelGestationO->deleteLater();
                    modelGestation1->deleteLater();
                    modelGestation2->deleteLater();
                    m_report->deleteLater();

                    return;
                }

                // prezentam
                m_report->setShowProgressDialog(true);
                m_report->printToPDF(m_filePDF + "/Raport_ecografic_nr_" + QString::number(m_nr_report) + "_sarcina2.pdf");
                QString str_txt = (trim == 2) ? "trim.II" : "trim.III";
                emit setTextInfo("S-a descarcat documentul 'Raport ecografic' - sarcina " + str_txt);
            }
        }

        // eliberam memoria
        model_img->deleteLater();
        model_organization->deleteLater();
        model_patient->deleteLater();
        modelOrgansInternal->deleteLater();
        modelUrinarySystem->deleteLater();
        modelProstate->deleteLater();
        modelGynecology->deleteLater();
        modelBreast->deleteLater();
        modelThyroid->deleteLater();
        modelGestationO->deleteLater();
        modelGestation1->deleteLater();
        modelGestation2->deleteLater();
        m_report->deleteLater();

        emit setTextInfo("Exportul documentelor s-a finisat cu succes.");
    }

}

void HandlerFunctionThread::exportDocumentImages(QSqlDatabase dbImageConnection)
{
    if (m_id_report == -1) {
        emit errorExportDocs("Nu este determinat ID documentului 'ReportEcho' pentru exportare in PDF !!!");
        return;
    }

    emit setTextInfo("Se pregateste exportarea imaginelor ...");

    QSqlQuery qry(dbImageConnection);
    qry.prepare("SELECT "
                "  image_1,"
                "  image_2,"
                "  image_3,"
                "  image_4,"
                "  image_5 "
                "FROM "
                "  imagesReports "
                "WHERE "
                "  id_orderEcho = ? AND id_reportEcho = ?;");
    qry.addBindValue(m_id_order);
    qry.addBindValue(m_id_report);
    if (! qry.exec()) {
        emit errorExportDocs("SQL Error: " + qry.lastError().text());
        return;
    } else {
        if (qry.next()){
            QSqlRecord rec = qry.record();
            QVector<QByteArray> imageByteArrays;
            QVector<QString> imageNames = {"image_1", "image_2", "image_3", "image_4", "image_5"};

            for (const auto& imageName : imageNames) {
                QByteArray imageData = QByteArray::fromBase64(qry.value(rec.indexOf(imageName)).toByteArray());
                imageByteArrays.append(imageData);
            }

            for (int i = 0; i < imageByteArrays.size(); ++i) {
                if (! imageByteArrays[i].isEmpty()) {
                    QPixmap pixmap;
                    if (pixmap.loadFromData(imageByteArrays[i])) {
                        QString filePath = m_filePDF + "/" + QString("Image_report_%1_nr_%2.jpg").arg(QString::number(m_nr_report), QString::number(i + 1));
                        if (!pixmap.save(filePath)) {
                            emit errorExportDocs("Eroare la salvarea imaginii: " + filePath);
                            return;
                        } else {
                            emit setTextInfo("Imagine salvată: " + filePath);
                        }
                    } else {
                        emit errorExportDocs("Eroare la încărcarea imaginii din QByteArray!");
                        return;
                    }
                }
            }
        }
    }

    emit setTextInfo("Imaginile sunt salvate cu succes ...");
}
