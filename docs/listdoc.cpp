#include "listdoc.h"
#include "ui_listdoc.h"

ListDoc::ListDoc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListDoc)
{
    ui->setupUi(this);

    db = new DataBase(this); // conectarea la BD
    menu  = new QMenu(this); // meniu contextual
    popUp = new PopUp(this); // mesaje

    QString strQuery;
    model = new BaseSqlQueryModel(strQuery, ui->tabView); // model principal pu solicitarea din BD
    proxy = new BaseSortFilterProxyModel(this);           // proxy-model pu sortarea datelor
    proxy->setListFormType(BaseSortFilterProxyModel::ListFormType::ListDocuments);

    QString strQryOrganizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";    // solicitarea
    QString strQryContracts     = "SELECT id,name FROM contracts WHERE deletionMark = 0;";
    QString strQueryUsers       = "SELECT id,name FROM users WHERE deletionMark = 0;";
    modelOrganizations = new BaseSqlQueryModel(strQryOrganizations, ui->FilterComboOrganization); // modele pu filtrarea
    modelContracts     = new BaseSqlQueryModel(strQryContracts, ui->filterComboContract);
    modelUsers         = new BaseSqlQueryModel(strQueryUsers, ui->filterComboUser);
    modelOrganizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelContracts->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelUsers->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->FilterComboOrganization->setModel(modelOrganizations);
    ui->filterComboContract->setModel(modelContracts);
    ui->filterComboUser->setModel(modelUsers);

    initBtnToolBar();    // initializarea butoanelor tool-barului
    initBtnFilter();     // initializam butoanele filtrului

    loadSizeSectionPeriodTable(true);

    if (itemsFilterPeriod.isEmpty()){
        ui->filterStartDateTime->setDateTime(QDateTime::fromString("2021-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss"));
        ui->filterEndDateTime->setDateTime(QDateTime::fromString(QDate::currentDate().toString("yyyy-MM-dd") + " 23:59:59", "yyyy-MM-dd hh:mm:ss")); // QDateTime::fromString(QDate::currentDate().toString("yyyy-MM-dd") + " 23:59:59", "yyyy-MM-dd hh:mm:ss")
        onStartDateTimeChanged();
        onEndDateTimeChanged();
    }

    updateTableView();   // actualizam/completam datele tabelei
    updateHeaderTable(); // actualizam denumirea sectiilor

    ui->groupBoxFilterComplex->setVisible(false);     // initial

    connect(ui->filterStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDoc::onStartDateTimeChanged);
    connect(ui->filterEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDoc::onEndDateTimeChanged);

    connect(ui->FilterComboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&ListDoc::indexChangedFilterComboOrganization));
    connect(ui->filterComboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&ListDoc::indexChangedFilterComboContract));
    connect(ui->filterComboUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&ListDoc::indexChangedFilterComboUser));

    connect(ui->tabView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &ListDoc::onDoubleClickedTable);
    connect(ui->tabView, &QWidget::customContextMenuRequested, this, &ListDoc::slotContextMenuRequested);

}

ListDoc::~ListDoc()
{
    delete menu;
    delete popUp;
    delete toolBar;
    delete proxy;
    delete modelOrganizations;
    delete modelContracts;
    delete modelUsers;
    delete model;
    delete db;
    delete ui;
}

