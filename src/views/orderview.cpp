#include "orderview.h"
#include "ui_orderview.h"

OrderView::OrderView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OrderView)
    , m_settings(globals().pathSettingsCommon)
    , m_db(db)
    , m_dbProvider(this)
    , popUp(new PopUp(this))
    , menuSetFilter(new QMenu(this))
    , toolBar(new ToolBarCustom(this,
                                 ToolBarCustom::AddEditDelete |
                                 ToolBarCustom::Filter |
                                 ToolBarCustom::UpdateColumn |
                                 ToolBarCustom::PrintEmail |
                                 ToolBarCustom::ReportViewTab |
                                 ToolBarCustom::PeriodSearch))
    , modelViewOrder(new QSqlQueryModel(this))
    , modelViewReport(new QSqlQueryModel(this))
    , styleBtnMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    setWindowTitle(tr("Lista documentelor: Comanda ecografică"));

    loadFilterJournalBySettings();

    updateModelOrganizations();
    updateModelContracts();
    updateModelUsers();

    initTableView();
    loadFilterData();
    updateTableView();

    initToolBar();
    initBtnFilter();

    initBoxFilterAndTableFooter();

    ui->btnOpenOrder->setStyleSheet(
        globals().isSystemThemeDark
            ? "color: #fff;"
            : "color: #000;"
        );

    ui->btnOpenReport->setStyleSheet(
        globals().isSystemThemeDark
            ? "color: #fff;"
            : "color: #000;"
        );
}

OrderView::~OrderView()
{
    delete ui;
}

void OrderView::onScroll(int value)
{
    auto *sb = ui->tableView->verticalScrollBar();
    if (!sb)
        return;

    // când ajunge aproape de final
    if (value < sb->maximum() - 8)
        return;

    if (!proxyTable || !modelTable)
        return;

    if (modelTable->canFetchMore())
        modelTable->fetchMore();
}

// **********************************************************************************
// --- procesarea butoanelor toolBar-lui

void OrderView::onAddDoc()
{
    OrderDialog *doc = new OrderDialog(m_db, this);
    doc->setAttribute(Qt::WA_DeleteOnClose);
    doc->setProperty("isNew", true);
    connect(doc, &OrderDialog::PostDocument,
            this, &OrderView::updateTableView, Qt::UniqueConnection);
    doc->show();
}

void OrderView::onEditDoc()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());

    OrderDialog *doc = new OrderDialog(m_db, this);
    doc->setAttribute(Qt::WA_DeleteOnClose);
    doc->setProperty("isNew", false);
    doc->setProperty("id", item.id);
    connect(doc, &OrderDialog::PostDocument,
            this, &OrderView::updateTableView, Qt::UniqueConnection);
    doc->show();
}

void OrderView::onDeleteDoc()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    const int id_order = item.id;

    QSqlQuery qCheck;
    qCheck.prepare(
        globals().thisMySQL
        ? R"(
            SELECT
                CONCAT('Raport ecografic nr.', r.numberDoc , ' din ' ,
                DATE_FORMAT(r.dateDoc, '%d.%m.%Y %H:%i:%s')) AS report
            FROM reportEcho r
            WHERE r.id_orderEcho = :id_orderEcho)"
        : R"(
            SELECT
                'Raport ecografic nr.' || r.numberDoc || ' din ' ||
                strftime('%d.%m.%Y %H:%M:%S', r.dateDoc) AS report
            FROM reportEcho r
            WHERE r.id_orderEcho = :id_orderEcho)"
    );
    qCheck.bindValue(":id_orderEcho", id_order);

    if (!qCheck.exec()) {
        qCritical(logCritical()).noquote()
        << "SQL error:" << qCheck.lastError().text()
        << "\nLast query:" << qCheck.lastQuery();
        return;
    }

    const bool exist = qCheck.next(); // doar un document

    if (exist) {
        QMessageBox messageBox(
            QMessageBox::Question,
            tr("Eliminarea documentului."),
            tr("Există documente subordonate care vor fi eliminate.<br>Doriți să continuați?"),
            QMessageBox::NoButton,
            this
            );
        messageBox.setDetailedText(tr("Va fi eliminat documentul subordonat:\n%1")
                                       .arg(qCheck.value("report").toString()));
        QPushButton *yesButton    = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messageBox.addButton(tr("Anulare"), QMessageBox::RejectRole);

        yesButton->setStyleSheet(styleBtnMessageBox);
        noButton->setStyleSheet(styleBtnMessageBox);
        cancelButton->setStyleSheet(styleBtnMessageBox);

        messageBox.exec();

        if (messageBox.clickedButton() != yesButton)
            return;
    }

    QSqlQuery qDelete;
    qDelete.prepare("DELETE FROM orderEcho WHERE id = :id");
    qDelete.bindValue(":id", id_order);

    if (qDelete.exec()) {
        popUp->setPopupText(tr("Documentul este eliminat<br> cu succes din baza de date."));
        popUp->show();

        qInfo(logInfo())
            << QString("Eliminat documentul 'Comanda ecografica nr.%1' cu ID='%2' din baza de date.")
                   .arg(item.numberDoc.trimmed())
                   .arg(item.id);

        updateTableView();
    } else {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(tr("Eliminarea documentului"));
        msg->setTextTitle(tr("Nu s-a putut de eliminat documentul nr.%1 din baza de date")
                              .arg(item.numberDoc.trimmed()));
        msg->setDetailedText("SQL error: " + qDelete.lastError().text() +
                             "\nLast query: " + qDelete.lastQuery());
        msg->exec();
    }
}

void OrderView::onAddFilter()
{
    ui->groupBoxFilter->setHidden(!ui->groupBoxFilter->isHidden());
}

