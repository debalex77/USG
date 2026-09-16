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

#include <customs/custommessage.h>

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
    // Dialogul poate fi deschis înaintea migrării 4.0.1, când view-urile noi
    // pentru combo-box-uri încă nu există. Tabelele de bază sunt disponibile
    // și în schema 3.0.7.
    QString qry_users = QStringLiteral(
        "SELECT id, name FROM users WHERE deletionMark = 0 ORDER BY name");
    model_users = new BaseSqlQueryModel(qry_users, ui->comboUser);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboUser->setModel(model_users);
    if (model_users->rowCount() > 20){
        ui->comboUser->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboUser->setStyleSheet("combobox-popup: 0;");
        ui->comboUser->setMaxVisibleItems(15);
    }

    QString qry_organizations = QStringLiteral(
        "SELECT id, name FROM organizations WHERE deletionMark = 0 ORDER BY name");
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

    connect(this, &CloudServerConfig::ID_OrganizationChanged,
            this, &CloudServerConfig::slot_ID_OrganizationChanged, Qt::UniqueConnection);
    connect(this, &CloudServerConfig::ID_userChanged,
            this, &CloudServerConfig::slot_ID_userChanged, Qt::UniqueConnection);

    connect(ui->btnOK, &QPushButton::clicked,
            this, &CloudServerConfig::saveDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QPushButton::clicked,
            this, &CloudServerConfig::saveData, Qt::UniqueConnection);
    connect(ui->btnClose, &QPushButton::clicked,
            this, &CloudServerConfig::close, Qt::UniqueConnection);
}

void CloudServerConfig::connectionLineEdit()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged,
                this, &CloudServerConfig::dataWasModified, Qt::UniqueConnection);
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
            this, QOverload<int>::of(&CloudServerConfig::indexChangedComboOrganization), Qt::UniqueConnection);
    connect(ui->comboUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&CloudServerConfig::indexChangedComboUser), Qt::UniqueConnection);
    connect(ui->comboOrganization, &QComboBox::currentIndexChanged,
            this, &CloudServerConfig::dataWasModified, Qt::UniqueConnection);
    connect(ui->comboUser, &QComboBox::currentIndexChanged,
            this, &CloudServerConfig::dataWasModified, Qt::UniqueConnection);
}

void CloudServerConfig::disconnectionComboBox()
{
    disconnect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&CloudServerConfig::indexChangedComboOrganization));
    disconnect(ui->comboUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&CloudServerConfig::indexChangedComboUser));
    disconnect(ui->comboOrganization, &QComboBox::currentIndexChanged,
               this, &CloudServerConfig::dataWasModified);
    disconnect(ui->comboUser, &QComboBox::currentIndexChanged,
               this, &CloudServerConfig::dataWasModified);
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

bool CloudServerConfig::verifyStoredPassword(const QByteArray &realKey, QString *error)
{
    if (error)
        error->clear();

    QSqlQuery qry(db->getDatabase());
    qry.prepare(QStringLiteral(
        "SELECT password, iv FROM cloudServer "
        "WHERE id_organizations = ? AND id_users = ? LIMIT 1"));
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);
    if (!qry.exec() || !qry.next()) {
        if (error)
            *error = qry.lastError().text();
        return false;
    }

    const QByteArray payload = CryptoManager::fromBase64(qry.value(0).toString());
    if (payload.size() <= 16) {
        if (error)
            *error = tr("Parola criptată salvată este incompletă.");
        return false;
    }

    CryptoManager::EncryptedData encrypted;
    encrypted.cipherText = payload.first(payload.size() - 16);
    encrypted.tag = payload.last(16);
    encrypted.iv = CryptoManager::fromBase64(qry.value(1).toString());

    bool decrypted = false;
    const QByteArray storedPassword =
        CryptoManager::decryptText(encrypted, realKey, &decrypted);
    if (!decrypted || QString::fromUtf8(storedPassword) != ui->txt_password->text()) {
        if (error)
            *error = tr("Verificarea parolei criptate după salvare a eșuat.");
        return false;
    }

    return true;
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

