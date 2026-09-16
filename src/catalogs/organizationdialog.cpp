#include "organizationdialog.h"
#include "ui_organizationdialog.h"

#include <QBuffer>

static const int max_length_comment = 255; // lungimea maxima a cometariului

OrganizationDialog::OrganizationDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , m_statusCatalog(StatusObject::Unknow) // initial
    , ui(new Ui::OrganizationDialog)
    , m_settings(globals().pathSettingsCommon)
    , m_db(db)
    , popUp(new PopUp(this))
    , styleBtnMessageBox(m_db.getStyleForButtonMessageBox())
    , toolBar(new ToolBarCustom(this,
                                 ToolBarCustom::AddEditDelete |
                                 ToolBarCustom::UpdateColumn))
    , modelContract(new OrganizationContractModel(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("Organizația %1").arg("[*]"));
    setWindowIcon(QIcon(":/img/catalogs/company.png"));

    // initializam imaginea stampilei
    ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->img_stamp->setTextFormat(Qt::RichText);
    ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    loadFilterBySettings();
    initModelContract();
    loadsizeSectionsContractView();

    initBtnToolBar();
    initConnections();
}

OrganizationDialog::~OrganizationDialog()
{
    delete ui;
}

bool OrganizationDialog::setDeleteMarkOrganization(QString &err)
{
    return handleDeletionMark(err);
}

void OrganizationDialog::slot_IsNewChanged()
{
    if (m_isNew){
        setStatusCatalog(StatusObject::Unknow);
        slot_StatusCatalogChanged(); // fortam apelarea din cauza ca in macro:
    }                                // if (m_##name == value) return;
}

void OrganizationDialog::slot_IdChanged()
{
    if (m_id <= 0)
        return;

    // blocam signale pu modificarea formei
    QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : std::as_const(items))
        itemsBlockers.emplace_back(item);

    QSignalBlocker cm(ui->editComment);

    QSqlQuery qry;
    qry.prepare("SELECT * FROM organizations WHERE id = ?");
    qry.addBindValue(m_id);
    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
            << "SQL error:" << qry.lastError().text()
            << "\nQuery:" << qry.lastQuery();
        return;
    }

    if (qry.next()) {
        QSqlRecord record = qry.record();

        setStatusCatalog(
            StatusObject::determineStatusObject(record.value("deletionMark").toInt())
            );

        ui->editName->setText(record.value("name").toString());
        ui->editIDNP->setText(record.value("IDNP").toString());
        ui->editTVA->setText(record.value("TVA").toString());
        ui->editAddress->setText(record.value("address").toString());
        ui->editTelephone->setText(record.value("telephone").toString());
        ui->editEmail->setText(record.value("email").toString());
        ui->editComment->setText(record.value("comment").toString());

        // contract
        const int id_contract = record.value("id_contracts").toInt();
        if (id_contract > 0) {
            setIdContract(id_contract);
            updateModelContract();
        }

        // stampila
        QByteArray arr_stamp = QByteArray::fromBase64(record.value("stamp").toByteArray());
        QPixmap pix_stamp;
        if (! arr_stamp.isEmpty() && pix_stamp.loadFromData(arr_stamp)){
            ui->img_stamp->setPixmap(pix_stamp.scaled(200, 200,
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation));
        }
    }
}

void OrganizationDialog::slot_IdContractChanged()
{
    modelContract->setMainContractId(m_idContract);

    QModelIndex index = ui->tableViewContracts->currentIndex();

    if (!index.isValid())
        return;

    const int row = index.row();
    QVariantMap rowData = modelContract->rowDataByRow(row);
    const QString nameContract = rowData["name"].toString();

    popUp->setPopupText(tr("Contractul <b>%1</b><br>setat implicit.")
                            .arg(nameContract));
    popUp->show();
}

void OrganizationDialog::slot_StatusCatalogChanged()
{
    switch (m_statusCatalog) {
    case StatusObject::Unknow:
        setWindowTitle(tr("Organizația (crearea) %1").arg("[*]"));
        break;
    case StatusObject::ZeroWrite:
        setWindowTitle(tr("Organizația (salvată) %1").arg("[*]"));
        break;
    case StatusObject::DeletionMark:
        setWindowTitle(tr("Organizația (marcată pentru eliminare) %1").arg("[*]"));
        break;
    default:
        setWindowTitle(tr("Organizația %1").arg("[*]"));
        break;
    }
}

void OrganizationDialog::dataWasModified()
{
    setWindowModified(true);
}

