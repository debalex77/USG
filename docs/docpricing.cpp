#include "docpricing.h"
#include "ui_docpricing.h"

DocPricing::DocPricing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocPricing)
{
    ui->setupUi(this);

    setTitleDoc(); // setam titlu documentului

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

    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocPricing::onDateTimeChanged);

    connect(ui->tableView->model(), &QAbstractItemModel::dataChanged, this, &DocPricing::dataWasModified); // modificarea datelor tabelului
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),   // la apasare btn Key_Return sau Key_Enter
            this, &DocPricing::onClickedRowTable);                                    // intram in regimul de redactare a sectiei 'Cost'

    connect(ui->btnPrint, &QAbstractButton::clicked, this, &DocPricing::onPrint);
    connect(ui->btnOK, &QAbstractButton::clicked, this, &DocPricing::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &DocPricing::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &DocPricing::onClose);

#if defined(Q_OS_WIN)
    ui->frame_table->setStyle(style_fusion);
    ui->frame_footer->setStyle(style_fusion);
#endif
}

DocPricing::~DocPricing()
{
    delete btnAdd;
    delete btnEdit;
    delete btnDeletion;
    delete btnFormsTable;
    delete toolBar;
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            str_date = str_date.replace(QRegExp("T"), " ").replace(".000","");
#else
            str_date = str_date.replace(QRegularExpression("T"), " ").replace(".000","");
#endif
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
    ui->tableView->edit(proxy->index(row, column_Price));
}

void DocPricing::deletionRowTable()
{
    int row = ui->tableView->currentIndex().row();
    proxy->removeRow(row);
}

void DocPricing::completeTableDocPricing()
{
    if (modelTable->rowCount() > 0){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::question(this,
                                      tr("Verificarea datelor."),
                                      tr("Tabela nu este goală. Doriți să goliți tabela ?"),
                                      QMessageBox::Yes | QMessageBox::No);
        if (YesNo == QMessageBox::Yes)
            modelTable->clear();
    }

    int lastIdTable = db->getLastIdForTable("pricingsTable");
    int lastIdDoc = db->getLastIdForTable("pricings");

    /* solicitarea pu completarea tabelei din clasa 'CatForSqlTableModel'
     * si tabela 'investigations' din BD sqlite */
    QMap<QString, QString> items;
    modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
    QString strQry = "SELECT cod,name FROM investigations WHERE deletionMark = 0;";
    if (db->getDataFromQuery(strQry, items)){
        QMapIterator<QString, QString> it(items);
        while (it.hasNext()) {
            it.next();
            int _row = modelTable->rowCount();
            modelTable->insertRow(_row);
            modelTable->setData(modelTable->index(_row, column_Id), lastIdTable + _row + 1);
            modelTable->setData(modelTable->index(_row, column_DeletionMark), idx_write);
            modelTable->setData(modelTable->index(_row, column_IdPricings), lastIdDoc);
            modelTable->setData(modelTable->index(_row, column_Cod),  it.key());
            modelTable->setData(modelTable->index(_row, column_Name), it.value());
            modelTable->setData(modelTable->index(_row, column_Price), 0);
        }
    }
}

