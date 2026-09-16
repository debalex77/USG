#include "userdialog.h"
#include "ui_userdialog.h"

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
        if (globals().firstLaunch)
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
    return true;
}

bool UserDialog::userExistsByName()
{
    QSqlQuery q;
    q.prepare(m_db.getTextSQL(":/sql/queries/check_object_exists_by_name.sql"));
    q.addBindValue(ui->userName->text());
    if (q.exec() && q.next()) {
        return q.value(0).toInt() > 0;
    }

    return false;
}

bool UserDialog::handleInsert()
{
    // verificam utilizatorul dupa nume
    if (userExistsByName()) {
        CustomMessage msg;
        msg.setWindowTitle(tr("Verificarea datelor."));
        msg.setTextTitle(tr("Utilizatorul cu nume \"<b>%1</b>\" există în baza de date")
                              .arg(ui->userName->text()));
        msg.setDetailedText(tr("Alegeți alt nume a utilizatorului pentru validare."));
        msg.exec();
        return false;
    }

    QUuid uuid = QUuid::createUuid();

    QSqlQuery q;
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
    q.addBindValue(ui->userName->text());
    q.addBindValue(QVariant()); // password
    q.addBindValue(QCryptographicHash::hash(ui->userPassword->text().toUtf8(), QCryptographicHash::Sha256).toHex());
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.addBindValue(uuid.toRfc4122());
    if (!q.exec()) {
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

    // setam variabile globale si salvam in QSettings
    if (globals().firstLaunch) {

        // variabile globale pu titlu mainwindow
        globals().idUserApp   = m_id;
        globals().nameUserApp = ui->userName->text();

        // salvam in fisierul .conf
        AppSettings::saveRememberedUser(m_id, ui->userName->text(), true);
    }

    // inseram datele in tabele 'userPreferences'
    m_db.insertSetTableSettingsUsers();

    return true;
}

bool UserDialog::handleUpdate()
{
    if (m_id <= 0)
        return false;

    QSqlQuery q;
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
    q.addBindValue(ui->userName->text());
    q.addBindValue(QVariant()); // password
    q.addBindValue(QCryptographicHash::hash(ui->userPassword->text().toUtf8(), QCryptographicHash::Sha256).toHex());
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.addBindValue(m_id);
    if (!q.exec() && q.next()) {
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
    return true;
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