void OrganizationDialog::onAddContract()
{
    if (!confirmSaveIfModified())
        return;

    ContractDialog *contract = new ContractDialog(m_db, this);
    contract->setAttribute(Qt::WA_DeleteOnClose);
    contract->setProperty("isNew", true);
    contract->setProperty("idOrganization", m_id);
    connect(contract, &ContractDialog::contractCreated,
            this, &OrganizationDialog::updateModelContract);
    contract->show();
}

void OrganizationDialog::onEditContract()
{
    if (m_id <= 0)
        return;

    if (!confirmSaveIfModified())
        return;

    QModelIndex index = ui->tableViewContracts->currentIndex();

    if (!index.isValid())
        return;

    const int row = index.row();

    const int id = modelContract->idByRow(row);
    if (id <= 0)
        return;

    ContractDialog *contract = new ContractDialog(m_db, this);
    contract->setAttribute(Qt::WA_DeleteOnClose);
    contract->setProperty("isNew", false);
    contract->setProperty("id", id);
    connect(contract, &ContractDialog::contractChanged,
            this, &OrganizationDialog::updateModelContract);
    contract->show();
}

void OrganizationDialog::onDeleteContract()
{
    const QModelIndex idx = ui->tableViewContracts->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return;
    }

    const int row = idx.row();

    // extragem datele contractului
    const QVariantMap items = modelContract->rowDataByRow(row);
    bool isMark = items["deletionMark"].toInt() == StatusObject::DeletionMark;
    const int id = items["id"].toInt();
    if (id == 0)
        return;

    // verificam daca contractul selectat este implicit
    if (id == m_idContract) {
        QMessageBox messageBox(QMessageBox::Question,
                               tr("Verificarea datelor"),
                               tr("Contractul selectat este implicit.<br>"
                                  "Doriți să continuati?"),
                               QMessageBox::NoButton,
                               this);

        QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(styleBtnMessageBox);
        noButton->setStyleSheet(styleBtnMessageBox);
        messageBox.exec();

        if (messageBox.clickedButton() == yesButton) {}
        if (messageBox.clickedButton() == noButton) return;
    }

    // marcam contractul
    QString err;
    ContractDialog *contract = new ContractDialog(m_db, this);
    contract->setProperty("isNew", false);
    contract->setProperty("id", id);
    if (isMark)
        contract->setProperty("statusCatalog", StatusObject::ZeroWrite);
    else
        contract->setProperty("statusCatalog", StatusObject::DeletionMark);
    connect(contract, &ContractDialog::contractDeletedMark,
            this, &OrganizationDialog::updateModelContract, Qt::UniqueConnection);
    if (contract->setDeleteMarkOrganization(err)) {
        popUp->setPopupText(
            isMark
                ? tr("Contractul <b>%1</b><br>"
                     "nu mai este marcată pentru eliminare.")
                      .arg(items["name"].toString())
                : tr("Contractul <b>%1</b><br>"
                     "a fost marcată pentru eliminare.")
                      .arg(items["name"].toString())
            );
        popUp->show();

        // daca vrem sa marcam pu eliminare si
        // ID contractului == cu ID contractMain eliminam contract implicit
        if (! isMark || id == m_idContract)
            onDeleteContractMain();
    } else {
        if (err.isEmpty())
            return;
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Marcarea contractului '%1' nu s-a efectuat !!!")
                              .arg(items["name"].toString()));
        msg->setDetailedText(err);
        msg->exec();
        msg->deleteLater();
    }
    contract->deleteLater();
}

void OrganizationDialog::onShowHideColumnTableContract()
{

}

void OrganizationDialog::onSetContractMain()
{
    if (m_id <= 0)
        return;

    if (!confirmSaveIfModified())
        return;

    QModelIndex index = ui->tableViewContracts->currentIndex();

    if (!index.isValid())
        return;

    const int row = index.row();
    QVariantMap rowData = modelContract->rowDataByRow(row);
    const QString nameContract = rowData["name"].toString();

    const int id = modelContract->idByRow(row);
    if (id <= 0)
        return;

    QSqlQuery q;
    q.prepare("UPDATE organizations SET id_contracts = :id_contracts WHERE id = :id");
    q.bindValue(":id_contracts", id);
    q.bindValue(":id", m_id);
    if (q.exec()) {
        setIdContract(id);
    } else {
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Contractul '%1' nu este setat implicit !!!").arg(nameContract));
        message->setDetailedText(q.lastError().text());
        message->exec();
        message->deleteLater();
    }
}

