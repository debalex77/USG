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

#include "contonline.h"
#include "qabstractitemview.h"
#include "ui_contonline.h"

ContOnline::ContOnline(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ContOnline)
{
    ui->setupUi(this);

    setWindowTitle(tr("Agentul contului online %1").arg("[*]"));

    db = new DataBase(this);
    popUp = new PopUp(this);
    email_core = new EmailCore(this);

    QString qry_organizations = "SELECT id, name FROM organizations WHERE deletionMark = 0;";
    model_organizations = new BaseSqlQueryModel(qry_organizations, ui->comboOrganizations);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboOrganizations->setModel(model_organizations);
    if (model_organizations->rowCount() > 20){
        ui->comboOrganizations->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganizations->setStyleSheet("combobox-popup: 0;");
        ui->comboOrganizations->setMaxVisibleItems(15);
    }

    connect(ui->btnConnection, &QPushButton::clicked, this, &ContOnline::checkingConnection);
    connect(this, &ContOnline::IdChangedOrganization, this, &ContOnline::slot_IdChangedOrganization);
    connect(ui->comboOrganizations, &QComboBox::currentIndexChanged, this, &ContOnline::currentIndexChangedOrganization);
    connect(ui->btnOK, &QPushButton::clicked, this, &ContOnline::saveDataClose);
    connect(ui->btnWrite, &QPushButton::clicked, this, &ContOnline::saveData);
    connect(ui->btnClose, &QPushButton::clicked, this, &ContOnline::close);
}

ContOnline::~ContOnline()
{
    delete model_organizations;
    delete email_core;
    delete popUp;
    delete db;
    delete ui;
}

bool ContOnline::existEmail()
{
    QSqlQuery qry;
    qry.prepare("SELECT count(email) FROM contsOnline WHERE id_organizations = ? AND id_users = ?;");
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return false;
    } else {
        if (qry.next()){
            return qry.value(0).toInt() > 0;
        } else {
            return false;
        }
    }
}

void ContOnline::setHashUserApp()
{
    QSqlQuery qry;
    qry.prepare("SELECT hash FROM users WHERE id = ?");
    qry.addBindValue(globals().idUserApp);
    if (qry.exec() && qry.next()) {
        m_hashUserApp = QByteArray::fromHex(qry.value(0).toString().toUtf8());
    }
}

void ContOnline::slot_IdChangedOrganization()
{
    if (m_idOrganization == -1 || m_idOrganization == 0)
        return;

    // 1.setam combobox dupa 'm_idOrganizations' transmis
    auto index_organization = model_organizations->match(model_organizations->index(0, 0), Qt::UserRole, m_idOrganization, 1, Qt::MatchExactly);
    if (! index_organization.isEmpty()){
        ui->comboOrganizations->setCurrentIndex(index_organization.first().row());
    }

    if (ui->comboOrganizations->currentIndex() == 0) // <-selecteaza->
        return;

    // 2.verificam daca sunt date in tabela 'contsOnline'
    QSqlQuery qry;
    qry.prepare("SELECT "
                "  contsOnline.email,"
                "  contsOnline.smtp_server,"
                "  contsOnline.port,"
                "  contsOnline.username,"
                "  contsOnline.password,"
                "  contsOnline.iv, "
                "  users.hash AS hashUser "
                " FROM "
                "  contsOnline "
                " INNER JOIN "
                "  users ON users.id = contsOnline.id_users "
                " WHERE id_organizations = ? AND id_users = ?;");
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return;
    } else {
        if (qry.next()) {
            QSqlRecord rec = qry.record();
            ui->txt_email->setText(qry.value(rec.indexOf("email")).toString());
            ui->txt_smtp_server->setText(qry.value(rec.indexOf("smtp_server")).toString());
            ui->txt_port->setText(qry.value(rec.indexOf("port")).toString());
            ui->txt_user->setText(qry.value(rec.indexOf("username")).toString());

            QByteArray hash_user         = QByteArray::fromHex(qry.value(rec.indexOf("hashUser")).toString().toUtf8());
            QByteArray iv                = QByteArray::fromBase64(qry.value(rec.indexOf("iv")).toString().toUtf8());
            QByteArray encryptedPassword = QByteArray::fromBase64(qry.value(rec.indexOf("password")).toString().toUtf8());
            QByteArray decryptedPassword = crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
            ui->txt_password->setText(QString::fromUtf8(decryptedPassword));
            ui->txt_timeAnswerServer->setText("10");
            return;
        }
    }

    // 3.daca nu sunt date in tabela 'contsOnline' extragem email din catalogul 'Organizations'
    qry.prepare("SELECT email FROM organizations WHERE id = ?;");
    qry.addBindValue(m_idOrganization);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return;
    } else {
        if (qry.next()) {
            ui->txt_email->setText(qry.value(0).toString());
            ui->txt_smtp_server->setText("smtp.gmail.com");
            ui->txt_port->setText("465");
            ui->txt_user->setText(qry.value(0).toString());
            ui->txt_timeAnswerServer->setText("10");
        }
    }

    // 4. extragem hash-ul utilizatorului
    setHashUserApp();
}