void OrderView::onSetFilter()
{
    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!isValidIndex(proxyIndex))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(proxyIndex);
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());

    menuSetFilter->clear();

    QAction *filterOrganization = menuSetFilter->addAction(
        QIcon(":/img/catalogs/company.png"),
        tr("... filtru după -> %1").arg(item.organizationName)
        );

    QAction *filterPacient = menuSetFilter->addAction(
        QIcon(":/img/catalogs/pacient.png"),
        tr("... filtru după -> %1").arg(item.patientName)
        );

    QAction *filterAuthor = menuSetFilter->addAction(
        QIcon(":/img/catalogs/user.png"),
        tr("... filtru după -> %1").arg(item.userName)
        );

    const QPoint pos = toolBar->getBtnSetFilter()->mapToGlobal(
        QPoint(0, toolBar->getBtnSetFilter()->height())
        );

    QAction *selectedAction = menuSetFilter->exec(pos);
    if (!selectedAction)
        return;

    if (selectedAction == filterOrganization) {
        ui->comboOrganizations->setCurrentIndex(modelOrganizations->rowById("id", item.idOrganizations));
        m_filter.idOrganization = item.idOrganizations;

    } else if (selectedAction == filterPacient) {
        m_filter.patientId = item.patientId;
        m_filter.patientName = item.patientName;

    } else if (selectedAction == filterAuthor) {
        ui->comboUsers->setCurrentIndex(modelUsers->rowById("id", item.idUsers));
        m_filter.idUser = item.idUsers;

    } else {
        return;
    }

    updateTableView();
    updateTextPeriod();
}

void OrderView::onDeleteFilter()
{
    clearFilter();
}

void OrderView::onUpdateTableView()
{
    updateTableView();
}

void OrderView::onHideShowColumn()
{
    if (!m_columnsController)
        return;

    auto btn = toolBar->getBtnHideShowColumn();
    QPoint p = QPoint(0, btn->height());
    const QPoint globalPos = btn->mapToGlobal(p);
    m_columnsController->showMenu(globalPos);
}

void OrderView::onPrintDoc()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());

    OrderDialog *doc = new OrderDialog(m_db, this);
    doc->setAttribute(Qt::WA_DeleteOnClose);
    doc->setProperty("isNew", false);
    doc->setProperty("id", item.id);
    doc->onPrintDocument(PrintType::Preview);
    doc->close();
}

void OrderView::onSendEmail()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    if (!sourceIndex.isValid())
        return;

    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());

    QSqlQuery reportCheck(m_db.getDatabase());
    reportCheck.prepare(QStringLiteral(
        "SELECT id FROM reportEcho "
        "WHERE id_orderEcho = :id_orderEcho AND deletionMark = 2 LIMIT 1"));
    reportCheck.bindValue(":id_orderEcho", item.id);
    if (!reportCheck.exec() || !reportCheck.next()) {
        QMessageBox::warning(this,
                             tr("Transmiterea prin e-mail"),
                             tr("Comanda selectată nu are un raport ecografic validat."),
                             QMessageBox::Ok);
        return;
    }

    const QString exportDirectory = globals().main_path_save_documents;
    if (!QDir().mkpath(exportDirectory)) {
        QMessageBox::warning(this,
                             tr("Transmiterea prin e-mail"),
                             tr("Nu poate fi creat directorul temporar pentru export:\n%1")
                                 .arg(QDir::toNativeSeparators(exportDirectory)),
                             QMessageBox::Ok);
        return;
    }

    if (loader)
        loader->close();
    loader = new ProcessingAction(this);
    loader->setAttribute(Qt::WA_DeleteOnClose);
    loader->setProperty("txtInfo", tr("Se pregătesc documentele în format PDF ..."));
    connect(loader, &QObject::destroyed, this, [this]() { loader = nullptr; });
    loader->show();

    DatesDocForExportEmail data{};
    data.id_user                      = globals().idUserApp;
    data.thisMySQL                    = globals().thisMySQL;
    data.id_order                     = item.id;
    data.id_report                    = -1;
    data.id_patient                   = item.patientId;
    data.unitMeasure                  = globals().unitMeasure;
    data.logo_byteArray               = globals().c_logo_byteArray;
    data.stamp_organization_byteArray = globals().main_stamp_organization;
    data.stamp_doctor_byteArray       = globals().stamp_main_doctor;
    data.signature_doctor_byteArray   = globals().signature_main_doctor;
    data.pathTemplatesDocs            = globals().pathTemplatesDocs;
    data.filePDF                      = exportDirectory;

    // LimeReport utilizeaza un ScriptEngineManager global bazat pe QJSEngine.
    // Crearea/distrugerea motoarelor de raport in thread-uri diferite corupe
    // starea singletonului si provoaca abort la inchiderea aplicatiei.
    // Exportul LimeReport trebuie executat in thread-ul GUI, unde este folosit
    // si in restul aplicatiei.
    auto *worker = new DocEmailExporterWorker(&m_dbProvider, data, this);

    connect(worker, &DocEmailExporterWorker::setTextInfo,
            this, &OrderView::updateEmailExportProgress, Qt::QueuedConnection);
    connect(worker, &DocEmailExporterWorker::finished,
            this, &OrderView::launchEmailAgent, Qt::QueuedConnection);
    connect(worker, &DocEmailExporterWorker::finished,
            worker, &QObject::deleteLater);

    QTimer::singleShot(0, worker, &DocEmailExporterWorker::process);
}

void OrderView::updateEmailExportProgress(const QString &text)
{
    if (loader)
        loader->setProperty("txtInfo", text);
}