void DocPricing::filterRegExpChanged()
{
    if (comboSearch->currentIndex() == 0)
        proxy->setFilterKeyColumn(column_Cod);
    else
        proxy->setFilterKeyColumn(column_Name);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QRegExp regExp(editSearch->text(), Qt::CaseInsensitive, QRegExp::RegExp);
    proxy->setFilterRegExp(regExp);
#else
    QRegularExpression regExp(editSearch->text(), QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(regExp);
#endif
}

void DocPricing::getDataSelectable()
{
//    int _id       = catInvestigations->getSelectId();
    QString _cod  = catInvestigations->getSelectCod();
    QString _name = catInvestigations->getSelectName();

    for (int n = 0; n < proxy->rowCount(); n++) {
        if (ui->tableView->model()->data(proxy->index(n, column_Cod), Qt::DisplayRole).toString() == _cod){
            QMessageBox::warning(this, tr("Atentie"),
                                 tr("Investigatia <b>'%1 - %2'</b> exista in tabel.").arg(_cod, _name), QMessageBox::Ok);
            ui->tableView->setCurrentIndex(proxy->index(n, column_Name));
            return;
        }
    }
    int lastIdTable = db->getLastIdForTable("pricingsTable");

    int _row = modelTable->rowCount();
    modelTable->insertRow(_row);
    modelTable->setData(modelTable->index(_row, column_Id), lastIdTable + _row + 1);
    modelTable->setData(modelTable->index(_row, column_DeletionMark), idx_write);
    modelTable->setData(modelTable->index(_row, column_IdPricings), m_id);
    modelTable->setData(modelTable->index(_row, column_Cod), _cod);
    modelTable->setData(modelTable->index(_row, column_Name), _name);
    modelTable->setData(modelTable->index(_row, column_Price), 0);
    ui->tableView->setCurrentIndex(modelTable->index(_row, column_Price));
}

void DocPricing::onClickedRowTable(const QModelIndex &index)
{
    int _row = index.row();
    QModelIndex indexEdit = proxy->index(_row, column_Price);
    ui->tableView->setCurrentIndex(indexEdit);
    ui->tableView->edit(indexEdit);
}

void DocPricing::getDataObject()
{
    itemsData.clear();
    if (m_id != idx_unknow)
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

void DocPricing::onPrint()
{
    if (m_itNew)
        return;
    if (ui->comboOrganization->currentText().isEmpty())
        return;

    QString strStream;
    QTextStream out(&strStream);

    QDir dir;
    QString img_path_logo = dir.toNativeSeparators(QDir::tempPath() + "/USG_pricing_" + QString::number(m_id));

    const int rowCount    = ui->tableView->model()->rowCount();
    const int columnCount = ui->tableView->model()->columnCount();

    out << "<html\n>"
        << "<head\n>"
        << "<meta Content = \"Text/html; charset=Windows-1251\">\n"
        << QString("<title>%1</title>\n").arg(tr("Prețurile investigațiilor ecografice"))
        << "</head>\n";

    QSqlQuery qry;
    qry.prepare("SELECT constants.id_organizations, "
                "constants.logo,"
                "organizations.IDNP, "
                "organizations.name, "
                "organizations.address, "
                "organizations.telephone, "
                "organizations.email "
                "FROM constants "
                "INNER JOIN organizations ON constants.id_organizations = organizations.id "
                "WHERE constants.id_users = :id_users;");
    qry.bindValue(":id_users", m_idUser);
    if (qry.exec() && qry.next()){
        QSqlRecord rec = qry.record();
        QByteArray logo_array = QByteArray::fromBase64(qry.value(rec.indexOf("logo")).toByteArray());
        QPixmap px_logo;
        if (! logo_array.isEmpty() && px_logo.loadFromData(logo_array, "png")){
            if (px_logo.save(img_path_logo, "png")){
                out << QString("<body><img src=%1 width=\"600\" height=\"80\" alt=\"Logo\"></body>").arg(img_path_logo);
            }
        }
        out << "<style> .caption {text-align:  right;}</style>"     // date despre organizatia
            << "<div class=\"caption\">\n"
            << QString("<p>%1<br>"
                       "<b><u>c/f:</u></b> %2<br>"
                       "<b><u>adresa:</u></b> %3<br>"
                       "<b><u>telefon:</u></b> %4<br>"
                       "<b><u>e-mail:</u></b> %5<br<br</p>\n").arg(qry.value(rec.indexOf("name")).toString(),
                        qry.value(rec.indexOf("IDNP")).toString(),
                        qry.value(rec.indexOf("address")).toString(),
                        qry.value(rec.indexOf("telephone")).toString(),
                        qry.value(rec.indexOf("email")).toString())
            << "</div>";
    }

    out << "<style> .text {text-align:  center;}</style>"       // titlu formei de tipar
        << "<div class=\"text\">\n"
        << QString("<p><h4>%1%2 din data %3<br>pentru %4</h4></p>\n").arg(tr("Prețurile investigațiilor ecografice nr."),
                                                                          ui->editNumberDoc->text(),
                                                                          ui->dateTimeDoc->date().toString("dd.MM.yyyy"),
                                                                          ui->comboOrganization->currentText())
        << "</div>"
        << "<body bgcolor = #ffffff link= 5000A0>\n>"
        << "<table border = 1 cellspacing=0 cellpadding=2>\n";

    out << "<thead><tr bgcolor=#f0f0f0>";
    for (int column = 0; column < columnCount; column++)
        if (!ui->tableView->isColumnHidden(column))
            out << QString("<th>%1</th>").arg(ui->tableView->model()->headerData(column, Qt::Horizontal).toString());
    out << "</tr></thead>\n";

    for (int row = 0; row < rowCount; row++){
        out << "<tr>";
        for (int column = 0; column < columnCount; column++){
            if (!ui->tableView->isColumnHidden(column)){
                QString data = ui->tableView->model()->data(ui->tableView->model()->index(row, column)).toString().simplified();
                out << QString("<td bkcolor=0>%1</td>").arg(!data.isEmpty() ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out << "</table>\n"
        << "</body>\n"
        << "</html>\n";

    QTextDocument* document = new QTextDocument();
    document->setHtml(strStream);
    documentPrint = document;

    QPrinter printer;
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, 0);
    dialog->resize(800, 600);

    connect(dialog, &QPrintPreviewDialog::paintRequested, this, [=](QPrinter *previewPrinter) {
            document->print(previewPrinter);
        });
    dialog->exec();

    if (QFile(img_path_logo).exists()){
        QString str_cmd;
#if defined(Q_OS_LINUX)
        str_cmd = "rm -f " + img_path_logo;
#elif defined(Q_OS_MACOS)
        str_cmd = "rm -f " + img_path_logo;
#elif defined(Q_OS_WIN)
        str_cmd = "del /f " + img_path_logo;
#endif
        try {
            system(str_cmd.toStdString().c_str());
        } catch (...) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Eliminarea imaginei."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Eliminarea imaginei din directoriul temporar nu s-a efectuat !!!"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            throw;
        }
    }
    delete dialog;
    delete document;
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

    if (m_post == idx_unknow)  // daca a fost apasat btnOk = propritatea trebuia sa fie m_post == idx_post
        setPost(idx_write);    // setam post = idx_write

    if (m_itNew){
        m_id = db->getLastIdForTable("pricings") + 1; // incercare de a seta 'id' documentului

        QString strQry = insertDataTablePricings();
        if (! db->execQuery(strQry)){
            QMessageBox::warning(this, tr("Validarea documentului"), tr("Documentul nu este %1 !!! <br> Adresați-vă administratorului aplicației.")
                                 .arg((m_post == idx_post) ? tr("validat") : tr("salvat")), QMessageBox::Ok);
            m_id = idx_unknow;   // setam la valoarea initiala
            setPost(idx_unknow); // setam m_post la valoarea initiala
            return false;
        }

        for (int i = 0; i < modelTable->rowCount(); ++i) {                           // corectam valoarea indexului 'deletionMark'
            modelTable->setData(modelTable->index(i, column_DeletionMark), m_post);  // si 'id' documentului din tabela
            modelTable->setData(modelTable->index(i, column_IdPricings), m_id);
        }

        if (! modelTable->submitAll()){                // daca nu a fost salvata tabela:
            db->removeObjectById("pricings", m_id);    // 1.eliminam datele salvate mai sus din sqlite
            m_id = idx_unknow;                         // 2.setam la valoarea initiala
            setPost(idx_unknow);                       // 3.setam m_post la valoarea initiala
            qCritical(logCritical()) << modelTable->lastError().text();
            return false;
        }

        if (m_post == idx_write){
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
            m_id = idx_unknow;   // setam la valoarea initiala
            setPost(idx_unknow); // setam m_post la valoarea initiala
            return false;
        }

        if (! modelTable->submitAll()){                // daca nu a fost salvata tabela:
            qCritical(logCritical()) << modelTable->lastError().text();
            return false;
        }
        if (m_post == idx_write){
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
    setPost(idx_post); // setam proprietatea 'post'

    if (onWritingData())
        QDialog::accept();
}

void DocPricing::onClose()
{
    emit mCloseThisForm();
    this->close();
}

void DocPricing::setTitleDoc()
{
    setWindowTitle(tr("Formarea prețurilor %1").arg("[*]"));
}

void DocPricing::initBtnToolBar()
{
    toolBar       = new QToolBar(this);
    btnAdd        = new QToolButton(this);
    btnEdit       = new QToolButton(this);
    btnDeletion   = new QToolButton(this);
    btnFormsTable = new QToolButton(this);

    btnAdd->setIcon(QIcon(":/img/add_x32.png").pixmap(16,16));
    btnAdd->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnAdd->setText(tr("Adaugă"));
    btnAdd->setStyleSheet("border: 1px solid #8f8f91;"
                             "padding-left: 5px;"
                             "text-align: left;"
                             "border-radius: 4px;"
                             "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                             "height: 18px;");
    btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));

    btnEdit->setIcon(QIcon(":/img/edit_x32.png"));
    btnEdit->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnEdit->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnEdit->setShortcut(QKeySequence(Qt::Key_F2));

    btnDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    btnDeletion->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnDeletion->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnDeletion->setShortcut(QKeySequence(Qt::Key_Delete));

    btnFormsTable->setIcon(QIcon(":/img/select_x32.png").pixmap(16,16));
    btnFormsTable->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnFormsTable->setText(tr("Completează"));
    btnFormsTable->setStyleSheet("border: 1px solid #8f8f91;"
                             "padding-left: 5px;"
                             "text-align: left;"
                             "border-radius: 4px;"
                             "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                             "height: 18px; width: 90px;");

    editSearch = new QLineEdit(this);
    editSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    editSearch->setClearButtonEnabled(true);

    comboSearch = new QComboBox(this);
    comboSearch->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    comboSearch->addItems(QStringList() << tr("cod") << tr("denumirea"));
    comboSearch->setCurrentIndex(0);

    toolBar->addWidget(btnAdd);
    toolBar->addWidget(btnEdit);
    toolBar->addWidget(btnDeletion);
    toolBar->addSeparator();
    toolBar->addWidget(btnFormsTable);
    toolBar->addSeparator();
    toolBar->addWidget(editSearch);
    toolBar->addSeparator();
    toolBar->addWidget(comboSearch);

    QSpacerItem *itemSpacer = new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addSpacerItem(itemSpacer);

    connect(btnFormsTable, &QAbstractButton::clicked, this, &DocPricing::completeTableDocPricing);
    connect(btnAdd, &QAbstractButton::clicked, this, &DocPricing::addRowTable);
    connect(btnEdit, &QAbstractButton::clicked, this, &DocPricing::editRowTable);
    connect(btnDeletion, &QAbstractButton::clicked, this, &DocPricing::deletionRowTable);

    connect(editSearch, &QLineEdit::textChanged, this, &DocPricing::filterRegExpChanged);
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

    ui->btnPrint->setIcon(QIcon(":/img/print_x32.png"));
    ui->btnPrint->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnPrint->setLayout(new QGridLayout);
    ui->btnPrint->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_P));

    ui->btnOK->setIcon(QIcon(":/img/accept_button.png"));
    ui->btnOK->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnOK->setLayout(new QGridLayout);
    ui->btnOK->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_Return));

    ui->btnWrite->setIcon(QIcon(":/img/write_doc.png"));
    ui->btnWrite->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnWrite->setLayout(new QGridLayout);
    ui->btnWrite->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_S));

    ui->btnClose->setIcon(QIcon(":/img/close_x32.png"));
    ui->btnClose->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnClose->setLayout(new QGridLayout);
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
}