void ListDoc::initBtnToolBar()
{
    toolBar         = new QToolBar(this);
    btnSelect       = new QToolButton(toolBar);
    btnAdd          = new QToolButton(toolBar);
    btnEdit         = new QToolButton(toolBar);
    btnDeletion     = new QToolButton(toolBar);
    btnFilter       = new QToolButton(toolBar);
    btnAddFilter    = new QToolButton(toolBar);
    btnFilterRemove = new QToolButton(toolBar);
    btnUpdateTable  = new QToolButton(toolBar);
    btnPost         = new QToolButton(toolBar);
    btnUnpost       = new QToolButton(toolBar);
    btnPrint        = new QToolButton(toolBar);
    btnPeriodDate   = new QToolButton(toolBar);

    btnSelect->setIcon(QIcon(":/img/select_x32.png").pixmap(16,16));
    btnSelect->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnSelect->setText(tr("Selectează"));
    btnSelect->setStyleSheet("border: 1px solid #8f8f91;"
                             "padding-left: 5px;"
                             "text-align: left;"
                             "border-radius: 4px;"
                             "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                             "height: 18px; width: 100px;");
    btnSelect->setVisible(m_modeSelection);

    btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    btnAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnAdd->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));

    btnEdit->setIcon(QIcon(":/img/edit_x32.png"));
    btnEdit->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnEdit->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnEdit->setShortcut(QKeySequence(Qt::Key_F2));

    btnDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    btnDeletion->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnDeletion->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnDeletion->setShortcut(QKeySequence(Qt::Key_Delete));

    btnAddFilter->setIcon(QIcon(":/img/filter-add-icon.png"));
    btnAddFilter->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnAddFilter->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");

    btnFilter->setIcon(QIcon(":/img/filter-icon.png"));
    btnFilter->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFilter->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");

    btnFilterRemove->setIcon(QIcon(":/img/filter-delete-icon.png"));
    btnFilterRemove->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFilterRemove->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");

    btnUpdateTable->setIcon(QIcon(":/img/update_x32.png"));
    btnUpdateTable->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnUpdateTable->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnUpdateTable->setShortcut(QKeySequence(Qt::Key_F5));

    btnPost->setIcon(QIcon(":/img/document_accept.png").pixmap(14, 14));
    btnPost->setText(tr("Validează"));
    btnPost->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnPost->setStyleSheet("border: 1px solid #8f8f91; "
                           "padding-left: 5px; padding-right: 5px;"
                           "text-align: left; "
                           "border-radius: 4px; "
                           "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                           "height: 18px; width: 100px;");

    btnUnpost->setIcon(QIcon(":/img/document_remove.png").pixmap(14, 14));
    btnUnpost->setText(tr("Invalidează"));
    btnUnpost->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnUnpost->setStyleSheet("border: 1px solid #8f8f91; "
                           "padding-left: 5px; padding-right: 5px;"
                           "text-align: left; "
                           "border-radius: 4px; "
                           "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                           "height: 18px; width: 100px;");

    btnPrint->setIcon(QIcon(":/img/print_x32.png").pixmap(14, 14));
    btnPrint->setText(tr("Printează"));
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnPrint->setStyleSheet("border: 1px solid #8f8f91; "
                           "padding-left: 5px; padding-right: 5px;"
                           "text-align: left; "
                           "border-radius: 4px; "
                           "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                           "height: 18px; width: 100px;");

    btnPeriodDate->setIcon(QIcon(":/img/date_period.png"));
    btnPeriodDate->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnPeriodDate->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");

    toolBar->addWidget(btnSelect);
    toolBar->addSeparator();
    toolBar->addWidget(btnAdd);
    toolBar->addWidget(btnEdit);
    toolBar->addWidget(btnDeletion);
    toolBar->addSeparator();
    toolBar->addWidget(btnAddFilter);
    toolBar->addWidget(btnFilter);
    toolBar->addWidget(btnFilterRemove);
    toolBar->addSeparator();
    toolBar->addWidget(btnUpdateTable);
    toolBar->addSeparator();
    toolBar->addWidget(btnPost);
    toolBar->addWidget(btnUnpost);
    toolBar->addSeparator();
    toolBar->addWidget(btnPrint);
    toolBar->addSeparator();
    toolBar->addWidget(btnPeriodDate);

    QSpacerItem *itemSpacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addSpacerItem(itemSpacer);
    lablePeriodDate = new QLabel(this);
    lablePeriodDate->setStyleSheet("color: blue; font-weight: bold;");
    ui->layoutToolBar->addWidget(lablePeriodDate);

    if (m_modeSelection)
        connect(btnSelect, &QAbstractButton::clicked, this, &ListDoc::selectDocPricing);
    connect(btnAdd, &QAbstractButton::clicked, this, &ListDoc::createNewDocPricing);    
    connect(btnEdit, &QAbstractButton::clicked, this, &ListDoc::editDocPricing);
    connect(btnDeletion, &QAbstractButton::clicked, this, &ListDoc::deletionMarkDocPricing);

    connect(btnAddFilter, &QAbstractButton::clicked, this, &ListDoc::addFilterForListDoc);
    connect(btnFilter, &QAbstractButton::clicked, this, &ListDoc::setFilterByIdOrganization);
    connect(btnFilterRemove, &QAbstractButton::clicked, this, &ListDoc::removeFilterByIdOrganization);

    connect(btnUpdateTable, &QAbstractButton::clicked, this, &ListDoc::updateTableView);

    connect(btnPost, &QAbstractButton::clicked, this, &ListDoc::postDoc);
    connect(btnUnpost, &QAbstractButton::clicked, this, &ListDoc::unpostDoc);
    connect(btnPrint, &QAbstractButton::clicked, this, &ListDoc::onPrintDoc);

    connect(btnPeriodDate, &QAbstractButton::clicked, this, &ListDoc::openFilterPeriod);

    connect(this, &ListDoc::ItFilterChanged, this, &ListDoc::slot_ItFilterChanged);
    connect(this, &ListDoc::ItFilterPeriodChanged, this, &ListDoc::slot_ItFilterPeriodChanged);
    connect(this, &ListDoc::IdOrganizationChanged, this, &ListDoc::slot_IdOrganizationChanged);
    connect(this, &ListDoc::IdContractChanged, this, &ListDoc::slot_IdContractChanged);
    connect(this, &ListDoc::IdUserChanged, this, &ListDoc::slot_IdUserChanged);
}

void ListDoc::initBtnFilter()
{
    ui->btnFilterApply->setIcon(QIcon(":/img/accept_button.png"));
    ui->btnFilterApply->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnFilterApply->setLayout(new QGridLayout);

    ui->btnFilterClear->setIcon(QIcon(":/img/button_delete.png"));
    ui->btnFilterClear->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnFilterClear->setLayout(new QGridLayout);

    ui->btnFilterClose->setIcon(QIcon(":/img/close_x32.png"));
    ui->btnFilterClose->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnFilterClose->setLayout(new QGridLayout);

    connect(ui->btnFilterApply, &QAbstractButton::clicked, this, &ListDoc::applyFilter);
    connect(ui->btnFilterClear, &QAbstractButton::clicked, this, &ListDoc::clearItemsFilter);
    connect(ui->btnFilterClose, &QAbstractButton::clicked, this, &ListDoc::closeFilterComplex);
}

void ListDoc::loadSizeSectionPeriodTable(bool only_period)
{
    QSqlQuery qry;

    //*****************************************************
    //---------- setarea perioadei
    if (only_period){
        QString str = QString("SELECT "
                              "dateStart,"
                              "dateEnd "
                              "FROM settingsForms "
                              "WHERE typeForm LIKE %1 AND dateStart %2 AND dateEnd %2;")
                          .arg("'%DocPricing%'", (globals::thisMySQL) ? "IS NOT Null" : "NOT NULL");
        qry.prepare(str);
        if (qry.exec()){
            qry.next();
            QString str_date_start;
            QString str_date_end;
            if (globals::thisMySQL){
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                str_date_start = qry.value(0).toString().replace(QRegExp("T"), " ").replace(".000","");
                str_date_end   = qry.value(1).toString().replace(QRegExp("T"), " ").replace(".000","");
#else
                str_date_start = qry.value(0).toString().replace(QRegularExpression("T"), " ").replace(".000","");
                str_date_end   = qry.value(1).toString().replace(QRegularExpression("T"), " ").replace(".000","");
#endif
            } else {
                str_date_start = qry.value(0).toString();
                str_date_end   = qry.value(1).toString();
            }
            ui->filterStartDateTime->setDateTime(QDateTime::fromString(str_date_start, "yyyy-MM-dd hh:mm:ss"));
            ui->filterEndDateTime->setDateTime(QDateTime::fromString(str_date_end, "yyyy-MM-dd hh:mm:ss"));
        } else {
            ui->filterStartDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 01, 01), QTime(00,00,00)));
            ui->filterEndDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 12, 31), QTime(23,59,59)));

            //---------- prezentarea erorii
            if (! qry.lastError().text().isEmpty()){
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Determinarea dimensiunilor coloanelor"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Nu au fost determinate dimensiunile colanelor tabelei si directia sortarii !!!"));
                msgBox.setDetailedText(qry.lastError().text());
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setStyleSheet("QPushButton{width:120px;}");
                msgBox.exec();
            }
        }
        return;
    }

    //******************************************************
    //---------- setarea dimensiunilor si directiei sortarii

    qry.prepare(QString("SELECT "
                        "numberSection,"
                        "sizeSection,"
                        "directionSorting,"
                        "dateStart,"
                        "dateEnd "
                        "FROM settingsForms "
                        "WHERE typeForm LIKE %1;").arg("'%DocPricing%'"));
    if (qry.exec()){
        int num = -1;
        while (qry.next()) {
            num ++;
            const int current_size = qry.value(1).toInt();
            const int direction_sorting = qry.value(2).toInt();

            //---------- setarea dimensiunilor sectiilor
            if (current_size >= 0)
                ui->tabView->setColumnWidth(num, qry.value(1).toInt());
            else
                ui->tabView->setColumnWidth(num, sizeSectionDefault(num));

            //---------- setarea directiei sortarii
            if (direction_sorting > 0)
                ui->tabView->horizontalHeader()->setSortIndicator(num, (direction_sorting == 0) ? Qt::AscendingOrder : Qt::DescendingOrder);
        }
    } else {

        //---------- setarea dimensiunilor sectiilor
        for (int numSection = 0; numSection < ui->tabView->horizontalHeader()->count(); ++numSection){
            ui->tabView->setColumnWidth(numSection, sizeSectionDefault(numSection));
        }

        //---------- setarea directiei sortarii
        ui->tabView->horizontalHeader()->setSortIndicator(section_numberDoc, Qt::DescendingOrder);

        //---------- prezentarea erorii
        if (! qry.lastError().text().isEmpty()){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Determinarea dimensiunilor coloanelor"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Nu au fost determinate dimensiunile colanelor tabelei si directia sortarii !!!"));
            msgBox.setDetailedText(qry.lastError().text());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }
    }

    // pozitionam cursorul
    ui->tabView->selectRow(0);
}

