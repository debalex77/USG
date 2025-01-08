#include "docpricing.h"
#include "ui_docpricing.h"

DocPricing::DocPricing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocPricing)
{
    ui->setupUi(this);

    setTitleDoc(); // setam titlu documentului

    if (globals::showDocumentsInSeparatWindow)
        setWindowFlags(Qt::Window);

    db    = new DataBase(this); // conectarea la BD
    menu  = new QMenu(this);    // meniu contextual
    popUp = new PopUp(this);    // vizualizarea mesajelor informationale
    timer = new QTimer(this);
    catInvestigations = new CatForSqlTableModel(this);

    ui->dateTimeDoc->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
    ui->dateTimeDoc->setCalendarPopup(true);

    QString strQryOrganizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";
    QString strQryContracts     = "SELECT id,name FROM contracts WHERE deletionMark = 0 AND notValid = 0;";
    QString strQryTypesPrices   = "SELECT id,name FROM typesPrices WHERE deletionMark = 0;";

    modelOrganizations = new BaseSqlQueryModel(strQryOrganizations, ui->comboOrganization);
    modelContracts     = new BaseSqlQueryModel(strQryContracts, ui->comboContract);
    modelTypesPrices   = new BaseSqlQueryModel(strQryTypesPrices, ui->comboTypesPricing);
    modelTable = new BaseSqlTableModel(this);
    proxy      = new BaseSortFilterProxyModel(this);

    modelOrganizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelContracts->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelTypesPrices->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);

    ui->comboOrganization->setModel(modelOrganizations);
    ui->comboContract->setModel(modelContracts);
    ui->comboTypesPricing->setModel(modelTypesPrices);

    initBtnToolBar(); // initierea butoanelor tool barului
    initFooterDoc();  // setam imaginea si numele utilizatorului

    updateTableView();

    connect(this, &DocPricing::ItNewChanged, this, &DocPricing::slot_ItNewChanged);
    connect(this, &DocPricing::IdChanged, this, &DocPricing::slot_IdChanged);                          // conectarea la modificarea
    connect(this, &DocPricing::IdOrganizationChanged, this, &DocPricing::slot_IdOrganizationChanged);  // proprietatilor clasei
    connect(this, &DocPricing::IdContractChanged, this, &DocPricing::slot_IdContractChanged);          // si actiunea dupa modificare:
    connect(this, &DocPricing::IdTypePriceChanged, this, &DocPricing::slot_IdTypePriceChanged);        // completarea/modificarea formei
    connect(this, &DocPricing::IdUserChanged, this, &DocPricing::slot_IdUserChanged);

    connectionsToIndexChangedCombobox(); // conectarea la modificarea indexului combobox-urilor

    connect(ui->btnOpenCatOrganization, &QToolButton::clicked, this, &DocPricing::openCatOrganization);
    connect(ui->btnOpenCatContract, &QToolButton::clicked, this, &DocPricing::openCatContract);

    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::onDateTimeChanged);

    connect(ui->tableView->model(), &QAbstractItemModel::dataChanged, this, &DocPricing::dataWasModified); // modificarea datelor tabelului
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),   // la apasare btn Key_Return sau Key_Enter
            this, &DocPricing::onClickedRowTable);                                    // intram in regimul de redactare a sectiei 'Cost'

    connect(ui->btnPrint, &QAbstractButton::clicked, this, [this]()
            {
        onPrint(Enums::TYPE_PRINT::OPEN_PREVIEW);
            });

    connect(ui->btnOK, &QAbstractButton::clicked, this, &DocPricing::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &DocPricing::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &DocPricing::onClose);

}

DocPricing::~DocPricing()
{
    delete db;
    delete popUp;
    delete modelOrganizations;
    delete modelContracts;
    delete modelTypesPrices;
    delete modelTable;
    delete proxy;
    delete catInvestigations;
    delete timer;
    delete ui;
}

void DocPricing::onPrintDocument(Enums::TYPE_PRINT type_print)
{
    onPrint(type_print);
}

void DocPricing::dataWasModified()
{
    setWindowModified(true);
}