void OrderView::launchEmailAgent(const QVector<DatesForAgentEmail> &exportedData)
{
    if (loader)
        loader->close();

    if (exportedData.isEmpty()
        || exportedData.constFirst().nr_order.trimmed().isEmpty()) {
        QMessageBox::critical(this,
                              tr("Transmiterea prin e-mail"),
                              tr("Exportul documentelor nu s-a finalizat. Verificați jurnalul aplicației."),
                              QMessageBox::Ok);
        return;
    }

    const DatesForAgentEmail &data = exportedData.constFirst();
    AgentSendEmail::MailContext context;
    context.thisReports       = false;
    context.nrOrder           = data.nr_order;
    context.nrReport          = data.nr_report;
    context.emailFrom         = globals().main_email_organization;
    context.emailTo           = data.emailTo;
    context.namePatient       = data.name_patient;
    context.nameDoctor        = data.name_doctor_execute;
    context.dateInvestigation = QDate::fromString(
        data.str_dateInvestigation.left(10), Qt::ISODate
    );
    if (!context.dateInvestigation.isValid()) {
        context.dateInvestigation = QDate::fromString(
            data.str_dateInvestigation.left(10),
            QStringLiteral("dd.MM.yyyy")
        );
    }

    auto *agent = new AgentSendEmail(m_db, this);
    agent->setAttribute(Qt::WA_DeleteOnClose);
    agent->setContext(context);
    agent->show();
}

void OrderView::onCreateReport()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex  = proxyTable->mapToSource(idx);
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());

    const int id_order   = item.id;
    const int id_patient = item.patientId;
    const QString orderDisplayText = QStringLiteral("Comanda ecografică nr.%1 din %2")
                                         .arg(item.numberDoc.trimmed(), item.dateDocText);

    ReportDialog::ReportDialogParameters params;

    // determinam daca este salvat document 'Report', deschidem document
    QSqlQuery q;
    q.prepare("SELECT id, deletionMark FROM reportEcho WHERE id_orderEcho = :id_orderEcho");
    q.bindValue(":id_orderEcho", id_order);
    if(q.exec() && q.next()) {
        // parametrii
        params.isNew       = false;
        params.id          = q.value("id").toInt();
        params.idOrder     = id_order;
        params.idPatient   = id_patient;
        params.status      = DocStatus::determineStatusDoc(q.value("deletionMark").toInt());
        params.orderDisplayText = orderDisplayText;
        // deschidem 'ReportDialog'
        auto *report = new ReportDialog(m_db, params, this);
        report->setAttribute(Qt::WA_DeleteOnClose);
        connect(report, &ReportDialog::reportChanged,
                this, &OrderView::updateTableView);
        connect(report, &ReportDialog::reportPost,
                this, &OrderView::updateTableView);
        report->show();

        return;
    }

    // determinam investigatii din 'Order'
    QStringList codes;
    q.prepare(R"(SELECT cod FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho)");
    q.bindValue(":id_orderEcho", id_order);
    if (q.exec()) {
        while (q.next())
            codes << q.value("cod").toString();
    }

    CustomDialogInvestig dlg(this);
    dlg.setCodes(codes);

    if (dlg.exec() != QDialog::Accepted)
        return;

    params.isNew            = true;
    params.idPatient        = id_patient;
    params.idOrder          = id_order;
    params.status           = DocStatus::determineStatusDoc(item.deletionMark);
    params.systems          = dlg.selectedSystems();
    params.orderDisplayText = orderDisplayText;

    auto *report = new ReportDialog(m_db, params, this);
    report->setAttribute(Qt::WA_DeleteOnClose);
    connect(report, &ReportDialog::reportCreated,
            this, &OrderView::updateTableView);
    connect(report, &ReportDialog::reportChanged,
            this, &OrderView::updateTableView);
    connect(report, &ReportDialog::reportPost, this,
            [this](){
                updateTableView();
                popUp->setPopupText(tr("Documentul a fost salvat cu succes<br> in baza de date."));
                popUp->show();
            });
    report->show();

}

void OrderView::onViewTabOrder()
{
    m_viewTabVisible = !m_viewTabVisible;
    ui->groupBox_table_order->setVisible(m_viewTabVisible);
    ui->groupBox_table_report->setVisible(m_viewTabVisible);

    if (m_viewTabVisible) {

        connect(ui->btnOpenOrder, &QAbstractButton::clicked,
                this, &OrderView::openPreviewOrder, Qt::UniqueConnection);

        connect(ui->btnOpenReport, &QAbstractButton::clicked,
                this, &OrderView::openPreviewReport, Qt::UniqueConnection);

        updatePrintButtons(globals().showDesignerMenuPrint);

    } else {

        disconnect(ui->btnOpenOrder, &QAbstractButton::clicked,
                   this, &OrderView::openPreviewOrder);
        disconnect(ui->btnOpenReport, &QAbstractButton::clicked,
                   this, &OrderView::openPreviewReport);
    }

    if (!m_viewTabVisible)
        return;

    updateDocumentPreview();

    const int totalHeight = qMax(ui->splitter_3->height(), 1);
    ui->splitter_3->setSizes({qMax(totalHeight * 2 / 3, 200),
                              qMax(totalHeight / 3, 160)});
}

