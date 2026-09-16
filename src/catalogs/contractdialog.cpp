#include "contractdialog.h"
#include "ui_contractdialog.h"

ContractDialog::ContractDialog(DataBase &db, QWidget *parent) :
    QDialog(parent),
    m_statusCatalog(StatusObject::Unknow) // initial
    , ui(new Ui::ContractDialog)
    , m_db(db)
    , styleButtonMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    setWindowTitle(tr("Contract %1").arg("[*]"));

    ui->editName->setMaxLength(50);

    ui->dateInit->setDate(QDate::currentDate()); // setam data

    updateModelOrganizations();
    updateModelTypesPrices();

    initConnections();
}

ContractDialog::~ContractDialog()
{
    delete ui;
}

QString ContractDialog::getNameParentContract()
{
    return ui->comboOrganizations->currentText();
}

bool ContractDialog::setDeleteMarkOrganization(QString &err)
{
    return handleDeletionMark(err);
}

void ContractDialog::initConnections()
{
    /** combobox - Organization & typePrices */
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ContractDialog::changedIndexComboOrganization, Qt::UniqueConnection);
    connect(ui->comboTypesPrices, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ContractDialog::currentIndexTypesPricesChanged, Qt::UniqueConnection);

    /** modificarea datelor formei */
    connect(ui->editName, &QLineEdit::textChanged,
            this, &ContractDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->dateInit, &QDateEdit::dateChanged,
            this, &ContractDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->checkBoxNotValid, &QCheckBox::checkStateChanged,
            this, &ContractDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->editComment, &QPlainTextEdit::textChanged,
            this, &ContractDialog::dataWasModified, Qt::UniqueConnection);

    /** btn OK, Write, Cancel */
    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &ContractDialog::onWritingDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &ContractDialog::onWritingData, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &ContractDialog::onClose, Qt::UniqueConnection);

    /** comenzi rapide la tastatura */
    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
}

void ContractDialog::updateModelOrganizations()
{
    if (model_organizations)
        delete model_organizations;

    QString str = m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql");
    model_organizations = new QueryRolesModel(str, ui->comboOrganizations);
    model_organizations->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganizations->setModel(model_organizations);
    ui->comboOrganizations->setModelColumn(model_organizations->columnIndex("name"));
}

void ContractDialog::updateModelTypesPrices()
{
    if (model_typesPrices)
        delete model_typesPrices;

    QString str = m_db.getTextSQL(":/sql/queries/typePrices_combo_view.sql");
    model_typesPrices = new QueryRolesModel(str, ui->comboTypesPrices);
    model_typesPrices->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboTypesPrices->setModel(model_typesPrices);
    ui->comboTypesPrices->setModelColumn(model_typesPrices->columnIndex("name"));
}

