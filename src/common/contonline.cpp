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

#include <customs/custommessage.h>

ContOnline::ContOnline(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ContOnline)
{
    ui->setupUi(this);

    setWindowTitle(tr("Agentul contului online %1").arg("[*]"));

    db = new DataBase(this);
    popUp = new PopUp(this);
    email_core = new EmailCore(this);

    QString qry_organizations = db->getTextSQL(":/sql/queries/organizations_combo_view.sql");
    model_organizations = new BaseSqlQueryModel(qry_organizations, ui->comboOrganizations);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboOrganizations->setModel(model_organizations);
    if (model_organizations->rowCount() > 20){
        ui->comboOrganizations->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganizations->setStyleSheet("combobox-popup: 0;");
        ui->comboOrganizations->setMaxVisibleItems(15);
    }

    connect(ui->btnConnection, &QPushButton::clicked,
            this, &ContOnline::checkingConnection, Qt::UniqueConnection);
    connect(this, &ContOnline::IdChangedOrganization,
            this, &ContOnline::slot_IdChangedOrganization, Qt::UniqueConnection);
    connect(ui->comboOrganizations, &QComboBox::currentIndexChanged,
            this, &ContOnline::currentIndexChangedOrganization, Qt::UniqueConnection);

    connect(ui->btnOK, &QPushButton::clicked,
            this, &ContOnline::saveDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QPushButton::clicked,
            this, &ContOnline::saveData, Qt::UniqueConnection);
    connect(ui->btnClose, &QPushButton::clicked,
            this, &ContOnline::close, Qt::UniqueConnection);
}

ContOnline::~ContOnline()
{
    delete ui;
}

bool ContOnline::existEmail()
{
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            count(email)
        FROM
            contsOnline
        WHERE
            id_organizations = ? AND
            id_users = ?
    )");
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:"
                               << qry.lastError().text();
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
    qry.prepare(R"(SELECT hash FROM users WHERE id = ?)");
    qry.addBindValue(globals().idUserApp);
    if (qry.exec() && qry.next()) {
        m_hashUserApp = QByteArray::fromHex(qry.value(0).toString().toUtf8());
    }
}

void ContOnline::slot_IdChangedOrganization()
{
    if (m_idOrganization <= 0)
        return;

    // 1.setam combobox dupa 'm_idOrganizations' transmis
    auto index_organization = model_organizations->match(model_organizations->index(0, 0),
                                                         Qt::UserRole,
                                                         m_idOrganization,
                                                         1,
                                                         Qt::MatchExactly);
    if (! index_organization.isEmpty())
        ui->comboOrganizations->setCurrentIndex(index_organization.first().row());

    if (ui->comboOrganizations->currentIndex() == 0) // <-selecteaza->
        return;

    // 2.verificam daca sunt date in tabela 'contsOnline'
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            contsOnline.email,
            contsOnline.smtp_server,
            contsOnline.port,
            contsOnline.username,
            contsOnline.password,
            contsOnline.iv,
            users.hash AS hashUser
        FROM
            contsOnline
        INNER JOIN
            users ON users.id = contsOnline.id_users
        WHERE
            id_organizations = ? AND
            id_users = ?
    )");
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    if (! qry.exec()) {
        qWarning(logWarning()) << "SQL Error:"
                               << qry.lastError().text();
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
            QByteArray decryptedPassword = QByteArray{};//crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
            ui->txt_password->setText(QString::fromUtf8(decryptedPassword));
            ui->txt_timeAnswerServer->setText("10");
            return;
        }
    }

    // 3.daca nu sunt date in tabela 'contsOnline' extragem email din catalogul 'Organizations'
    qry.prepare(R"(SELECT email FROM organizations WHERE id = ?)");
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

    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = QByteArray{};//crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   // m_hashUserApp,
                                                                   // iv);
    QVector<QVariant> data;
    data.append(db->getLastIdForTable("contsOnline") + 1);
    data.append(m_idOrganization);
    data.append(globals().idUserApp);
    data.append(ui->txt_email->text());
    data.append(ui->txt_smtp_server->text());
    data.append(ui->txt_port->text().toInt());
    data.append(ui->txt_user->text());
    data.append(encryptedPassword.toBase64());
    data.append(iv.toBase64());

    QString str_err;
    if (! db->execPreparedFromFile(db->getDatabase(),
                                  ":/sql/queries/contsOnline_insert.sql",
                                  data,
                                  &str_err))
    {
        qCritical(logCritical()) << "Eroarea de inserare a datelor in tabela 'contsOnline' - SQL Error:"
                                 << str_err;
        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor contului '%1' nu s-a efectuat !!!")
                              .arg(ui->txt_email->text()));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();
        return false;
    } else {
        qInfo(logInfo()) << "Inserarea datelor in tabela 'contsOnline'.";
        return true;
    }

}

bool ContOnline::updateDataIntoTableContsOnline()
{
    if (m_hashUserApp.isEmpty())
        setHashUserApp();

    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = QByteArray{};//crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   // m_hashUserApp,
                                                                   // iv);
    QVector<QVariant> data;
    data.append(globals().idUserApp);
    data.append(ui->txt_email->text());
    data.append(ui->txt_smtp_server->text());
    data.append(ui->txt_port->text().toInt());
    data.append(ui->txt_user->text());
    data.append(encryptedPassword.toBase64());
    data.append(iv.toBase64());
    data.append(m_idOrganization);

    QString str_err;
    if (! db->execPreparedFromFile(db->getDatabase(),
                                  ":/sql/queries/contsOnline_update.sql",
                                  data,
                                  &str_err))
    {
        qCritical(logCritical()) << "Eroarea modificarii datelor in tabela 'contsOnline' - SQL Error:"
                                 << str_err;
        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Actualizarea datelor contului '%1' nu s-a efectuat !!!")
                              .arg(ui->txt_email->text()));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();
        return false;
    } else {
        qInfo(logInfo()) << "Actualizarea datelor in tabela 'contsOnline'.";
        return true;
    }
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
