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

#include "catusers.h"
#include "ui_catusers.h"

#include <customs/custommessage.h>

CatUsers::CatUsers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatUsers)
{
    ui->setupUi(this);

    db = new DataBase(this); // alocam memoria p-u salvarea datelor in BD
    popUp = new PopUp(this); // alocam memoria p-u mesaje

    //*************************** edit password ******************************

    QFile fileStyleBtn(":/styles/style_btn.css");
    fileStyleBtn.open(QFile::ReadOnly);
    QString appStyleBtn(fileStyleBtn.readAll());

    ui->editNameUser->setMaxLength(50);

    show_hide_password = new QToolButton(this);
    show_hide_password->setIcon(QIcon(":/img/password_hide.png"));
    show_hide_password->setStyleSheet(appStyleBtn);
    show_hide_password->setCursor(Qt::PointingHandCursor);
    show_hide_password->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    edit_password = new QLineEdit(this);
    edit_password->setStyleSheet(appStyleBtn);
    edit_password->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    edit_password->setEchoMode(QLineEdit::Password);
    edit_password->setMaxLength(50);
    edit_password->setPlaceholderText(tr("...lungimea maximum 50 de caractere"));

    QHBoxLayout* layout_password = new QHBoxLayout;
    layout_password->setContentsMargins(0, 0, 0, 0);
    layout_password->setSpacing(0);
    layout_password->addWidget(edit_password);
    layout_password->addWidget(show_hide_password);
    ui->editPasswdUser->setLayout(layout_password);

    setTabOrder(ui->editNameUser, edit_password); // !!! ordinea de traversare a obiectelor !!!
    setTabOrder(edit_password, ui->btnOK);

    connect(show_hide_password, &QAbstractButton::clicked, this, [this]()
    {
        if (edit_password->echoMode() == QLineEdit::Password || edit_password->echoMode() == QLineEdit::PasswordEchoOnEdit){
            show_hide_password->setIcon(QIcon(":/img/password_show.png"));
            edit_password->setEchoMode(QLineEdit::Normal);
            ui->editPasswdUser->setEchoMode(QLineEdit::Normal);
        } else {
            show_hide_password->setIcon(QIcon(":/img/password_hide.png"));
            edit_password->setEchoMode(QLineEdit::Password);
            ui->editPasswdUser->setEchoMode(QLineEdit::Password);
        }
    });
    connect(edit_password, &QLineEdit::textChanged, this, &CatUsers::textChangedPasswd);

    //******************************************************************************

    if (globals().firstLaunch){
        ui->editNameUser->setText("admin");
        ui->editNameUser->setEnabled(false);
        popUp->setPopupText(tr("Pentru lucru cu baza de date \"<b>%1</b>\" <br>este necesar de creat primul utilizator.")
                            .arg((globals().thisMySQL == true) ? globals().mySQLnameBase : globals().sqliteNameBase));
        popUp->show();
    }

    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(this, &CatUsers::ItNewChanged, this, &CatUsers::slot_ItNewChanged);
    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatUsers::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatUsers::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatUsers::onClose);

    connect(ui->editNameUser, &QLineEdit::textEdited, this, &CatUsers::dataWasModified);
    connect(edit_password, &QLineEdit::textEdited, this, &CatUsers::dataWasModified);

    connect(this, &CatUsers::IdChanged, this, &CatUsers::changedIdObject);  // modificarea proprietatilor
    connect(this, &CatUsers::ItNewChanged, this, &CatUsers::slot_ItNewChanged);
}

CatUsers::~CatUsers()
{
    delete db;
    delete popUp;
    delete ui;
}

void CatUsers::dataWasModified()
{
    setWindowModified(true);
}

void CatUsers::slot_ItNewChanged()
{
    if (m_itNew){
        if (globals().firstLaunch){
            setWindowTitle(tr("Crearea administratorului aplicației %1").arg("[*]"));
        } else {
            setWindowTitle(tr("Utilizator (crearea) %1").arg("[*]"));
        }
    } else {
        setWindowTitle(tr("Utilizator (salvat) %1").arg("[*]"));
    }
}

void CatUsers::textChangedPasswd()
{
    if (! edit_password->text().isEmpty())
        edit_password->setPlaceholderText("");
}

