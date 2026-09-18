#include "pricingdialog.h"
#include "ui_pricingdialog.h"
#include <QScopeGuard>

PricingDialog::PricingDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PricingDialog)
    , m_db(db)
    , popUp(new PopUp(this))
    , modelTable(new TableDocModel(this))
    , proxy (new SortModel(this))
    , menu(new QMenu(this))
    , timer(new QTimer(this))
    , styleForButtonMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    // initial setam m_post = -1
    // macros seteaza m_post = 0 (write)
    // din cauza data setam -1
    setPost(DocStatus::Unknow);

    setTitleDoc(); // setam titlu documentului

    if (globals().showDocumentsInSeparatWindow)
        setWindowFlags(Qt::Window);

    setupDateDocFormat(); // setarile datei

    updateModelOrganizations();
    updateModelContracts();
    updateModelTypesPrices();

    initBtnToolBar(); // initierea butoanelor tool barului

    initTable();

    initFooterDoc();  // setam imaginea si numele utilizatorului

    initConnections();

    if (globals().isSystemThemeDark)
        ui->frame_table->setObjectName("customFrame");
}

PricingDialog::~PricingDialog()
{
    delete ui;
}

void PricingDialog::onPrintDocument(PrintType::Column type_print)
{
    onPrint(type_print);
}

void PricingDialog::dataWasModified()
{
    setWindowModified(true);
}

void PricingDialog::updateTimer()
{
    QDateTime now = QDateTime::currentDateTime();

    {
        QSignalBlocker b(ui->dateTimeDoc);
        ui->dateTimeDoc->setDateTime(now);
    }

    setWindowTitle(tr("Formarea prețurilor (crearea) %1 %2")
                       .arg(" nr." + ui->editNumberDoc->text() + " din " +
                                now.toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
}

void PricingDialog::onDateTimeChanged()
{
    timer->stop();
}

// **********************************************************************************
// --- procesarea slot-lor

void PricingDialog::slot_IsNewChanged()
{
    if (m_isNew){

        setWindowTitle(tr("Formarea prețurilor (crearea) %1").arg("[*]"));
        setIdUser(globals().idUserApp);

        // setam data
        ui->dateTimeDoc->setDateTime(QDateTime::currentDateTime());
        connect(timer, &QTimer::timeout,
                this, &PricingDialog::updateTimer, Qt::UniqueConnection);
        connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
                this, &PricingDialog::dataWasModified, Qt::UniqueConnection);
        connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
                this, &PricingDialog::onDateTimeChanged, Qt::UniqueConnection);
        timer->start(1000);

        // numar
        ui->editNumberDoc->setText(QString::number(m_db.getLastNumberDoc("pricings") + 1));
        ui->editNumberDoc->setEnabled(false);

    }
}

