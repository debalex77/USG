#include "pricingview.h"
#include "ui_pricingview.h"

PricingView::PricingView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PricingView)
    , m_settings(globals().pathSettingsCommon)
    , m_db(db)
    , popUp(new PopUp(this))
    , menu(new QMenu(this))
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
    , toolButtonStyleForIcon(m_db.toolButtonStyleForIcon())
{
    ui->setupUi(this);

    setWindowTitle(tr("Lista documentelor: Formarea prețurilor"));

    // incarcam setarile jurnalului
    loadDocJournalSettings();

    initPeriod(); // initiem perioada

    updateModelOrganizations();
    updateModelContracts();
    updateModelUsers();

    initBtnToolBar();    // initializarea butoanelor tool-barului
    initBtnFilter();     // initializam butoanele filtrului

    loadDataFilter();
    initTableView();

    ui->groupBoxFilter->setHidden(true); // initial
    initConnections();

    updateTableView();   // actualizam/completam datele tabelei
    loadSizeSection();
}

PricingView::~PricingView()
{
    delete ui;
}

void PricingView::loadDocJournalSettings()
{
    const QDateTime m_dtStart = QDateTime::fromString("2021-01-01T00:00:00", Qt::ISODate);
    const QDateTime m_dtEnd   = QDateTime(QDate::currentDate(), QTime(23, 59, 59));

    const QJsonObject obj = m_settings.getJsonObject(type_doc);
    if (obj.isEmpty()) {
        journalSettings.startDate = m_dtStart;
        journalSettings.endDate   = m_dtEnd;
        return;
    }

    // --- perioada
    const QDateTime dtStart = QDateTime::fromString(
        obj.value("startDate").toString(), Qt::ISODateWithMs);
    const QDateTime dtEnd = QDateTime::fromString(
        obj.value("endDate").toString(), Qt::ISODateWithMs);

    journalSettings.startDate = dtStart.isValid() ? dtStart : m_dtStart;
    journalSettings.endDate   = dtEnd.isValid() ? dtEnd : m_dtEnd;

    // --- filtru
    const QJsonObject filterObj = obj.value("filter").toObject();
    journalSettings.saveFilter     = filterObj.value("saveFilter").toBool(false);
    journalSettings.nrDoc          = filterObj.value("nr_doc").toString();
    journalSettings.idOrganization = filterObj.value("id_organization").toInt(0);
    journalSettings.idContract     = filterObj.value("id_contract").toInt(0);
    journalSettings.idUser         = filterObj.value("id_user").toInt(0);

    // --- sortarea sectiilor
    const QJsonObject sortObj = obj.value("sort").toObject();
    journalSettings.sortSection = sortObj.value("section").toInt(0);
    journalSettings.sortOrder = sortObj.value("direction").toInt(0) == 0
                                    ? Qt::AscendingOrder
                                    : Qt::DescendingOrder;

    // --- size section
    const QJsonObject sectionsObj = obj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            journalSettings.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = obj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            journalSettings.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void PricingView::initPeriod()
{
    ui->dateStart->setMaximumDateTime(QDateTime::currentDateTime());
    ui->dateEnd->setMinimumDateTime(ui->dateStart->dateTime());

    ui->dateStart->setDateTime(journalSettings.startDate);
    ui->dateEnd->setDateTime(journalSettings.endDate);
    updateTextPeriod();
}

void PricingView::initBtnToolBar()
{
    ui->btnSelectPeriod->setStyleSheet(toolButtonStyleForText);

    QList<QToolButton*> tbs = this->findChildren<QToolButton*>();
    for (QToolButton *tb : std::as_const(tbs)) {
        if (tb == ui->btnSelectPeriod)
            continue;
        tb->setStyleSheet(toolButtonStyleForIcon);
        tb->setMouseTracking(true);
        tb->installEventFilter(this);
    }

    connect(ui->btnAdd, &QAbstractButton::clicked,
            this, &PricingView::createNewDoc, Qt::UniqueConnection);
    connect(ui->btnEdit, &QAbstractButton::clicked,
            this, &PricingView::editDoc, Qt::UniqueConnection);
    connect(ui->btnDeletion, &QAbstractButton::clicked,
            this, &PricingView::deleteDoc, Qt::UniqueConnection);

    connect(ui->btnAddFilter, &QAbstractButton::clicked,
            this, &PricingView::openFilter, Qt::UniqueConnection);
    connect(ui->btnSetFilter, &QAbstractButton::clicked,
            this, &PricingView::setFilterByIDOrganization, Qt::UniqueConnection);
    connect(ui->btnDeleteFilter, &QAbstractButton::clicked,
            this, &PricingView::clearFilter, Qt::UniqueConnection);

    connect(ui->btnUpdateTable, &QAbstractButton::clicked,
            this, &PricingView::updateTableView, Qt::UniqueConnection);
    connect(ui->btnPrint, &QAbstractButton::clicked,
            this, &PricingView::printDoc, Qt::UniqueConnection);
    connect(ui->btnPeriod, &QAbstractButton::clicked,
            this, &PricingView::openCustomPeriod, Qt::UniqueConnection);
}

void PricingView::initBtnFilter()
{
    connect(ui->btnSelectPeriod, &QPushButton::clicked,
            this, &PricingView::openCustomPeriod, Qt::UniqueConnection);
    connect(ui->btnApplyFilter, &QAbstractButton::clicked,
            this, &PricingView::applyFilter, Qt::UniqueConnection);
    connect(ui->btnClearFilter, &QAbstractButton::clicked,
            this, &PricingView::clearFilter, Qt::UniqueConnection);
    connect(ui->btnCloseFilter, &QAbstractButton::clicked,
            this, &PricingView::openFilter, Qt::UniqueConnection);
}

void PricingView::updateTextPeriod()
{
    ui->txtPeriod->setText(
        tr("Perioada: ") +
        ui->dateStart->dateTime().toString("dd.MM.yyyy") +
        " - " + ui->dateEnd->dateTime().toString("dd.MM.yyyy")
        );
}

void PricingView::updateModelOrganizations()
{
    if (modelOrganizations)
        delete modelOrganizations;

    QString str = m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql");
    modelOrganizations = new QueryRolesModel(str, ui->comboOrganizations);
    modelOrganizations->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganizations->setModel(modelOrganizations);
    ui->comboOrganizations->setModelColumn(modelOrganizations->columnIndex("name"));

    if (journalSettings.idOrganization > 0 &&
        ui->comboOrganizations->currentIndex() == 0)
        ui->comboOrganizations->setCurrentIndex(modelOrganizations->rowById("id", journalSettings.idOrganization));
}

void PricingView::updateModelContracts()
{
    if (modelContracts)
        delete modelContracts;

    if (journalSettings.idOrganization <= 0) {
        QString str = m_db.getTextSQL(":/sql/queries/contracts_view.sql");
        modelContracts = new QueryRolesModel(str, ui->comboContracts);
        modelContracts->setEmptyRowEnabled(true);
        ui->comboContracts->setModel(modelContracts);
        ui->comboContracts->setModelColumn(modelContracts->columnIndex("contract_owner"));
    } else {
        QSqlQuery qry;
        qry.prepare(m_db.getTextSQL(
            globals().thisSqlite
                ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
                : ":/sql/queries/contracts_select_by_organization_mysql.sql")
                    );
        qry.addBindValue(journalSettings.idOrganization);
        if (!qry.exec()) {
            qCritical(logCritical())
            << "SQL error:" << qry.lastError().text()
            << "Last query:" << qry.lastQuery();
            return;
        }
        modelContracts = new QueryRolesModel(nullptr, ui->comboContracts);
        modelContracts->setQuery(std::move(qry));
        modelContracts->setEmptyRowEnabled(true);
        ui->comboContracts->setModel(modelContracts);
        ui->comboContracts->setModelColumn(modelContracts->columnIndex("name"));
    }

    if (journalSettings.idContract > 0 &&
        ui->comboContracts->currentIndex() == 0)
        ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", journalSettings.idContract));
}

void PricingView::updateModelUsers()
{
    if (modelUsers)
        delete modelUsers;

    QString str = m_db.getTextSQL(":/sql/queries/users_combo_view.sql");
    modelUsers = new QueryRolesModel(str, ui->comboUsers);
    modelUsers->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboUsers->setModel(modelUsers);
    ui->comboUsers->setModelColumn(modelUsers->columnIndex("name"));

    if (journalSettings.idUser > 0 &&
        ui->comboUsers->currentIndex() == 0)
        ui->comboUsers->setCurrentIndex(modelUsers->rowById("id", journalSettings.idUser));
}

void PricingView::loadDataFilter()
{
    if (!journalSettings.saveFilter)
        return;

    ui->saveFilter->setChecked(journalSettings.saveFilter);

    if (!journalSettings.nrDoc.isEmpty())
        ui->numberDoc->setText(journalSettings.nrDoc);
    if (journalSettings.idOrganization > 0)
        ui->comboOrganizations->setCurrentIndex(modelOrganizations->rowById("id", journalSettings.idOrganization));
    if (journalSettings.idContract > 0)
        ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", journalSettings.idContract));
    if (journalSettings.idUser > 0)
        ui->comboUsers->setCurrentIndex(modelUsers->rowById("id", journalSettings.idUser));
}

void PricingView::loadSizeSection()
{
    auto *header = ui->tabView->horizontalHeader();
    if (!header)
        return;

    ui->tabView->setUpdatesEnabled(false);

    const int colCount = header->count();

    for (int col = 0; col < colCount; ++col) {
        const int width = journalSettings.sectionSizes.value(col, header->defaultSectionSize());
        header->resizeSection(col, width);

        const bool hidden = journalSettings.hiddenSections.value(col, false);
        ui->tabView->setColumnHidden(col, hidden);
    }

    const int sortSection = journalSettings.sortSection;
    const Qt::SortOrder sortOrder = journalSettings.sortOrder;

    if (sortSection >= 0 && sortSection < colCount) {
        header->setSortIndicator(sortSection, sortOrder);
        ui->tabView->sortByColumn(sortSection, sortOrder);
    }

    if (ui->tabView->model() && ui->tabView->model()->rowCount() > 0)
        ui->tabView->selectRow(0);

    ui->tabView->setUpdatesEnabled(true);
}

void PricingView::initTableView()
{
    model = new PricingModel(this);
    proxy = new PricingSortModel(this);
}

void PricingView::updateTableView()
{
    // pregatim solicitarea
    QString sql = m_db.getTextSQL(":/sql/queries_doc/pricings_journal.sql");

    // nr.doc
    sql.replace("%nr_doc%",
                ui->numberDoc->text().trimmed().isEmpty()
                    ? ""
                    : " AND p.numberDoc = ? ");

    // organizatia
    sql.replace("%id_org%",
                journalSettings.idOrganization <= 0
                    ? ""
                    : " AND p.id_organizations = ? ");

    // contract
    sql.replace("%id_cont%",
                journalSettings.idContract <= 0
                    ? ""
                    : " AND p.id_contracts = ? ");

    // autor
    sql.replace("%id_us%",
                journalSettings.idUser <= 0
                    ? ""
                    : " AND p.id_users = ? ");

    // executam solicitarea
    QSqlQuery qry;
    qry.prepare(sql);
    if (!ui->numberDoc->text().trimmed().isEmpty())
        qry.addBindValue(ui->numberDoc->text().trimmed());

    if (journalSettings.idOrganization > 0)
        qry.addBindValue(journalSettings.idOrganization);

    if (journalSettings.idContract > 0)
        qry.addBindValue(journalSettings.idContract);

    if (journalSettings.idUser > 0)
        qry.addBindValue(journalSettings.idUser);

    // obligatorii
    qry.addBindValue(ui->dateStart->dateTime()
                         .toString("yyyy-MM-dd hh:mm:ss"));
    qry.addBindValue(ui->dateEnd->dateTime()
                         .toString("yyyy-MM-dd hh:mm:ss"));

    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
        << "SQL error:" << qry.lastError().text()
        << "\nQuery:\n" << qry.lastQuery();
    }

    QVector<PricingsJournal::Item> items;

    while (qry.next()) {

        PricingsJournal::Item item;
        item.id              = qry.value(PricingsJournal::Id).toInt();
        item.deletionMark    = qry.value(PricingsJournal::DeletionMark).toInt();
        item.numberDoc       = qry.value(PricingsJournal::NumberDoc).toString();
        item.dateDoc         = qry.value(PricingsJournal::DateDoc).toDateTime();
        item.dateDocText     = item.dateDoc.toString("dd.MM.yyyy hh:mm:ss"); // pu afisare rapida
        item.idOrganizations = qry.value(PricingsJournal::Id_Organizations).toInt();
        item.idContracts     = qry.value(PricingsJournal::Id_Contracts).toInt();
        item.idTypesPrices   = qry.value(PricingsJournal::Id_TypesPrices).toInt();
        item.idUsers         = qry.value(PricingsJournal::Id_Users).toInt();

        item.organizationName= qry.value(PricingsJournal::OrganizationName).toString();
        item.contractName    = qry.value(PricingsJournal::ContractName).toString();
        item.typePriceName   = qry.value(PricingsJournal::TypePriceName).toString();
        item.userName        = qry.value(PricingsJournal::UserName).toString();

        item.comment         = qry.value(PricingsJournal::Comment).toString();
        item.uuid            = qry.value(PricingsJournal::Uuid).toByteArray();

        items.append(item);
    }

    model->setItems(items);
    proxy->setSourceModel(model);
    proxy->setSortRole(PricingModel::SortRole);
    ui->tabView->setModel(proxy);

    // ascundem sectiile
    ui->tabView->hideColumn(PricingsJournal::Id);
    ui->tabView->hideColumn(PricingsJournal::Id_Organizations);
    ui->tabView->hideColumn(PricingsJournal::Id_Contracts);
    ui->tabView->hideColumn(PricingsJournal::Id_TypesPrices);
    ui->tabView->hideColumn(PricingsJournal::Id_Users);
    ui->tabView->hideColumn(PricingsJournal::Uuid);

    // pozitionam delegatul pu centrarea imaginei
    ui->tabView->setItemDelegateForColumn(PricingsJournal::DeletionMark, new CenterIconDelegate(ui->tabView));

    // setarile pu tabView
    ui->tabView->setSortingEnabled(true);
    ui->tabView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tabView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tabView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tabView->horizontalHeader()->setStretchLastSection(true);
    ui->tabView->setContextMenuPolicy(Qt::CustomContextMenu);     // initializam meniu contextual

    // setam cursorul
    if (m_currentRow != -1)
        ui->tabView->selectRow(m_currentRow);
    else
        ui->tabView->selectRow(0);
}

