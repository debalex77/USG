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

#include "authorizationuser.h"
#include "data/appsettings.h"
#include "ui_authorizationuser.h"

AuthorizationUser::AuthorizationUser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorizationUser)
{
    ui->setupUi(this);

    setWindowTitle(tr("Autorizarea utilizatorului"));
    setWindowIcon(QIcon(":/img/autorization_x32.png"));
    db = new DataBase(this);

    //*************************** edit password ******************************

    QFile fileStyleBtn(":/styles/style_btn.css");
    fileStyleBtn.open(QFile::ReadOnly);
    QString appStyleBtn(fileStyleBtn.readAll());

    ui->editLogin->setMaxLength(50);

    show_hide_password = new QToolButton(this);
    show_hide_password->setIcon(QIcon(":/img/lock.png"));
    show_hide_password->setStyleSheet(appStyleBtn);
    show_hide_password->setCursor(Qt::PointingHandCursor);
    show_hide_password->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    edit_password = new QLineEdit(this);
    edit_password->setStyleSheet(appStyleBtn);
    edit_password->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    edit_password->setEchoMode(QLineEdit::Password);
    edit_password->setMaxLength(50);
    edit_password->setPlaceholderText(tr("...maximum 50 caractere"));

    QHBoxLayout* layout_password = new QHBoxLayout;
    layout_password->setContentsMargins(0, 0, 0, 0);
    layout_password->setSpacing(0);
    layout_password->addWidget(edit_password);
    layout_password->addWidget(show_hide_password);
    ui->editPasswd->setLayout(layout_password);
    ui->editPasswd->setEchoMode(QLineEdit::Password);

    setTabOrder(ui->editLogin, edit_password);
    setTabOrder(edit_password, ui->btnOK);

    connect(show_hide_password, &QAbstractButton::clicked, this, [this]()
    {
        if (edit_password->echoMode() == QLineEdit::Password || edit_password->echoMode() == QLineEdit::PasswordEchoOnEdit){
            show_hide_password->setIcon(QIcon(":/img/unlock.png"));
            edit_password->setEchoMode(QLineEdit::Normal);
            ui->editPasswd->setEchoMode(QLineEdit::Normal);
        } else {
            show_hide_password->setIcon(QIcon(":/img/lock.png"));
            edit_password->setEchoMode(QLineEdit::Password);
            ui->editPasswd->setEchoMode(QLineEdit::Password);
        }
    });

    connect(edit_password, &QLineEdit::textChanged, this, &AuthorizationUser::textChangedPasswd);

    connect(ui->btnOK, &QAbstractButton::clicked, this, &AuthorizationUser::onAccepted);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &AuthorizationUser::onClose);

    connect(this, &AuthorizationUser::IdChanged, this, &AuthorizationUser::slot_IdChanged);
}

AuthorizationUser::~AuthorizationUser()
{
    delete db;
    delete ui;
}

DatabaseProvider *AuthorizationUser::dbProvider()
{
    return &m_dbProvider;
}

void AuthorizationUser::setDataConstants()
{
    // 1. Creăm thread-ul pentru trimiterea emailului
    QThread *thread = new QThread();

    // 2. Setam date necesare pentru procesarea
    DataConstantsWorker::GeneralData data;
    data.thisMySQL       = globals().thisMySQL;
    data.id_user         = globals().idUserApp;
    data.id_doctor       = globals().c_id_doctor;
    data.id_organization = globals().c_id_organizations;

    auto worker = new DataConstantsWorker(dbProvider(), data);

    // 3. mutal in flux nou
    worker->moveToThread(thread);

    // 4. conectarea pentru procesare si emiterea signalului de finisare
    connect(thread,  &QThread::started,  worker, &DataConstantsWorker::process);
    connect(worker, &DataConstantsWorker::finished, this, &AuthorizationUser::onDataReceived, Qt::QueuedConnection);

    // 5. clean‑up worker & thread
    connect(worker, &DataConstantsWorker::finished, thread, &QThread::quit);
    connect(worker, &DataConstantsWorker::finished, worker, &QObject::deleteLater);
    connect(thread,  &QThread::finished, thread, &QObject::deleteLater);

    // 6. Pornim thread-ul
    thread->start();
}

void AuthorizationUser::slot_IdChanged()
{
    if (globals().idUserApp <= 0)
        return;


    QVariantMap map_column {
        { "name",  QVariant() },
        { "lastConnection", QVariant() }
    };

    // container cu conditia
    QMap<QString, QVariant> where;
    where["id"] = globals().idUserApp;

    err.clear();

    QVariantMap map_result = db->selectSingleRow(this->metaObject()->className(),
                                                 "users", map_column, where, err);

    if (! map_result.isEmpty()) {
        ui->checkBoxMemory->setChecked(globals().memoryUser);
        if (globals().memoryUser) {
            ui->editLogin->setText(map_result["name"].toString());
            ui->label_info->setText(tr("Ultima accesare: ") +
                                    map_result["lastConnection"]
                                    .toDateTime().toString("dd.MM.yyyy hh:mm:ss"));
        } else {
            ui->label_info->setVisible(false);
            this->adjustSize();
        }
    }
}

void AuthorizationUser::textChangedPasswd()
{
    if (! edit_password->text().isEmpty())
        edit_password->setPlaceholderText(tr(""));
}

void AuthorizationUser::onDataReceived()
{
    qInfo(logInfo()) << "Actualizate variabile globale: "
                        "constante, "
                        "datele organizatiei, "
                        "datele doctorului, "
                        "datele cloud serverului.";

    /**** !!! NOTA !!! *****************************************************
     *
     *  Distrugem manual dupa ce au fost finisat setarea variabilor globale
     *  si dupa emiterea signalului DataConstantsWorker::finished
     *
    **********************************************************************/
    this->deleteLater();
}