void OrganizationDialog::onDeleteContractMain()
{
    if (m_id <= 0 || m_isNew)
        return;

    QSqlQuery q;
    q.prepare("UPDATE organizations SET id_contracts = :id_contracts WHERE id = :id");
    q.bindValue(":id_contracts", QVariant());
    q.bindValue(":id", m_id);
    if (q.exec()) {
        m_idContract = -1;
        slot_IdContractChanged(); // fortam apelarea
        popUp->setPopupText(tr("Organizatia <b>%1</b> nu are contract implicit.")
                                .arg(ui->editName->text()));
        popUp->show();
    } else {
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Eroare de eliminare a contractului implicit !!!"));
        message->setDetailedText(q.lastError().text());
        message->exec();
        message->deleteLater();
    }
}

void OrganizationDialog::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = ? WHERE id = ?");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_id);
    if (qry.exec()){
        ui->img_stamp->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->img_stamp->setTextFormat(Qt::RichText);
        ui->img_stamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminată din baza de date."));
        popUp->show();
        globals().main_stamp_organization = nullptr;
    } else {
        // formam textul erorii
        err.clear();
        err << this->metaObject()->className()
            << "[clearImageStamp]"
            << tr("Eroare la eliminarea imaginei din baza de date %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());
        // logarea
        qCritical(logCritical()) << err;
        // prezentam textul erorii
        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(QGuiApplication::applicationDisplayName());
        message->setTextTitle(tr("Eliminarea imaginei nu s-a efectuat !!!"));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
    }
}

bool OrganizationDialog::loadFile(const QString &fileName)
{
    // 1. Citim imaginea cu auto-transformare
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                     .arg(QDir::toNativeSeparators(fileName),
                                          reader.errorString()));
        return false;
    }

    // 2. Convertim imaginea într-un QPixmap scalat o singură dată
    QPixmap scaledPixmap = QPixmap::fromImage(newImage)
                               .scaled(200, 200,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);

    // 3. Setăm imaginea în QLabel potrivit
    ui->img_stamp->setPixmap(scaledPixmap);

    // 4. Citim fișierul binar pentru stocare în baza de date
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning())
            << tr("Nu s-a putut deschide fișierul pentru citire: ")
            << fileName;
        return true; // Nu blocăm operația chiar dacă nu-l putem salva
    }

    // 5. pregatim byteArray
    QByteArray inByteArray = file.readAll();
    file.close();

    // 6. actualizam datele organizatiei
    QSqlQuery qry;
    qry.prepare("UPDATE organizations SET stamp = ? WHERE id = ?");
    qry.addBindValue(inByteArray.toBase64());
    qry.addBindValue(m_id);
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
        globals().main_stamp_organization = QByteArray::fromBase64(inByteArray.toBase64());
        qInfo(logInfo())
            << QStringLiteral("A fost inserata imaginea organizatiei %1")
                   .arg(ui->editName->text());
    } else {

        err.clear();
        err << this->metaObject()->className()
            << "[loadFile]"
            << tr("Eroare la salvarea imaginii în baza de date %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qWarning(logWarning()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea imaginei nu s-a efectuat !!!"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }

    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
                                                  ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void OrganizationDialog::onLinkActivatedForOpenImage(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id <= 0){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea validarii"),
                                 tr("Pentru a seta imagine este necesar de salvat datele.<br>"
                                    "Doriti sa salvati datele ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(styleBtnMessageBox);
        noButton->setStyleSheet(styleBtnMessageBox);
        cancelButton->setStyleSheet(styleBtnMessageBox);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton)
            onWritingData();
        else if (messange_box.clickedButton() == noButton)
            return;
        else if (messange_box.clickedButton() == cancelButton)
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst());
    dialog.close();
}

void OrganizationDialog::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void OrganizationDialog::onDoubleClickedTableContracts(const QModelIndex &index)
{
    Q_UNUSED(index)
    onEditContract();
}

void OrganizationDialog::initModelContract()
{
    ui->tableViewContracts->setModel(modelContract);
    ui->tableViewContracts->setColumnHidden(0, true); // ID
    ui->tableViewContracts->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewContracts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewContracts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewContracts->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableViewContracts->verticalHeader()->setDefaultSectionSize(12);
    ui->tableViewContracts->setColumnWidth(1, 5); // deletionMark -> Icon
    ui->tableViewContracts->setFocus();
    ui->tableViewContracts->selectRow(0);
}

