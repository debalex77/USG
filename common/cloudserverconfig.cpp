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

#include "cloudserverconfig.h"
#include "qabstractitemview.h"
#include "ui_cloudserverconfig.h"

#include <QMessageBox>

CloudServerConfig::CloudServerConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CloudServerConfig)
{
    ui->setupUi(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    setWindowTitle(tr("Setări cloud serverului %1").arg("[*]"));

    db = new DataBase(this);
    popUp = new PopUp(this);

    initSetModels();
    initConnection();
}

CloudServerConfig::~CloudServerConfig()
{
    delete model_users;
    delete model_organizations;
    delete ui;
}

int CloudServerConfig::get_ID_Organization() const
{
    return m_id_organization;
}

void CloudServerConfig::set_ID_Organization(const int ID_Organization)
{
    m_id_organization = ID_Organization;
    emit ID_OrganizationChanged();
}

int CloudServerConfig::get_ID_user() const
{
    return m_id_user;
}

void CloudServerConfig::set_ID_user(const int ID_user)
{
    m_id_user = ID_user;
    emit ID_userChanged();
}

void CloudServerConfig::initSetModels()
{
    QString qry_users = "SELECT id,name FROM users WHERE deletionMark = 0;";
    model_users = new BaseSqlQueryModel(qry_users, ui->comboUser);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboUser->setModel(model_users);
    if (model_users->rowCount() > 20){
        ui->comboUser->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboUser->setStyleSheet("combobox-popup: 0;");
        ui->comboUser->setMaxVisibleItems(15);
    }

    QString qry_organizations = "SELECT id, name FROM organizations WHERE deletionMark = 0;";
    model_organizations = new BaseSqlQueryModel(qry_organizations, ui->comboOrganization);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboOrganization->setModel(model_organizations);
    if (model_organizations->rowCount() > 20){
        ui->comboOrganization->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganization->setStyleSheet("combobox-popup: 0;");
        ui->comboOrganization->setMaxVisibleItems(15);
    }
}

void CloudServerConfig::initConnection()
{
    connectionLineEdit();
    connectionComboBox();

    connect(this, &CloudServerConfig::ID_OrganizationChanged, this, &CloudServerConfig::slot_ID_OrganizationChanged);
    connect(this, &CloudServerConfig::ID_userChanged, this, &CloudServerConfig::slot_ID_userChanged);

    connect(ui->btnOK, &QPushButton::clicked, this, &CloudServerConfig::saveDataClose);
    connect(ui->btnWrite, &QPushButton::clicked, this, &CloudServerConfig::saveData);
    connect(ui->btnClose, &QPushButton::clicked, this, &CloudServerConfig::close);
}

void CloudServerConfig::connectionLineEdit()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CloudServerConfig::dataWasModified);
    }
}

void CloudServerConfig::disconnectionLineEdit()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &CloudServerConfig::dataWasModified);
    }
}

void CloudServerConfig::connectionComboBox()
{
    connect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&CloudServerConfig::indexChangedComboOrganization));
    connect(ui->comboUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&CloudServerConfig::indexChangedComboUser));
    connect(ui->comboOrganization, &QComboBox::currentIndexChanged, this, &CloudServerConfig::dataWasModified);
    connect(ui->comboUser, &QComboBox::currentIndexChanged, this, &CloudServerConfig::dataWasModified);
}

void CloudServerConfig::disconnectionComboBox()
{
    disconnect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&CloudServerConfig::indexChangedComboOrganization));
    disconnect(ui->comboUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&CloudServerConfig::indexChangedComboUser));
    disconnect(ui->comboOrganization, &QComboBox::currentIndexChanged, this, &CloudServerConfig::dataWasModified);
    disconnect(ui->comboUser, &QComboBox::currentIndexChanged, this, &CloudServerConfig::dataWasModified);
}

QByteArray CloudServerConfig::getHashUserApp()
{
    QSqlQuery qry;
    qry.prepare("SELECT hash FROM users WHERE id = ?");
    qry.addBindValue(m_id_user);
    if (qry.exec() && qry.next()) {
        return QByteArray::fromHex(qry.value(0).toString().toUtf8());
    } else {
        return QByteArray();
    }
}

