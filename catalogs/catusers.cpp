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
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatUsers::close);

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

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

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

void CatUsers::changedIdObject()
{
    if (m_itNew)
        return;

    QVariantMap map_column {
        { "name",  QVariant() },
        { "lastConnection", QVariant() }
    };

    // container cu conditia
    QMap<QString, QVariant> where;
    where["id"] = m_Id;

    err.clear();

    QVariantMap map_result = db->selectSingleRow(this->metaObject()->className(),
                                                 "users", map_column, where, err);

    if (! map_result.isEmpty()) {
        ui->editNameUser->setText(map_result["name"].toString());
        ui->label_info->setText(tr("Ultima accesare: ") +
                                map_result["lastConnection"]
                                .toDateTime().toString("dd.MM.yyyy hh:mm:ss"));
    }
    setNameUser(ui->editNameUser->text());
}

// *******************************************************************
// **************** VALIDAREA DATELOR ********************************

bool CatUsers::onWritingData()
{
    if (! db->getDatabase().isOpen())
        db->connectToDataBase();

    bool returnBool;

    returnBool = m_itNew
                ? handleInsert()
                : handleUpdate();

    if (returnBool) {
        if (m_itNew) {
            emit mCreateNewUser(); // emitem signal ca a fost creat user nou
            setItNew(false);       // modificam proprietatea
        } else {
            emit mChangedDataUser(); // emitem signal ca au fost modificate date user-lui
        }
    }

    return returnBool;
}

void CatUsers::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

// *******************************************************************
// ********** PROCESAREA INSERARII, MODIFICARII DATELOR **************

QVariantMap CatUsers::getDataObject()
{
    QVariantMap map;

    map["id"]           = m_Id;
    map["deletionMark"] = 0;
    map["name"]         = ui->editNameUser->text();
    map["password"]     = QVariant();
    map["hash"]         = QCryptographicHash::hash(edit_password->text().toUtf8(), QCryptographicHash::Sha256).toHex();
    map["lastConnection"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    return map;
}

bool CatUsers::handleInsert()
{
    // 1. verificam user cu acelasi NUME in BD
    if (db->existNameObject("users", ui->editNameUser->text())){
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Verificarea datelor."));
        msg->setTextTitle(tr("Utilizatorul cu nume \"<b>%1</b>\" exist\304\203 în baza de date")
                          .arg(ui->editNameUser->text()));
        msg->setDetailedText(tr("Alege\310\233i alt nume a utilizatorului pentru validare."));
        msg->exec();
        msg->deleteLater();
        return false;
    }

    // 2. setam ID
    setId(db->getLastIdForTable("users") + 1);

    // 3. verificam daca este corect determinat ID
    if (m_Id <= 0) {
        err << this->metaObject()->className()
        << "[handleInsert]:"
        << "Nu este determinat ID utilizatorului !!!";
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Verificarea datelor."));
        msg->setTextTitle(tr("Nu este determinat ID utilizatorului !!!")
                              .arg(ui->editNameUser->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
        qWarning(logWarning()) << err;
        return false;
    }

    // 4. pregatim datele
    QVariantMap map = getDataObject();

    // 5. inseram datele
    if (db->insertIntoTable(this->metaObject()->className(), "users", map, err)){
        popUp->setPopupText(tr("Datele utilizatorului <b>%1</b><br>"
                               "au fost salvate in baza de date.")
                            .arg(ui->editNameUser->text()));
        popUp->show();
        qInfo(logInfo()) << QStringLiteral("Datele utilizatorului %1 au fost salvate in baza de date.")
                                .arg(ui->editNameUser->text());
    } else {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Inserarea datelor."));
        msg->setTextTitle(tr("Datele utilizatorului '%1' nu au fost salvate in baza de date")
                              .arg(ui->editNameUser->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
        qCritical(logCritical()) << QStringLiteral("Datele utilizatorului %1 nu au fost salvate in baza de date %2")
                                        .arg(ui->editNameUser->text(), err.join("\n"));
        return false;
    }

    // 6. setam variabile globale + salvam in fisierul .conf
    if (globals().firstLaunch) {

        // variabile globale pu titlu mainwindow
        globals().idUserApp = m_Id;
        globals().nameUserApp = ui->editNameUser->text();

        // ID si nume utilizatorului cryptat
        QString id_encode        = db->encode_string(QString::number(m_Id));
        QString name_user_encode = db->encode_string(ui->editNameUser->text());

        // salvam in fisierul .conf
        AppSettings *app_settings = new AppSettings(this);
        app_settings->setKeyAndValue("on_start", "idUserApp", id_encode);
        app_settings->setKeyAndValue("on_start", "nameUserApp", name_user_encode);
        app_settings->setKeyAndValue("on_start", "memoryUser", 1);
        app_settings->deleteLater();
    }

    // 7. inseram datele in tabele 'userPreferences'
    db->insertSetTableSettingsUsers();

    return true;

}

bool CatUsers::handleUpdate()
{
    // 1. verificam daca este corect determinat ID
    if (m_Id <= 0) {
        err << this->metaObject()->className()
        << "[handleUpdate]:"
        << "Nu este determinat ID utilizatorului !!!";
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Verificarea datelor."));
        msg->setTextTitle(tr("Nu este determinat ID utilizatorului !!!")
                              .arg(ui->editNameUser->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
        qWarning(logWarning()) << err;
        return false;
    }

    // 2. pregatim datele
    QVariantMap map = getDataObject();

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id"] = m_Id;
    if (m_Id > 0)
        where["id"] = m_Id;

    // actualizam datele
    if (db->updateTable(this->metaObject()->className(), "users", map, where, err)) {
        popUp->setPopupText(tr("Datele utilizatorului <b>%1</b><br>"
                               "au fost modificate in baza de date.")
                                .arg(ui->editNameUser->text()));
        popUp->show();
        qInfo(logInfo()) << QStringLiteral("Datele utilizatorului %1 au fost modificate in baza de date.")
                                .arg(ui->editNameUser->text());
    } else {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Modificare datelor."));
        msg->setTextTitle(tr("Datele utilizatorului '%1' nu au fost modificate in baza de date")
                              .arg(ui->editNameUser->text()));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
        qCritical(logCritical()) << QStringLiteral("Datele utilizatorului %1 nu au fost modificate in baza de date %2")
                                        .arg(ui->editNameUser->text(), err.join("\n"));
        return false;
    }

    return true;
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

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
