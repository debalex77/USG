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

AuthorizationUser::AuthorizationUser(DataBase &db,
                                     QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorizationUser)
    , m_db(db)
{
    ui->setupUi(this);

    setWindowTitle(tr("Autorizarea utilizatorului"));
    setWindowIcon(QIcon(":/img/catalogs/autorization.png"));

    //*************************** edit password ******************************

    QFile fileStyleBtn(":/styles/style_btn.css");
    fileStyleBtn.open(QFile::ReadOnly);
    QString appStyleBtn(fileStyleBtn.readAll());

    ui->editLogin->setMaxLength(50);

    show_hide_password = new QToolButton(this);
    show_hide_password->setIcon(QIcon(":/img/common/lock.png"));
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
        if (edit_password->echoMode() == QLineEdit::Password ||
            edit_password->echoMode() == QLineEdit::PasswordEchoOnEdit)
        {
            show_hide_password->setIcon(QIcon(":/img/common/unlock.png"));
            edit_password->setEchoMode(QLineEdit::Normal);
            ui->editPasswd->setEchoMode(QLineEdit::Normal);
        } else {
            show_hide_password->setIcon(QIcon(":/img/common/lock.png"));
            edit_password->setEchoMode(QLineEdit::Password);
            ui->editPasswd->setEchoMode(QLineEdit::Password);
        }
    });

    connect(edit_password, &QLineEdit::textChanged,
            this, &AuthorizationUser::textChangedPasswd, Qt::UniqueConnection);

    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &AuthorizationUser::onAccepted, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &AuthorizationUser::onClose, Qt::UniqueConnection);

    connect(this, &AuthorizationUser::IdChanged,
            this, &AuthorizationUser::slot_IdChanged, Qt::UniqueConnection);
}

AuthorizationUser::~AuthorizationUser()
{
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
    connect(worker, &DataConstantsWorker::finished,
            this, &AuthorizationUser::onDataReceived, Qt::QueuedConnection);

    // 5. clean‑up worker & thread
    connect(worker, &DataConstantsWorker::finished, thread, &QThread::quit);
    connect(worker, &DataConstantsWorker::finished, worker, &QObject::deleteLater);
    connect(thread,  &QThread::finished, thread, &QObject::deleteLater);

    // 6. Pornim thread-ul
    thread->start();
}

void AuthorizationUser::slot_IdChanged()
{
    ui->checkBoxMemory->setChecked(globals().memoryUser);

    // Numele memorat provine din configurația aplicației și nu trebuie să
    // depindă de interogarea auxiliară pentru data ultimei conectări.
    if (globals().memoryUser && !globals().nameUserApp.trimmed().isEmpty())
        ui->editLogin->setText(globals().nameUserApp);

    if (m_Id <= 0)
        return;

    QString str_qry =
        globals().thisSqlite
            ? m_db.getTextSQL(":/sql/queries/users_by_id_select_sqlite.sql")
            : m_db.getTextSQL(":/sql/queries/users_by_id_select_mariadb.sql");

    QSqlQuery qry(m_db.getDatabase());
    if (!qry.prepare(str_qry)) {
        ui->label_info->setVisible(false);
        qCritical(logCritical())
            << QStringLiteral("Nu poate fi pregătită citirea utilizatorului memorat cu ID '%1': %2")
                   .arg(m_Id)
                   .arg(qry.lastError().text());
        return;
    }
    qry.addBindValue(m_Id);
    if (qry.exec() && qry.next()) {
        if (globals().memoryUser)
            ui->label_info->setText(tr("Ultima conexiune: ") + qry.value(UsersSections::LastConnection).toString());
        qInfo(logInfo())
            << QStringLiteral("Citite date utilizatorului memorat cu ID '%1'")
                   .arg(m_Id);
    } else {
        ui->label_info->setVisible(false);
        this->adjustSize();
        qCritical(logCritical())
            << QStringLiteral("Nu sunt determinate date utilizatorului cu ID '%1'")
                   .arg(m_Id);
        qCritical(logCritical()) << qry.lastQuery();
        qCritical(logCritical()) << qry.lastError().text();
    }

}

void AuthorizationUser::textChangedPasswd()
{
    if (! edit_password->text().isEmpty())
        edit_password->setPlaceholderText(tr(""));
}