bool CloudServerConfig::insertDataIntoTableCloudServer()
{
    if (m_id_organization <= 0)
        return false;

    if (m_id_user <= 0)
        return false;

    QByteArray hash_user = getHashUserApp();
    if (hash_user.isEmpty())
        return false;

    const QByteArray realKey = CryptoManager::deriveCloudKey(hash_user,
                                                              m_id_organization);
    if (realKey.size() != 32) {
        qWarning(logWarning()) << "Nu s-a putut deriva cheia parolei cloud.";
        return false;
    }
    const CryptoManager::EncryptedData encrypted =
        CryptoManager::encryptText(ui->txt_password->text(), realKey);
    if (!encrypted.isValid())
        return false;
    const QByteArray encryptedPassword = encrypted.cipherText + encrypted.tag;
    QVector<QVariant> data;
    data.append(db->getLastIdForTable("cloudServer") + 1);
    data.append(m_id_organization);
    data.append(m_id_user);
    data.append(ui->txt_host->text());
    data.append(ui->txt_nameBase->text());
    data.append(ui->txt_port->text());
    data.append(ui->txt_option->text());
    data.append(ui->txt_user->text());
    data.append(CryptoManager::toBase64(encryptedPassword));
    data.append(CryptoManager::toBase64(encrypted.iv));

    QString str_err;
    if (! db->execPreparedFromFile(db->getDatabase(),
                                  ":/sql/queries/cloudServer_insert.sql",
                                  data,
                                  &str_err))
    {
        qCritical(logCritical()) << "Eroarea de inserare a datelor in tabela 'cloudServer' - SQL Error:"
                                 << str_err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor cloudServer nu s-a efectuat !!!"));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();

        return false;
    } else {
        QString verifyError;
        if (!verifyStoredPassword(realKey, &verifyError)) {
            qCritical(logCritical()) << "Verificarea parolei cloud după INSERT a eșuat:"
                                     << verifyError;
            return false;
        }
        qInfo(logInfo()) << "Inserarea datelor in tabela 'cloudServer'.";
        globals().cloud_host          = ui->txt_host->text();
        globals().cloud_nameBase      = ui->txt_nameBase->text();
        globals().cloud_port          = ui->txt_port->text();
        globals().cloud_optionConnect = ui->txt_option->text();
        globals().cloud_user          = ui->txt_user->text();
        globals().cloud_passwd        = ui->txt_password->text();
        globals().cloud_configured    = true;
        globals().cloud_srv_exist     = true;
        qInfo(logInfo()) << "Actualizate variabile globale pentru sincronizare cu serverul.";
        return true;
    }

}

bool CloudServerConfig::updateDataIntoTableCloudServer()
{
    QByteArray hash_user = getHashUserApp();
    if (hash_user.isEmpty())
        return false;

    const QByteArray realKey = CryptoManager::deriveCloudKey(hash_user,
                                                              m_id_organization);
    if (realKey.size() != 32) {
        qWarning(logWarning()) << "Nu s-a putut deriva cheia parolei cloud.";
        return false;
    }
    const CryptoManager::EncryptedData encrypted =
        CryptoManager::encryptText(ui->txt_password->text(), realKey);
    if (!encrypted.isValid())
        return false;
    const QByteArray encryptedPassword = encrypted.cipherText + encrypted.tag;
    QVector<QVariant> data;
    data.append(ui->txt_host->text());
    data.append(ui->txt_nameBase->text());
    data.append(ui->txt_port->text());
    data.append(ui->txt_option->text());
    data.append(ui->txt_user->text());
    data.append(CryptoManager::toBase64(encryptedPassword));
    data.append(CryptoManager::toBase64(encrypted.iv));
    data.append(m_id_organization);
    data.append(m_id_user);

    QString str_err;
    if (! db->execPreparedFromFile(db->getDatabase(),
                                  ":/sql/queries/cloudServer_update.sql",
                                  data,
                                  &str_err))
    {
        qCritical(logCritical()) << "Eroarea de actualizare a datelor in tabela 'cloudServer' - SQL Error:"
                                 << str_err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Actualizarea datelor cloudServer nu s-a efectuat !!!"));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();

        return false;
    } else {
        QString verifyError;
        if (!verifyStoredPassword(realKey, &verifyError)) {
            qCritical(logCritical()) << "Verificarea parolei cloud după UPDATE a eșuat:"
                                     << verifyError;
            globals().cloud_srv_exist = false;
            return false;
        }
        qInfo(logInfo()) << "Actualizarea datelor in tabela 'cloudServer'.";
        globals().cloud_host          = ui->txt_host->text();
        globals().cloud_nameBase      = ui->txt_nameBase->text();
        globals().cloud_port          = ui->txt_port->text();
        globals().cloud_optionConnect = ui->txt_option->text();
        globals().cloud_user          = ui->txt_user->text();
        globals().cloud_passwd        = ui->txt_password->text();
        globals().cloud_configured    = true;
        globals().cloud_srv_exist     = true;
        qInfo(logInfo()) << "Actualizate variabile globale pentru sincronizare cu serverul.";
        return true;
    }

}