bool AuthorizationUser::onControlAccept()
{
    //*****************************************************************************************
    // 1. Verificam daca este complectat login-ul
    if (ui->editLogin->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat <b>Login</b> !!!"
                                "<br>Accesul este interzis."), QMessageBox::Ok);
        qWarning(logWarning()) << tr("Incercarea accesului în aplicația fără indicarea numelui utilizatorului !!!");
        return false;
    }

    //*****************************************************************************************
    // 2. Determinam daca este utilizatorul dupa 'Nume' in BD si returnam ID utilizatorului
    if (! db->getDatabase().open())
        db->connectToDataBase();

    if (! db->existNameObjectReturnId("users", ui->editLogin->text(), m_Id)){
        QMessageBox::warning(this,
                             tr("Controlul accesului"),
                             tr("Utilizatorul cu nume <b>%1</b> nu a fost depistat in baza de date !!!<br>"
                                "Accesul este interzis.")
                             .arg(ui->editLogin->text()), QMessageBox::Ok);
        qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                               << tr("Accesul la aplicație. Utilizatorul cu nume '%1' nu a fost depistat in baza de date.")
                                  .arg(ui->editLogin->text());
        return false;
    }

    //*****************************************************************************************
    // 3. Verificam daca a fost extras ID utilizatorului
    if (m_Id == -1 || m_Id == 0){
        QMessageBox::warning(this,
                             tr("Controlul accesului"),
                             tr("Utilizatorul <b>%1</b> a fost depistat in baza de date, <b>'id'</b> "
                                "utilizatorului nu a fost determinat (sau determinat incorect) !!!"
                                "<br>Adresați-vă administratorului aplicației.>"
                                "<br>Accesul este interzis.")
                             .arg(ui->editLogin->text()), QMessageBox::Ok);
        qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                               << tr("Accesul la aplicație. Utilizatorul cu nume '%1' a fost depistat in baza de date, "
                                     "dar 'id' utilizatorului nu a fost determinat.")
                                  .arg(ui->editLogin->text());
        return false;
    }

    //*****************************************************************************************
    // 4. Extragem datele utilizatorului inclusiv parola
    QMap<QString, QString> items;
    if (m_Id != -1 && ! db->getObjectDataById("users", m_Id, items)){
        QMessageBox::warning(this,
                             tr("Controlul accesului"),
                             tr("Utilizatorul <b>%1</b> cu <b>id='%2'</b> "
                                "nu este depistat în baza de date !!!")
                             .arg(items.constFind("name").value(), items.constFind("id").value()),
                             QMessageBox::Ok);
        qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                               << tr("Accesul la aplicație. Utilizatorul '%1' cu id='%2' nu a fost depistat in baza de date.")
                                  .arg(ui->editLogin->text(), QString::number(m_Id));
        return false;
    }

    //*****************************************************************************************
    // 5. Decriptarea parolei + verificarea corectitudinei parolei
    if (QCryptographicHash::hash(edit_password->text().toUtf8(), QCryptographicHash::Sha256).toHex() != items.constFind("hash").value()){
        QMessageBox::warning(this,
                             tr("Controlul accesului"),
                             tr("Parola utilizatorului <b>'%1'</b> este incorectă !!!<br> "
                                "Accesul este interzis.")
                                 .arg(ui->editLogin->text()),
                             QMessageBox::Ok);
        qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                               << tr("Accesul la aplicație. Utilizatorul '%1' cu id='%2' - întroducerea parolei incorecte.")
                                  .arg(ui->editLogin->text(), QString::number(m_Id));
        return false;
    }

    //*****************************************************************************************
    // 6. Setam variabile globale necesare
    globals().idUserApp   = m_Id;
    globals().nameUserApp = items.constFind("name").value();
    globals().memoryUser  = ui->checkBoxMemory->isChecked();

    // ---- ID si nume utilizatorului cryptat
    QString id_encode        = db->encode_string(QString::number(m_Id));
    QString name_user_encode = db->encode_string(ui->editLogin->text());

    // ---- salvam in fisierul .conf
    AppSettings *app_settings = new AppSettings(this);
    app_settings->setKeyAndValue("on_start", "idUserApp", id_encode);
    app_settings->setKeyAndValue("on_start", "nameUserApp", name_user_encode);
    app_settings->setKeyAndValue("on_start", "memoryUser", ui->checkBoxMemory->isChecked() ? 1 : 0);
    app_settings->deleteLater();

    QSqlQuery qry;
    qry.prepare(QString("UPDATE users SET lastConnection = '%1' WHERE id = '%2' AND deletionMark = '0';")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                         QString::number(m_Id)));
    if (! qry.exec())
        qInfo(logInfo()) << tr("Nu este inregistrat timpul si data conectarii utilizatorului - %1").arg(qry.lastError().text());


    qInfo(logInfo()) << tr("Accesul la aplicatia. Autorizarea reusita a utilizatorului '%1' cu id='%2'.")
                        .arg(ui->editLogin->text(), QString::number(m_Id));

    //*****************************************************************************************
    // 7. setam datele constantelor
    setDataConstants();

    return true;
}

void AuthorizationUser::onAccepted()
{
    if (onControlAccept())
        QDialog::accept();
}

void AuthorizationUser::onClose()
{
    this->close();
}

void AuthorizationUser::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Autorizarea utilizatorului"));
    }
}

void AuthorizationUser::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Tab){
      this->focusNextChild();
    }
}
