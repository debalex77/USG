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
#include "infrastructure/security/passwordhasher.h"
#include "settings/appsettings.h"
#include "settings/settingsservice.h"
#include "ui_authorizationuser.h"

#include <QDateTime>
#include <QVersionNumber>

#include <algorithm>

namespace {

// După atâtea încercări eșuate pentru același login începe pauza.
constexpr int failedAttemptsBeforeDelay = 3;
constexpr int firstDelaySeconds = 30;
constexpr int maxDelaySeconds = 300;

int lockoutDelaySeconds(int failedAttempts)
{
    if (failedAttempts < failedAttemptsBeforeDelay)
        return 0;
    // 30, 60, 120, 240, apoi limitat la 300 de secunde.
    const int doublings = std::min(failedAttempts - failedAttemptsBeforeDelay, 4);
    return std::min(firstDelaySeconds << doublings, maxDelaySeconds);
}

}

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
                updateLockoutState();
            });

    m_okText = ui->btnOK->text();
    m_lockoutTimer.setInterval(1000);
    connect(&m_lockoutTimer, &QTimer::timeout,
            this, &AuthorizationUser::updateLockoutState);
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

    auto worker = new DataConstantsWorker(data);

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
    // Pentru un nume tastat nu afișăm ultima accesare: eticheta ar indica dacă
    // utilizatorul există. O afișăm doar pentru utilizatorul memorat în profil.
    const QString login = ui->editLogin->text().trimmed();
    if (m_Id > 0 && globals().memoryUser
        && login.compare(globals().nameUserApp.trimmed(), Qt::CaseInsensitive) == 0) {
        refreshLastConnection(m_Id);
        return;
    }
    ui->label_info->hide();
}