void OrderView::updateDocumentPreview()
{
    if (!m_viewTabVisible || !modelTable || !proxyTable)
        return;

    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        modelViewOrder->clear();
        modelViewReport->clear();
        ui->tableReport->hide();
        ui->btnOpenReport->hide();
        ui->btnPrintReport->hide();
        ui->text_empty_report->show();
        return;
    }

    const QModelIndex sourceIndex = proxyTable->mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return;

    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    if (item.id <= 0)
        return;

    QSqlQuery orderQuery(m_db.getDatabase());
    orderQuery.prepare(QStringLiteral(R"(
        SELECT
            cod AS 'Cod',
            name AS 'Investigația',
            %1
        FROM
            orderEchoTable
        WHERE
            id_orderEcho = :id_orderEcho AND
            deletionMark = 2
        ORDER BY id
    )").arg(
            globals().thisMySQL
            ? "FORMAT(price, 2) AS 'Preț'"
            : "printf('%.2f', price) AS 'Preț'")
    );
    orderQuery.bindValue(":id_orderEcho", item.id);
    if (!orderQuery.exec()) {
        qWarning(logWarning()).noquote()
            << "OrderView preview order error:" << orderQuery.lastError().text()
            << "\nLast query:" << orderQuery.lastQuery();
        modelViewOrder->clear();
    } else {
        modelViewOrder->setQuery(std::move(orderQuery));
    }

    ui->tableOrder->setModel(modelViewOrder);
    ui->tableOrder->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableOrder->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableOrder->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QSqlQuery reportQuery(m_db.getDatabase());
    reportQuery.prepare(QStringLiteral(R"(
        SELECT
            id,
            concluzion AS 'Concluzie'
        FROM
            reportEcho
        WHERE
            id_orderEcho = :id_orderEcho AND
            deletionMark = 2
        ORDER BY
            id DESC LIMIT 1
    )"));
    reportQuery.bindValue(":id_orderEcho", item.id);
    if (!reportQuery.exec()) {
        qWarning(logWarning()).noquote()
            << "OrderView preview report error:" << reportQuery.lastError().text()
            << "\nLast query:" << reportQuery.lastQuery();
        modelViewReport->clear();
    } else {
        modelViewReport->setQuery(std::move(reportQuery));
    }

    const bool hasReport = modelViewReport->rowCount() > 0;
    ui->tableReport->setVisible(hasReport);
    ui->btnOpenReport->setVisible(hasReport);
    ui->btnPrintReport->setVisible(hasReport);
    ui->text_empty_report->setVisible(!hasReport);

    if (hasReport) {
        ui->tableReport->setModel(modelViewReport);
        ui->tableReport->hideColumn(0);
        ui->tableReport->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableReport->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableReport->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableReport->horizontalHeader()->setStretchLastSection(true);
    }
}

void OrderView::onOpenPeriod()
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

void OrderView::onSearchPacients()
{

}

void OrderView::openPreviewOrder()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;
    onDoubleClickedTableView(idx);
}

void OrderView::printPreviewOrder()
{
    printOrder(PrintType::Preview);
}

void OrderView::openPreviewReport()
{
    onCreateReport();
}

void OrderView::printPreviewReport()
{
    printReport(PrintType::Preview);
}

void OrderView::updatePrintButtons(bool showDesignerMenuPrint)
{
    if (!m_viewTabVisible
        || !ui->groupBox_table_order->isVisible()
        || !ui->groupBox_table_report->isVisible())
        return;

    globals().showDesignerMenuPrint = showDesignerMenuPrint;

    configurePrintButton(ui->btnPrintOrder, [this](PrintType::Column typePrint) {
        printOrder(typePrint);
    });
    configurePrintButton(ui->btnPrintReport, [this](PrintType::Column typePrint) {
        printReport(typePrint);
    });
}

void OrderView::configurePrintButton(
    QPushButton *button,
    const std::function<void(PrintType::Column)> &printAction)
{
    if (!button)
        return;

    disconnect(button, &QAbstractButton::clicked, nullptr, nullptr);

    if (QMenu *oldMenu = button->menu()) {
        button->setMenu(nullptr);
        oldMenu->deleteLater();
    }

    QString buttonStyle = m_db.getStyleForButtonMessageBox();
    buttonStyle += QStringLiteral(R"(
        QPushButton { padding-right: 10px; }
        QPushButton::menu-indicator {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            right: 7px;
        }
    )");
    button->setStyleSheet(buttonStyle);

    if (!globals().showDesignerMenuPrint) {
        connect(button, &QAbstractButton::clicked, this,
                [printAction]() { printAction(PrintType::Preview); });
        return;
    }

    auto *menu = new QMenu(button);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->setStyleSheet(globals().isSystemThemeDark
        ? QStringLiteral(R"(
            QMenu { background:#2b2b2b; color:#fff; border:1px solid #555;
                    border-radius:6px; padding:4px; font-size:13px; }
            QMenu::item { min-width:150px; padding:6px 18px 6px 8px; border-radius:4px; }
            QMenu::item:selected { background:#0078d7; color:#fff; }
        )")
        : QStringLiteral(R"(
            QMenu { background:#fff; color:#000; border:1px solid #b8b8b8;
                    border-radius:6px; padding:4px; font-size:13px; }
            QMenu::item { min-width:150px; padding:6px 18px 6px 8px; border-radius:4px; }
            QMenu::item:selected { background:#0078d7; color:#fff; }
        )"));

    QAction *previewAction = menu->addAction(QIcon(":/img/actions/print.png"),
                                              tr("Deschide preview"));
    QAction *designerAction = menu->addAction(QIcon(":/images/design.png"),
                                               tr("Deschide designer"));
    connect(previewAction, &QAction::triggered, this,
            [printAction]() { printAction(PrintType::Preview); });
    connect(designerAction, &QAction::triggered, this,
            [printAction]() { printAction(PrintType::Designer); });
    button->setMenu(menu);
}

void OrderView::printOrder(PrintType::Column typePrint)
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    if (!sourceIndex.isValid())
        return;

    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    OrderDialog document(m_db, this);
    document.setProperty("isNew", false);
    document.setProperty("id", item.id);
    document.onPrintDocument(typePrint);
}

void OrderView::printReport(PrintType::Column typePrint)
{
    if (!modelViewReport || modelViewReport->rowCount() <= 0)
        return;

    const int reportId = modelViewReport->index(0, 0).data().toInt();
    if (reportId <= 0)
        return;

    ReportDialog::ReportDialogParameters params;
    params.isNew = false;
    params.id = reportId;
    params.status = DocStatus::Post;

    const QModelIndex idx = ui->tableView->currentIndex();
    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    if (sourceIndex.isValid()) {
        const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
        params.idOrder = item.id;
        params.idPatient = item.patientId;
        params.orderDisplayText = QStringLiteral("Comanda ecografică nr.%1 din %2")
                                      .arg(item.numberDoc.trimmed(), item.dateDocText);
    }

    ReportDialog document(m_db, params, this);
    document.onPrintDocument(typePrint);

}


// **********************************************************************************
// --- procesarea actiunilor cu tableView

void OrderView::onClickedTableView(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    m_currentRow = index.row();
    updateDocumentPreview();
}

void OrderView::onDoubleClickedTableView(const QModelIndex &index)
{
    Q_UNUSED(index);
    onEditDoc();
}

void OrderView::onColumnsChanged()
{
    if (!m_columnsController || !ui || !ui->tableView)
        return;

    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    m_filter.hiddenSections = m_columnsController->hiddenSections();

    const int count = header->count();
    for (int col = 0; col < count; ++col) {

        if (!ui->tableView->isColumnHidden(col) && header->sectionSize(col) <= 0) {
            int width = m_filter.sectionSizes.value(col, 0);
            if (width <= 0) {
                width = header->sectionSizeHint(col);
                if (width <= 0)
                    width = header->defaultSectionSize();
            }
            header->resizeSection(col, width);
        }

    }

    ui->tableView->update();

    saveSettingsJournal();
}

void OrderView::indexChangedCombo(int index)
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
            m_filter.idOrganization = id_organization;
        if (id_contracts > 0) {
            m_filter.idContract = id_contracts;
            ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", id_contracts));
        }

    } else if (combo == ui->comboContracts) {
        // determinam rolul
        auto roleID = modelContracts->roleForColumn("id");
        // ID necesare
        const int id_contracts = ui->comboContracts->currentData(roleID).toInt();
        // completam structura setarilor jurnalului
        if (id_contracts > 0)
            m_filter.idContract = id_contracts;

    } else if (combo == ui->comboUsers) {
        // determinam rolul
        auto roleID = modelUsers->roleForColumn("id");
        // ID necesare
        const int id_user = ui->comboUsers->currentData(roleID).toInt();
        // completam structura setarilor jurnalului
        if (id_user > 0)
            m_filter.idUser = id_user;

    }
}