void PricingView::initConnections()
{
    QList<QDateTimeEdit*> dts = findChildren<QDateTimeEdit*>();
    for (QDateTimeEdit *dt : std::as_const(dts))
        connect(dt, &QDateTimeEdit::dateTimeChanged,
                this, &PricingView::validatePeriodAndUpdate, Qt::UniqueConnection);

    QList<QComboBox*> combos = findChildren<QComboBox*>();
    for (QComboBox *combo : std::as_const(combos)) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &PricingView::indexChangedCombo, Qt::UniqueConnection);
    }

    connect(ui->tabView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &PricingView::onDoubleClickedTable, Qt::UniqueConnection);
    connect(ui->tabView, &QWidget::customContextMenuRequested,
            this, &PricingView::slotContextMenuRequested, Qt::UniqueConnection);
}

void PricingView::openCustomPeriod()
{
    CustomPeriod dlg(this);

    dlg.setDateStart(ui->dateStart->date());
    dlg.setDateEnd(ui->dateEnd->date());

    if (dlg.exec() == QDialog::Accepted) {
        ui->dateStart->setDateTime(dlg.getDateStart());
        ui->dateEnd->setDateTime(dlg.getDateEnd());

        updateTextPeriod();
        updateTableView();
    }
}

void PricingView::applyFilter()
{
    /** combo sunt introduse deja
     *  la modificarea indexului */

    // doar necesar
    journalSettings.nrDoc      = ui->numberDoc->text();
    journalSettings.saveFilter = ui->saveFilter->isChecked();

    // acutualizam tabela
    updateTableView();
    openFilter();
}

