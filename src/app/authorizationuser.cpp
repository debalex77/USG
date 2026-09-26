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
#include <ui/widgets/balloontip.h>
#include "common/cloudconnectioncontext.h"
#include "common/sessioncontext.h"
#include "settings/appsettings.h"
#include "settings/settingsservice.h"
#include "ui_authorizationuser.h"

#include <QDateTime>

AuthorizationUser::AuthorizationUser(DataBase &db,
                                     QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorizationUser)
    , m_db(db)
{
    qRegisterMetaType<OrganizationContextData>();
    qRegisterMetaType<DoctorContextData>();
    qRegisterMetaType<CloudConnectionData>();

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
    m_lastConnectionTimer.setSingleShot(true);
    m_lastConnectionTimer.setInterval(180);
    connect(&m_lastConnectionTimer, &QTimer::timeout,
            this, &AuthorizationUser::updateLastConnectionForLogin);
    connect(ui->editLogin, &QLineEdit::textChanged,
            this, [this]() {
                ui->label_info->hide();
                m_lastConnectionTimer.start();
            });
    connect(ui->editLogin, &QLineEdit::editingFinished,
            this, [this]() {
                m_lastConnectionTimer.stop();
                updateLastConnectionForLogin();
            });

    ui->label_info->hide();
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
    const Settings::OrganizationSettings organization =
        SettingsService::instance().organization();
    data.thisMySQL       = MainDatabaseConnectionContext::instance().isMariaDb();
    data.id_user         = SessionContext::instance().userId();
    data.id_doctor       = organization.defaultDoctorId;
    data.id_organization = organization.organizationId;

    auto worker = new DataConstantsWorker(dbProvider(), data);

    // 3. mutal in flux nou
    worker->moveToThread(thread);

    // 4. conectarea pentru procesare si emiterea signalului de finisare
    connect(thread,  &QThread::started,  worker, &DataConstantsWorker::process);
    connect(worker, &DataConstantsWorker::finished,
            this,
            [](bool,
               const CloudConnectionData &cloudConnection,
               int organizationId,
               int defaultDoctorId,
               int defaultNurseId,
               const QString &ultrasoundDeviceName,
               const QByteArray &logoData,
               const OrganizationContextData &organizationData,
               const DoctorContextData &doctorData) {
                Settings::SynchronizationSettings synchronization;
                synchronization.configured = cloudConnection.configured;
                synchronization.enabled = cloudConnection.enabled;
                SettingsService::instance().setSynchronization(synchronization);
                CloudConnectionContext::instance().setData(cloudConnection);

                Settings::OrganizationSettings organization;
                organization.organizationId = organizationId;
                organization.defaultDoctorId = defaultDoctorId;
                organization.defaultNurseId = defaultNurseId;
                organization.ultrasoundDeviceName = ultrasoundDeviceName;
                organization.logoData = logoData;
                SettingsService::instance().setOrganization(organization);
                OrganizationContext::instance().setData(organizationData);
                DoctorContext::instance().setData(doctorData);
            },
            Qt::QueuedConnection);
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

    if (globals().memoryUser && !globals().nameUserApp.trimmed().isEmpty())
        ui->editLogin->setText(globals().nameUserApp);

    if (m_Id <= 0 || !globals().memoryUser)
        return;

    refreshLastConnection(m_Id);
}

void AuthorizationUser::updateLastConnectionForLogin()
{
    refreshLastConnection();
}

void AuthorizationUser::refreshLastConnection(int userId)
{
    ui->label_info->hide();

    const QString login = ui->editLogin->text().trimmed();
    if (userId <= 0 && login.isEmpty())
        return;

    const QSqlDatabase database = m_db.getDatabase();
    if (!database.isValid() || !database.isOpen())
        return;

    QSqlQuery query(database);
    const QString statement = userId > 0
        ? QStringLiteral(R"(
            SELECT name, lastConnection
            FROM users
            WHERE id = ? AND deletionMark = 0
        )")
        : QStringLiteral(R"(
            SELECT name, lastConnection
            FROM users
            WHERE name = ? AND deletionMark = 0
        )");
    if (!query.prepare(statement)) {
        qWarning(logWarning()) << tr("Citirea ultimei accesări a eșuat:")
                               << query.lastError().text();
        return;
    }
    query.addBindValue(userId > 0 ? QVariant(userId) : QVariant(login));
    if (!query.exec()) {
        qWarning(logWarning()) << tr("Citirea ultimei accesări a eșuat:")
                               << query.lastError().text();
        return;
    }
    if (!query.next()) {
        if (userId > 0 && !login.isEmpty())
            refreshLastConnection();
        return;
    }

    const QVariant storedDate = query.value(1);
    if (storedDate.isNull() || storedDate.toString().trimmed().isEmpty()) {
        ui->label_info->setText(tr("Ultima accesare: niciodată"));
    } else {
        QDateTime lastConnection = storedDate.toDateTime();
        if (!lastConnection.isValid())
            lastConnection = QDateTime::fromString(storedDate.toString(),
                                                   QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        if (!lastConnection.isValid())
            lastConnection = QDateTime::fromString(storedDate.toString(),
                                                   QStringLiteral("dd.MM.yyyy HH:mm:ss"));
        if (!lastConnection.isValid())
            lastConnection = QDateTime::fromString(storedDate.toString(), Qt::ISODate);
        ui->label_info->setText(lastConnection.isValid()
                                    ? tr("Ultima accesare: %1")
                                          .arg(lastConnection.toString(QStringLiteral("dd.MM.yyyy HH:mm:ss")))
                                    : tr("Ultima accesare: indisponibilă"));
    }
    ui->label_info->show();
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
        BalloonTip::showBalloonFor(ui->editLogin,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat <b>Login</b> !!!"
                                              "<br>Accesul este interzis."),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        ui->editLogin->setFocus();
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
        SessionContext::instance().setAuthenticatedUserId(m_Id);
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

void AuthorizationUser::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (!ui->editLogin->text().trimmed().isEmpty()) {
        QTimer::singleShot(0, this, &AuthorizationUser::updateLastConnectionForLogin);
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
