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

#include "userdialog.h"
#include "ui_userdialog.h"
#include "common/sessioncontext.h"

#include "settings/settingsrepository.h"
#include "settings/settingsservice.h"
#include "infrastructure/security/passwordhasher.h"

UserDialog::UserDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , m_statusCatalog(StatusObject::Unknow)
    , ui(new Ui::UserDialog) // initial
    , m_db(db)
    , popUp(new PopUp(this))
    , styleForButtonMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    const QList<QLineEdit*> eds = findChildren<QLineEdit*>();
    for (QLineEdit *ed : eds)
        connect(ed, &QLineEdit::textChanged,
                this, &UserDialog::dataWasModified);

    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &UserDialog::onSaveAndClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &UserDialog::onSave, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &UserDialog::close, Qt::UniqueConnection);
}

UserDialog::~UserDialog()
{
    delete ui;
}

bool UserDialog::setDeleteMarkUser(QString &err)
{
    err.clear();
    QSqlQuery q;
    q.prepare("UPDATE users SET deletionMark = :deletionMark WHERE id = :id");
    q.bindValue(":deletionMark", StatusObject::statusObjectToInt(m_statusCatalog));
    q.bindValue(":id", m_id);
    if (!q.exec()) {
        err = "SQL error: " + q.lastError().text();
        err += "\nQuery:" + q.lastQuery();
        qCritical(logCritical()).noquote()
            << "SQL error:" << err;
        return false;
    }

    emit userDeletedMark();

    return true;
}

void UserDialog::setInitialAdministrator(bool initialAdministrator)
{
    m_initialAdministrator = initialAdministrator;
}

void UserDialog::slot_IsNewChanged()
{
    if (m_isNew){
        setStatusCatalog(StatusObject::Unknow);
        slot_StatusCatalogChanged(); // fortam apelarea
    }
}

void UserDialog::slot_IdChanged()
{
    QSqlQuery q;
    q.prepare("SELECT * FROM users WHERE id = :id");
    q.bindValue(":id", m_id);
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "SQL error:" << q.lastError().text()
            << "\nLast query:" << q.lastQuery();
        return;
    }

    if (!q.next()) {
        qWarning(logWarning())
            << QStringLiteral("Nu a fost gasit utilizatorul cu ID '%1'").arg(m_id);
        return;
    }

    ui->userName->setText(q.value("name").toString());
    ui->label_info->setText(tr("Ultima accesare: %1")
                                .arg(q.value("lastConnection").toDateTime().toString("dd.MM.yyyy hh:mm:ss")));
}

void UserDialog::slot_StatusCatalogChanged()
{
    switch (m_statusCatalog) {
    case StatusObject::Unknow:
        if (m_initialAdministrator)
            setWindowTitle(tr("Crearea administratorului aplicației %1").arg("[*]"));
        else
            setWindowTitle(tr("Utilizator (crearea) %1").arg("[*]"));
        break;
    case StatusObject::ZeroWrite:
        setWindowTitle(tr("Utilizator (salvată) %1").arg("[*]"));
        break;
    case StatusObject::DeletionMark:
        setWindowTitle(tr("Utilizator (marcată pentru eliminare) %1").arg("[*]"));
        break;
    default:
        setWindowTitle(tr("Utilizator %1").arg("[*]"));
        break;
    }
}

void UserDialog::dataWasModified()
{
    setWindowModified(true);
}