void OrderView::onApplyFilter()
{
    updateTableView();
    ui->groupBoxFilter->setHidden(true);
    saveSettingsJournal();
}

void OrderView::slotContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = ui->tableView->indexAt(pos);
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

    QAction *selectedAction = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == actionNewDoc)
        onAddDoc();
    else if (selectedAction == actionEditDoc)
        onEditDoc();
    else if (selectedAction == actionDeleteDoc)
        onDeleteDoc();
    else if (selectedAction == actionPrintDoc)
        onPrintDoc();
}

void OrderView::loadFilterJournalBySettings()
{
    const QDateTime m_dtStart = QDateTime::fromString("2021-01-01T00:00:00", Qt::ISODate);
    const QDateTime m_dtEnd   = QDateTime(QDate::currentDate(), QTime(23, 59, 59));

    const QJsonObject obj = m_settings.getJsonObject(m_typeJournal);
    if (obj.isEmpty()) {
        m_filter.startDate = m_dtStart;
        m_filter.endDate   = m_dtEnd;
        return;
    }

    // --- perioada
    const QDateTime dtStart = QDateTime::fromString(
        obj.value("startDate").toString(), Qt::ISODateWithMs);
    const QDateTime dtEnd = QDateTime::fromString(
        obj.value("endDate").toString(), Qt::ISODateWithMs);

    m_filter.startDate = dtStart.isValid() ? dtStart : m_dtStart;
    m_filter.endDate   = dtEnd.isValid() ? dtEnd : m_dtEnd;

    // --- filtru
    const QJsonObject filterObj = obj.value("filter").toObject();
    m_filter.saveFilter     = filterObj.value("saveFilter").toBool(false);
    m_filter.nrDoc          = filterObj.value("nr_doc").toString();
    m_filter.idOrganization = filterObj.value("id_organization").toInt(0);
    m_filter.idContract     = filterObj.value("id_contract").toInt(0);
    m_filter.idUser         = filterObj.value("id_user").toInt(0);

    // --- sortarea sectiilor
    const QJsonObject sortObj = obj.value("sort").toObject();
    m_filter.sortSection = sortObj.value("section").toInt(0);
    m_filter.sortOrder = sortObj.value("direction").toInt(0) == 0
                             ? Qt::AscendingOrder
                             : Qt::DescendingOrder;

    // --- size section
    const QJsonObject sectionsObj = obj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = obj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void OrderView::initBoxFilterAndTableFooter()
{
    ui->groupBoxFilter->setHidden(true);

    m_viewTabVisible = false;
    ui->groupBox_table_order->setHidden(true);
    ui->groupBox_table_report->setHidden(true);

    ui->tableView->viewport()->setMouseTracking(true);
    ui->tableView->viewport()->installEventFilter(this);
}

void OrderView::updateTextPeriod()
{
    QString str;

    str += (tr("Perioada: ") +
            ui->dateStart->dateTime().toString("dd.MM.yyyy") + " - " +
            ui->dateEnd->dateTime().toString("dd.MM.yyyy"));

    if (ui->comboOrganizations->currentIndex() > 0)
        str += tr("; filtru: ") + ui->comboOrganizations->currentText();
    if (!m_filter.patientName.isEmpty())
        str += tr("; pacient: ") + m_filter.patientName;
    if (ui->comboUsers->currentIndex() > 0)
        str += tr("; autor: ") + ui->comboUsers->currentText();

    toolBar->setTextPeriod(str);
}

void OrderView::loadFilterData()
{
    ui->dateStart->setDateTime(m_filter.startDate);
    ui->dateEnd->setDateTime(m_filter.endDate);

    if (!m_filter.saveFilter) {
        updateTextPeriod();
        return;
    }

    ui->saveFilter->setChecked(m_filter.saveFilter);

    if (!m_filter.nrDoc.isEmpty())
        ui->numberDoc->setText(m_filter.nrDoc);
    if (m_filter.idOrganization > 0)
        ui->comboOrganizations->setCurrentIndex(modelOrganizations->rowById("id", m_filter.idOrganization));
    if (m_filter.idContract > 0)
        ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", m_filter.idContract));
    if (m_filter.idUser > 0)
        ui->comboUsers->setCurrentIndex(modelUsers->rowById("id", m_filter.idUser));

    updateTextPeriod();
}

void OrderView::loadSizeSection()
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    ui->tableView->setUpdatesEnabled(false);

    const bool oldStretch = header->stretchLastSection();
    header->setStretchLastSection(false);

    const int colCount = header->count();

    // 1. restauram latimile
    for (int col = 0; col < colCount; ++col) {

        int width = m_filter.sectionSizes.value(col, 0);

        if (width <= 0) {
            width = header->sectionSizeHint(col);
            if (width <= 0)
                width = header->defaultSectionSize();
        }

        header->resizeSection(col, width);
    }

    // 2. restauram hide/show
    if (m_columnsController)
        m_columnsController->setHiddenSections(m_filter.hiddenSections);

    header->setStretchLastSection(oldStretch);

    if (ui->tableView->model() && ui->tableView->model()->rowCount() > 0)
        ui->tableView->selectRow(0);

    ui->tableView->setUpdatesEnabled(true);
}