void DocPricing::slot_ItNewChanged()
{
    if (m_itNew){

        setWindowTitle(tr("Formarea prețurilor (crearea) %1").arg("[*]"));

        if (m_idUser == -1)
            setIdUser(globals::idUserApp);

        connect(timer, &QTimer::timeout, this, &DocPricing::updateTimer);
        timer->start(1000);

        int lastNumberDoc = db->getLastNumberDoc("pricings");           // determinam ultimul numar a documentului
        ui->editNumberDoc->setText(QString::number(lastNumberDoc + 1)); // setam numarul documentului
    }
}

void DocPricing::updateTimer()
{
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::dataWasModified);
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::onDateTimeChanged);
    disconnect(timer, &QTimer::timeout, this, &DocPricing::updateTimer);
    QDateTime time = QDateTime::currentDateTime();
    ui->dateTimeDoc->setDateTime(time);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::onDateTimeChanged);
    connect(timer, &QTimer::timeout, this, &DocPricing::updateTimer);
}

void DocPricing::onDateTimeChanged()
{
    disconnect(timer, &QTimer::timeout, this, &DocPricing::updateTimer);
}

void DocPricing::slot_IdChanged()
{
    if (m_id == -1)
        return;

    disconnectionsToIndexChangedCombobox(); // deconectarea de la modificarea indexului combobox-urilor
                                            // ca sa nu fie activata modificarea formei - dataWasModified()

    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::dataWasModified);
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::onDateTimeChanged);

    QMap<QString, QString> items;
    if (db->getObjectDataById("pricings", m_id, items)){
        ui->editNumberDoc->setText(items.constFind("numberDoc").value());
        ui->editNumberDoc->setDisabled(!ui->editNumberDoc->text().isEmpty());
        if (globals::thisMySQL){
            QString str_date = items.constFind("dateDoc").value();
            static const QRegularExpression replaceT("T");
            static const QRegularExpression removeMilliseconds("\\.000");
            str_date = str_date.replace(replaceT, " ").replace(removeMilliseconds,"");
            ui->dateTimeDoc->setDateTime(QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss"));
        } else
            ui->dateTimeDoc->setDateTime(QDateTime::fromString(items.constFind("dateDoc").value(), "yyyy-MM-dd hh:mm:ss"));
        int id_organization = items.constFind("id_organizations").value().toInt();
        int id_contract     = items.constFind("id_contracts").value().toInt();
        int id_typesPrices  = items.constFind("id_typesPrices").value().toInt();
        int id_user         = items.constFind("id_users").value().toInt();
        m_post              = items.constFind("deletionMark").value().toInt();
        if (id_organization != 0)
            setIdOrganization(id_organization); // setam proprietatea IdOrganization
        if (id_contract != 0)
            setIdContract(id_contract);         // setam proprietatea - IdContract
        if (id_typesPrices != 0)
            setIdTypePrice(id_typesPrices);     // setam proprietatea - IdTypePrice
        updateTableView();
        if (id_user != 0)
            setIdUser(id_user);
        ui->editComment->setText(items.constFind("comment").value());
        setWindowTitle(tr("Formarea prețurilor (validat) %1 %2").arg("nr." + ui->editNumberDoc->text() + " din " + ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
    }
    connectionsToIndexChangedCombobox(); // activarea conectarii la modificarea indexului combobox-urilor
}

void DocPricing::slot_IdOrganizationChanged()
{
    if (m_idOrganization == -1)
        return;
    auto startOrganization = modelOrganizations->index(0, 0);
    auto indexOrganization = modelOrganizations->match(startOrganization, Qt::UserRole, m_idOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty())
        ui->comboOrganization->setCurrentIndex(indexOrganization.first().row());
}

void DocPricing::slot_IdContractChanged()
{
    if (m_idContract == -1)
        return;
    auto indexContract = modelContracts->match(modelContracts->index(0,0), Qt::UserRole, m_idContract, 1, Qt::MatchExactly);
    if (!indexContract.isEmpty())
        ui->comboContract->setCurrentIndex(indexContract.first().row());
}

void DocPricing::slot_IdTypePriceChanged()
{
    if (m_idTypePrice == -1)
        return;
    auto indexTypePrice = modelTypesPrices->match(modelTypesPrices->index(0,0), Qt::UserRole, m_idTypePrice, 1, Qt::MatchExactly);
    if (!indexTypePrice.isEmpty())
        ui->comboTypesPricing->setCurrentIndex(indexTypePrice.first().row());
}

void DocPricing::slot_IdUserChanged()
{
    if (m_idUser == -1)
        return;
}

void DocPricing::indexChangedComboOrganization(const int arg1)
{
    Q_UNUSED(arg1)
    int id_organization = ui->comboOrganization->itemData(ui->comboOrganization->currentIndex(), Qt::UserRole).toInt();
    setIdOrganization(id_organization);
    dataWasModified();

    QMap<QString, QString> items;
    QString strQry = "SELECT organizations.name,"
                     "organizations.id_contracts AS id_contract,"
                     "contracts.name AS nameContract,"
                     "contracts.id_typesPrices AS id_typePrice,"
                     "typesPrices.name AS nameTypePrice "
                     "FROM organizations "
                     "INNER JOIN contracts ON organizations.id_contracts = contracts.id "
                     "INNER JOIN typesPrices ON contracts.id_typesPrices = typesPrices.id "
                     "WHERE organizations.id = " + QString::number(id_organization) + ";";
    if (db->getDataFromQueryByRecord(strQry, items)){
        if (items.count() == 0)
            return;
        int id_contract  = items.constFind("id_contract").value().toInt();
        int id_typePrice = items.constFind("id_typePrice").value().toInt();

        modelContracts->clear();                                        // actualizam solicitarea
        QString strQryContracts = "SELECT id,name FROM contracts "      // filtru dupa organizatia
                                  "WHERE deletionMark = 0 AND id_organizations = "
                                    + QString::number(id_organization)+ " AND notValid = 0;";
        modelContracts->setQuery(strQryContracts);
        if (id_contract == 0)
            ui->comboContract->setCurrentIndex(0); // setam indexul 0 - selecteaza
        else
            setIdContract(id_contract);

        if (id_typePrice == 0)
            ui->comboTypesPricing->setCurrentIndex(0); // setam indexul 0 - selecteaza
        else
            setIdTypePrice(id_typePrice);
    }
}

void DocPricing::indexChangedComboContract(const int arg1)
{
    Q_UNUSED(arg1)
    if (m_idOrganization == -1)
        return;

    int id_contract = ui->comboContract->itemData(ui->comboContract->currentIndex(), Qt::UserRole).toInt();
    setIdContract(id_contract);
    dataWasModified();

    QMap<QString, QString> items;
    if (db->getObjectDataById("contracts", id_contract, items)){
        int id_typePrice = items.constFind("id_typesPrices").value().toInt();
        if (id_typePrice == 0)
            ui->comboTypesPricing->setCurrentIndex(0);
        else
            setIdTypePrice(id_typePrice);
    }
}

void DocPricing::indexChangedComboTypePrice(const int arg1)
{
    Q_UNUSED(arg1)
    int id_typePrice = ui->comboTypesPricing->itemData(ui->comboTypesPricing->currentIndex(), Qt::UserRole).toInt();
    setIdTypePrice(id_typePrice);
    dataWasModified();
}

void DocPricing::openCatOrganization()
{
    if (m_idOrganization == 0 || m_idOrganization == Enums::IDX_UNKNOW)
        return;

    qInfo(logInfo()) << tr("Editarea/vizualizarea datelor organizatiei '%1' cu id='%2'.")
                            .arg(ui->comboOrganization->currentText(), QString::number(m_idOrganization));

    CatOrganizations *cat_organization = new CatOrganizations(this);
    cat_organization->setAttribute(Qt::WA_DeleteOnClose);
    cat_organization->setProperty("itNew", false);
    cat_organization->setProperty("Id", m_idOrganization);
    cat_organization->show();
}

void DocPricing::openCatContract()
{
    if (m_idContract == 0 || m_idContract == Enums::IDX_UNKNOW)
        return;

    qInfo(logInfo()) << tr("Editarea/vizualizarea datelor contractului '%1' cu id='%2'.")
                            .arg(ui->comboContract->currentText(), QString::number(m_idContract));

    CatContracts *cat_contract = new CatContracts(this);
    cat_contract->setAttribute(Qt::WA_DeleteOnClose);
    cat_contract->setProperty("itNew", false);
    cat_contract->setProperty("Id", m_idContract);
    cat_contract->show();

}

void DocPricing::addRowTable()
{
    delete catInvestigations; // sa nu dublam toolBar si btn in clasa CatForSqlTableModel
                              // initial am alocat memoria
    catInvestigations = new CatForSqlTableModel(this);
    catInvestigations->setProperty("typeCatalog", catInvestigations->Investigations);
    catInvestigations->setProperty("typeForm", catInvestigations->SelectForm);
    catInvestigations->setGeometry(1000, 400, 800, 400);
    catInvestigations->show();

    connect(catInvestigations, &CatForSqlTableModel::mSelectData, this, &DocPricing::getDataSelectable);
}

void DocPricing::editRowTable()
{
    int row = ui->tableView->currentIndex().row();
    ui->tableView->edit(proxy->index(row, Enums::PRICING_COLUMN_PRICE));
}

void DocPricing::deletionRowTable()
{
    int row = ui->tableView->currentIndex().row();
    proxy->removeRow(row);
}

void DocPricing::completeTableDocPricing()
{
    if (modelTable->rowCount() > 0){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Tabela nu este goal\304\203. Dori\310\233i s\304\203 goli\310\233i tabela ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            updateTableView();
        } else if (messange_box.clickedButton() == noButton) {

        }
    }

    int lastIdTable = db->getLastIdForTable("pricingsTable");
    int lastIdDoc = db->getLastIdForTable("pricings");

    /* solicitarea pu completarea tabelei din clasa 'CatForSqlTableModel'
     * si tabela 'investigations' din BD sqlite */
    QMap<QString, QString> items;
    modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
    QString strQry = "SELECT cod,name FROM investigations WHERE deletionMark = 0 AND use = '1';";
    if (db->getDataFromQuery(strQry, items)){
        QMapIterator<QString, QString> it(items);
        while (it.hasNext()) {
            it.next();
            int _row = modelTable->rowCount();
            modelTable->insertRow(_row);
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_ID), lastIdTable + _row + 1);
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_DEL_MARK), Enums::IDX_WRITE);
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_ID_PRICINGS), lastIdDoc);
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_COD),  it.key());
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_NAME), it.value());
            modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_PRICE), 0);
        }
    }
}