void AuthorizationUser::onDataReceived(bool success)
{
    m_loadingData = false;
    ui->btnOK->setEnabled(true);
    ui->btnCancel->setEnabled(true);
    ui->editLogin->setEnabled(true);
    edit_password->setEnabled(true);

    if (!success) {
        QMessageBox::critical(this,
                              tr("Inițializarea aplicației"),
                              tr("Datele necesare aplicației nu au putut fi încărcate. "
                                 "Verificați conexiunea și jurnalul aplicației."),
                              QMessageBox::Ok);
        return;
    }

    qInfo(logInfo()) << "Actualizate variabile globale: "
                        "constante, "
                        "datele organizatiei, "
                        "datele doctorului, "
                        "datele cloud serverului.";

    QDialog::accept();
}

bool AuthorizationUser::onControlAccept()
{
    // Verificam daca este complectat login-ul
    if (ui->editLogin->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este indicat <b>Login</b> !!!"
                                "<br>Accesul este interzis."), QMessageBox::Ok);
        qWarning(logWarning()) << tr("Incercarea accesului în aplicația fără indicarea numelui utilizatorului !!!");
        return false;
    }

    // Protecție suplimentară: autorizarea nu trebuie să execute interogări
    // dacă fluxul de lansare nu a reușit să deschidă baza de date.
    QSqlDatabase database = m_db.getDatabase();
    if (!database.isValid() || !database.isOpen()) {
        if (!m_db.connectToDataBase()) {
            QMessageBox::critical(this,
                                  tr("Conectarea la baza de date"),
                                  tr("Baza de date nu este deschisă. Autorizarea nu poate continua."),
                                  QMessageBox::Ok);
            qCritical(logCritical())
                << tr("%1 - onControlAccept(): baza de date nu este deschisă.")
                       .arg(metaObject()->className());
            return false;
        }
        database = m_db.getDatabase();
    }

    // extragem datele utilizatorului
    QSqlQuery qry(database);
    qry.prepare("SELECT * FROM users WHERE name = ? AND deletionMark = 0");
    qry.addBindValue(ui->editLogin->text());
    if (qry.exec() && qry.next()) {
        const int authenticatedUserId = qry.value(UsersSections::Id).toInt();

        // verificam daca hash parolei = cu hash-ul din bd
        const QString pwd_hex = QString::fromLatin1(
            QCryptographicHash::hash(edit_password->text().toUtf8(), QCryptographicHash::Sha256).toHex()
            );

        const QString db_hash = qry.value(UsersSections::Hash).toString();

        if (pwd_hex != db_hash){
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

        // Actualizăm ID-ul numai după validarea parolei. Astfel o încercare
        // nereușită nu reîncarcă inutil datele utilizatorului memorat.
        m_Id = authenticatedUserId;

        // Setam variabile globale necesare
        globals().idUserApp   = m_Id;
        globals().nameUserApp = ui->editLogin->text();
        globals().memoryUser  = ui->checkBoxMemory->isChecked();

        // Salvăm atomic datele utilizatorului memorat, fără construirea dialogului
        // complet AppSettings.
        if (!AppSettings::saveRememberedUser(m_Id, ui->editLogin->text(),
                                             ui->checkBoxMemory->isChecked())) {
            qWarning(logWarning())
                << tr("Preferința de memorare a utilizatorului nu a putut fi salvată.");
        }

        // actualizam timpul si data conectarii utilizatorului
        qry.prepare("UPDATE users SET lastConnection = ? WHERE id = ? AND deletionMark = 0;");
        qry.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        qry.addBindValue(m_Id);
        if (! qry.exec())
            qInfo(logInfo()) << tr("Nu este inregistrat timpul si data conectarii utilizatorului '%1': %2")
                                    .arg(ui->editLogin->text(),
                                         qry.lastError().text());

    } else {
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

    qInfo(logInfo()) << tr("Accesul la aplicatia. Autorizarea reusita a utilizatorului '%1' cu id='%2'.")
                        .arg(ui->editLogin->text(), QString::number(m_Id));

    // setam datele constantelor
    setDataConstants();

    return true;
}

void AuthorizationUser::onAccepted()
{
    if (m_loadingData)
        return;

    if (onControlAccept()) {
        m_loadingData = true;
        ui->btnOK->setEnabled(false);
        ui->btnCancel->setEnabled(false);
        ui->editLogin->setEnabled(false);
        edit_password->setEnabled(false);
    }
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
    if(event->key()==Qt::Key_Return ||
        event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Tab) {
      this->focusNextChild();
    }
}