void PricingDialog::slot_IdChanged()
{
    if (m_id <= 0)
        return;

    // blocam signale ca sa nu fie modificarea formei
    QList<QComboBox*> combos = findChildren<QComboBox*>();
    std::vector<QSignalBlocker> comboBlockers; // vector STL care va contine obiecte QSignalBlocker
    comboBlockers.reserve(combos.size());      // rezervarea de memorie pentru elemente

    for (QComboBox *combo : std::as_const(combos)) {
        comboBlockers.emplace_back(combo); // vectorul creeaza obiectul in interiorul sau = QSignalBlocker(combo)
    }

    QSignalBlocker b_date(ui->dateTimeDoc);
    QSignalBlocker b_nr(ui->editNumberDoc);
    QSignalBlocker b_comm(ui->editComment);
    QSignalBlocker b_table(ui->tableView);

    QSqlQuery qry;
    qry.prepare(m_db.getTextSQL(":/sql/queries_doc/pricing_byID.sql"));
    qry.addBindValue(m_id);
    if (!qry.exec()) {
        qCritical(logCritical())
        << "SQL error:" << qry.lastError().text()
        << "Query error:" << qry.lastQuery();
        return;
    }

    if (qry.next()) {
        // nr.documentului
        ui->editNumberDoc->setText(qry.value(PricingSections::NumberDoc).toString());
        ui->editNumberDoc->setDisabled(true);

        // data
        const QString dt = qry.value(PricingSections::DateDoc).toString();
        ui->dateTimeDoc->setDateTime(QDateTime::fromString(dt, Qt::ISODate));

        // ID-le
        setIdOrganization(qry.value(PricingSections::Id_Organizations).toInt());
        setIdContract(qry.value(PricingSections::Id_Contracts).toInt());
        setIdTypePrice(qry.value(PricingSections::Id_TypesPrices).toInt());
        setIdUser(qry.value(PricingSections::Id_Users).toInt());

        // comentariu
        ui->editComment->setText(qry.value(PricingSections::Comment).toString());

    }

    updateTableView();

    setWindowTitle(tr("Formarea prețurilor (validat) %1 %2")
                       .arg("nr." + ui->editNumberDoc->text() + " din " +
                                ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
}

void PricingDialog::slot_IdOrganizationChanged()
{
    if (m_idOrganization < 0)
        return;

    ui->comboOrganization->setCurrentIndex(modelOrganizations->rowById("id", m_idOrganization));
}

void PricingDialog::slot_IdContractChanged()
{
    if (m_idContract < 0)
        return;

    ui->comboContract->setCurrentIndex(modelContracts->rowById("id", m_idContract));
}

void PricingDialog::slot_IdTypePriceChanged()
{
    if (m_idTypePrice < 0)
        return;

    ui->comboTypesPricing->setCurrentIndex(modelTypesPrices->rowById("id", m_idTypePrice));
}

void PricingDialog::slot_IdUserChanged()
{
    if (m_idUser <= 0)
        return;
}

void PricingDialog::slot_PostChanged()
{
    qInfo(logInfo()) << "A fost schimbat statutul documenului";
}

// **********************************************************************************
// --- procesarea idx combo

void PricingDialog::indexChangedCombo(int index)
{
    Q_UNUSED(index);

    QComboBox *combo = qobject_cast<QComboBox*>(sender());

    if (combo == ui->comboOrganization) {
        // determinam rolul
        auto roleID            = modelOrganizations->roleForColumn("id");
        auto role_id_contract  = modelOrganizations->roleForColumn("id_contracts");
        auto role_id_typePrice = modelOrganizations->roleForColumn("id_typePrice");
        // variabile
        const int id_organization = ui->comboOrganization->currentData(roleID).toInt();
        const int id_contracts    = ui->comboOrganization->currentData(role_id_contract).toInt();
        const int id_typePrice    = ui->comboOrganization->currentData(role_id_typePrice).toInt();

        // setam ID organizatiei
        if (id_organization > 0)
            setIdOrganization(id_organization);

        // actualizam modelul contracte, apoi setam ID contractului
        updateModelContracts();

        // setam ID contractului
        if (id_contracts > 0)
            setIdContract(id_contracts);

        // setam ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);

        dataWasModified(); // modificam forma

    } else if (combo == ui->comboContract) {
        // determinam rolul
        auto roleID            = modelContracts->roleForColumn("id");
        auto role_id_typePrice = modelContracts->roleForColumn("id_typesPrices");
        // ID necesare
        const int id_contracts = ui->comboContract->currentData(roleID).toInt();
        const int id_typePrice = ui->comboContract->currentData(role_id_typePrice).toInt();

        // setam ID contractului
        if (id_contracts > 0)
            setIdContract(id_contracts);

        // ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);
        else
            ui->comboTypesPricing->setCurrentIndex(0);

        dataWasModified(); // modificarea formei

    } else if (combo == ui->comboTypesPricing) {
        // rolul si ID necesare
        auto roleID = modelTypesPrices->roleForColumn("id");
        const int id_typePrice = ui->comboTypesPricing->currentData(roleID).toInt();

        // setam ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);

        dataWasModified(); // modificam forma

    }
}

// **********************************************************************************
// --- procesarea actiunilor btn si table

void PricingDialog::openCatOrganization()
{
    if (m_idOrganization <= 0)
        return;

    qInfo(logInfo()) << tr("Editarea/vizualizarea datelor organizatiei '%1' cu id='%2'.")
                            .arg(ui->comboOrganization->currentText(),
                                 QString::number(m_idOrganization));

    OrganizationDialog *organization = new OrganizationDialog(m_db, this);
    organization->setAttribute(Qt::WA_DeleteOnClose);
    organization->setProperty("isNew", false);
    organization->setProperty("id", m_idOrganization);
    organization->show();
}

void PricingDialog::openCatContract()
{
    if (m_idContract <= 0)
        return;

    qInfo(logInfo()) << tr("Editarea/vizualizarea datelor contractului '%1' cu id='%2'.")
                            .arg(ui->comboContract->currentText(),
                                 QString::number(m_idContract));

    ContractDialog *cat_contract = new ContractDialog(m_db, this);
    cat_contract->setAttribute(Qt::WA_DeleteOnClose);
    cat_contract->setProperty("isNew", false);
    cat_contract->setProperty("id", m_idContract);
    cat_contract->show();

}

void PricingDialog::addRowTable()
{
    CatalogTableEditor *catalog = new CatalogTableEditor(m_db,
                                                         CatalogType::FormType::Selection,
                                                         CatalogType::Type::Investigations,
                                                         this);
    catalog->setGeometry(1000, 400, 800, 400);
    connect(catalog, &CatalogTableEditor::dataSelected,
            this, &PricingDialog::getDataSelectable, Qt::UniqueConnection);
    catalog->show();
}

void PricingDialog::editRowTable()
{
    int row = ui->tableView->currentIndex().row();
    ui->tableView->edit(proxy->index(row, PricingsTableSection::Price));
}

void PricingDialog::deletionRowTable()
{
    int row = ui->tableView->currentIndex().row();
    proxy->removeRow(row);
}

void PricingDialog::populateTable()
{
    if (modelTable->rowCount() > 0){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Tabela nu este goal\304\203. <br>"
                                    "Dori\310\233i s\304\203 goli\310\233i tabela ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(styleForButtonMessageBox);
        noButton->setStyleSheet(styleForButtonMessageBox);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            modelTable->removeRows(0, modelTable->rowCount());
        } else if (messange_box.clickedButton() == noButton) {

        }
    }

    /* solicitarea pu completarea tabelei din clasa 'CatalogTableEditor'
     * si tabela 'investigations' din BD sqlite */
    QMap<QString, QString> items;
    QString strQry =
        R"(
        SELECT
            cod,
            name
        FROM
            investigations
        WHERE
            deletionMark = 0 AND
            `use` = 1
        GROUP BY cod
        ORDER BY cod
    ;)";

    QSqlQuery qry;
    qry.prepare(strQry);
    if (!qry.exec()) {
        qCritical(logCritical())
        << "SQL error:" << qry.lastError().text()
        << "Query error:" << qry.lastQuery();
    }

    while (qry.next()) {
        const int row = modelTable->rowCount();
        modelTable->insertRow(row);
        modelTable->setData(modelTable->index(row, PricingsTableSection::DeletionMark), DocStatus::Write);
        modelTable->setData(modelTable->index(row, PricingsTableSection::Id_Pricings), 0);
        modelTable->setData(modelTable->index(row, PricingsTableSection::Cod),  qry.value(0).toString());
        modelTable->setData(modelTable->index(row, PricingsTableSection::Name), qry.value(1).toString());
        modelTable->setData(modelTable->index(row, PricingsTableSection::Price), 0);
    }

}