void ContOnline::currentIndexChangedOrganization(int index)
{
    const int id_organization = ui->comboOrganizations->itemData(index, Qt::UserRole).toInt();
    if (id_organization != 0)
        setIdOrganization(id_organization);
}

bool ContOnline::insertDataIntoTableContsOnline()
{
    if (m_hashUserApp.isEmpty())
        return false;

    QSqlQuery qry;
    qry.prepare("INSERT INTO contsOnline ("
                "id,"
                "id_organizations, "
                "id_users, "
                "email, "
                "smtp_server, "
                "port, "
                "username, "
                "password,"
                "iv) VALUES (?,?,?,?,?,?,?,?,?);");
    qry.addBindValue(db->getLastIdForTable("contsOnline") + 1);
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    qry.addBindValue(ui->txt_email->text());
    qry.addBindValue(ui->txt_smtp_server->text());
    qry.addBindValue(ui->txt_port->text().toInt());
    qry.addBindValue(ui->txt_user->text());

    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   m_hashUserApp,
                                                                   iv);

    qry.addBindValue(encryptedPassword.toBase64());
    qry.addBindValue(iv.toBase64());
    if (! qry.exec()) {
        qWarning(logWarning()) << "Eroarea de inserare a datelor in tabela 'contsOnline' - SQL Error:" << qry.lastError().text();
        return false;
    } else {
        qInfo(logInfo()) << "Inserarea datelor in tabela 'contsOnline'.";
    }

    return true;
}

bool ContOnline::updateDataIntoTableContsOnline()
{
    if (m_hashUserApp.isEmpty())
        setHashUserApp();

    QSqlQuery qry;
    qry.prepare("UPDATE contsOnline SET "
                " id_users      = ?,"
                " email         = ?,"
                " smtp_server   = ?,"
                " port          = ?,"
                " username      = ?,"
                " password      = ?,"
                " iv            = ? "
                "WHERE "
                " id_organizations = ?;");
    qry.addBindValue(globals().idUserApp);
    qry.addBindValue(ui->txt_email->text());
    qry.addBindValue(ui->txt_smtp_server->text());
    qry.addBindValue(ui->txt_port->text().toInt());
    qry.addBindValue(ui->txt_user->text());
    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   m_hashUserApp,
                                                                   iv);
    qry.addBindValue(encryptedPassword.toBase64());
    qry.addBindValue(iv.toBase64());
    qry.addBindValue(m_idOrganization);
    if (! qry.exec()) {
        qWarning(logWarning()) << "Eroarea de actualizare a datelor in tabela 'contsOnline' - SQL Error:" << qry.lastError().text();
        return false;
    } else {
        qInfo(logInfo()) << "Actualizarea datelor in tabela 'contsOnline'.";
    }
    return true;
}

bool ContOnline::saveData()
{
    if (ui->comboOrganizations->currentIndex() == 0) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat\304\203 organiza\310\233ia !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->txt_email->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat e-mail !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->txt_smtp_server->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat 'SMTP server' !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (existEmail()) {
        if (updateDataIntoTableContsOnline()) {
            popUp->setPopupText(tr("Datele adresei electronice au fost<br>"
                                   "actualizate cu succes in baza de date."));
            popUp->show();
        }
    } else {
        if (insertDataIntoTableContsOnline()) {
            popUp->setPopupText(tr("Datele adresei electronice au fost<br>"
                                   "inserate cu succes in baza de date."));
            popUp->show();
        }
    }

    return true;
}

void ContOnline::saveDataClose()
{
    if (saveData())
        QDialog::accept();
}

void ContOnline::checkingConnection()
{
    if (ui->txt_email->text().isEmpty())
        return;
}