void DocPricing::updateTableView()
{
    if (m_itNew){
        modelTable->clear();
        modelTable->setTable("pricingsTable");
        modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
        modelTable->setSort(column_Cod, Qt::AscendingOrder);
        modelTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
        proxy->setSourceModel(modelTable);
        ui->tableView->setModel(proxy);
        modelTable->select();
        ui->tableView->selectRow(0);
    } else {
        modelTable->clear();
        modelTable->setTable("pricingsTable");
        modelTable->setFilter(QString("id_pricings=%1").arg(m_id));
        modelTable->setSort(column_Cod, Qt::AscendingOrder);
        modelTable->setEditStrategy(QSqlTableModel::OnManualSubmit);
        proxy->setSourceModel(modelTable);
        ui->tableView->setModel(proxy);
        modelTable->select();
        ui->tableView->selectRow(0);
    }
    ui->tableView->hideColumn(column_Id);           // id
    ui->tableView->hideColumn(column_DeletionMark); // deletionMark
    ui->tableView->hideColumn(column_IdPricings);   // id_pricings
    ui->tableView->horizontalHeader()->setStretchLastSection(true);         // extinderea ultimei sectiei
    ui->tableView->setSortingEnabled(true);                                 // setam posibilitatea sortarii
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);     // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // setam multipla selectie
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);     // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);                            // initializam meniu contextual
    ui->tableView->horizontalHeader()->setSortIndicator(3, Qt::SortOrder::AscendingOrder); // sortarea dupa 3 sectie(Cod)
    ui->tableView->verticalHeader()->setDefaultSectionSize(16);    
    ui->tableView->setColumnWidth(column_Cod, 70);   // codul
    ui->tableView->setColumnWidth(column_Name, 500); // denuimirea investigatiei
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
        const QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Modificarea datelor"),
                                                                        tr("Datele au fost modificate.\n"
                                                                           "Doriți să salvați aceste modificări ?"),
                                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            onWritingDataClose();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
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
            QModelIndex indexEdit = proxy->index(_row, column_Price);
            onClickedRowTable(indexEdit);
            ui->tableView->clearFocus();
        } else {
            this->focusNextChild();
        }
    }
}