void OrganizationDialog::updateModelContract()
{
    modelContract->setMainContractId(m_idContract);
    int currentId = -1;

    const QModelIndex currentIndex = ui->tableViewContracts->currentIndex();
    if (currentIndex.isValid())
        currentId = modelContract->idByRow(currentIndex.row());

    QList<QVariantMap> rows;

    QSqlQuery q;
    q.prepare(R"(
        SELECT
            id,
            deletionMark,
            name,
            dateInit
        FROM contracts
        WHERE
            id_organizations = ? AND
            notValid = 0
        ORDER BY
            name
    )");
    q.addBindValue(m_id);
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "updateModelContract error:"
            << q.lastError().text();
        modelContract->clear();
        return;
    }

    while (q.next()) {
        QVariantMap row;
        row.insert("id",           q.value("id"));
        row.insert("deletionMark", q.value("deletionMark"));
        row.insert("name",         q.value("name"));
        row.insert("dateInit",     q.value("dateInit"));
        rows.append(row);
    }

    modelContract->setRows(rows);

    if (currentId >= 0) {
        const int row = modelContract->rowById(currentId);
        if (row >= 0) {
            ui->tableViewContracts->selectRow(row);
            ui->tableViewContracts->scrollTo(modelContract->index(row, 0));
        }
    }
}

void OrganizationDialog::loadFilterBySettings()
{
    const QJsonObject rootObj = m_settings.getJsonObject(className);
    if (rootObj.isEmpty()) {
        return;
    }

    const QJsonObject tablegObj = rootObj.value("contractTable").toObject();
    if (tablegObj.isEmpty()) {
        return;
    }

    // --- sortarea sectiilor
    m_filterContract.sortSection = tablegObj.value("sort").toObject().value("section").toInt(0);
    m_filterContract.sortOrder = tablegObj.value("sort").toObject().value("direction").toInt(0) == 0
                             ? Qt::AscendingOrder
                             : Qt::DescendingOrder;

    // --- size section
    const QJsonObject sectionsObj = tablegObj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filterContract.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = tablegObj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filterContract.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void OrganizationDialog::initBtnToolBar()
{
    toolBar->setStyles(m_db.toolButtonStyleForIcon(),
                       m_db.toolButtonStyleForText());

    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addSpacing(10);

    QToolButton *btnContractMain = new QToolButton(this);
    btnContractMain->setStyleSheet(m_db.toolButtonStyleForText());
    btnContractMain->setText(tr("Contract implicit"));

    ui->layoutToolBar->addWidget(btnContractMain);
    ui->layoutToolBar->addStretch();

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &OrganizationDialog::onAddContract, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &OrganizationDialog::onEditContract, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &OrganizationDialog::onDeleteContract, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::updateTable,
            this, &OrganizationDialog::updateModelContract, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::hideShowColumn,
            this, &OrganizationDialog::onShowHideColumnTableContract, Qt::UniqueConnection);

    connect(btnContractMain, &QToolButton::clicked,
            this, &OrganizationDialog::onSetContractMain, Qt::UniqueConnection);
}

void OrganizationDialog::initConnections()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++)
        connect(list[n], &QLineEdit::textChanged,
                this, &OrganizationDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->editComment, &QTextEdit::textChanged,
            this, &OrganizationDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->img_stamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&OrganizationDialog::onLinkActivatedForOpenImage),
            Qt::UniqueConnection);

    connect(ui->btnClearStamp, &QToolButton::clicked,
            this, &OrganizationDialog::clearImageStamp, Qt::UniqueConnection);

    connect(ui->editComment, &QTextEdit::textChanged,
            this, &OrganizationDialog::controlLengthComment, Qt::UniqueConnection);

    connect(ui->tableViewContracts, &QTableView::doubleClicked,
            this, &OrganizationDialog::onDoubleClickedTableContracts, Qt::UniqueConnection);

    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &OrganizationDialog::onWritingDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &OrganizationDialog::onWritingData, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &OrganizationDialog::close, Qt::UniqueConnection);
}

bool OrganizationDialog::confirmSaveIfModified()
{
    if (!this->isWindowModified())
        return true;

    QMessageBox messageBox(QMessageBox::Question,
                           tr("Verificarea datelor"),
                           tr("Datele obiectului <b>%1</b> nu sunt salvate.<br>"
                              "Doriți să salvați datele?")
                               .arg(ui->editName->text()),
                           QMessageBox::NoButton,
                           this);

    QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);

    yesButton->setStyleSheet(styleBtnMessageBox);
    noButton->setStyleSheet(styleBtnMessageBox);

    messageBox.exec();

    if (messageBox.clickedButton() == yesButton) {
        return onWritingData(); // ✔ return direct
    }

    if (messageBox.clickedButton() == noButton) {
        return false;
    }

    return false;
}