void PricingView::clearFilter()
{
    ui->numberDoc->setText("");
    ui->comboOrganizations->setCurrentIndex(0);
    ui->comboContracts->setCurrentIndex(0);
    ui->comboUsers->setCurrentIndex(0);
    ui->saveFilter->setChecked(false);

    journalSettings.nrDoc = "";
    journalSettings.idOrganization = 0;
    journalSettings.idContract = 0;
    journalSettings.idUser = 0;
    journalSettings.saveFilter = false;

    updateTableView();
    openFilter();
}

void PricingView::openFilter()
{
    ui->groupBoxFilter->setHidden(ui->groupBoxFilter->isVisible());
}

void PricingView::setFilterByIDOrganization()
{
    const QModelIndex proxyIndex = ui->tabView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return;
    }

    const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    const PricingsJournal::Item &item = model->itemAt(sourceIndex.row());

    ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", item.idOrganizations));
    updateTableView();
    // prezentam mesaj
    popUp->setPopupText(tr("Instalat filtru după organizatia<br><b>%1</b>")
                            .arg(item.organizationName));
    popUp->show();
}

void PricingView::indexChangedCombo(int index)
{
    Q_UNUSED(index);

    QComboBox *combo = qobject_cast<QComboBox*>(sender());

    if (combo == ui->comboOrganizations) {
        // determinam rolul
        auto roleID            = modelOrganizations->roleForColumn("id");
        auto role_id_contract  = modelOrganizations->roleForColumn("id_contracts");
        // variabile
        const int id_organization = ui->comboOrganizations->currentData(roleID).toInt();
        const int id_contracts    = ui->comboOrganizations->currentData(role_id_contract).toInt();
        // completam structura setarilor jurnalului
        if (id_organization > 0)
            journalSettings.idOrganization = id_organization;
        if (id_contracts > 0) {
            journalSettings.idContract = id_contracts;
            ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", id_contracts));
        }

    } else if (combo == ui->comboContracts) {
        // determinam rolul
        auto roleID = modelContracts->roleForColumn("id");
        // ID necesare
        const int id_contracts = ui->comboContracts->currentData(roleID).toInt();
        // completam structura setarilor jurnalului
        if (id_contracts > 0)
            journalSettings.idContract = id_contracts;

    } else if (combo == ui->comboUsers) {
        // determinam rolul
        auto roleID = modelUsers->roleForColumn("id");
        // ID necesare
        const int id_user = ui->comboUsers->currentData(roleID).toInt();
        // completam structura setarilor jurnalului
        if (id_user > 0)
            journalSettings.idUser = id_user;

    }
}