void OrderView::saveSettingsJournal()
{
    if (!ui || !ui->tableView)
        return;

    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    // perioada
    m_settings.setValue(m_typeJournal, "startDate", ui->dateStart->dateTime());
    m_settings.setValue(m_typeJournal, "endDate",   ui->dateEnd->dateTime());

    // filtru
    m_settings.setValue(m_typeJournal, "filter/saveFilter", ui->saveFilter->isChecked());
    m_settings.setValue(m_typeJournal, "filter/id_organization", m_filter.idOrganization);
    m_settings.setValue(m_typeJournal, "filter/id_contract", m_filter.idContract);
    m_settings.setValue(m_typeJournal, "filter/id_user", m_filter.idUser);
    m_settings.setValue(m_typeJournal, "filter/nr_doc",
                        ui->numberDoc->text().trimmed().isEmpty()
                            ? QVariant()
                            : QVariant(ui->numberDoc->text().trimmed()));

    // sortare
    m_settings.setValue(m_typeJournal, "sort/section",
                        header->sortIndicatorSection());
    m_settings.setValue(m_typeJournal, "sort/direction",
                        static_cast<int>(header->sortIndicatorOrder()));

    const int lastVisible = lastVisibleSection();

    // sections + hidden
    const int count = header->count();
    for (int section = 0; section < count; ++section) {

        m_settings.setValue(m_typeJournal,
                            QString("hide_show_sections/%1").arg(section),
                            header->isSectionHidden(section) ? 1 : 0);

        // NU salvam latimea pentru ultima sectie vizibila, fiindca e stretch-uită
        if (section == lastVisible)
            continue;

        const int width = header->sectionSize(section);
        if (width > 0) {
            m_settings.setValue(m_typeJournal,
                                QString("sections/%1").arg(section),
                                width);
        }

    }

    m_settings.save();
}

void OrderView::updateModelOrganizations()
{
    if (modelOrganizations)
        delete modelOrganizations;

    QString str = m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql");
    modelOrganizations = new QueryRolesModel(str, ui->comboOrganizations);
    modelOrganizations->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganizations->setModel(modelOrganizations);
    ui->comboOrganizations->setModelColumn(modelOrganizations->columnIndex("name"));

    if (m_filter.idOrganization > 0 &&
        ui->comboOrganizations->currentIndex() == 0)
        ui->comboOrganizations->setCurrentIndex(modelOrganizations->rowById("id", m_filter.idOrganization));
}

void OrderView::updateModelContracts()
{
    if (modelContracts)
        delete modelContracts;

    if (m_filter.idOrganization <= 0) {
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
        qry.addBindValue(m_filter.idOrganization);
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

    if (m_filter.idContract > 0 &&
        ui->comboContracts->currentIndex() == 0)
        ui->comboContracts->setCurrentIndex(modelContracts->rowById("id", m_filter.idContract));
}

void OrderView::updateModelUsers()
{
    if (modelUsers)
        delete modelUsers;

    QString str = m_db.getTextSQL(":/sql/queries/users_combo_view.sql");
    modelUsers = new QueryRolesModel(str, ui->comboUsers);
    modelUsers->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboUsers->setModel(modelUsers);
    ui->comboUsers->setModelColumn(modelUsers->columnIndex("name"));

    if (m_filter.idUser > 0 &&
        ui->comboUsers->currentIndex() == 0)
        ui->comboUsers->setCurrentIndex(modelUsers->rowById("id", m_filter.idUser));
}

int OrderView::lastVisibleSection() const
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return -1;

    for (int section = header->count() - 1; section >= 0; --section) {
        if (!header->isSectionHidden(section))
            return section;
    }

    return -1;
}

void OrderView::initTableView()
{
    modelTable = new OrderJournalModel(m_db, this);
    proxyTable = new SortModel(this);

    modelTable->setBatchSize(100);

    proxyTable->setSourceModel(modelTable);
    proxyTable->setSortRole(OrderJournalModel::SortRole);
    proxyTable->setDynamicSortFilter(false);

    ui->tableView->setModel(proxyTable);

    ui->tableView->hideColumn(OrderJournal::Id);
    ui->tableView->hideColumn(OrderJournal::Id_Organization);
    ui->tableView->hideColumn(OrderJournal::Id_Contract);
    ui->tableView->hideColumn(OrderJournal::PatientId);
    ui->tableView->hideColumn(OrderJournal::Id_Doctor);
    ui->tableView->hideColumn(OrderJournal::Id_User);
    ui->tableView->hideColumn(OrderJournal::PatientSearch);
    ui->tableView->hideColumn(OrderJournal::Uuid);

    ui->tableView->setItemDelegateForColumn(OrderJournal::DeletionMark,
                                            new CenterIconDelegate(ui->tableView));
    ui->tableView->setItemDelegateForColumn(OrderJournal::AttachedImages,
                                            new CenterIconDelegate(ui->tableView));
    ui->tableView->setItemDelegateForColumn(OrderJournal::CardPayment,
                                            new CenterIconDelegate(ui->tableView));

    ui->tableView->setWordWrap(false);
    ui->tableView->setTextElideMode(Qt::ElideRight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    if (!m_columnsController)
        m_columnsController = new TableColumnsController(ui->tableView, this);

    m_columnsController->setFixedHiddenColumns({
        OrderJournal::Id,
        OrderJournal::Id_Organization,
        OrderJournal::Id_Contract,
        OrderJournal::PatientId,
        OrderJournal::Id_Doctor,
        OrderJournal::Id_User,
        OrderJournal::PatientSearch,
        OrderJournal::Uuid
    });

    m_columnsController->setExcludedFromMenuColumns({
        OrderJournal::DeletionMark,
        OrderJournal::CardPayment,
        OrderJournal::AttachedImages
    });

    connect(m_columnsController, &TableColumnsController::columnsChanged,
            this, &OrderView::onColumnsChanged);

    loadSizeSection();

    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &OrderView::onScroll, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),
            this, &OrderView::onClickedTableView, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &OrderView::onDoubleClickedTableView, Qt::UniqueConnection);

    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &OrderView::slotContextMenuRequested, Qt::UniqueConnection);
}