bool OrganizationDialog::controlRequiredObjects()
{
    if (ui->editName->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->editName,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicata \"<b>Denumirea organizatiei</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }

    if (ui->editIDNP->text().isEmpty()) {
        BalloonTip::showBalloonFor(ui->editIDNP,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este indicat \"<b>IDNO</b>\" !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return false;
    }

    return true;
}

bool OrganizationDialog::confirmIfDuplicateExists()
{
    const QString organizationName = ui->editName->text().trimmed();

    if (organizationName.isEmpty())
        return false;

    const QString strQry =
        m_db.getTextSQL(":/sql/queries/check_object_exists_by_name.sql")
            .arg("organizations");

    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(organizationName);

    if (!qry.exec() || !qry.next())
        return false;

    const int exist = qry.value(0).toInt();
    if (exist <= 0)
        return false;

    const QString text = tr("Organizația cu denumirea <b>%1</b><br>"
                            "există în baza de date !!!<br>"
                            "Doriți să continuați ?")
                             .arg(organizationName.toHtmlEscaped());

    QMessageBox messageBox(QMessageBox::Question,
                           tr("Verificarea datelor"),
                           text,
                           QMessageBox::NoButton,
                           this);

    QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);

    yesButton->setStyleSheet(styleBtnMessageBox);
    noButton->setStyleSheet(styleBtnMessageBox);

    messageBox.exec();

    return messageBox.clickedButton() == noButton;
}

bool OrganizationDialog::handleInsert()
{
    if (confirmIfDuplicateExists())
        return false;

    QVector<QVariant> data;
    data.append(m_db.getLastIdForTable("organizations") + 1);
    data.append(StatusObject::statusObjectToInt(m_statusCatalog));
    data.append(ui->editIDNP->text());
    data.append(ui->editTVA->text().isEmpty()
                    ? QVariant()
                    : ui->editTVA->text());
    data.append(ui->editName->text());
    data.append(ui->editAddress->text().isEmpty()
                    ? QVariant()
                    : ui->editAddress->text());
    data.append(ui->editTelephone->text().isEmpty()
                    ? QVariant()
                    : ui->editTelephone->text());
    data.append(ui->editEmail->text().isEmpty()
                    ? QVariant()
                    : ui->editEmail->text());
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append((m_idContract <= 0)
                    ? QVariant()
                    : m_idContract);

    QByteArray image_data;
    QPixmap pix = ui->img_stamp->pixmap();
    if (! pix.isNull()) {
        QBuffer buffer(&image_data);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "PNG");
    }
    data.append(image_data.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QByteArray))
                    : image_data.toBase64());

    QUuid uuid = QUuid::createUuid();
    data.append(uuid.toRfc4122());

    QString err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                  ":/sql/queries/organizations_insert.sql",
                                  data,
                                  &err))
    {
        // logarea
        qCritical(logCritical()) << err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor organizatiei '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(err);
        msg->exec();
        msg->deleteLater();
        return false;
    }
    emit organizationCreated(data);
    return true;
}

bool OrganizationDialog::handleUpdate()
{
    QVector<QVariant> data;
    data.append(StatusObject::statusObjectToInt(m_statusCatalog));
    data.append(ui->editIDNP->text());
    data.append(ui->editTVA->text().isEmpty()
                    ? QVariant()
                    : ui->editTVA->text());
    data.append(ui->editName->text());
    data.append(ui->editAddress->text().isEmpty()
                    ? QVariant()
                    : ui->editAddress->text());
    data.append(ui->editTelephone->text().isEmpty()
                    ? QVariant()
                    : ui->editTelephone->text());
    data.append(ui->editEmail->text().isEmpty()
                    ? QVariant()
                    : ui->editEmail->text());
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append((m_idContract <= 0)
                    ? QVariant()
                    : m_idContract);

    QByteArray image_data;
    QPixmap pix = ui->img_stamp->pixmap();
    if (! pix.isNull()) {
        QBuffer buffer(&image_data);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "PNG");
    }
    data.append(image_data.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QByteArray))
                    : image_data.toBase64());

    data.append(m_id);

    QString err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                  ":/sql/queries/organizations_update.sql",
                                  data,
                                  &err))
    {
        // logarea
        qCritical(logCritical()) << err;

        // prezentarea mesajului cu eroare
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Modificarea datelor organizatiei '%1' nu s-a efectuat !!!")
                              .arg(ui->editName->text()));
        msg->setDetailedText(err);
        msg->exec();
        msg->deleteLater();
        return false;
    }

    emit organizationChanged(data);
    return true;
}