void DocPricing::filterRegExpChanged()
{
    QString typeSearch = ui->editSearch->getSearchOptionSelected();
    if (typeSearch == "code")
        proxy->setFilterKeyColumn(Enums::PRICING_COLUMN_COD);
    else if (typeSearch == "name")
        proxy->setFilterKeyColumn(Enums::PRICING_COLUMN_NAME);
    else
        return;

    QRegularExpression regExp(ui->editSearch->text(), QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(regExp);
}

void DocPricing::getDataSelectable()
{
//    int _id       = catInvestigations->getSelectId();
    QString _cod  = catInvestigations->getSelectCod();
    QString _name = catInvestigations->getSelectName();

    for (int n = 0; n < proxy->rowCount(); n++) {
        if (ui->tableView->model()->data(proxy->index(n, Enums::PRICING_COLUMN_COD), Qt::DisplayRole).toString() == _cod){
            QMessageBox::warning(this, tr("Atentie"),
                                 tr("Investigatia <b>'%1 - %2'</b> exista in tabel.").arg(_cod, _name), QMessageBox::Ok);
            ui->tableView->setCurrentIndex(proxy->index(n, Enums::PRICING_COLUMN_NAME));
            return;
        }
    }
    int lastIdTable = db->getLastIdForTable("pricingsTable");

    int _row = modelTable->rowCount();
    modelTable->insertRow(_row);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_ID), lastIdTable + _row + 1);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_DEL_MARK), Enums::IDX_WRITE);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_ID_PRICINGS), m_id);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_COD), _cod);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_NAME), _name);
    modelTable->setData(modelTable->index(_row, Enums::PRICING_COLUMN_PRICE), 0);
    ui->tableView->setCurrentIndex(modelTable->index(_row, Enums::PRICING_COLUMN_PRICE));
}