void PricingDialog::filterRegExpChanged()
{
    QString typeSearch = ui->editSearch->getSearchOptionSelected();
    if (typeSearch == "code")
        proxy->setFilterKeyColumn(PricingsTableSection::Cod);
    else if (typeSearch == "name")
        proxy->setFilterKeyColumn(PricingsTableSection::Name);
    else
        return;

    QRegularExpression regExp(ui->editSearch->text(),
                              QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(regExp);
}

void PricingDialog::getDataSelectable(const QVariantMap data)
{
    const QString cod  = data["cod"].toString();
    const QString name = data["name"].toString();

    for (int n = 0; n < proxy->rowCount(); n++) {
        QModelIndex idx_cod = proxy->index(n, PricingsTableSection::Cod);
        const QString current_cod = ui->tableView->model()->data(idx_cod, Qt::DisplayRole).toString();
        if (current_cod == cod){
            QMessageBox::warning(this,
                                 tr("Atentie"),
                                 tr("Investigatia <b>'%1 - %2'</b> exista in tabel.")
                                     .arg(cod, name),
                                 QMessageBox::Ok);
            ui->tableView->setCurrentIndex(proxy->index(n, PricingsTableSection::Name));
            return;
        }
    }

    int row = modelTable->rowCount();
    modelTable->insertRow(row);
    modelTable->setData(modelTable->index(row, PricingsTableSection::DeletionMark), DocStatus::Write);
    modelTable->setData(modelTable->index(row, PricingsTableSection::Id_Pricings), m_id);
    modelTable->setData(modelTable->index(row, PricingsTableSection::Cod), cod);
    modelTable->setData(modelTable->index(row, PricingsTableSection::Name), name);
    modelTable->setData(modelTable->index(row, PricingsTableSection::Price), 0);
    ui->tableView->setCurrentIndex(modelTable->index(row, PricingsTableSection::Price));
}

void PricingDialog::onClickedRowTable(const QModelIndex &index)
{
    const int row = index.row();
    QModelIndex idx_price = proxy->index(row, PricingsTableSection::Price);
    ui->tableView->setCurrentIndex(idx_price);
    ui->tableView->edit(idx_price);
}

// **********************************************************************************
// --- printasre, validare, inchidere

void PricingDialog::onPrint(PrintType::Column type_print)
{
    // *************************************************************************************
    // alocam memoria
    m_report = new LimeReport::ReportEngine(this);
    const auto cleanup = qScopeGuard([this] {
        m_report->deleteLater();
        m_report = nullptr;
        delete m_owner;
        m_owner = nullptr;
        delete m_investigations;
        m_investigations = nullptr;
    });

    m_report->setPreviewWindowTitle(tr("Lista pre\310\233urilor nr.") + ui->editNumberDoc->text() +
                                    tr(" din ") + ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss"));

    // solicitarea pentru grupe
    m_owner = new QSqlQuery(m_db.getDatabase());
    if (!m_owner->exec(
        R"(
        SELECT DISTINCT
            investigationsGroup.id AS group_id,
            investigationsGroup.name AS owner,
            investigationsGroup.cod
        FROM
            investigations
        INNER JOIN
            investigationsGroup ON investigations.owner = investigationsGroup.id
        WHERE
            investigations.owner IS NOT NULL
        ORDER BY
            investigationsGroup.cod ASC;
        )")) {
        qCritical(logCritical()) << "PricingDialog: citirea grupelor pentru printare a eșuat:"
                                 << m_owner->lastError().text();
        QMessageBox::warning(this, tr("Printarea documentului"),
                             tr("Grupele de investigații nu au putut fi citite. Verificați jurnalul."));
        return;
    }
    if (!m_owner->first()) {
        qInfo(logInfo()) << "PricingDialog: nu există grupe de investigații pentru printare.";
        QMessageBox::information(this, tr("Printarea documentului"),
                                 tr("Nu există investigații asociate grupelor pentru printare."));
        return;
    }

    // solicitarea elementelor investigatiilor
    QMessageBox messange_box(QMessageBox::Question,
                             tr("Prezentarea listei pre\310\233urilor"),
                             tr("De prezentat investiga\310\233iile f\304\203r\304\203 pre\310\233 ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(styleForButtonMessageBox);
    noButton->setStyleSheet(styleForButtonMessageBox);
    messange_box.exec();

    QString str_investigations;
    if (messange_box.clickedButton() == yesButton) {

        str_investigations =
            QStringLiteral(R"(
                SELECT
                    investigations.cod,
                    investigations.name,
                    %1 AS price
                FROM
                    investigations
                INNER JOIN
                    pricingsTable ON pricingsTable.cod = investigations.cod
                WHERE
                    pricingsTable.id_pricings = ? AND
                    investigations.owner = ?;
            )").arg((globals().thisMySQL)
                         ? "FORMAT(pricingsTable.price, 2)"
                         : "printf('%.2f', pricingsTable.price)");
        // .arg((globals().thisMySQL) ?
        //          "CASE WHEN FORMAT(pricingsTable.price, 2) = '0.00' THEN '' ELSE FORMAT(pricingsTable.price, 2) END" :
        //          "CASE WHEN printf('%.2f', pricingsTable.price) = '0.00' THEN '' ELSE printf('%.2f', pricingsTable.price) END");

    } else if (messange_box.clickedButton() == noButton) {

        str_investigations =
            QStringLiteral(R"(
                SELECT
                    investigations.cod,
                    investigations.name,
                    %1 AS price
                FROM
                    investigations
                INNER JOIN
                    pricingsTable ON pricingsTable.cod = investigations.cod
                WHERE
                    pricingsTable.id_pricings = ? AND
                    pricingsTable.price > 0 AND
                    investigations.owner = ?
            )").arg((globals().thisMySQL)
                         ? "FORMAT(pricingsTable.price, 2)"
                         : "printf('%.2f', pricingsTable.price)");

    }

    if (str_investigations.isEmpty())
        return;

    m_investigations = new QSqlQuery(m_db.getDatabase());
    if (!m_investigations->prepare(str_investigations)) {
        qCritical(logCritical()) << "PricingDialog: pregătirea investigațiilor pentru printare a eșuat:"
                                 << m_investigations->lastError().text();
        return;
    }
    m_investigations->bindValue(0, m_id);
    m_investigations->bindValue(1, m_owner->value("group_id"));
    if ( !m_investigations->exec()) {
        qCritical() << "Error: Unable to execute investigations query:"
                    << m_investigations->lastError().text();
        return;
    }

    // conectarile
    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_owner");
    connect(callbackDatasource,
            QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this,
            QOverload<LimeReport::CallbackInfo, QVariant &>::of(&PricingDialog::slotGetCallbackData));
    connect(callbackDatasource,
            QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&PricingDialog::slotChangePos));

    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_items");
    connect(callbackDatasource,
            QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this,
            QOverload<LimeReport::CallbackInfo, QVariant &>::of(&PricingDialog::slotGetCallbackDataItems));
    connect(callbackDatasource,
            QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this,
            QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&PricingDialog::slotChangePosItems));

    // variabile
    m_report->dataManager()->setReportVariable("id_pricing", m_id);
    m_report->dataManager()->setReportVariable("organization", ui->comboOrganization->currentText());
    m_report->dataManager()->setReportVariable("date", ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy"));

    // incarcarea fisierului
    if (! m_report->loadFromFile(globals().docsTemplatesPath + "/Pricing.lrxml")){
        QDir dir;
        CustomMessage *msgBox = new CustomMessage(this);
        msgBox->setWindowTitle(tr("Printarea documentului"));
        msgBox->setTextTitle(tr("Documentul nu poate fi printat."));
        msgBox->setDetailedText(tr("Nu a fost gasit fi\310\231ierul formei de tipar:\n%1")
                                    .arg(dir.toNativeSeparators(globals().docsTemplatesPath + "/Pricing.lrxml")));
        msgBox->exec();
        msgBox->deleteLater();

        return;
    }

#ifdef QT_DEBUG
    Q_UNUSED(type_print)
    m_report->designReport();
#else
    if (type_print == PrintType::Designer){
        m_report->designReport();
    } else {
        m_report->previewReport();
    }
#endif

}

bool PricingDialog::controlRequiredObjects()
{
    if (ui->comboOrganization->currentIndex() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectată <b>'Organizația'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    if (ui->comboContract->currentIndex() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Contractul'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    if (ui->comboTypesPricing->currentIndex() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Tipul prețului'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    if (ui->tableView->model()->rowCount() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Tabela cu investigații și prețurile este pustie !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    if (m_idUser == -1){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este determinat <b>'Autorul'</b> documentului !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    return true;
}

bool PricingDialog::onWritingData()
{
    if (m_post == DocStatus::Unknow)  // daca -1, atunci presupunem
        setPost(DocStatus::Write);    // ca a fost apasat btnWrite

    if (! controlRequiredObjects())   // verificam completarea datelor
        return false;

    QString err;

    if (m_isNew){

        m_id = m_db.getLastIdForTable("pricings") + 1; // setam 'id'

        if (!insertDataTablePricings(err)) {
            CustomMessage *msg = new CustomMessage(this);
            msg->setWindowTitle(QGuiApplication::applicationDisplayName());
            msg->setTextTitle(tr("Validarea documentului nu s-a efectuat."));
            msg->setDetailedText(err);
            msg->exec();
            msg->deleteLater();

            setId(StatusObject::Unknow);
            setPost(DocStatus::Unknow);
            return false;
        }

        // corectam deletionMark = m_post + setam ID (Id_Pricings)
        for (int i = 0; i < modelTable->rowCount(); ++i) {
            modelTable->setData(modelTable->index(i, PricingsTableSection::DeletionMark), m_post);
            modelTable->setData(modelTable->index(i, PricingsTableSection::Id_Pricings), m_id);
        }

        // efectuam inscrierea 'pricingsTable'
        if (! modelTable->submitAll()){                // daca nu a fost salvata tabela:
            m_db.removeObjectById("pricings", m_id);    // 1.eliminam datele salvate mai sus din sqlite
            setId(StatusObject::Unknow);               // 2.setam la valoarea initiala
            setPost(DocStatus::Unknow);                // 3.setam m_post la valoarea initiala

            CustomMessage *msg = new CustomMessage(this);
            msg->setWindowTitle(QGuiApplication::applicationDisplayName());
            msg->setTextTitle(tr("Validarea documentului nu s-a efectuat."));
            msg->setDetailedText(modelTable->lastError().text());
            msg->exec();
            msg->deleteLater();

            qCritical(logCritical())
                << modelTable->lastError().text();
            return false;
        }

    } else {

        if (!updateDataTablePricings(err)) {
            CustomMessage *msg = new CustomMessage(this);
            msg->setWindowTitle(QGuiApplication::applicationDisplayName());
            msg->setTextTitle(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msg->setDetailedText(modelTable->lastError().text());
            msg->exec();
            msg->deleteLater();

            setPost(DocStatus::Unknow);
            return false;
        }

        // corectam deletionMark = m_post
        for (int i = 0; i < modelTable->rowCount(); ++i) {
            modelTable->setData(modelTable->index(i, PricingsTableSection::DeletionMark), m_post);
        }

        if (! modelTable->submitAll()){ // daca nu a fost salvata tabela:

            CustomMessage *msg = new CustomMessage(this);
            msg->setWindowTitle(QGuiApplication::applicationDisplayName());
            msg->setTextTitle(tr("Actualizarea datelor tabelei documentului nu s-a efectuat."));
            msg->setDetailedText(modelTable->lastError().text());
            msg->exec();
            msg->deleteLater();

            setPost(DocStatus::Unknow);
            qCritical(logCritical())
                << modelTable->lastError().text();
            return false;
        }

    }

    // emitem mesaj
    if (m_post == DocStatus::Write || m_post == DocStatus::Post){
        updateTableView();
        popUp->setPopupText(tr("Documentul a fost %1 cu succes in baza de date.")
                                .arg(m_post == DocStatus::Post
                                         ? tr("validat")
                                         : tr("salvat"))
                            );
        popUp->show();
        setIsNew(false);
    }

    qInfo(logInfo()) << QStringLiteral("Documentul nr.%1 este salvat/modificat in BD.")
                            .arg(ui->editNumberDoc->text());

    emit PostDocument(); // pu actualizarea listei documentelor
    return true;
}

void PricingDialog::onWritingDataClose()
{
    setPost(DocStatus::Post); // setam proprietatea 'post'

    if (onWritingData())
        QDialog::accept();
}

void PricingDialog::onClose()
{
    emit mCloseThisForm();
    this->close();
}

// **********************************************************************************
// --- procesarea slot-lor

void PricingDialog::slotGetCallbackData(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_owner)
        return;
    prepareData(m_owner, info, data);
}

void PricingDialog::slotGetCallbackDataItems(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_investigations)
        return;
    prepareData(m_investigations, info, data);
}

void PricingDialog::prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data)
{
    switch (info.dataType) {
    case LimeReport::CallbackInfo::ColumnCount:
        data = qry->record().count();
        break;
    case LimeReport::CallbackInfo::IsEmpty:
        data = ! qry->first();
        break;
    case LimeReport::CallbackInfo::HasNext:
        data = qry->next();
        qry->previous();
        break;
    case LimeReport::CallbackInfo::ColumnHeaderData:
        if (info.index < qry->record().count())
            data = qry->record().fieldName(info.index);
        break;
    case LimeReport::CallbackInfo::ColumnData:
        data = qry->value(qry->record().indexOf(info.columnName));
        break;
    default:
        break;
    }
}

void PricingDialog::slotChangePos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    result = false;
    QSqlQuery *ds = m_owner;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();

    if (result){
        m_investigations->bindValue(0, m_id);
        m_investigations->bindValue(1, m_owner->value("group_id"));
        if (!m_investigations->exec()) {
            qCritical() << "Error: Unable to execute investigations query (slotChangePos):"
                        << m_investigations->lastError().text();
            result = false;
            return;
        }
    }
}

void PricingDialog::slotChangePosItems(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    result = false;
    QSqlQuery *ds = m_investigations;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();
}

// **********************************************************************************
// --- functii de initiere

void PricingDialog::setTitleDoc()
{
    setWindowTitle(tr("Formarea prețurilor %1").arg("[*]"));
}

void PricingDialog::setupDateDocFormat()
{
    ui->dateTimeDoc->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
    ui->dateTimeDoc->setCalendarPopup(true);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &PricingDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &PricingDialog::onDateTimeChanged, Qt::UniqueConnection);
}

void PricingDialog::initBtnToolBar()
{
    QString style_toolButton = m_db.toolButtonStyleForText();
    ui->btnOpenCatOrganization->setStyleSheet(style_toolButton);
    ui->btnOpenCatContract->setStyleSheet(style_toolButton);
    ui->btnFormsTable->setStyleSheet(style_toolButton);

    QString style_btnToolBar = m_db.toolButtonStyleForIcon();
    ui->btnAdd->setStyleSheet(style_btnToolBar);
    ui->btnEdit->setStyleSheet(style_btnToolBar);
    ui->btnDeletion->setStyleSheet(style_btnToolBar);

    ui->editSearch->setSearchOptionSelected("code");
    connect(ui->btnFormsTable, &QAbstractButton::clicked,
            this, &PricingDialog::populateTable, Qt::UniqueConnection);
    connect(ui->btnAdd, &QAbstractButton::clicked,
            this, &PricingDialog::addRowTable, Qt::UniqueConnection);
    connect(ui->btnEdit, &QAbstractButton::clicked,
            this, &PricingDialog::editRowTable, Qt::UniqueConnection);
    connect(ui->btnDeletion, &QAbstractButton::clicked,
            this, &PricingDialog::deletionRowTable, Qt::UniqueConnection);
}

void PricingDialog::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/catalogs/user.png").pixmap(18,18);
    QLabel* labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    labelAuthor = new QLabel(this);
    labelAuthor->setText(globals().nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

// **********************************************************************************
// --- functii de initiere a modelelor

void PricingDialog::updateModelOrganizations()
{
    if (modelOrganizations)
        delete modelOrganizations;

    QString str = m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql");
    modelOrganizations = new QueryRolesModel(str, ui->comboOrganization);
    modelOrganizations->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganization->setModel(modelOrganizations);
    ui->comboOrganization->setModelColumn(modelOrganizations->columnIndex("name"));
}

void PricingDialog::updateModelContracts()
{
    if (modelContracts)
        delete modelContracts;

    if (m_idOrganization <= 0) {
        QString str = m_db.getTextSQL(":/sql/queries/contracts_view.sql");
        modelContracts = new QueryRolesModel(str, ui->comboContract);
        modelContracts->setEmptyRowEnabled(true);
        ui->comboContract->setModel(modelContracts);
        ui->comboContract->setModelColumn(modelContracts->columnIndex("contract_owner"));
    } else {
        QSqlQuery qry;
        qry.prepare(m_db.getTextSQL(
            globals().thisSqlite
                ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
                : ":/sql/queries/contracts_select_by_organization_mysql.sql")
                    );
        qry.addBindValue(m_idOrganization);
        if (!qry.exec()) {
            qCritical(logCritical())
            << "SQL error:" << qry.lastError().text()
            << "Last query:" << qry.lastQuery();
            return;
        }
        modelContracts = new QueryRolesModel(nullptr, ui->comboContract);
        modelContracts->setQuery(std::move(qry));
        modelContracts->setEmptyRowEnabled(true);
        ui->comboContract->setModel(modelContracts);
        ui->comboContract->setModelColumn(modelContracts->columnIndex("name"));
    }
}

void PricingDialog::updateModelTypesPrices()
{
    if (modelTypesPrices)
        delete modelTypesPrices;

    QString str = m_db.getTextSQL(":/sql/queries/typePrices_combo_view.sql");
    modelTypesPrices = new QueryRolesModel(str, ui->comboTypesPricing);
    modelTypesPrices->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboTypesPricing->setModel(modelTypesPrices);
    ui->comboTypesPricing->setModelColumn(modelTypesPrices->columnIndex("name"));
}

// **********************************************************************************
// --- actualizarea tabelei

void PricingDialog::initTable()
{
    modelTable->setPurpose(TableDocModel::Table_destination);
    modelTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelTable->setTable(QStringLiteral("pricingsTable"));

    proxy->setSourceModel(modelTable);
    proxy->setFilterKeyColumn(OrderTableSections::Name);

    ui->tableView->setModel(proxy);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tableView->hideColumn(PricingsTableSection::Id);
    ui->tableView->hideColumn(PricingsTableSection::DeletionMark);
    ui->tableView->hideColumn(PricingsTableSection::Id_Pricings);

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSortIndicator(
        PricingsTableSection::Cod, Qt::AscendingOrder);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);

    ui->tableView->setColumnWidth(PricingsTableSection::Cod, 70);
    ui->tableView->setColumnWidth(PricingsTableSection::Name, 560);

    connect(ui->editSearch, &QLineEdit::textChanged,
            this, &PricingDialog::filterRegExpChanged, Qt::UniqueConnection);

    updateHeaderTable();
}

void PricingDialog::updateTableView()
{
    if (m_id <= 0)
        return;

    modelTable->setFilter(
        QStringLiteral("id_pricings=%1 AND price > 0").arg(m_id));
    modelTable->setSort(PricingsTableSection::Cod, Qt::AscendingOrder);
    modelTable->select();

    if (proxy->rowCount() > 0) {
        ui->tableView->selectRow(0);
    }
}

void PricingDialog::updateHeaderTable()
{
    QStringList _headers;
    _headers << tr("")    // id
             << tr("")    // deletionMark
             << tr("")    // id_pricings
             << tr("Cod")
             << tr("Denumirea investigației")
             << tr("Costul");
    for (int n = 0, m = 0; n < modelTable->columnCount(); n++, m++) {
        modelTable->setHeaderData(n, Qt::Horizontal, _headers[m]);
    }
}

// **********************************************************************************
// --- inserarea si actualizarea datelor

bool PricingDialog::insertDataTablePricings(QString &err)
{
    QVector<QVariant> data;
    data.append(m_id);
    data.append(m_post);
    data.append(ui->editNumberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(m_idTypePrice);
    data.append(m_idOrganization);
    data.append(m_idContract);
    data.append(m_idUser);
    data.append((ui->editComment->text().isEmpty())
                    ? QVariant()
                    : ui->editComment->text());

    QUuid uuid = QUuid::createUuid();
    data.append(uuid.toRfc4122());

    m_db.getDatabase().transaction();

    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   ":/sql/queries_doc/pricing_insert.sql",
                                   data,
                                   &err))
    {
        m_db.getDatabase().rollback();
        return false;
    } else {
        m_db.getDatabase().commit();
        return true;
    }
}