bool ContractDialog::controlRequiredObjects()
{
    if (ui->comboOrganizations->currentIndex() <= 0){
        BalloonTip::showBalloonFor(ui->comboOrganizations,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor."),
                                   tr("Nu este indicata \"<b>Organizatia</b>\" !!!"),
                                   3000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    if (ui->comboTypesPrices->currentIndex() <= 0){
        BalloonTip::showBalloonFor(ui->comboTypesPrices,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor."),
                                   tr("Nu este indicat \"<b>Tipul pretului</b>\" !!!"),
                                   3000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    if (ui->editName->text().isEmpty()){
        BalloonTip::showBalloonFor(ui->editName,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor."),
                                   tr("Nu este indicata \"<b>Denumirea</b>\" contractului !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    return true;
}

bool ContractDialog::insertIntoTableContracts(QStringList &err)
{
    // verificam daca este corect determinat ID
    if (m_id <= 0) {
        err << this->metaObject()->className()
        << "[insertIntoTableContracts]:"
        << "Nu este indicat ID contractului !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // verificam ID organizatiei
    if (m_idOrganization <= 0) {
        err << this->metaObject()->className()
        << "[insertIntoTableContracts]:"
        << "Nu este indicat ID organizatiei !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // pregatim datele
    QVector<QVariant> data;
    data.append(m_id);
    data.append(StatusObject::statusObjectToInt(m_statusCatalog));
    data.append(m_idOrganization);
    data.append(m_idTypePrice);
    data.append(ui->editName->text());
    data.append(ui->dateInit->date().toString("yyyy-MM-dd"));
    data.append(globals().thisMySQL
                    ? QVariant(ui->checkBoxNotValid->isChecked())
                    : QVariant(int(ui->checkBoxNotValid->isChecked()))
                );
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText()
                );
    QUuid uuid = QUuid::createUuid();
    data.append(uuid.toRfc4122());

    QString str_err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   ":/sql/queries/contracts_insert.sql",
                                   data,
                                   &str_err))
    {
        // logarea
        qCritical(logCritical()) << str_err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor contractului '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();
        return false;
    }
    return true;
}

bool ContractDialog::updateDataTableContracts(QStringList &err)
{
    // verificam ID contract
    if (m_id <= 0) {
        err << this->metaObject()->className()
        << "[insertIntoTableContracts]:"
        << "Nu este indicat ID contractului !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // verificam ID organizatiei
    if (m_idOrganization <= 0) {
        err << this->metaObject()->className()
        << "[insertIntoTableContracts]:"
        << "Nu este indicat ID organizatiei !!!";
        qWarning(logWarning()) << err;
        return false;
    }

    // pregatim datele
    QVector<QVariant> data;
    data.append(StatusObject::statusObjectToInt(m_statusCatalog));
    data.append(m_idOrganization);
    data.append(m_idTypePrice);
    data.append(ui->editName->text());
    data.append(ui->dateInit->date().toString("yyyy-MM-dd"));
    data.append(globals().thisMySQL
                    ? QVariant(ui->checkBoxNotValid->isChecked())
                    : QVariant(int(ui->checkBoxNotValid->isChecked()))
                );
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText()
                );
    data.append(m_id);

    QString str_err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   ":/sql/queries/contracts_update.sql",
                                   data,
                                   &str_err))
    {
        // logarea
        qCritical(logCritical()) << str_err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Actualizarea datelor contractului '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();
        return false;
    }
    return true;
}

bool ContractDialog::handleDeletionMark(QString &err)
{
    err.clear();

    if (m_id <= 0) {
        err = tr("Nu este determinat ID-ul contractului.");
        return false;
    }

    QSqlQuery q;
    q.prepare("UPDATE contracts SET deletionMark = :deletionMark WHERE id = :id");
    q.bindValue(":deletionMark", StatusObject::statusObjectToInt(m_statusCatalog));
    q.bindValue(":id", m_id);
    if (!q.exec()) {
        err = "SQL error: " + q.lastError().text();
        err += "\nQuery:" + q.lastQuery();
        qCritical(logCritical()).noquote()
            << "SQL error:" << err;
        return false;
    }

    emit contractDeletedMark();

    return true;
}

void ContractDialog::dataWasModified()
{
    setWindowModified(true);
}

void ContractDialog::slot_IsNewChanged()
{
    if (m_isNew){
        setStatusCatalog(StatusObject::Unknow);
        slot_StatusCatalogChanged();
        ui->editName->setText(tr("Contract comercial"));
    }
}

void ContractDialog::slot_IdChanged()
{
    if (m_id <= 0)
        return;

    // blocam signale ca sa nu fie modificarea formei
    QList<QComboBox*> combos = findChildren<QComboBox*>();
    QList<QLineEdit*> lines = findChildren<QLineEdit*>();

    std::vector<QSignalBlocker> comboBlockers; // vector STL care va contine obiecte QSignalBlocker
    comboBlockers.reserve(combos.size());      // rezervarea de memorie pentru elemente

    std::vector<QSignalBlocker> lineBlockers; // vector STL care va contine obiecte QSignalBlocker
    lineBlockers.reserve(lines.size());       // rezervarea de memorie pentru elemente

    for (QComboBox *combo : std::as_const(combos)) {
        comboBlockers.emplace_back(combo); // vectorul creeaza obiectul in interiorul sau = QSignalBlocker(combo)
    }

    for (QLineEdit *line_ed : std::as_const(lines)) {
        lineBlockers.emplace_back(line_ed); // vectorul creeaza obiectul in interiorul sau = QSignalBlocker(line_ed)
    }
    QSignalBlocker dt(ui->dateInit);
    QSignalBlocker b(ui->editComment);

    // textul solicitarii
    QString str_qry =
        globals().thisSqlite
            ? m_db.getTextSQL(":/sql/queries/contracts_select_by_id_sqlite.sql")
            : m_db.getTextSQL(":/sql/queries/contracts_select_by_id_mysql.sql");

    // solicitarea
    QSqlQuery qry;
    qry.prepare(str_qry);
    qry.addBindValue(m_id);
    if (qry.exec() && qry.next()) {

        QSqlRecord rec = qry.record();

        // setam status catalogului
        setStatusCatalog(
            StatusObject::determineStatusObject(rec.value("deletionMarc").toInt())
            );

        // combo 'Organizatia'
        int id_org = qry.value(rec.indexOf("id_organizations")).toInt();
        setIdOrganization(id_org);

        // combo 'Tipul preturilor'
        int _id_type_price = qry.value(rec.indexOf("id_typesPrices")).toInt();
        setIdTypePrice(_id_type_price);

        ui->editName->setText(qry.value(rec.indexOf("name")).toString());
        ui->dateInit->setDate(QDate::fromString(qry.value(rec.indexOf("dateInit")).toString(), "dd.MM.yyyy"));
        ui->checkBoxNotValid->setChecked(qry.value(rec.indexOf("notValid")).toBool());
        ui->editComment->setPlainText(qry.value(rec.indexOf("comment")).toString());
    }
    setWindowTitle(tr("Contract (salvat) %1").arg("[*]"));
}

void ContractDialog::slot_IdOrganizationChanged()
{
    if (m_idOrganization < 0)
        return;

    ui->comboOrganizations->setCurrentIndex(model_organizations->rowById("id", m_idOrganization));
}

void ContractDialog::slot_IdTypePriceChanged()
{
    if (m_idTypePrice < 0)
        return;

    ui->comboTypesPrices->setCurrentIndex(model_typesPrices->rowById("id", m_idTypePrice));
}

void ContractDialog::slot_StatusCatalogChanged()
{
    switch (m_statusCatalog) {
    case StatusObject::Unknow:
        setWindowTitle(tr("Contract (crearea) %1").arg("[*]"));
        break;
    case StatusObject::ZeroWrite:
        setWindowTitle(tr("Contract (salvat) %1").arg("[*]"));
        break;
    case StatusObject::DeletionMark:
        setWindowTitle(tr("Contract (marcat pentru eliminare) %1").arg("[*]"));
        break;
    default:
        setWindowTitle(tr("Contract (crearea) %1").arg("[*]"));
        break;
    }

}

void ContractDialog::changedIndexComboOrganization(const int index)
{
    Q_UNUSED(index);

    // determinam rolul
    auto roleID            = model_organizations->roleForColumn("id");
    auto role_id_typePrice = model_organizations->roleForColumn("id_typePrice");
    // variabile
    const int id_organization = ui->comboOrganizations->currentData(roleID).toInt();
    const int id_typePrice    = ui->comboOrganizations->currentData(role_id_typePrice).toInt();

    // setam ID organizatiei
    if (id_organization > 0)
        setIdOrganization(id_organization);

    // setam ID typePrice
    if (id_typePrice > 0)
        setIdTypePrice(id_typePrice);

    dataWasModified(); // modificam forma
}

void ContractDialog::currentIndexTypesPricesChanged(const int index)
{
    Q_UNUSED(index);

    // rolul si ID necesare
    auto roleID = model_typesPrices->roleForColumn("id");
    const int id_typePrice = ui->comboTypesPrices->currentData(roleID).toInt();

    // setam ID typePrice
    if (id_typePrice > 0)
        setIdTypePrice(id_typePrice);

    dataWasModified(); // modificam forma
}

bool ContractDialog::onWritingData()
{
    // verificam completarea campurilor obligatorii
    if (! controlRequiredObjects())
        return false;

    QStringList err; // anuntam variabila pu erori

    /* cream o functie lambda locala in interiorul pu oprimizarea codului
     * si eliminarea dublajului */
    auto showError = [&](const QString &title) {
        CustomMessage *message = new CustomMessage(this);
        message->setTextTitle(title.arg(ui->editName->text()));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
    };

    if (m_isNew){

        setId(m_db.getLastIdForTable("contracts") + 1);

        if (! insertIntoTableContracts(err)){
            showError(tr("Salvarea datelor contractului \"<b>%1</b>\" nu s-a efectuat."));
            return false;
        }

        setIsNew(false);          // setam ca obiectul nu este nou
        emit contractCreated(); // emitem signalul pu actualizarea tabela in Organizatia

    } else {

        if (! updateDataTableContracts(err)){
            showError(tr("Modificarea datelor contractului \"<b>%1</b>\" nu s-a efectuat."));
            return false;
        }

        emit contractChanged(); // emitem signalul pu actualizarea tabela in Organizatia
    }

    setWindowModified(false);

    return true;
}

void ContractDialog::onWritingDataClose()
{
    if (m_statusCatalog == StatusObject::Unknow)
        setStatusCatalog(StatusObject::ZeroWrite);

    if (!onWritingData()){
        setStatusCatalog(StatusObject::Unknow);
        return;
    }
    QDialog::accept();
}

void ContractDialog::onClose()
{
    this->close();
}

void ContractDialog::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(styleButtonMessageBox);
        noButton->setStyleSheet(styleButtonMessageBox);
        cancelButton->setStyleSheet(styleButtonMessageBox);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton){
            onWritingDataClose();
            event->accept();
        } else if (messange_box.clickedButton() == noButton){
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void ContractDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Contract %1").arg("[*]"));
    }
}

void ContractDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
    }
}