void DocPricing::onClickedRowTable(const QModelIndex &index)
{
    int _row = index.row();
    QModelIndex indexEdit = proxy->index(_row, Enums::PRICING_COLUMN_PRICE);
    ui->tableView->setCurrentIndex(indexEdit);
    ui->tableView->edit(indexEdit);
}

void DocPricing::getDataObject()
{
    itemsData.clear();
    if (m_id != Enums::IDX_UNKNOW)
    itemsData.insert("id",               QString::number(m_id));
    itemsData.insert("deletionMark",     (m_post) ? QString::number(2) : QString::number(0));
    itemsData.insert("numberDoc",        ui->editNumberDoc->text());
    itemsData.insert("dateDoc",          ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    itemsData.insert("id_typesPrices",   QString::number(m_idTypePrice));
    itemsData.insert("id_organizations", QString::number(m_idOrganization));
    itemsData.insert("id_contracts",     QString::number(m_idContract));
    itemsData.insert("id_users",         QString::number(m_idUser));
    itemsData.insert("comment",          ui->editComment->text());
}

void DocPricing::onPrint(Enums::TYPE_PRINT type_print)
{
    // *************************************************************************************
    // alocam memoria
    m_report = new LimeReport::ReportEngine(this);

    m_report->setPreviewWindowTitle(tr("Lista pre\310\233urilor nr.") + ui->editNumberDoc->text() +
                                    tr(" din ") + ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss"));

    // solicitarea pentru grupe
    m_owner = new QSqlQuery("SELECT "
                            " investigations.owner, "
                            " investigationsGroup.cod "
                            "FROM "
                            " investigations "
                            "INNER JOIN "
                            " investigationsGroup ON investigations.owner = investigationsGroup.name "
                            "WHERE "
                            " investigations.owner IS NOT NULL "
                            "GROUP BY "
                            " investigationsGroup.cod "
                            "ORDER BY "
                            " investigationsGroup.cod ASC;");
    if (! m_owner->first()) {
        qCritical() << "Error: Unable to fetch investigationsGroup data:" << m_owner->lastError().text();
        delete m_report;
        return;
    }

    // solicitarea elementelor investigatiilor
    QMessageBox messange_box(QMessageBox::Question,
                             tr("Prezentarea listei pre\310\233urilor"),
                             tr("De prezentat investiga\310\233iile f\304\203r\304\203 pre\310\233 ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    QString str_investigations;
    if (messange_box.clickedButton() == yesButton) {

        str_investigations =
            QString(
                "SELECT "
                " investigations.cod, "
                " investigations.name,"
                " %1 AS price "
                "FROM "
                " investigations "
                "INNER JOIN "
                " pricingsTable ON pricingsTable.cod = investigations.cod "
                "WHERE "
                " pricingsTable.id_pricings = ? AND "
                " investigations.owner = ?;")
                .arg((globals::thisMySQL) ? "FORMAT(pricingsTable.price, 2)" : "printf('%.2f', pricingsTable.price)");
                // .arg((globals::thisMySQL) ?
                //          "CASE WHEN FORMAT(pricingsTable.price, 2) = '0.00' THEN '' ELSE FORMAT(pricingsTable.price, 2) END" :
                //          "CASE WHEN printf('%.2f', pricingsTable.price) = '0.00' THEN '' ELSE printf('%.2f', pricingsTable.price) END");

    } else if (messange_box.clickedButton() == noButton) {

        str_investigations =
            QString(
                "SELECT "
                " investigations.cod, "
                " investigations.name,"
                " %1 AS price "
                "FROM "
                " investigations "
                "INNER JOIN "
                " pricingsTable ON pricingsTable.cod = investigations.cod "
                "WHERE "
                " pricingsTable.id_pricings = ? AND "
                " pricingsTable.price > 0 AND "
                " investigations.owner = ?;").arg((globals::thisMySQL) ? "FORMAT(pricingsTable.price, 2)" : "printf('%.2f', pricingsTable.price)");

    }

    m_investigations = new QSqlQuery(str_investigations);
    m_investigations->bindValue(0, m_id);
    m_investigations->bindValue(1, "Org.interne"); // initial
    if ( !m_investigations->exec()) {
        qCritical() << "Error: Unable to execute investigations query:" << m_investigations->lastError().text();
        delete m_report;
        return;
    }

    // conectarile
    LimeReport::ICallbackDatasource *callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_owner");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&DocPricing::slotGetCallbackData));
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&DocPricing::slotChangePos));

    callbackDatasource = m_report->dataManager()->createCallbackDatasource("list_items");
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo &, QVariant &>::of(&LimeReport::ICallbackDatasource::getCallbackData),
            this, QOverload<LimeReport::CallbackInfo, QVariant &>::of(&DocPricing::slotGetCallbackDataItems));
    connect(callbackDatasource, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&LimeReport::ICallbackDatasource::changePos),
            this, QOverload<const LimeReport::CallbackInfo::ChangePosType &, bool &>::of(&DocPricing::slotChangePosItems));

    // variabile
    m_report->dataManager()->setReportVariable("id_pricing", m_id);
    m_report->dataManager()->setReportVariable("organization", ui->comboOrganization->currentText());
    m_report->dataManager()->setReportVariable("date", ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy"));

    // incarcarea fisierului
    if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Pricing.lrxml")){
        QDir dir;
        CustomMessage *msgBox = new CustomMessage(this);
        msgBox->setWindowTitle(tr("Printarea documentului"));
        msgBox->setTextTitle(tr("Documentul nu poate fi printat."));
        msgBox->setDetailedText(tr("Nu a fost gasit fi\310\231ierul formei de tipar:\n%1").arg(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Pricing.lrxml")));
        msgBox->exec();
        msgBox->deleteLater();

        delete m_report;

        return;
    }

#ifdef QT_DEBUG
    Q_UNUSED(type_print)
    m_report->designReport();
#else
    if (type_print == Enums::TYPE_PRINT::OPEN_DESIGNER){
        m_report->designReport();
    } else {
        m_report->previewReport();
    }
#endif

    // eliberarea memoriei
    delete m_owner;
    delete m_investigations;

    m_report->deleteLater();
}

bool DocPricing::controlRequiredObjects()
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

bool DocPricing::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

//    QString strQry = "SELECT id,numberDoc FROM pricings ORDER BY id DESC LIMIT 1;";
//    QMap<QString, QString> _items;
//    if (db->getDataFromQueryByRecord(strQry, _items)){
//        int numDoc;
//        if (_items.count() == 0){
//            numDoc = 0;
//            m_id = 1;
//        } else {
//            numDoc = _items.constFind("numberDoc").value().toInt();
//            m_id = _items.constFind("id").value().toInt() + 1;
//        }
//        if (numDoc >= 0)
//            ui->editNumberDoc->setText(QString::number(numDoc + 1));
//    }
//    // determinam numarul documentului
//    setPost(true);    // setam proprietatea 'post' pu getDataObject():
//    getDataObject();  // itemsData.insert("deletionMark", (m_post) ? QString::number(2) : QString::number(0));
//    if (m_itNew){
//        if (!db->postDocument("pricings", itemsData)){
//            QMessageBox::warning(this, tr("Validarea documentului"),
//                                 tr("Documentul nu este validat !!! <br> Adresați-vă administratorului aplicației."),
//                                 QMessageBox::Ok, QMessageBox::Ok);
//            setPost(false); // setam proprietatea 'post' pu getDataObject() - vezi mai sus
//            return false;
//        }
//    } else {
//        if (!db->updateDataObject("pricings", m_id, itemsData)){
//            QMessageBox::warning(this, tr("Actualizarea datelor documentului"),
//                                 tr("Actualizarea datelor documentului nu este reușită !!! <br> Adresați-vă administratorului aplicației."),
//                                 QMessageBox::Ok, QMessageBox::Ok);
//            return false;
//        }
//    }

//    for (int i = 0; i < modelTable->rowCount(); ++i) {
//        modelTable->setData(modelTable->index(i, column_DeletionMark), m_post);
//        modelTable->setData(modelTable->index(i, column_IdPricings), m_id);
//    }

//    if (!modelTable->submitAll()){
//        qDebug() << modelTable->lastError().text();
//        return false;
//    }

    if (m_post == Enums::IDX_UNKNOW)  // daca a fost apasat btnOk = propritatea trebuia sa fie m_post == idx_post
        setPost(Enums::IDX_WRITE);    // setam post = idx_write

    if (m_itNew){
        m_id = db->getLastIdForTable("pricings") + 1; // incercare de a seta 'id' documentului

        QString strQry = insertDataTablePricings();
        if (! db->execQuery(strQry)){
            QMessageBox::warning(this, tr("Validarea documentului"), tr("Documentul nu este %1 !!! <br> Adresați-vă administratorului aplicației.")
                                 .arg((m_post == Enums::IDX_POST) ? tr("validat") : tr("salvat")), QMessageBox::Ok);
            m_id = Enums::IDX_UNKNOW;   // setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala
            return false;
        }

        for (int i = 0; i < modelTable->rowCount(); ++i) {                           // corectam valoarea indexului 'deletionMark'
            modelTable->setData(modelTable->index(i, Enums::PRICING_COLUMN_DEL_MARK), m_post);  // si 'id' documentului din tabela
            modelTable->setData(modelTable->index(i, Enums::PRICING_COLUMN_ID_PRICINGS), m_id);
        }

        if (! modelTable->submitAll()){                // daca nu a fost salvata tabela:
            db->removeObjectById("pricings", m_id);    // 1.eliminam datele salvate mai sus din sqlite
            m_id = Enums::IDX_UNKNOW;                         // 2.setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW);                       // 3.setam m_post la valoarea initiala
            qCritical(logCritical()) << modelTable->lastError().text();
            return false;
        }

        if (m_post == Enums::IDX_WRITE){
            updateTableView();
            popUp->setPopupText(tr("Documentul a fost salvat cu succes in baza de date."));
            popUp->show();
            setItNew(false);
        }
    } else {
        QString strQry = updateDataTablePricings();
        if (! db->execQuery(strQry)){
            QMessageBox::warning(this, tr("Validarea documentului"),
                                 tr("Datele documentului nu au fost actualizate !!! <br> Adresați-vă administratorului aplicației."),
                                 QMessageBox::Ok);
            m_id = Enums::IDX_UNKNOW;   // setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala
            return false;
        }

        if (! modelTable->submitAll()){                // daca nu a fost salvata tabela:
            qCritical(logCritical()) << modelTable->lastError().text();
            return false;
        }
        if (m_post == Enums::IDX_WRITE){
            updateTableView();
            popUp->setPopupText(tr("Datele documentului au fost<br>actualizate cu succes."));
            popUp->show();
            setItNew(false);
        }
    }
    emit PostDocument(); // pu actualizarea listei documentelor
    return true;
}

void DocPricing::onWritingDataClose()
{
    setPost(Enums::IDX_POST); // setam proprietatea 'post'

    if (onWritingData())
        QDialog::accept();
}

void DocPricing::onClose()
{
    emit mCloseThisForm();
    this->close();
}

void DocPricing::slotGetCallbackData(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_owner)
        return;
    prepareData(m_owner, info, data);
}