bool PricingDialog::updateDataTablePricings(QString &err)
{
    QVector<QVariant> data;

    data.append(m_post);
    data.append(ui->editNumberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(m_idTypePrice);
    data.append(m_idOrganization);
    data.append(m_idContract);
    data.append(m_idUser);
    data.append((ui->editComment->text().isEmpty())
                    ? QVariant()
                    : ui->editComment->text());
    data.append(m_id);

    m_db.getDatabase().transaction();

    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   ":/sql/queries_doc/pricing_update.sql",
                                   data,
                                   &err))
    {
        m_db.getDatabase().rollback();
        return false;
    } else {
        m_db.getDatabase().commit();
        return true;
    }
}

// **********************************************************************************
// --- conexiunilie

void PricingDialog::initConnections()
{
    // combo
    auto signal = QOverload<int>::of(&QComboBox::currentIndexChanged);
    auto slot   = &PricingDialog::indexChangedCombo;

    connect(ui->comboOrganization, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboContract, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboTypesPricing, signal, this, slot, Qt::UniqueConnection);


    // btn open catalogs
    connect(ui->btnOpenCatOrganization, &QToolButton::clicked,
            this, &PricingDialog::openCatOrganization, Qt::UniqueConnection);
    connect(ui->btnOpenCatContract, &QToolButton::clicked,
            this, &PricingDialog::openCatContract, Qt::UniqueConnection);

    // date
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &PricingDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &PricingDialog::onDateTimeChanged, Qt::UniqueConnection);

    // table
    connect(ui->tableView->model(), &QAbstractItemModel::dataChanged,
            this, &PricingDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),
            this, &PricingDialog::onClickedRowTable);

    // print
    connect(ui->btnPrint, &QAbstractButton::clicked, this, [this]()
            {
                onPrint(PrintType::Preview);
            });

    // btn footer
    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &PricingDialog::onWritingDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &PricingDialog::onWritingData, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &PricingDialog::onClose, Qt::UniqueConnection);
}

// **********************************************************************************
// --- evenimente formei

void PricingDialog::closeEvent(QCloseEvent *event)
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
        yesButton->setStyleSheet(styleForButtonMessageBox);
        noButton->setStyleSheet(styleForButtonMessageBox);
        cancelButton->setStyleSheet(styleForButtonMessageBox);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
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

void PricingDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        setTitleDoc();
        updateHeaderTable();
    }
}

void PricingDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        if (ui->tableView->focusWidget()){
            int row = ui->tableView->currentIndex().row();
            QModelIndex indexEdit = proxy->index(row, PricingsTableSection::Price);
            onClickedRowTable(indexEdit);
            ui->tableView->clearFocus();
        } else {
            this->focusNextChild();
        }
    }
}