void PricingView::createNewDoc()
{
    PricingDialog *pricing = new PricingDialog(m_db, this);
    pricing->setAttribute(Qt::WA_DeleteOnClose);
    pricing->setProperty("isNew", true);

    connect(pricing, &PricingDialog::PostDocument,
            this, &PricingView::updatePostDocs, Qt::UniqueConnection);

    pricing->show();
}

void PricingView::editDoc()
{
    const QModelIndex proxyIndex = ui->tabView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return;
    }

    const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    const PricingsJournal::Item &item = model->itemAt(sourceIndex.row());

    PricingDialog *pricing = new PricingDialog(m_db, this);
    pricing->setAttribute(Qt::WA_DeleteOnClose);
    pricing->setProperty("isNew", false);
    pricing->setProperty("id", item.id);

    connect(pricing, &PricingDialog::mCloseThisForm,
            this, &PricingView::updatePostDocs);

    pricing->show();
}

void PricingView::deleteDoc()
{
    const QModelIndex proxyIndex = ui->tabView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return;
    }

    const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    const PricingsJournal::Item &item = model->itemAt(sourceIndex.row());

    QString err;
    if (! m_db.deleteDocByID("pricings", item.id)) {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Eliminarea documentului nu s-a efectuat."));
        msg->setDetailedText(err);
        msg->exec();
        msg->deleteLater();
        return;
    }
    popUp->setPopupText(tr("Documentul nr.%1 din data %2<br>"
                           "a fost eliminat cu succes din baza de date.")
                            .arg(item.numberDoc, item.dateDocText));
    popUp->show();
    updateTableView();
}