void OrderView::updateTableView()
{
    qInfo(logInfo()) << "OrderView: vizualizarea/actualizarea jurnalului";
    if (!modelTable || !proxyTable)
        return;

    modelTable->setFilter(m_filter);
    modelTable->reload();

    if (m_filter.sortSection >= 0 &&
        m_filter.sortSection < proxyTable->columnCount())
    {
        ui->tableView->sortByColumn(m_filter.sortSection, m_filter.sortOrder);
    }

    if (proxyTable->rowCount() <= 0) {
        ui->tableView->clearSelection();
        modelViewOrder->clear();
        modelViewReport->clear();
        if (m_viewTabVisible) {
            ui->tableReport->hide();
            ui->btnOpenReport->hide();
            ui->btnPrintReport->hide();
            ui->text_empty_report->show();
        }
        return;
    }

    int rowToSelect = 0;
    if (m_currentRow >= 0 && m_currentRow < proxyTable->rowCount())
        rowToSelect = m_currentRow;

    ui->tableView->selectRow(rowToSelect);
    m_currentRow = rowToSelect;
    updateDocumentPreview();
}

void OrderView::initToolBar()
{
    toolBar->setStyles(m_db.toolButtonStyleForIcon(),
                       m_db.toolButtonStyleForText());

    ui->layoutToolBar->addWidget(toolBar);

    toolBar->getBtnAddDoc()->installEventFilter(this);
    toolBar->getBtnEditDoc()->installEventFilter(this);
    toolBar->getBtnDeletDoc()->installEventFilter(this);

    toolBar->getBtnAddFilter()->installEventFilter(this);
    toolBar->getBtnSetFilter()->installEventFilter(this);
    toolBar->getBtnDeleteFilter()->installEventFilter(this);

    toolBar->getBtnUpdateTable()->installEventFilter(this);
    toolBar->getBtnHideShowColumn()->installEventFilter(this);

    toolBar->getBtnPrintDoc()->installEventFilter(this);
    toolBar->getBtnSendEmail()->installEventFilter(this);

    toolBar->getBtnCreateReport()->installEventFilter(this);
    toolBar->getBtnViewTabOrder()->installEventFilter(this);

    toolBar->getBtnOpenPeriod()->installEventFilter(this);
    toolBar->getBtnSaecrPacient()->installEventFilter(this);

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &OrderView::onAddDoc, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &OrderView::onEditDoc, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &OrderView::onDeleteDoc, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::addFilter,
            this, &OrderView::onAddFilter, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::setFilter,
            this, &OrderView::onSetFilter, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteFilter,
            this, &OrderView::onDeleteFilter, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::updateTable,
            this, &OrderView::onUpdateTableView, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::hideShowColumn,
            this, &OrderView::onHideShowColumn, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::printDoc,
            this, &OrderView::onPrintDoc, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::sendEmail,
            this, &OrderView::onSendEmail, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::createReport,
            this, &OrderView::onCreateReport, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::searchPacients,
            this, &OrderView::onSearchPacients, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::openPeriod,
            this, &OrderView::onOpenPeriod, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::viewTabOrder,
            this, &OrderView::onViewTabOrder, Qt::UniqueConnection);
}

void OrderView::initBtnFilter()
{
    const QString style = m_db.toolButtonStyleForIcon();

    ui->btnSelectPeriod->setStyleSheet(style);
    ui->openCatOrganization->setStyleSheet(style);
    ui->openCatContract->setStyleSheet(style);
    ui->openCatUser->setStyleSheet(style);

    QList<QDateTimeEdit*> dts = findChildren<QDateTimeEdit*>();
    for (QDateTimeEdit *dt : std::as_const(dts))
        connect(dt, &QDateTimeEdit::dateTimeChanged,
                this, &OrderView::validatePeriodAndUpdate, Qt::UniqueConnection);

    connect(ui->btnSelectPeriod, &QToolButton::clicked,
            this, &OrderView::onOpenPeriod, Qt::UniqueConnection);

    QList<QComboBox*> combos = findChildren<QComboBox*>();
    for (QComboBox *combo : std::as_const(combos)) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &OrderView::indexChangedCombo, Qt::UniqueConnection);
    }

    connect(ui->btnApplyFilter, &QToolButton::clicked,
            this, &OrderView::onApplyFilter, Qt::UniqueConnection);
    connect(ui->btnCloseFilter, &QToolButton::clicked,
            this, &OrderView::onAddFilter, Qt::UniqueConnection);
}

bool OrderView::isValidIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return false;
    }
    return true;
}

void OrderView::clearFilter()
{
    m_filter.nrDoc.clear();
    m_filter.patientName.clear();
    m_filter.idOrganization = 0;
    m_filter.idContract     = 0;
    m_filter.idUser         = 0;
    m_filter.patientId      = 0;

    ui->numberDoc->clear();
    ui->comboOrganizations->setCurrentIndex(0);
    ui->comboContracts->setCurrentIndex(0);
    ui->comboUsers->setCurrentIndex(0);

    updateTableView();
    updateTextPeriod();
}