QString ListDoc::getStrQuery()
{
    QString strQuery = "SELECT pricings.id,"
                       "pricings.deletionMark,"
                       "pricings.numberDoc,"
                       "pricings.dateDoc,"
                       "organizations.id AS IdOrganization,"
                       "organizations.name AS Organization,"
                       "contracts.name AS Contract,"
                       "users.name AS Author,"
                       "pricings.comment FROM pricings "
                       "INNER JOIN organizations ON pricings.id_organizations = organizations.id "
                       "INNER JOIN contracts ON pricings.id_contracts = contracts.id "
                       "INNER JOIN users ON pricings.id_users = users.id";
    if (m_itFilter){
        if(!m_numberDoc.isEmpty() || m_idOrganization > 0 || m_idContract > 0 || m_idUser > 0)
            strQuery = strQuery + " WHERE ";
        if (!m_numberDoc.isEmpty())
            strQuery = strQuery + QString("pricings.numberDoc = %1").arg(m_numberDoc);
        if (m_idOrganization > 0){
            if (!m_numberDoc.isEmpty())
                strQuery = strQuery + " AND ";
            strQuery = strQuery + QString("pricings.id_organizations = %1").arg(m_idOrganization);
        }
        if (m_idContract > 0){
            if (!m_numberDoc.isEmpty() || ui->FilterComboOrganization->currentIndex() > 0)
                strQuery = strQuery + " AND ";
            strQuery = strQuery + QString("pricings.id_contracts = %1").arg(m_idContract);
        }
        if (m_idUser > 0){
            if (!m_numberDoc.isEmpty() || ui->FilterComboOrganization->currentIndex() > 0 || ui->filterComboContract->currentIndex() > 0)
                strQuery = strQuery + " AND ";
            strQuery = strQuery + QString("pricings.id_users = %1").arg(m_idUser);
        }
    }

    QString startDate = ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDate = ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (m_itFilter)
        strQuery = strQuery + " AND ";
    else
        strQuery = strQuery + " WHERE ";
    strQuery = strQuery + QString("pricings.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);

    strQuery = strQuery + ";";
    return strQuery;
}

void ListDoc::updateTableView()
{
    QString strQuery = getStrQuery();
    model->setQuery(strQuery); // solicitare
    proxy->setSourceModel(model);
    ui->tabView->setModel(proxy);

    ui->tabView->hideColumn(section_id);                               // id-ascundem
    ui->tabView->hideColumn(section_idOrganization);                   // ascundem IdOrganization    
    ui->tabView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tabView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tabView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tabView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tabView->setContextMenuPolicy(Qt::CustomContextMenu);  // initializam meniu contextual
    ui->tabView->verticalHeader()->setDefaultSectionSize(14);
    ui->tabView->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    if (m_currentRow != -1)
        ui->tabView->selectRow(m_currentRow);
    else
        ui->tabView->selectRow(0);

    loadSizeSectionPeriodTable();
}

void ListDoc::updateHeaderTable()
{
    setWindowTitle(tr("Documente: Formarea prețurilor"));
    model->setHeaderData(section_deletionMark, Qt::Horizontal, tr(""));
    model->setHeaderData(section_numberDoc, Qt::Horizontal, tr("Număr"));
    model->setHeaderData(section_dateDoc, Qt::Horizontal, tr("Data"));
    model->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația (id)"));
    model->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația"));
    model->setHeaderData(section_Contract, Qt::Horizontal, tr("Contract"));
    model->setHeaderData(section_author, Qt::Horizontal, tr("Autor"));
    model->setHeaderData(section_comment, Qt::Horizontal, tr("Comentariu"));
}

void ListDoc::selectDocPricing()
{
    if (ui->tabView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _id = proxy->data(proxy->index(ui->tabView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    SetIdDocSelection(_id);
}

void ListDoc::createNewDocPricing()
{
    docPricing = new DocPricing(this);
    docPricing->setAttribute(Qt::WA_DeleteOnClose);
    docPricing->setProperty("ItNew", true);    
    docPricing->show();
    connect(docPricing, &DocPricing::PostDocument, this, &ListDoc::updatePostDocs);
}

void ListDoc::editDocPricing()
{
    if (ui->tabView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _id = proxy->data(proxy->index(ui->tabView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();

    docPricing = new DocPricing(this);
    docPricing->setAttribute(Qt::WA_DeleteOnClose);
    docPricing->setProperty("ItNew", false);
    docPricing->setProperty("Id", _id);    
    docPricing->show();
    connect(docPricing, &DocPricing::mCloseThisForm, this, &ListDoc::updatePostDocs);
}

void ListDoc::deletionMarkDocPricing()
{
    if (ui->tabView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    int _row       = ui->tabView->currentIndex().row();
    int _id        = proxy->data(proxy->index(_row, section_id), Qt::DisplayRole).toInt();
    int _delMark   = proxy->data(proxy->index(_row, section_deletionMark), Qt::DisplayRole).toInt();
    QString numDoc = proxy->data(proxy->index(_row, section_numberDoc), Qt::DisplayRole).toString();

    QString strMSG;
    if (_delMark == 0)
        strMSG = tr("Documentul cu numărul '%1' a fost marcat \n"
                    "pentru eliminarea din baza de date.").arg(numDoc);
    else if (_delMark == 1)
        strMSG = tr("Documentul cu numărul '%1' a fost demarcat \n"
                    "pentru eliminarea din baza de date.").arg(numDoc);
    if (db->deletionMarkObject("pricings", _id)){
        // de adaugat procedura de marcare a tabelei documentului !!!!!!!!!!!!!!
        popUp->setPopupText(strMSG);
        popUp->show();
    } else {
        QMessageBox::warning(this, tr("Marcarea/demarcarea documentului"),
                             tr("Marcarea/demarcarea documentului cu numărul '%1' nu este reusita.").arg(numDoc), QMessageBox::Ok);
        return;
    }
    m_currentRow = ui->tabView->currentIndex().row(); // memoram current row
    updateTableView();
}

void ListDoc::onDoubleClickedTable(const QModelIndex &index)
{
    Q_UNUSED(index)
    editDocPricing();
}

void ListDoc::addFilterForListDoc()
{
    setItFilter(true);

    ui->btnFilterClear->setVisible(m_itFilter);

    if (itemsFilter.count() > 0){
        ui->filterEditNumberDoc->setText(itemsFilter.constFind("numberDoc").value().toString());
        ui->FilterComboOrganization->setCurrentIndex(itemsFilter.constFind("index_organization").value().toInt());
        ui->filterComboContract->setCurrentIndex(itemsFilter.constFind("index_contract").value().toInt());
        ui->filterComboUser->setCurrentIndex(itemsFilter.constFind("index_user").value().toInt());
    }

    if (ui->FilterComboOrganization->currentIndex() == 0)
        ui->filterComboContract->setEnabled(false);   // setam accesul la contract
}

void ListDoc::setFilterByIdOrganization()
{
    int row = ui->tabView->currentIndex().row();
    int id_organization = proxy->data(proxy->index(row, section_idOrganization), Qt::DisplayRole).toInt();
    m_itFilter = true;
    m_idOrganization = id_organization;
    m_idContract = -1;
    m_idUser = -1;
    itemsFilter.clear();
    m_currentRow = ui->tabView->currentIndex().row(); // memoram randul curent
    updateTableView();
}

void ListDoc::removeFilterByIdOrganization()
{
    m_itFilter = false;
    m_idOrganization = -1;
    m_idContract = -1;
    m_idUser = -1;
    itemsFilter.clear();
    m_currentRow = ui->tabView->currentIndex().row(); // memoram randul curent
    updateTableView();
}

bool ListDoc::controlRequiredObjects()
{
    return true;
}

void ListDoc::clearItemsFilter()
{
    ui->filterEditNumberDoc->clear();
    ui->FilterComboOrganization->setCurrentIndex(0); // setam indexul la toate comboboxurile la 0 (<-selecteaza->)
    ui->filterComboContract->setCurrentIndex(0);
    ui->filterComboUser->setCurrentIndex(0);

    m_idOrganization = -1;
    m_idContract = -1;
    m_idUser = -1;

    itemsFilter.clear();
    ui->filterComboContract->setEnabled(false);

    updateTableView(); // actualizam tabela
}

void ListDoc::postDoc()
{
    if (!controlRequiredObjects())
        return;
}

void ListDoc::unpostDoc()
{

}

void ListDoc::onPrintDoc()
{

}

void ListDoc::openFilterPeriod()
{
    setItFilterPeriod(true);
    ui->btnFilterClear->setVisible(m_itFilter);
}

void ListDoc::onStartDateTimeChanged()
{
//    if (ui->filterStartDateTime->dateTime() > ui->filterEndDateTime->dateTime()){
//        QMessageBox::warning(this, tr("Verificarea perioadei"),
//                             tr("Data lansării perioade nu poate fi mai mare \ndecât data finisării perioadei !!!"),
//                             QMessageBox::Ok, QMessageBox::Ok);
//        return;
//    }
    itemsFilterPeriod.insert("startDate", ui->filterStartDateTime->dateTime());
    lablePeriodDate->setText(tr("Perioada: ") + ui->filterStartDateTime->dateTime().toString("dd.MM.yyyy") + " - " + ui->filterEndDateTime->dateTime().toString("dd.MM.yyyy"));
;}

void ListDoc::onEndDateTimeChanged()
{
    if (ui->filterStartDateTime->dateTime() > ui->filterEndDateTime->dateTime()){
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data finisării perioadei nu poate fi mai mică decât \ndata lansării perioadei !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    itemsFilterPeriod.insert("endDate", ui->filterEndDateTime->dateTime());
    lablePeriodDate->setText(tr("Perioada: ") + ui->filterStartDateTime->dateTime().toString("dd.MM.yyyy") + " - " + ui->filterEndDateTime->dateTime().toString("dd.MM.yyyy"));
}

void ListDoc::indexChangedFilterComboOrganization(const int arg1)
{
    Q_UNUSED(arg1)
    int id_organization = ui->FilterComboOrganization->itemData(ui->FilterComboOrganization->currentIndex(), Qt::UserRole).toInt();
    if (id_organization > 0){
        itemsFilter.insert("index_organization", ui->FilterComboOrganization->currentIndex());
        setIdOrganization(id_organization);
    }
}

void ListDoc::indexChangedFilterComboContract(const int arg1)
{
    Q_UNUSED(arg1)
    int id_contract = ui->filterComboContract->itemData(ui->filterComboContract->currentIndex(), Qt::UserRole).toInt();
    if (id_contract > 0){
        itemsFilter.insert("index_contract", ui->filterComboContract->currentIndex());
        setIdContract(id_contract);
    }
}

void ListDoc::indexChangedFilterComboUser(const int arg1)
{
    Q_UNUSED(arg1)
    int id_user = ui->filterComboUser->itemData(ui->filterComboUser->currentIndex(), Qt::UserRole).toInt();
    if (id_user > 0){
        itemsFilter.insert("index_user", ui->filterComboUser->currentIndex());
        setIdUser(id_user);
    }
}

void ListDoc::applyFilter()
{
//    if (m_itFilter){
//        itemsFilter.clear();
//        itemsFilter.insert("numberDoc", ui->filterEditNumberDoc->text());
//        itemsFilter.insert("index_organization", ui->FilterComboOrganization->currentIndex());
//        itemsFilter.insert("index_contract", ui->filterComboContract->currentIndex());
//        itemsFilter.insert("index_user", ui->filterComboUser->currentIndex());
//    }
//    if (m_itFilterPeriod){
//        itemsFilterPeriod.clear();
//        itemsFilterPeriod.insert("startDate", ui->filterStartDateTime->dateTime());
//        itemsFilterPeriod.insert("endDate", ui->filterEndDateTime->dateTime());
//    }
    updateTableView();
    ui->groupBoxFilterComplex->setVisible(false);
}

void ListDoc::closeFilterComplex()
{
    if (m_itFilter){
        itemsFilter.insert("numberDoc", ui->filterEditNumberDoc->text());
        itemsFilter.insert("index_organization", ui->FilterComboOrganization->currentIndex());
        itemsFilter.insert("index_contract", ui->filterComboContract->currentIndex());
        itemsFilter.insert("index_user", ui->filterComboUser->currentIndex());
        setItFilter(false);
    }
    if (m_itFilterPeriod){
        itemsFilterPeriod.insert("startDate", ui->filterStartDateTime->dateTime());
        itemsFilterPeriod.insert("endDate", ui->filterEndDateTime->dateTime());
        setItFilterPeriod(false);
    }
}

void ListDoc::slot_ItFilterChanged()
{
    ui->groupBoxFilterComplex->setVisible(m_itFilter || m_itFilterPeriod);
    ui->groupBoxPeriod->setVisible(m_itFilterPeriod);
    ui->groupBoxFilter->setVisible(m_itFilter);
}

void ListDoc::slot_ItFilterPeriodChanged()
{
    ui->groupBoxFilterComplex->setVisible(m_itFilter || m_itFilterPeriod);
    ui->groupBoxPeriod->setVisible(m_itFilterPeriod);
    ui->groupBoxFilter->setVisible(m_itFilter);
}

void ListDoc::slot_IdOrganizationChanged()
{
    if (m_idOrganization == -1 || m_idOrganization == 0)
        return;

    ui->filterComboContract->setEnabled(ui->FilterComboOrganization->currentIndex() > 0); // setam accesul la contract

    QMap<QString, QString> items;
    if (db->getObjectDataById("organizations", m_idOrganization, items)){
        const int id_contract = items.constFind("id_contracts").value().toInt();
        if (id_contract > 0){
            modelContracts->setQuery(QString("SELECT id,name FROM contracts WHERE deletionMark = 0 AND id_organizations = %1;").arg(m_idOrganization));
            setIdContract(id_contract);
        }
    }
}

void ListDoc::slot_IdContractChanged()
{
    if (m_idOrganization == -1 || m_idContract == -1 || m_idContract == 0)
        return;
    auto indexContract = modelContracts->match(modelContracts->index(0,0), Qt::UserRole, m_idContract, 1, Qt::MatchExactly);
    if (!indexContract.isEmpty())
        ui->filterComboContract->setCurrentIndex(indexContract.first().row());
}

void ListDoc::slot_IdUserChanged()
{
    if (m_idUser == -1 || m_idUser == 0)
        return;
}

void ListDoc::slot_ModeSelectionChanged()
{
    btnSelect->setVisible(m_modeSelection);
}

void ListDoc::slotContextMenuRequested(QPoint pos)
{
    int _row = ui->tabView->currentIndex().row();
//    int _id  = proxy->data(proxy->index(_row, section_id), Qt::DisplayRole).toInt();
    int _deletionMark  = proxy->data(proxy->index(_row, section_deletionMark), Qt::DisplayRole).toInt();
//    int _id_organization = proxy->data(proxy->index(_row, section_idOrganization), Qt::DisplayRole).toInt();
//    QString nameOrganization = proxy->data(proxy->index(_row, section_Organization), Qt::DisplayRole).toString();
    QString numberDoc = proxy->data(proxy->index(_row, section_numberDoc), Qt::DisplayRole).toString();

    QString strQueryDeletionMark;
    if (_deletionMark == 0 || _deletionMark == 3)
        strQueryDeletionMark = tr("Marchează documentul cu nr.\"%1\" pentru eliminarea din baza de date.").arg(numberDoc);
    else if (_deletionMark == 1)
        strQueryDeletionMark = tr("Demarchează documentul cu nr.\"%1\" pentru eliminarea din baza de date.").arg(numberDoc);

    QAction* actionAddObject  = new QAction(QIcon(":/img/add_x32.png"), tr("Crearea documentului nou."), this);
    QAction* actionEditObject = new QAction(QIcon(":/img/edit_x32.png"), tr("Editează documentul cu nr.\"%1\".").arg(numberDoc), this);
    QAction* actionMarkObject = new QAction(QIcon(":/img/clear_x32.png"), strQueryDeletionMark, this);

    connect(actionAddObject, &QAction::triggered, this, &ListDoc::createNewDocPricing);
    connect(actionEditObject, &QAction::triggered, this, &ListDoc::editDocPricing);
    connect(actionMarkObject, &QAction::triggered, this, &ListDoc::deletionMarkDocPricing);

    menu->clear();
    menu->addAction(actionAddObject);
    menu->addAction(actionEditObject);
    menu->addAction(actionMarkObject);
    menu->popup(ui->tabView->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

void ListDoc::updatePostDocs()
{
    updateTableView();
}

void ListDoc::saveSizeSectionTable()
{
    // salvam dimensiunile setiilor
    for (int numSection = 0; numSection < ui->tabView->horizontalHeader()->count(); ++numSection) {

        // determinarea si setarea variabilei directiei de sortare
        int directionSorting = -1;
        if (numSection == ui->tabView->horizontalHeader()->sortIndicatorSection())
            directionSorting = ui->tabView->horizontalHeader()->sortIndicatorOrder();

        // variabile pu inserarea sau actualizarea datelor in tabela
        const int find_id = db->findIdFromTableSettingsForm(type_doc, numSection);
        const int size_section = ui->tabView->horizontalHeader()->sectionSize(numSection);

        // inserarea dimensiunilor sectiilor si directia sortarii
        if (find_id > 0)
            db->insertUpdateDataTableSettingsForm(false, type_doc, numSection, size_section, find_id, directionSorting);
        else
            db->insertUpdateDataTableSettingsForm(true, type_doc, numSection, size_section, -1, directionSorting);
    }

    // salvam perioada
    const int find_id_section_zero = db->findIdFromTableSettingsForm(type_doc, section_zero);
    if (find_id_section_zero > 0)
        db->updatePeriodInTableSettingsForm(find_id_section_zero,
                                            ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"),
                                            ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

int ListDoc::sizeSectionDefault(const int numberSection)
{
    switch (numberSection) {
    case section_id:
        return sz_id;
    case section_deletionMark:
        return sz_deletionMark;
    case section_numberDoc:
        return sz_numberDoc;
    case section_dateDoc:
        return sz_dateDoc;
    case section_idOrganization:
        return sz_id_organization;
    case section_Organization:
        return sz_organization;
    case section_Contract:
        return sz_contract;
    case section_author:
        return sz_author;
    default:
        return 0;
    }
}

void ListDoc::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSizeSectionTable();
    }
}

void ListDoc::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Lista documentelor: Formarea preturilor"));
        updateHeaderTable();
    }
}