bool CloudServerConfig::existServerConfig()
{
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            count(hostName)
        FROM
            cloudServer
        WHERE
            id_organizations = ? AND
            id_users = ?
    )");
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);
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

bool CloudServerConfig::insertDataIntoTableCloudServer()
{
    if (m_id_organization == -1)
        return false;

    if (m_id_user == -1)
        return false;

    QByteArray hash_user = getHashUserApp();
    if (hash_user.isEmpty())
        return false;

    QSqlQuery qry;
    qry.prepare(R"(
        INSERT INTO cloudServer (
            id,
            id_organizations,
            id_users,
            hostName,
            databaseName,
            port,
            connectionOption,
            username,
            password,
            iv)
        VALUES (?,?,?,?,?,?,?,?,?,?)
    )");
    qry.addBindValue(db->getLastIdForTable("cloudServer") + 1);
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);
    qry.addBindValue(ui->txt_host->text());
    qry.addBindValue(ui->txt_nameBase->text());
    qry.addBindValue(ui->txt_port->text());
    qry.addBindValue(ui->txt_option->text());
    qry.addBindValue(ui->txt_user->text());

    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   hash_user,
                                                                   iv);

    qry.addBindValue(encryptedPassword.toBase64());
    qry.addBindValue(iv.toBase64());
    if (! qry.exec()) {
        qWarning(logWarning()) << "Eroarea de inserare a datelor in tabela 'cloudServer' - SQL Error:"
                               << qry.lastError().text();
        return false;
    } else {
        qInfo(logInfo()) << "Inserarea datelor in tabela 'cloudServer'.";
        globals().cloud_host          = ui->txt_host->text();
        globals().cloud_nameBase      = ui->txt_nameBase->text();
        globals().cloud_port          = ui->txt_port->text();
        globals().cloud_optionConnect = ui->txt_option->text();
        globals().cloud_user          = ui->txt_user->text();
        globals().cloud_passwd        = ui->txt_password->text();
        globals().cloud_srv_exist     = true;
        qInfo(logInfo()) << "Actualizate variabile globale pentru sincronizare cu serverul.";
    }

    return true;
}

bool CloudServerConfig::updateDataIntoTableCloudServer()
{
    QByteArray hash_user = getHashUserApp();
    if (hash_user.isEmpty())
        return false;

    QSqlQuery qry;
    qry.prepare(R"(
        UPDATE cloudServer SET
            hostName         = ?,
            databaseName     = ?,
            port             = ?,
            connectionOption = ?,
            username         = ?,
            password         = ?,
            iv               = ?
        WHERE "
            id_organizations = ? AND
            id_users = ?
    )");
    qry.addBindValue(ui->txt_host->text());
    qry.addBindValue(ui->txt_nameBase->text());
    qry.addBindValue(ui->txt_port->text());
    qry.addBindValue(ui->txt_option->text());
    qry.addBindValue(ui->txt_user->text());

    QByteArray iv = crypto_manager->generateRandomIV();
    QByteArray encryptedPassword = crypto_manager->encryptPassword(ui->txt_password->text(),
                                                                   hash_user,
                                                                   iv);

    qry.addBindValue(encryptedPassword.toBase64());
    qry.addBindValue(iv.toBase64());
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);

    if (! qry.exec()) {
        qWarning(logWarning()) << "Eroarea de actualizare a datelor in tabela 'cloudServer' - SQL Error:" << qry.lastError().text();
        return false;
    } else {
        qInfo(logInfo()) << "Actualizarea datelor in tabela 'cloudServer'.";
        globals().cloud_host          = ui->txt_host->text();
        globals().cloud_nameBase      = ui->txt_nameBase->text();
        globals().cloud_port          = ui->txt_port->text();
        globals().cloud_optionConnect = ui->txt_option->text();
        globals().cloud_user          = ui->txt_user->text();
        globals().cloud_passwd        = ui->txt_password->text();
        globals().cloud_srv_exist     = true;
        qInfo(logInfo()) << "Actualizate variabile globale pentru sincronizare cu serverul.";
    }
    return true;
}

void CloudServerConfig::dataWasModified()
{
    setWindowModified(true);
}

