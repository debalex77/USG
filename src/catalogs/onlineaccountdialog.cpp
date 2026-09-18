#include "onlineaccountdialog.h"
#include "ui_onlineaccountdialog.h"
#include <data/database_common.h>

OnlineAccountDialog::OnlineAccountDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , m_statusCatalog(StatusObject::Unknow)
    , ui(new Ui::OnlineAccountDialog) // initial
    , m_db(db)
    , styleBtnMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    const QString schema = globals().thisMySQL
        ? QStringLiteral(":/sql/mariadb/tables/online_account.sql")
        : QStringLiteral(":/sql/sqlite/tables/online_account.sql");
    DataBaseCommon::execFileBatch(m_db.getDatabase(), schema, "onlineAccount");

    slot_IsNewChanged(); // fortam

    updateModelOrganization();
    initConnections();
    initFooter();
}

OnlineAccountDialog::~OnlineAccountDialog()
{
    delete ui;
}

void OnlineAccountDialog::slot_IsNewChanged()
{
    if (m_isNew){
        setStatusCatalog(StatusObject::Unknow);
        slot_StatusCatalogChanged(); // fortam apelarea din cauza ca in macro:
        if (globals().organizationID > 0)
            setIdOrganization(globals().organizationID);
    } else {
        setStatusCatalog(StatusObject::ZeroWrite);
        slot_StatusCatalogChanged();
    }
}

void OnlineAccountDialog::slot_IdChanged()
{
    if (m_id <= 0)
        return;

    m_loadingData = true;

    QSqlDatabase base = m_db.getDatabase();
    QSqlQuery qry(base);
    qry.prepare(R"(
        SELECT
            onlineAccount.id_organizations
        FROM
            onlineAccount
        WHERE
            id = :id
    )");
    qry.bindValue(":id", m_id);

    if (!qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        m_loadingData = false;
        return;
    }

    if (!qry.next()) {
        m_loadingData = false;
        return;
    }

    QSqlRecord rec = qry.record();
    const int idOrganization = qry.value(rec.indexOf("id_organizations")).toInt();
    setIdOrganization(idOrganization);

    slot_StatusCatalogChanged();

    m_loadingData = false;
}

void OnlineAccountDialog::slot_IdOrganizationChanged()
{
    if (m_idOrganization <= 0)
        return;

    ui->comboOrganizations->setCurrentIndex(modelOrganization->rowById("id", m_idOrganization));

    const int roleEmail = modelOrganization->roleForColumn("email");
    ui->email->setText(ui->comboOrganizations->currentData(roleEmail).toString());
    if (!ui->email->text().isEmpty())
        ui->email->setFocus();

    if (m_id <= 0)
        return;

    QString error;
    QByteArray realKey;

    QSqlDatabase base = m_db.getDatabase();
    if (!CryptoManager::loadOrCreateSplitKey(base, m_idOrganization, &realKey, &error)) {
        qWarning(logWarning()) << "Nu s-a putut incarca cheia split:" << error;
        return;
    }

    QSqlQuery qry(base);
    qry.prepare(R"(
        SELECT
            onlineAccount.email,
            onlineAccount.smtp_server,
            onlineAccount.port,
            onlineAccount.username,
            onlineAccount.password,
            onlineAccount.iv,
            onlineAccount.tag
        FROM
            onlineAccount
        WHERE
            id_organizations = :id_organizations AND
            id_users = :id_users AND
            id = :id
    )");
    qry.bindValue(":id_organizations", m_idOrganization);
    qry.bindValue(":id_users", globals().idUserApp);
    qry.bindValue(":id", m_id);

    if (!qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return;
    }

    if (!qry.next())
        return;

    QSqlRecord rec = qry.record();
    ui->email->setText(qry.value(rec.indexOf("email")).toString());
    ui->smtp_server->setText(qry.value(rec.indexOf("smtp_server")).toString());
    ui->port->setText(qry.value(rec.indexOf("port")).toString());
    ui->username->setText(qry.value(rec.indexOf("username")).toString());

    CryptoManager::EncryptedData data;
    data.cipherText = CryptoManager::fromBase64(qry.value(rec.indexOf("password")).toString());
    data.iv         = CryptoManager::fromBase64(qry.value(rec.indexOf("iv")).toString());
    data.tag        = CryptoManager::fromBase64(qry.value(rec.indexOf("tag")).toString());

    bool okDecrypt = false;
    const QByteArray decryptedPassword =
        CryptoManager::decryptText(data, realKey, &okDecrypt);

    if (!okDecrypt) {
        qWarning(logWarning()) << "Nu s-a putut decripta parola";
        ui->password->clear();
        return;
    }

    ui->password->setText(QString::fromUtf8(decryptedPassword));
}