void DocPricing::slotGetCallbackDataItems(LimeReport::CallbackInfo info, QVariant &data)
{
    if (! m_investigations)
        return;
    prepareData(m_investigations, info, data);
}

void DocPricing::prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data)
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

void DocPricing::slotChangePos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = m_owner;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();

    if (result){
        m_investigations->bindValue(0, m_id);
        m_investigations->bindValue(1, m_owner->value(m_owner->record().indexOf("owner")));
        if (!m_investigations->exec()) {
            qCritical() << "Error: Unable to execute investigations query (slotChangePos):" << m_investigations->lastError().text();
            delete m_report;
            return;
        }
    }
}

void DocPricing::slotChangePosItems(const LimeReport::CallbackInfo::ChangePosType &type, bool &result)
{
    QSqlQuery *ds = m_investigations;
    if (!ds)
        return;
    if (type == LimeReport::CallbackInfo::First)
        result = ds->first();
    else
        result = ds->next();
}

void DocPricing::setTitleDoc()
{
    setWindowTitle(tr("Formarea prețurilor %1").arg("[*]"));
}

void DocPricing::initBtnToolBar()
{
    QString style_toolButton = db->getStyleForToolButton();
    ui->btnOpenCatOrganization->setStyleSheet(style_toolButton);
    ui->btnOpenCatContract->setStyleSheet(style_toolButton);
    ui->btnFormsTable->setStyleSheet(style_toolButton);

    QString style_btnToolBar = db->getStyleForButtonToolBar();
    ui->btnAdd->setStyleSheet(style_btnToolBar);
    ui->btnEdit->setStyleSheet(style_btnToolBar);
    ui->btnDeletion->setStyleSheet(style_btnToolBar);

    ui->editSearch->setSearchOptionSelected("code");
    connect(ui->btnFormsTable, &QAbstractButton::clicked, this, &DocPricing::completeTableDocPricing);
    connect(ui->btnAdd, &QAbstractButton::clicked, this, &DocPricing::addRowTable);
    connect(ui->btnEdit, &QAbstractButton::clicked, this, &DocPricing::editRowTable);
    connect(ui->btnDeletion, &QAbstractButton::clicked, this, &DocPricing::deletionRowTable);

    connect(ui->editSearch, &QLineEdit::textChanged, this, &DocPricing::filterRegExpChanged);
}