void CloudServerConfig::dataWasModified()
{
    setWindowModified(true);
}

void CloudServerConfig::slot_ID_OrganizationChanged()
{
    disconnectionLineEdit();
    disconnectionComboBox();

    auto index_organization = model_organizations->match(model_organizations->index(0, 0),
                                                         Qt::UserRole,
                                                         m_id_organization,
                                                         1,
                                                         Qt::MatchExactly);
    if (! index_organization.isEmpty())
        ui->comboOrganization->setCurrentIndex(index_organization.first().row());

    QSqlQuery qry;
    qry.prepare(db->getTextSQL(":/sql/queries/cloudServer_select.sql"));
    qry.addBindValue(m_id_organization);
    qry.addBindValue(m_id_user);
    if (qry.exec() && qry.next()) {
        QSqlRecord rec = qry.record();
        ui->txt_host->setText(qry.value(rec.indexOf("hostName")).toString());
        ui->txt_nameBase->setText(qry.value(rec.indexOf("databaseName")).toString());
        ui->txt_port->setText(qry.value(rec.indexOf("port")).toString());
        ui->txt_option->setText(qry.value(rec.indexOf("connectionOption")).toString());
        ui->txt_user->setText(qry.value(rec.indexOf("username")).toString());

        const QByteArray payload =
            CryptoManager::fromBase64(qry.value(rec.indexOf("password")).toString());
        CryptoManager::EncryptedData encrypted;
        if (payload.size() > 16) {
            encrypted.cipherText = payload.first(payload.size() - 16);
            encrypted.tag = payload.last(16);
            encrypted.iv = CryptoManager::fromBase64(qry.value(rec.indexOf("iv")).toString());

            const QByteArray userHash = QByteArray::fromHex(
                qry.value(rec.indexOf("hashUser")).toString().toUtf8());
            const QByteArray realKey = CryptoManager::deriveCloudKey(userHash,
                                                                      m_id_organization);
            bool decrypted = false;
            if (realKey.size() == 32) {
                const QByteArray password =
                    CryptoManager::decryptText(encrypted, realKey, &decrypted);
                if (decrypted)
                    ui->txt_password->setText(QString::fromUtf8(password));
            }
            if (!decrypted)
                qWarning(logWarning()) << "Parola cloud nu a putut fi decriptată.";
        } else {
            ui->txt_password->clear();
        }
    }

    connectionComboBox();
    connectionLineEdit();
}

void CloudServerConfig::slot_ID_userChanged()
{
    disconnectionComboBox();

    auto index_user = model_users->match(model_users->index(0, 0),
                                         Qt::UserRole,
                                         m_id_user,
                                         1,
                                         Qt::MatchExactly);
    if (! index_user.isEmpty())
        ui->comboUser->setCurrentIndex(index_user.first().row());

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
            return true;
        }
    } else {
        if (insertDataIntoTableCloudServer()) {
            popUp->setPopupText(tr("Datele serverului cloud au fost<br>"
                                   "inserate cu succes in baza de date."));
            popUp->show();
            return true;
        }
    }

    QMessageBox::warning(this,
                         tr("Salvarea setărilor cloud"),
                         tr("Setările cloud nu au putut fi salvate. Verificați jurnalul aplicației."),
                         QMessageBox::Ok);
    return false;
}

bool CloudServerConfig::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Verificăm dacă tasta apăsată este Enter
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter)
        {
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
    if(event->key()==Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        this->focusNextChild();
    }
}