void PricingView::printDoc()
{
    const QModelIndex proxyIndex = ui->tabView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return;
    }

    const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
    const PricingsJournal::Item &item = model->itemAt(sourceIndex.row());

    PricingDialog *pricing = new PricingDialog(m_db, this);
    pricing->setAttribute(Qt::WA_DeleteOnClose);
    pricing->setProperty("isNew", false);
    pricing->setProperty("id", item.id);
    pricing->onPrintDocument(PrintType::Preview);
    pricing->deleteLater();
}

void PricingView::onDoubleClickedTable(const QModelIndex &index)
{
    Q_UNUSED(index)
    editDoc();
}

void PricingView::validatePeriodAndUpdate()
{
    const QDateTime startDate = ui->dateStart->dateTime();
    const QDateTime endDate   = ui->dateEnd->dateTime();

    if (startDate > endDate) {
        QMessageBox::warning(this,
                             tr("Verificarea perioadei"),
                             tr("Data de sfârșit nu poate fi mai mică decât data de început."),
                             QMessageBox::Ok);

        QSignalBlocker blockerStart(ui->dateStart);
        QSignalBlocker blockerEnd(ui->dateEnd);
        ui->dateStart->setDateTime(journalSettings.startDate);
        ui->dateEnd->setDateTime(journalSettings.endDate);
        return;
    }

    journalSettings.startDate = startDate;
    journalSettings.endDate = endDate;

    updateTextPeriod();
    updateTableView();
}