bool CatUsers::onWritingData()
{
    if (! db->getDatabase().isOpen())
        db->connectToDataBase();

    if (m_Id == -1){
        // setam id
        setId(db->getLastIdForTable("users") + 1);
        if (m_Id == -1 || m_Id == 0){
            qWarning(logWarning()) << this->metaObject()->className()
                                   << tr(": eroarea la atasarea 'id' utilizatorului '%1'.").arg(ui->editNameUser->text());
            QMessageBox::warning(this,
                                 tr("Crearea utilizatorului."),
                                 tr("Eroare la crearea utilizatorului <b>\"%1\"</b>.<br>"
                                    "Adresați-vă administratorului aplicației.")
                                 .arg(ui->editNameUser->text()), QMessageBox::Ok);
            return false;
        }

        // verificam NUME daca este in BD
        if (db->existNameObject("users", ui->editNameUser->text())){
            QMessageBox::warning(this,
                                 tr("Verificarea datelor."),
                                 tr("Utilizatorul cu nume \"<b>%1</b>\" există în baza de date.<br>"
                                    "Alegeți alt nume a utilizatorului pentru validare.")
                                 .arg(ui->editNameUser->text()), QMessageBox::Ok);
            return false;
        }

        // crearea utilizatorului
        QString details_error;
        if (insertDataTableUsers(details_error)){
            qInfo(logInfo()) << tr("A fost creat %1")
                                .arg((globals().firstLaunch) ?
                                             "primul utilizator cu nume 'admin'." :
                                             "utilizator cu nume '" + ui->editNameUser->text() + "'.");
        } else {
            CustomMessage *msgBox = new CustomMessage(this);
            msgBox->setWindowTitle(tr("Crearea utilizatorului"));
            msgBox->setTextTitle(tr("Validarea datelor utilizatorului '%1' nu s-a efectuat.").arg(ui->editNameUser->text()));
            msgBox->setDetailedText((details_error.isEmpty()) ? tr("eroarea indisponibila") : details_error);
            msgBox->exec();
            msgBox->deleteLater();
            return false;
        }
        // setam variabile globale
        if (globals().firstLaunch){
            globals().idUserApp   = m_Id;        // id primului utilizator (fara + 1 = a fost inscris primul users)
            globals().nameUserApp = "admin";     // nume primului utilizator
            db->insertSetTableSettingsUsers();  // setarile utilizatorului
        } else {
            db->insertSetTableSettingsUsers(); // setarile utilizatorului
        }

        setItNew(false);

        if (m_name_user == nullptr)
            setNameUser(ui->editNameUser->text());

        emit mCreateNewUser();

    } else {

        // verificam daca a fost transmis proprietatea ID utilizatorului
        if (m_Id == - 1){
            qWarning(logWarning()) << this->metaObject()->className()
                                   << tr(": nu este transmisa/indicata proprietatea <<-Id->> clasei %1").arg(metaObject()->className());
            QMessageBox::warning(this,
                                 tr("Modificarea datelor."),
                                 tr("Modificarea datelor utilizatorului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresați-vă administratorului aplicației.")
                                 .arg(ui->editNameUser->text()), QMessageBox::Ok);
            return false;
        }
        // modificam datele utilizatorului
        if (updateDataTableUsers()){
            qInfo(logInfo()) << tr("Datele utilizatorului cu nume '%1' id='%2' au fost actualizate.")
                                .arg(ui->editNameUser->text(), QString::number(m_Id));
        } else {
            QMessageBox::warning(this,
                                 tr("Modificarea datelor."),
                                 tr("Modificarea datelor utilizatorului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresați-vă administratorului aplicației.")
                                 .arg(ui->editNameUser->text()), QMessageBox::Ok);
            return false;
        }

        emit mChangedDataUser();
    }
    setItNew(false);

    return true;
}

void CatUsers::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

void CatUsers::onClose()
{
    this->close();
}

void CatUsers::changedIdObject()
{
    if (m_itNew)
        return;

    QMap<QString, QString> _items;
    if (db->getObjectDataById("users", m_Id, _items)){
        QMap<QString, QString>::const_iterator _iterName = _items.constFind("name");
        if (_iterName.key() == "name"){
            ui->editNameUser->setText(_iterName.value());
        }
        QMap<QString, QString>::const_iterator _iterPasswd = _items.constFind("password");
        if (_iterPasswd.key() == "password"){
            edit_password->setText(db->decode_string(_iterPasswd.value()));
        }
    }

    setNameUser(ui->editNameUser->text());
}

bool CatUsers::insertDataTableUsers(QString &details_error)
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO users("
                "id, deletionMark, name, password, hash, lastConnection) "
                "VALUES (?, ?, ?, ?, ?, ?);");
    qry.addBindValue(m_Id);
    qry.addBindValue(0);
    qry.addBindValue((globals().firstLaunch) ? "admin" : ui->editNameUser->text());
    qry.addBindValue(QVariant());
    qry.addBindValue(QCryptographicHash::hash(edit_password->text().toUtf8(), QCryptographicHash::Sha256).toHex());
    qry.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if (qry.exec()){
        return true;
    } else {
        details_error = qry.lastError().text();
        return false;
    }
}

bool CatUsers::updateDataTableUsers()
{
    QString str_qry = "UPDATE users SET "
                      "deletionMark = :deletionMark, "
                      "name         = :name, "
                      "password     = :password, "
                      "hash         = :hash "
                      "WHERE id = :id;";
    QSqlQuery qry;
    qry.prepare(str_qry);
    qry.bindValue(":id",           QString::number(m_Id));
    qry.bindValue(":deletionMark", QString::number(0));
    qry.bindValue(":name",         ui->editNameUser->text());
    qry.bindValue(":password",     QVariant());
    qry.bindValue(":hash",         QCryptographicHash::hash(edit_password->text().toUtf8(), QCryptographicHash::Sha256).toHex());
    if (qry.exec()){
        return true;
    } else {
        qCritical(logCritical()) << tr("Eroare la actualizare datelor utilizatorului '%1' cu id='%2': %3")
                                    .arg(ui->editNameUser->text(), QString::number(m_Id), qry.lastError().text());
        return false;
    }
}

void CatUsers::closeEvent(QCloseEvent *event)
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

        if (messange_box.clickedButton() == yesButton){
            onWritingDataClose();
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

void CatUsers::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
    }
}

void CatUsers::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Tab){
      this->focusNextChild();
    }
}