void DocPricing::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/user_x32.png").pixmap(18,18);
    QLabel* labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    labelAuthor = new QLabel(this);
    labelAuthor->setText(globals::nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void DocPricing::updateTableView()
{
    if (m_itNew){
        modelTable->clear();
        modelTable->setTable("pricingsTable");
        modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
        modelTable->setSort(Enums::PRICING_COLUMN_COD, Qt::AscendingOrder);
        modelTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
        proxy->setSourceModel(modelTable);
        ui->tableView->setModel(proxy);
        modelTable->select();
        ui->tableView->selectRow(0);
    } else {
        modelTable->clear();
        modelTable->setTable("pricingsTable");
        modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
        modelTable->setSort(Enums::PRICING_COLUMN_COD, Qt::AscendingOrder);
        modelTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
        proxy->setSourceModel(modelTable);
        ui->tableView->setModel(proxy);
        modelTable->select();
        ui->tableView->selectRow(0);
    }
    ui->tableView->hideColumn(Enums::PRICING_COLUMN_ID);           // id
    ui->tableView->hideColumn(Enums::PRICING_COLUMN_DEL_MARK); // deletionMark
    ui->tableView->hideColumn(Enums::PRICING_COLUMN_ID_PRICINGS);   // id_pricings
    ui->tableView->horizontalHeader()->setStretchLastSection(true);         // extinderea ultimei sectiei
    ui->tableView->setSortingEnabled(true);                                 // setam posibilitatea sortarii
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);     // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // setam multipla selectie
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);     // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);                            // initializam meniu contextual
    ui->tableView->horizontalHeader()->setSortIndicator(3, Qt::SortOrder::AscendingOrder); // sortarea dupa 3 sectie(Cod)
    // ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->setColumnWidth(Enums::PRICING_COLUMN_COD, 70);   // codul
    ui->tableView->setColumnWidth(Enums::PRICING_COLUMN_NAME, 500); // denuimirea investigatiei
    updateHeaderTable();
}