void PricingView::slotContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = ui->tabView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);

    QAction *actionNewDoc = menu.addAction(QIcon(":/img/toolBar/add.png"),
                                           tr("Creează document nou."));
    QAction *actionEditDoc = menu.addAction(QIcon(":/img/toolBar/edit.png"),
                                            tr("Editează documentul."));
    QAction *actionDeleteDoc = menu.addAction(QIcon(":/img/toolBar/delete.png"),
                                              tr("Elimină documentul"));
    menu.addSeparator();
    QAction *actionPrintDoc = menu.addAction(QIcon(":/img/actions/print.png"),
                                             tr("Printează documentul"));

    QAction *selectedAction = menu.exec(ui->tabView->viewport()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == actionNewDoc)
        createNewDoc();
    else if (selectedAction == actionEditDoc)
        editDoc();
    else if (selectedAction == actionDeleteDoc)
        deleteDoc();
    else if (selectedAction == actionPrintDoc)
        printDoc();
}

void PricingView::updatePostDocs()
{
    updateTableView();
}

void PricingView::saveSettingsJournal()
{
    // perioada
    m_settings.setValue(type_doc, "startDate", ui->dateStart->dateTime());
    m_settings.setValue(type_doc, "endDate",   ui->dateEnd->dateTime());

    // filtru
    m_settings.setValue(type_doc, "filter/saveFilter", ui->saveFilter->isChecked());
    m_settings.setValue(type_doc, "filter/id_organization", journalSettings.idOrganization);
    m_settings.setValue(type_doc, "filter/id_contract", journalSettings.idContract);
    m_settings.setValue(type_doc, "filter/id_user", journalSettings.idUser);
    m_settings.setValue(type_doc, "filter/nr_doc", (ui->numberDoc->text().trimmed().isEmpty())
                                                       ? QVariant()
                                                       : ui->numberDoc->text());

    for (int numSection = 0; numSection < ui->tabView->horizontalHeader()->count(); ++numSection) {

        // size sections
        int w = ui->tabView->horizontalHeader()->sectionSize(numSection);
        m_settings.setValue(type_doc, QString("sections/%1").arg(numSection), w);

        // sortarea
        m_settings.setValue(type_doc, "sort/section",
                            ui->tabView->horizontalHeader()->sortIndicatorSection());

        m_settings.setValue(type_doc, "sort/direction",
                            static_cast<int>(ui->tabView->horizontalHeader()->sortIndicatorOrder()));

        // show/hide section
        m_settings.setValue(type_doc,
                            QString("hide_show_sections/%1").arg(numSection),
                            ui->tabView->horizontalHeader()->isSectionHidden(numSection) ? 1 : 0);
    }

    m_settings.save();
}

void PricingView::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSettingsJournal();
    }
}

void PricingView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Lista documentelor: Formarea preturilor"));
    }
}

bool PricingView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnAdd){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnAdd->pos().x() - 20, ui->btnAdd->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Adaug\304\203 (ins.)"));  // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnEdit){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnEdit->pos().x() - 20, ui->btnEdit->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Editeaz\304\203 (F2)")); // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnDeletion){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnDeletion->pos().x() - 46, ui->btnDeletion->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Eliminare (Del)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnAddFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnAddFilter->pos().x() - 50, ui->btnAddFilter->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Adaug\304\203 filtru (Ctrl + F1)"));  // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnSetFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnSetFilter->pos().x() - 56, ui->btnSetFilter->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Filtrare dup\304\203<br>%1 (Ctrl + F2)")
                                    .arg(proxy->data(proxy->index(ui->tabView->currentIndex().row(), PricingsJournal::OrganizationName), Qt::DisplayRole).toString())); // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnDeleteFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnDeleteFilter->pos().x() - 50, ui->btnDeleteFilter->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Gole\310\231te filtru (Ctrl + F3)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnUpdateTable){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnUpdateTable->pos().x() - 60, ui->btnUpdateTable->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Actualizeaz\304\203 tabela (F5)"));  // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnPrint){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnPrint->pos().x() - 30, ui->btnPrint->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Printare (Ctrl + P)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnPeriod){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnPeriod->pos().x() - 60, ui->btnPeriod->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Perioada \310\231i filtru (Ctrl + Shift + P)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    }

    return false;
}

void PricingView::keyReleaseEvent(QKeyEvent *event)
{
    if (proxy->rowCount() == 0)
        return;

    if (event->key() == Qt::Key_End)
        ui->tabView->selectRow(proxy->rowCount() - 1);
    else if (event->key() == Qt::Key_Home)
        ui->tabView->selectRow(0);
}