void AuthorizationUser::refreshLastConnection(int userId)
{
    ui->label_info->hide();

    if (userId <= 0)
        return;

    const QSqlDatabase database = m_db.getDatabase();
    if (!database.isValid() || !database.isOpen())
        return;

    QSqlQuery query(database);
    if (!query.prepare(QStringLiteral(R"(
            SELECT name, lastConnection
            FROM users
            WHERE id = ? AND deletionMark = 0
        )"))) {
        qWarning(logWarning()) << tr("Citirea ultimei accesări a eșuat:")
                               << query.lastError().text();
        return;
    }
    query.addBindValue(userId);
    if (!query.exec()) {
        qWarning(logWarning()) << tr("Citirea ultimei accesări a eșuat:")
                               << query.lastError().text();
        return;
    }
    if (!query.next())
        return;

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

QString AuthorizationUser::attemptsKey() const
{
    // Numele se compară fără diferență între majuscule și minuscule.
    return ui->editLogin->text().trimmed().toCaseFolded();
}

int AuthorizationUser::remainingLockoutSeconds() const
{
    const auto it = m_lockedUntil.constFind(attemptsKey());
    if (it == m_lockedUntil.cend() || it->hasExpired())
        return 0;
    // Rotunjim în sus: „0 secunde” nu se afișează cât pauza mai durează.
    return int((it->remainingTime() + 999) / 1000);
}

void AuthorizationUser::updateLockoutState()
{
    const int seconds = remainingLockoutSeconds();
    if (seconds > 0) {
        ui->btnOK->setEnabled(false);
        ui->btnOK->setText(tr("Așteptați %1 s").arg(seconds));
        if (!m_lockoutTimer.isActive())
            m_lockoutTimer.start();
        return;
    }

    ui->btnOK->setText(m_okText);
    if (!m_loadingData)
        ui->btnOK->setEnabled(true);

    // Timerul rulează cât mai există o pauză activă pentru oricare login.
    const bool anyLockout = std::any_of(m_lockedUntil.cbegin(), m_lockedUntil.cend(),
                                        [](const QDeadlineTimer &deadline) {
                                            return !deadline.hasExpired();
                                        });
    if (!anyLockout)
        m_lockoutTimer.stop();
}

void AuthorizationUser::rejectCredentials()
{
    const QString key = attemptsKey();
    const int failedAttempts = ++m_failedAttempts[key];
    const int delaySeconds = lockoutDelaySeconds(failedAttempts);

    qWarning(logWarning()) << tr("Încercarea eșuată nr. %1 de autentificare pentru utilizatorul '%2'.")
                                  .arg(QString::number(failedAttempts), ui->editLogin->text());

    // Mesajul nu indică dacă utilizatorul există; cauza exactă rămâne în jurnal.
    QString message = tr("Numele utilizatorului sau parola sunt incorecte !!!<br>"
                         "Accesul este interzis.");

    if (delaySeconds > 0) {
        m_lockedUntil.insert(key, QDeadlineTimer(std::chrono::seconds(delaySeconds)));
        qWarning(logWarning()) << tr("Autentificarea utilizatorului '%1' este suspendată pentru %2 secunde "
                                     "după %3 încercări eșuate.")
                                      .arg(ui->editLogin->text(),
                                           QString::number(delaySeconds),
                                           QString::number(failedAttempts));
        message += tr("<br><br>Următoarea încercare va fi posibilă peste %1 secunde.")
                       .arg(delaySeconds);
        message += tr("<br><br>Ați uitat parola? Administratorul o poate reseta "
                      "din catalogul <b>Utilizatori</b>.");
    }

    updateLockoutState();

    QMessageBox::warning(this, tr("Controlul accesului"), message, QMessageBox::Ok);

    edit_password->clear();
    edit_password->setFocus();
}

void AuthorizationUser::onDataReceived(bool success)
{
    m_loadingData = false;
    ui->btnCancel->setEnabled(true);
    ui->editLogin->setEnabled(true);
    edit_password->setEnabled(true);
    updateLockoutState();

    if (!success) {
        QMessageBox::critical(this,
                              tr("Inițializarea aplicației"),
                              tr("Datele necesare aplicației nu au putut fi încărcate. "
                                 "Verificați conexiunea și jurnalul aplicației."),
                              QMessageBox::Ok);
        return;
    }

    qInfo(logInfo())
        << "Contextele aplicației au fost inițializate după autentificare.";

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

    // extragem datele utilizatorului; numele se compară fără diferență între
    // majuscule și minuscule (la MariaDB prin colația coloanei)
    QSqlQuery qry(database);
    qry.prepare(QStringLiteral("SELECT * FROM users WHERE name = ? %1 AND deletionMark = 0")
                    .arg(MainDatabaseConnectionContext::instance().isSqlite()
                             ? QStringLiteral("COLLATE NOCASE")
                             : QString()));
    qry.addBindValue(ui->editLogin->text().trimmed());
    if (qry.exec() && qry.next()) {
        const int authenticatedUserId = qry.value(UsersSections::Id).toInt();

        // Logarea are loc înaintea migrării: hash-ul vechi (SHA-256) este
        // acceptat și rescris doar dacă baza este deja migrată la 4.2.7
        // (upgradeLegacyPasswordHash).
        const QString db_hash = qry.value(UsersSections::Hash).toString();

        if (!PasswordHasher::verifyPassword(edit_password->text(), db_hash)){
            qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                                   << tr("Accesul la aplicație. Utilizatorul '%1' cu id='%2' - întroducerea parolei incorecte.")
                                          .arg(ui->editLogin->text(),
                                               QString::number(authenticatedUserId));
            rejectCredentials();
            return false;
        }

        m_failedAttempts.remove(attemptsKey());
        m_lockedUntil.remove(attemptsKey());

        if (PasswordHasher::isLegacyHash(db_hash))
            upgradeLegacyPasswordHash(database, authenticatedUserId, db_hash);

        // Actualizăm ID-ul numai după validarea parolei. Astfel o încercare
        // nereușită nu reîncarcă inutil datele utilizatorului memorat.
        m_Id = authenticatedUserId;

        // Setam variabile globale necesare
        // Pe o bază încă nemigrată coloana uuid poate lipsi; UUID-ul rămâne nul.
        const QUuid authenticatedUserUuid =
            QUuid::fromRfc4122(qry.value(UsersSections::Uuid).toByteArray());
        SessionContext::instance().setAuthenticatedUserId(m_Id, authenticatedUserUuid);
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
        qWarning(logWarning()) << tr("%1 - onAccepted()").arg(metaObject()->className())
                               << tr("Accesul la aplicație. Utilizatorul cu nume '%1' nu a fost depistat in baza de date.")
                                      .arg(ui->editLogin->text());
        // Același timp de răspuns ca la o parolă greșită.
        PasswordHasher::simulateVerification(edit_password->text());
        rejectCredentials();
        return false;
    }

    qInfo(logInfo()) << tr("Accesul la aplicatia. Autorizarea reusita a utilizatorului '%1' cu id='%2'.")
                        .arg(ui->editLogin->text(), QString::number(m_Id));

    // setam datele constantelor
    setDataConstants();

    return true;
}

void AuthorizationUser::upgradeLegacyPasswordHash(QSqlDatabase &database,
                                                  int userId,
                                                  const QString &legacyHash)
{
    // Înainte de migrarea 4.2.7 hash-ul vechi este necesar pentru recriptarea
    // parolei cloud, iar la MariaDB users.hash poate fi încă CHAR(64).
    // După migrare un hash vechi poate apărea doar de la un client mai vechi.
    QString versionError;
    const QVersionNumber schemaVersion =
        QVersionNumber::fromString(m_db.databaseSchemaVersion(&versionError));
    if (schemaVersion.isNull() || schemaVersion < QVersionNumber(4, 2, 7))
        return;

    const QString newHash = PasswordHasher::hashPassword(edit_password->text());
    if (newHash.isEmpty())
        return;

    QSqlQuery update(database);
    if (!update.prepare(QStringLiteral(R"(
            UPDATE users SET hash = ? WHERE id = ? AND hash = ?
        )"))) {
        qWarning(logWarning()) << "AuthorizationUser: actualizarea hash-ului parolei a eșuat:"
                               << update.lastError().text();
        return;
    }
    update.addBindValue(newHash);
    update.addBindValue(userId);
    update.addBindValue(legacyHash);
    if (!update.exec()) {
        qWarning(logWarning()) << "AuthorizationUser: actualizarea hash-ului parolei a eșuat:"
                               << update.lastError().text();
        return;
    }
    if (update.numRowsAffected() == 1)
        qInfo(logInfo()) << "AuthorizationUser: hash-ul SHA-256 al utilizatorului" << userId
                         << "a fost convertit în PBKDF2.";
}

void AuthorizationUser::onAccepted()
{
    if (m_loadingData || remainingLockoutSeconds() > 0)
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

void AuthorizationUser::reject()
{
    // Cât worker-ul încarcă datele, dialogul rămâne deschis: închiderea
    // (X, Esc, Alt+F4) ar opri aplicația cu firul de lucru încă activ.
    // QDialog::closeEvent ignoră închiderea dacă dialogul rămâne vizibil.
    if (m_loadingData)
        return;

    QDialog::reject();
}

void AuthorizationUser::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Autorizarea utilizatorului"));
        m_okText = ui->btnOK->text();
        updateLockoutState();
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