bool UserDialog::controlRequiredObjects()
{
    if (ui->userName->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->userName,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat numele utilizatorului !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    if (m_isNew && !m_initialAdministrator && ui->userPassword->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->userPassword,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicată parola utilizatorului !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    // La modificare câmpul gol păstrează parola existentă.
    const QString password = ui->userPassword->text();
    if (!password.isEmpty() && password.size() < PasswordHasher::minPasswordLength) {
        BalloonTip::showBalloonFor(ui->userPassword,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Parola trebuie să conțină cel puțin %1 caractere !!!")
                                       .arg(PasswordHasher::minPasswordLength),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    if (m_isNew && m_initialAdministrator && password.isEmpty()) {
        QMessageBox messageBox(QMessageBox::Warning,
                               tr("Administrator fără parolă"),
                               tr("Administratorul nu are parolă: oricine are acces la calculator "
                                  "va putea deschide aplicația și datele pacienților.\n\n"
                                  "Continuați fără parolă?"),
                               QMessageBox::NoButton, this);
        QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(styleForButtonMessageBox);
        noButton->setStyleSheet(styleForButtonMessageBox);
        messageBox.setDefaultButton(noButton);
        messageBox.exec();
        if (messageBox.clickedButton() != yesButton) {
            ui->userPassword->setFocus();
            return false;
        }
        qWarning(logWarning()) << "UserDialog: administratorul inițial a fost creat fără parolă.";
    }
    return true;
}

bool UserDialog::userExistsByName(bool *ok)
{
    if (ok)
        *ok = false;

    // Excludem utilizatorul curent: la modificare numele propriu nu este duplicat.
    // Indexul unic uq_users_name acoperă și utilizatorii marcați pentru ștergere,
    // iar numele se compară fără diferență între majuscule și minuscule
    // (la MariaDB prin colația coloanei).
    QSqlQuery q(m_db.getDatabase());
    const bool sqlite = MainDatabaseConnectionContext::instance().isSqlite();
    if (!q.prepare(QStringLiteral(R"(
            SELECT
                COUNT(id)
            FROM
                users
            WHERE
                name = ? %1
                AND id <> ?
        )").arg(sqlite ? QStringLiteral("COLLATE NOCASE") : QString()))) {
        qCritical(logCritical()) << "UserDialog: verificarea numelui utilizatorului a eșuat:"
                                 << q.lastError().text();
        return false;
    }
    q.addBindValue(ui->userName->text().trimmed());
    q.addBindValue(m_id);
    if (!q.exec() || !q.next()) {
        qCritical(logCritical()) << "UserDialog: verificarea numelui utilizatorului a eșuat:"
                                 << q.lastError().text();
        return false;
    }

    if (ok)
        *ok = true;
    return q.value(0).toInt() > 0;
}

bool UserDialog::controlUniqueUserName()
{
    bool ok = false;
    const bool exists = userExistsByName(&ok);
    if (!ok) {
        CustomMessage msg;
        msg.setWindowTitle(tr("Verificarea datelor."));
        msg.setTextTitle(tr("Nu s-a putut verifica unicitatea numelui utilizatorului \"<b>%1</b>\".")
                             .arg(ui->userName->text()));
        msg.setDetailedText(tr("Verificați conexiunea și jurnalul aplicației."));
        msg.exec();
        return false;
    }

    if (exists) {
        CustomMessage msg;
        msg.setWindowTitle(tr("Verificarea datelor."));
        msg.setTextTitle(tr("Utilizatorul cu nume \"<b>%1</b>\" există în baza de date")
                              .arg(ui->userName->text()));
        msg.setDetailedText(tr("Alegeți alt nume a utilizatorului pentru validare."));
        msg.exec();
        return false;
    }

    return true;
}

bool UserDialog::handleInsert()
{
    // verificam utilizatorul dupa nume
    if (!controlUniqueUserName())
        return false;

    QSqlDatabase database = m_db.getDatabase();
    if (!database.transaction()) {
        qCritical(logCritical()) << "UserDialog: transaction start failed:"
                                 << database.lastError().text();
        return false;
    }

    QString hashError;
    const QString passwordHash =
        passwordHashForStorage(database, ui->userPassword->text(), &hashError);
    if (passwordHash.isEmpty()) {
        database.rollback();
        qCritical(logCritical()) << "UserDialog: hash-ul parolei nu a putut fi calculat:"
                                 << hashError;
        return false;
    }

    QUuid uuid = QUuid::createUuid();

    QSqlQuery q(database);
    q.prepare(R"(
        INSERT INTO users (
            deletionMark,
            name,
            password,
            hash,
            lastConnection,
            uuid
        ) VALUES (?,?,?,?,?,?)
    )");
    q.addBindValue(StatusObject::statusObjectToInt(m_statusCatalog));
    q.addBindValue(ui->userName->text().trimmed());
    q.addBindValue(QVariant()); // password
    q.addBindValue(passwordHash);
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.addBindValue(uuid.toRfc4122());
    if (!q.exec()) {
        database.rollback();
        qCritical(logCritical()).noquote()
            << "SQL error:" << q.lastError().text()
            << "\nLast query:" << q.lastQuery();

        CustomMessage msg;
        msg.setWindowTitle(tr("Verificarea datelor."));
        msg.setTextTitle(tr("Nu s-a putut salva datele utilizatorului '%1'<br>"
                            "în baza de date.")
                             .arg(ui->userName->text()));
        msg.setDetailedText(q.lastError().text());
        msg.exec();
        return false;
    }

    // setam ID
    m_id = q.lastInsertId().toInt();

    // Initializam setarile utilizatorului prin repository. Pentru bazele care
    // au deja schema 4.2.0 sunt create randurile din schema noua, iar in
    // perioada de tranzitie repository-ul mentine si schema 4.1.x.
    SettingsRepository repository(m_db);
    SettingsRepository::PersistedSettings settings;
    settings.values.user.userId = m_id;
    QStringList settingsErrors;
    if (!repository.saveForUser(settings, settingsErrors)) {
        database.rollback();
        qCritical(logCritical())
            << tr("Nu au fost inițializate setările utilizatorului '%1':")
                   .arg(ui->userName->text())
            << settingsErrors.join(QStringLiteral("; "));
        return false;
    }

    if (!database.commit()) {
        const QString error = database.lastError().text();
        database.rollback();
        qCritical(logCritical())
            << tr("Nu a putut fi confirmată crearea utilizatorului '%1':")
                   .arg(ui->userName->text())
            << error;
        return false;
    }

    // Actualizăm sesiunea și profilul numai după confirmarea tranzacției și
    // numai pentru administratorul inițial; utilizatorii creați ulterior
    // (inclusiv din asistentul primei lansări) nu înlocuiesc sesiunea curentă.
    if (m_initialAdministrator) {
        SessionContext::instance().setCandidateUserId(m_id);
        globals().nameUserApp = ui->userName->text().trimmed();
        if (!AppSettings::saveRememberedUser(m_id, ui->userName->text().trimmed(), true)) {
            qWarning(logWarning())
                << tr("Utilizatorul a fost creat, dar memorarea lui în profil a eșuat.");
        }
        SettingsService::instance().setSnapshot(settings.values);
    }

    return true;
}

bool UserDialog::handleUpdate()
{
    if (m_id <= 0)
        return false;

    // verificam utilizatorul dupa nume (redenumirea nu poate crea duplicate)
    if (!controlUniqueUserName())
        return false;

    QSqlDatabase database = m_db.getDatabase();
    if (!database.transaction()) {
        qCritical(logCritical()) << "UserDialog: transaction start failed:"
                                 << database.lastError().text();
        return false;
    }

    QSqlQuery current(database);
    current.prepare(QStringLiteral(R"(
        SELECT
            hash
        FROM
            users
        WHERE
            id = ?
        LIMIT 1
    )"));
    current.addBindValue(m_id);
    if (!current.exec() || !current.next()) {
        const QString error = current.lastError().text();
        database.rollback();
        qCritical(logCritical()) << "UserDialog: current password hash read failed:" << error;
        return false;
    }

    const QString oldHash = current.value(0).toString();
    current.finish();

    // Cheia parolei cloud nu mai depinde de hash-ul utilizatorului (4.2.7),
    // deci schimbarea parolei nu cere recriptarea configurației cloud.
    const bool passwordChanged = !ui->userPassword->text().isEmpty();
    QString hashError;
    const QString newHash = passwordChanged
        ? passwordHashForStorage(database, ui->userPassword->text(), &hashError)
        : oldHash;
    if (newHash.isEmpty()) {
        database.rollback();
        qCritical(logCritical()) << "UserDialog: hash-ul parolei este invalid:" << hashError;
        return false;
    }

    QSqlQuery q(database);
    q.prepare(R"(
        UPDATE users SET
            deletionMark   = ?,
            name           = ?,
            password       = ?,
            hash           = ?,
            lastConnection = ?
        WHERE
            id = ?
    )");
    q.addBindValue(StatusObject::statusObjectToInt(m_statusCatalog));
    q.addBindValue(ui->userName->text().trimmed());
    q.addBindValue(QVariant()); // password
    q.addBindValue(newHash);
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.addBindValue(m_id);
    if (!q.exec()) {
        database.rollback();
        qCritical(logCritical()).noquote()
        << "SQL error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();

        CustomMessage msg;
        msg.setWindowTitle(tr("Verificarea datelor."));
        msg.setTextTitle(tr("Nu s-a putut actualiza datele utilizatorului '%1'<br>"
                            "în baza de date.")
                             .arg(ui->userName->text()));
        msg.setDetailedText(q.lastError().text());
        msg.exec();
        return false;
    }
    if (!database.commit()) {
        const QString error = database.lastError().text();
        database.rollback();
        qCritical(logCritical()) << "UserDialog: transaction commit failed:" << error;
        return false;
    }
    return true;
}

QString UserDialog::passwordHashForStorage(const QSqlDatabase &database,
                                          const QString &password,
                                          QString *error) const
{
    if (error)
        error->clear();

    // Administratorul inițial poate fi creat pe o bază MariaDB încă nemigrată,
    // unde users.hash este CHAR(64): hash-ul curent ar fi trunchiat. Scriem
    // atunci hash-ul vechi, pe care migrarea 4.2.7 îl convertește.
    if (MainDatabaseConnectionContext::instance().isMariaDb()) {
        QSqlQuery column(database);
        if (!column.prepare(QStringLiteral(R"(
                SELECT
                    CHARACTER_MAXIMUM_LENGTH
                FROM
                    INFORMATION_SCHEMA.COLUMNS
                WHERE
                    TABLE_SCHEMA = DATABASE()
                    AND TABLE_NAME = 'users'
                    AND COLUMN_NAME = 'hash'
            )"))
            || !column.exec() || !column.next()) {
            if (error)
                *error = column.lastError().text();
            return QString();
        }
        if (column.value(0).toLongLong() < 255) {
            qInfo(logInfo()) << "UserDialog: users.hash nu este încă migrat; se salvează hash-ul vechi.";
            return PasswordHasher::legacyHash(password);
        }
    }

    const QString hash = PasswordHasher::hashPassword(password);
    if (hash.isEmpty() && error)
        *error = QStringLiteral("PBKDF2 a eșuat.");
    return hash;
}

bool UserDialog::onSave()
{
    if (!controlRequiredObjects())
        return false;

    bool returnBool = false;
    returnBool = m_isNew
                     ? handleInsert()
                     : handleUpdate();
    if (returnBool) {
        if (m_isNew) {
            emit userCreated();
            emit userCreatedReturnID(m_id);
            setIsNew(false);

            qInfo(logInfo())
                << QStringLiteral("Utilizatorul '%1' inserat cu succes in baza de date cu ID '%2'")
                       .arg(ui->userName->text())
                       .arg(m_id);
        } else {
            emit userChanged();

            qInfo(logInfo())
                << QStringLiteral("Datale ttilizatorul '%1' cu ID '%2' au fost modificate cu succes.")
                       .arg(ui->userName->text(), m_id);
        }
    }

    if (returnBool) {
        popUp->setPopupText(tr("Utilizatorul a fost salvat cu succes<br> in baza de date."));
        popUp->show();
        setWindowModified(false);
    }

    return returnBool;
}

bool UserDialog::onSaveAndClose()
{
    const auto oldStatus = m_statusCatalog;
    setStatusCatalog(StatusObject::ZeroWrite);

    if (!onSave()) {
        setStatusCatalog(oldStatus);
        return false;
    }

    accept();
    return true;
}

void UserDialog::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()) {
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Doriți să salvați aceste modificări ?"),
                                 QMessageBox::NoButton, this);

        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);

        yesButton->setStyleSheet(styleForButtonMessageBox);
        noButton->setStyleSheet(styleForButtonMessageBox);
        cancelButton->setStyleSheet(styleForButtonMessageBox);

        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            const bool ok = onSave();
            if (ok) {
                event->accept();
            } else {
                event->ignore();
            }
            return;
        }

        if (messange_box.clickedButton() == noButton) {
            event->accept();
            return;
        }

        event->ignore();
        return;
    }

    event->accept();
}

void UserDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void UserDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
        this->focusNextChild();

    QDialog::keyPressEvent(event);
}