void DocPricing::updateHeaderTable()
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

QString DocPricing::insertDataTablePricings()
{
    return QString("INSERT INTO pricings ("
                             "id,"
                             "deletionMark,"
                             "numberDoc,"
                             "dateDoc,"
                             "id_typesPrices,"
                             "id_organizations,"
                             "id_contracts,"
                             "id_users,"
                             "comment"
                         ") VALUES ('%1','%2','%3','%4','%5','%6','%7','%8','%9');")
            .arg(QString::number(m_id),
                 QString::number(m_post),
                 ui->editNumberDoc->text(),
                 ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 QString::number(m_idTypePrice),
                 QString::number(m_idOrganization),
                 QString::number(m_idContract),
                 QString::number(m_idUser),
                 ui->editComment->text());
}

QString DocPricing::updateDataTablePricings()
{
    return QString("UPDATE pricings SET "
                    "deletionMark = '%2', "
                    "numberDoc = '%3',"
                    "dateDoc = '%4',"
                    "id_typesPrices = '%5',"
                    "id_organizations = '%6',"
                    "id_contracts = '%7',"
                    "id_users = '%8',"
                    "comment = '%9' WHERE id = '%1';")
            .arg(QString::number(m_id),
                 QString::number(m_post),
                 ui->editNumberDoc->text(),
                 ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 QString::number(m_idTypePrice),
                 QString::number(m_idOrganization),
                 QString::number(m_idContract),
                 QString::number(m_idUser),
                 ui->editComment->text());
}

void DocPricing::connectionsToIndexChangedCombobox()
{
    connect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboOrganization));

    connect(ui->comboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboContract));

    connect(ui->comboTypesPricing, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboTypePrice));
}

void DocPricing::disconnectionsToIndexChangedCombobox()
{
    disconnect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboOrganization));

    disconnect(ui->comboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboContract));

    disconnect(ui->comboTypesPricing, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocPricing::indexChangedComboTypePrice));
}

void DocPricing::closeEvent(QCloseEvent *event)
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
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
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

void DocPricing::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        setTitleDoc();
        updateHeaderTable();
    }
}

void DocPricing::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        if (ui->tableView->focusWidget()){
            int _row = ui->tableView->currentIndex().row();
            QModelIndex indexEdit = proxy->index(_row, Enums::PRICING_COLUMN_PRICE);
            onClickedRowTable(indexEdit);
            ui->tableView->clearFocus();
        } else {
            this->focusNextChild();
        }
    }
}