bool OrganizationDialog::handleDeletionMark(QString &err)
{
    err.clear();

    if (m_id <= 0) {
        err = tr("Nu este determinat ID-ul organizației.");
        return false;
    }

    if (m_isNew) {
        err = tr("Organizația nouă nu poate fi marcată pentru ștergere.");
        return false;
    }

    QSqlQuery q;
    q.prepare("UPDATE organizations SET deletionMark = :deletionMark WHERE id = :id");
    q.bindValue(":deletionMark", StatusObject::statusObjectToInt(m_statusCatalog));
    q.bindValue(":id", m_id);
    if (!q.exec()) {
        err = "SQL error: " + q.lastError().text();
        err += "\nQuery:" + q.lastQuery();
        qCritical(logCritical()).noquote()
            << "SQL error:" << err;
        return false;
    }

    emit organizationDeletedMark();

    return true;
}

bool OrganizationDialog::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    // anuntam variabila returnarii
    bool returnBool;

    // procesarea inserarii si actualizarii
    returnBool = m_isNew
                     ? handleInsert()
                     : handleUpdate();

    if (m_isNew)
        setIsNew(false); // setam ca nu este nou

    // modificarea formei
    if (returnBool)
        setWindowModified(false);

    return returnBool;
}

void OrganizationDialog::onWritingDataClose()
{
    if (m_statusCatalog == StatusObject::Unknow)
        setStatusCatalog(StatusObject::ZeroWrite);

    if (!onWritingData()){
        setStatusCatalog(StatusObject::Unknow);
        return;
    }
    accept();
}

void OrganizationDialog::saveSizeSectionsContractView()
{
    static const QString nameClass = "OrganizationDialog";

    for (int numSection = 0; numSection < ui->tableViewContracts->horizontalHeader()->count(); ++numSection) {

        // size sections
        int w = ui->tableViewContracts->horizontalHeader()->sectionSize(numSection);
        m_settings.setValue(nameClass, QString("contractTable/sections/%1").arg(numSection), w);

        // sortarea
        m_settings.setValue(nameClass, "contractTable/sort/section",
                            ui->tableViewContracts->horizontalHeader()->sortIndicatorSection());

        m_settings.setValue(nameClass, "contractTable/sort/direction",
                            static_cast<int>(ui->tableViewContracts->horizontalHeader()->sortIndicatorOrder()));

        // show/hide section
        m_settings.setValue(nameClass,
                            QString("contractTable/hide_show_sections/%1").arg(numSection),
                            ui->tableViewContracts->horizontalHeader()->isSectionHidden(numSection) ? 1 : 0);
    }

    m_settings.save();
}

void OrganizationDialog::loadsizeSectionsContractView()
{
    auto *header = ui->tableViewContracts->horizontalHeader();
    if (!header)
        return;

    ui->tableViewContracts->setUpdatesEnabled(false);

    const int colCount = header->count();

    for (int col = 0; col < colCount; ++col) {
        const int width = m_filterContract.sectionSizes.value(col, header->defaultSectionSize());
        header->resizeSection(col, width);

        // const bool hidden = m_filter.hiddenSections.value(col, false);
        // ui->tableView->setColumnHidden(col, hidden);
    }

    const int sortSection = m_filterContract.sortSection;
    const Qt::SortOrder sortOrder = m_filterContract.sortOrder;

    if (sortSection >= 0 && sortSection < colCount) {
        header->setSortIndicator(sortSection, sortOrder);
        ui->tableViewContracts->sortByColumn(sortSection, sortOrder);
    }

    if (ui->tableViewContracts->model() && ui->tableViewContracts->model()->rowCount() > 0)
        ui->tableViewContracts->selectRow(0);

    ui->tableViewContracts->setUpdatesEnabled(true);
}

void OrganizationDialog::closeEvent(QCloseEvent *event)
{
    saveSizeSectionsContractView();

    if (confirmSaveIfModified()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void OrganizationDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        if (m_isNew){
            setWindowTitle(tr("Organizația (crearea) %1").arg("[*]"));
        } else {
            setWindowTitle(tr("Organizația (salvată) %1").arg("[*]"));
        }
    }
}

void OrganizationDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
    }
}