void OrderView::validatePeriodAndUpdate()
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
        ui->dateStart->setDateTime(m_filter.startDate);
        ui->dateEnd->setDateTime(m_filter.endDate);
        return;
    }

    m_filter.startDate = startDate;
    m_filter.endDate = endDate;

    updateTextPeriod();
    updateTableView();
}

void OrderView::reject()
{
    if (auto *sub = qobject_cast<QMdiSubWindow *>(parentWidget())) {
        sub->close();      // inchide subfereastra MDI si dialogul intern
        return;
    }

    QDialog::reject();     // fallback normal
}

// **********************************************************************************
// --- evenimentele formei si functii protected

bool OrderView::previewImagesDocs(QEvent *event)
{
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    QModelIndex index = ui->tableView->indexAt(helpEvent->pos());

    if (! index.isValid())
        return false;

    int id_order = proxyTable->index(index.row(), OrderJournal::Id).data(Qt::UserRole).toInt();

    QPixmap outPixmap1, outPixmap2, outPixmap3;

    QSqlQuery qry(globals().thisSqlite ? m_db.getDatabaseImage() : m_db.getDatabase());
    qry.prepare(R"(
        SELECT
            image_1,
            image_2,
            image_3
        FROM
            imagesReports
        WHERE
            id_orderEcho = :id_orderEcho
    )");
    qry.bindValue(":id_orderEcho", id_order);

    if (qry.exec() && qry.next()) {

        QByteArray outByteArray1 = QByteArray::fromBase64(qry.value(0).toByteArray());
        if (! outByteArray1.isEmpty())
            outPixmap1.loadFromData(outByteArray1, "JPEG");

        QByteArray outByteArray2 = QByteArray::fromBase64(qry.value(1).toByteArray());
        if (! outByteArray2.isEmpty())
            outPixmap2.loadFromData(outByteArray2, "JPEG");

        QByteArray outByteArray3 = QByteArray::fromBase64(qry.value(2).toByteArray());
        if (! outByteArray3.isEmpty())
            outPixmap3.loadFromData(outByteArray3, "JPEG");

    }

    auto toBase64ImageTag = [](const QPixmap &pixmap) -> QString {
        if (pixmap.isNull())
            return "";

        QPixmap scaled = pixmap.scaled(400, 350, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        scaled.save(&buffer, "PNG");
        QString base64 = QString::fromLatin1(byteArray.toBase64());
        return QString("<img src='data:image/png;base64,%1' style='display:inline-block; margin-right:5px;'>")
            .arg(base64);
    };

    QString html = QString("<div style='white-space:nowrap;'>%1%2%3</div>")
                       .arg(toBase64ImageTag(outPixmap1),
                            toBase64ImageTag(outPixmap2),
                            toBase64ImageTag(outPixmap3));

    if (! html.isEmpty() && ! outPixmap1.isNull()) {
        QToolTip::showText(helpEvent->globalPos(),
                           html, ui->tableView->viewport());
        return true;
    }

    return false;
}

void OrderView::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSettingsJournal();
    }
}

bool OrderView::eventFilter(QObject *obj, QEvent *event)
{
    // previzualizarea imaginelor atasate
    if (event->type() == QEvent::ToolTip) {
        return previewImagesDocs(event);
    }

    auto *button = qobject_cast<QToolButton *>(obj);
    if (!button || !popUp)
        return QDialog::eventFilter(obj, event);

    if (event->type() == QEvent::Leave) {
        popUp->hidePop();
        return true;
    }
    if (event->type() != QEvent::Enter)
        return QDialog::eventFilter(obj, event);

    QString text;
    if (button == toolBar->getBtnAddDoc())
        text = tr("Adaugă (Ins)");
    else if (button == toolBar->getBtnDeletDoc())
        text = tr("Elimină (Del)");
    else if (button == toolBar->getBtnEditDoc())
        text = tr("Editează (F2)");
    else if (button == toolBar->getBtnAddFilter())
        text = tr("Deschide filtru (Ctrl + F1)");
    else if (button == toolBar->getBtnSetFilter())
        text = tr("Filtru rapid (Ctrl + F2)");
    else if (button == toolBar->getBtnDeleteFilter())
        text = tr("Șterge filtru (Ctrl + F3)");
    else if (button == toolBar->getBtnUpdateTable())
        text = tr("Actualizează (F5)");
    else if (button == toolBar->getBtnHideShowColumn())
        text = tr("Ascunde/prezintă secții<br> (Ctrl + H)");
    else if (button == toolBar->getBtnPrintDoc())
        text = tr("Printare (Ctrl + P)");
    else if (button == toolBar->getBtnSendEmail())
        text = tr("Trimite e-mail (Ctrl + M)");
    else if (button == toolBar->getBtnCreateReport())
        text = tr("Crearea raportului (Ctrl + R)");
    else if (button == toolBar->getBtnViewTabOrder())
        text = tr("Vizualizarea concluziei (Ctrl + T)");
    else if (button == toolBar->getBtnOpenPeriod())
        text = tr("Perioada (Ctrl + Shift + P)");
    else if (button == toolBar->getBtnSaecrPacient())
        text = tr("Cauta (Ctrl + F)");

    if (text.isEmpty())
        return QDialog::eventFilter(obj, event);


    const QPoint position = button->mapToGlobal(QPoint(0, button->height()));
    popUp->setPopupText(text);
    popUp->showFromGeometryTimer(position);

    return true;
}

void OrderView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Lista documentelor: Comanda ecografica"));
    }
}

void OrderView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End){
        ui->tableView->selectRow(modelTable->rowCount() - 1);
    } else if (event->key() == Qt::Key_Home){
        ui->tableView->selectRow(0);
    }

    // if (pressed_btn_viewTab == -1)
    //     return;

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down){
        QModelIndex index = ui->tableView->currentIndex();
        onClickedTableView(index);
    }
}