void CloudServerConfig::slot_ID_OrganizationChanged()
{
    disconnectionLineEdit();
    disconnectionComboBox();

    auto index_organization = model_organizations->match(model_organizations->index(0, 0), Qt::UserRole, m_id_organization, 1, Qt::MatchExactly);
    if (! index_organization.isEmpty()){
        ui->comboOrganization->setCurrentIndex(index_organization.first().row());
    }

    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            cloudServer.hostName,
            cloudServer.databaseName,
            cloudServer.port,
            cloudServer.connectionOption,
            cloudServer.username,
            cloudServer.password,
            cloudServer.iv,
            users.hash AS hashUser
        FROM
            cloudServer
        INNER JOIN
            users ON users.id = cloudServer.id_users
        WHERE
            cloudServer.id_organizations = ? AND
            cloudServer.id_users = ?
    )");
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);
    if (qry.exec() && qry.next()) {
        QSqlRecord rec = qry.record();
        ui->txt_host->setText(qry.value(rec.indexOf("hostName")).toString());
        ui->txt_nameBase->setText(qry.value(rec.indexOf("databaseName")).toString());
        ui->txt_port->setText(qry.value(rec.indexOf("port")).toString());
        ui->txt_option->setText(qry.value(rec.indexOf("connectionOption")).toString());
        ui->txt_user->setText(qry.value(rec.indexOf("username")).toString());

        QByteArray hash_user         = QByteArray::fromHex(qry.value(rec.indexOf("hashUser")).toString().toUtf8());
        QByteArray iv                = QByteArray::fromBase64(qry.value(rec.indexOf("iv")).toString().toUtf8());
        QByteArray encryptedPassword = QByteArray::fromBase64(qry.value(rec.indexOf("password")).toString().toUtf8());
        QByteArray decryptedPassword = crypto_manager->decryptPassword(encryptedPassword, hash_user, iv);
        ui->txt_password->setText(QString::fromUtf8(decryptedPassword));
    }

    connectionComboBox();
    connectionLineEdit();
}

void CloudServerConfig::slot_ID_userChanged()
{
    disconnectionComboBox();

    auto index_user = model_users->match(model_users->index(0, 0), Qt::UserRole, m_id_user, 1, Qt::MatchExactly);
    if (! index_user.isEmpty()){
        ui->comboUser->setCurrentIndex(index_user.first().row());
    }

    connectionComboBox();
}

void CloudServerConfig::indexChangedComboOrganization(const int index)
{
    int id_organization = ui->comboOrganization->itemData(index, Qt::UserRole).toInt();
    set_ID_Organization(id_organization);
    dataWasModified();
}

void CloudServerConfig::indexChangedComboUser(const int index)
{
    int id_user = ui->comboUser->itemData(index, Qt::UserRole).toInt();
    set_ID_user(id_user);
    dataWasModified();
}

void CloudServerConfig::saveDataClose()
{
    if (saveData())
        QDialog::accept();
}

bool CloudServerConfig::saveData()
{
    if (ui->comboOrganization->currentIndex() == 0) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat\304\203 organiza\310\233ia !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (ui->comboUser->currentIndex() == 0) {
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat utilizatorul !!!"),
                             QMessageBox::Ok);
        return false;
    }

    if (existServerConfig()) {
        if (updateDataIntoTableCloudServer()) {
            popUp->setPopupText(tr("Datele serverului cloud au fost<br>"
                                   "actualizate cu succes in baza de date."));
            popUp->show();
        }
    } else {
        if (insertDataIntoTableCloudServer()) {
            popUp->setPopupText(tr("Datele serverului cloud au fost<br>"
                                   "inserate cu succes in baza de date."));
            popUp->show();
        }
    }

    return true;
}

bool CloudServerConfig::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Verificăm dacă tasta apăsată este Enter
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // Trecem la următorul widget în lanțul de focus
            focusNextChild();
            return true; // Marchează evenimentul ca procesat
        }
    }

    return QDialog::eventFilter(obj, event);
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void CloudServerConfig::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            saveDataClose();
            event->accept();
        } else if (messange_box.clickedButton() == noButton) {
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton) {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void CloudServerConfig::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Setări cloud serverului %1").arg("[*]"));
    }
}

void CloudServerConfig::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
    }
}