void OnlineAccountDialog::slot_StatusCatalogChanged()
{
    switch (m_statusCatalog) {
    case StatusObject::Unknow:
        setWindowTitle(tr("Online account (crearea) %1").arg("[*]"));
        break;
    case StatusObject::ZeroWrite:
        setWindowTitle(tr("Online account (salvat) %1").arg("[*]"));
        break;
    case StatusObject::DeletionMark:
        setWindowTitle(tr("Online account (marcată pentru eliminare) %1").arg("[*]"));
        break;
    default:
        setWindowTitle(tr("Online account %1").arg("[*]"));
        break;
    }
}

void OnlineAccountDialog::dataWasModified()
{
    if (!m_loadingData)
        setWindowModified(true);
}

void OnlineAccountDialog::changedIndexComboOrganization(const int index)
{
    Q_UNUSED(index);

    // determinam rolul
    auto roleID = modelOrganization->roleForColumn("id");
    const int id_organization = ui->comboOrganizations->currentData(roleID).toInt();

    // setam ID organizatiei
    if (id_organization > 0)
        setIdOrganization(id_organization);

    if (!m_loadingData)
        dataWasModified();
}

bool OnlineAccountDialog::controlRequiredObjects()
{
    if (ui->comboOrganizations->currentIndex() == 0) {
        BalloonTip::showBalloonFor(ui->comboOrganizations,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicata \"<b>Organizatia</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    if (ui->email->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->email,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat \"<b>E-mail</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    if (ui->smtp_server->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->smtp_server,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat \"<b>SMTP server</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    if (ui->port->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->port,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat \"<b>Port</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    if (ui->username->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->username,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat \"<b>User name</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    if (ui->password->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->password,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicata \"<b>Parola</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }
    return true;
}

bool OnlineAccountDialog::handleInsert()
{
    QString error;
    QByteArray realKey;

    QSqlDatabase base = m_db.getDatabase();
    if (!CryptoManager::loadOrCreateSplitKey(base, m_idOrganization, &realKey, &error)) {
        qWarning(logWarning()) << "Nu s-a putut incarca cheia split:" << error;
        return false;
    }

    const CryptoManager::EncryptedData enc =
        CryptoManager::encryptText(ui->password->text(), realKey);

    if (!enc.isValid()) {
        qWarning(logWarning()) << "Nu s-a putut cripta parola";
        return false;
    }

    QSqlQuery qry(base);
    qry.prepare(R"(
        INSERT INTO onlineAccount (
            id_organizations,
            id_users,
            email,
            smtp_server,
            port,
            username,
            password,
            iv,
            tag
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(globals().idUserApp);
    qry.addBindValue(ui->email->text().trimmed());
    qry.addBindValue(ui->smtp_server->text().trimmed());
    qry.addBindValue(ui->port->text().trimmed());
    qry.addBindValue(ui->username->text().trimmed());
    qry.addBindValue(CryptoManager::toBase64(enc.cipherText));
    qry.addBindValue(CryptoManager::toBase64(enc.iv));
    qry.addBindValue(CryptoManager::toBase64(enc.tag));

    if (!qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return false;
    }

    m_id = qry.lastInsertId().toInt();

    return true;
}

bool OnlineAccountDialog::handleUpdate()
{
    QString error;
    QByteArray realKey;

    QSqlDatabase base = m_db.getDatabase();
    if (!CryptoManager::loadOrCreateSplitKey(base, m_idOrganization, &realKey, &error)) {
        qWarning(logWarning()) << "Nu s-a putut incarca cheia split:" << error;
        return false;
    }

    const CryptoManager::EncryptedData enc =
        CryptoManager::encryptText(ui->password->text(), realKey);

    if (!enc.isValid()) {
        qWarning(logWarning()) << "Nu s-a putut cripta parola";
        return false;
    }

    QSqlQuery qry(base);
    qry.prepare(R"(
        UPDATE onlineAccount SET
            email            = :email,
            smtp_server      = :smtp_server,
            port             = :port,
            username         = :username,
            password         = :password,
            iv               = :iv,
            tag              = :tag
        WHERE
            id = :id AND
            id_organizations = :id_organizations AND
            id_users = :id_users
    )");
    qry.bindValue(":email",            ui->email->text().trimmed());
    qry.bindValue(":smtp_server",      ui->smtp_server->text().trimmed());
    qry.bindValue(":port",             ui->port->text().trimmed());
    qry.bindValue(":username",         ui->username->text().trimmed());
    qry.bindValue(":password",         CryptoManager::toBase64(enc.cipherText));
    qry.bindValue(":iv",               CryptoManager::toBase64(enc.iv));
    qry.bindValue(":tag",              CryptoManager::toBase64(enc.tag));
    qry.bindValue(":id",               m_id);
    qry.bindValue(":id_organizations", m_idOrganization);
    qry.bindValue(":id_users",         globals().idUserApp);

    if (!qry.exec()) {
        qWarning(logWarning()) << "SQL Error:" << qry.lastError().text();
        return false;
    }

    if (qry.numRowsAffected() <= 0) {
        qWarning(logWarning()) << "Nu a fost actualizat niciun rand in onlineAccount";
        return false;
    }

    return true;
}

bool OnlineAccountDialog::onSave()
{
    if (!controlRequiredObjects())
        return false;

    // anuntam variabila returnarii
    bool returnBool;

    // procesarea inserarii si actualizarii
    returnBool = m_isNew
                     ? handleInsert()
                     : handleUpdate();

    if (returnBool) {
        if (m_isNew)
            emit onlineAccountCreated();
        else
            emit onlineAccountChanged();

        if (m_isNew)
            setIsNew(false);

        setWindowModified(false);
    }

    // modificarea formei
    if (returnBool)
        setWindowModified(false);

    return returnBool;
}

bool OnlineAccountDialog::onSaveAndClose()
{
    const StatusObject::Column oldStatus = m_statusCatalog;

    if (m_statusCatalog == StatusObject::Unknow)
        setStatusCatalog(StatusObject::ZeroWrite);

    if (!onSave()) {
        setStatusCatalog(oldStatus);
        return false;
    }

    accept();
    return true;
}

void OnlineAccountDialog::updateModelOrganization()
{
    if (modelOrganization)
        delete modelOrganization;

    QString str = "SELECT * FROM organizations WHERE deletionMark = 0";
    modelOrganization = new QueryRolesModel(str, ui->comboOrganizations);
    modelOrganization->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganizations->setModel(modelOrganization);
    ui->comboOrganizations->setModelColumn(modelOrganization->columnIndex("name"));
}

void OnlineAccountDialog::initConnections()
{
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OnlineAccountDialog::changedIndexComboOrganization, Qt::UniqueConnection);

    const QList<QLineEdit*> eds = this->findChildren<QLineEdit*>();
    for (QLineEdit *ed : eds)
        connect(ed, &QLineEdit::textChanged,
                this, &OnlineAccountDialog::dataWasModified);

    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &OnlineAccountDialog::onSaveAndClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &OnlineAccountDialog::onSave, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &OnlineAccountDialog::close, Qt::UniqueConnection);

    /** comenzi rapide la tastatura */
    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));

}

void OnlineAccountDialog::initFooter()
{
    QPixmap pixAutor = QIcon(":/img/catalogs/user.png").pixmap(18,18);
    auto labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    auto labelAuthor = new QLabel(this);
    labelAuthor->setText(globals().nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
}

bool OnlineAccountDialog::confirmSaveIfModified()
{
    if (!isWindowModified())
        return true;

    QMessageBox messageBox(QMessageBox::Question,
                           tr("Verificarea datelor"),
                           tr("Datele obiectului <b>%1</b> nu sunt salvate.<br>"
                              "Doriți să salvați datele?")
                               .arg(ui->email->text()),
                           QMessageBox::NoButton,
                           this);

    QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);

    yesButton->setStyleSheet(styleBtnMessageBox);
    noButton->setStyleSheet(styleBtnMessageBox);

    messageBox.exec();

    if (messageBox.clickedButton() == yesButton)
        return onSave();

    if (messageBox.clickedButton() == noButton)
        return true;   // inchide fara salvare

    return false;
}

void OnlineAccountDialog::closeEvent(QCloseEvent *event)
{
    if (confirmSaveIfModified()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void OnlineAccountDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void OnlineAccountDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
        return;
    }
    QDialog::keyPressEvent(event);
}
