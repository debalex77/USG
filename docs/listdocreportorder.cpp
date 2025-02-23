#include "listdocreportorder.h"
#include "ui_listdocreportorder.h"

#include <QCache>

ListDocReportOrder::ListDocReportOrder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListDocReportOrder)
{
    ui->setupUi(this);

    db               = new DataBase(this); // conectarea la BD
    menu             = new QMenu(this);    // meniu contextual
    setUpMenu_order  = new QMenu(this);    // meniu contextual butonului 'print' documentului 'order'
    setUpMenu_report = new QMenu(this);    // meniu contextual butonului 'print' documentului 'report'
    popUp            = new PopUp(this);    // mesaje
    customPeriod     = new CustomPeriod(this);

    modelTable = new PaginatedSqlModel(this);

    QString str_qry_view_table_order;
    model_view_table_order = new BaseSqlQueryModel(str_qry_view_table_order, ui->view_table);
    model_view_table_order->setProperty("modelParent", BaseSqlQueryModel::ModelParent::ViewTableOrder);

    QString str_qry_view_table_report;
    model_view_table_report = new BaseSqlQueryModel(str_qry_view_table_report, ui->view_table);
    model_view_table_report->setProperty("modelParent", BaseSqlQueryModel::ModelParent::ViewTableReport);

    proxyTable = new BaseSortFilterProxyModel(this);  // proxy-model pu sortarea datelor
    proxyTable->setListFormType(BaseSortFilterProxyModel::ListFormType::ListDocuments);

    QString strQryOrganizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";    // solicitarea
    QString strQryContracts     = "SELECT id,name FROM contracts WHERE deletionMark = 0;";
    modelOrganizations = new BaseSqlQueryModel(strQryOrganizations, ui->filterComboOrganization); // modele pu filtrarea
    modelContracts     = new BaseSqlQueryModel(strQryContracts, ui->filterComboContract);
    modelOrganizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelContracts->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->filterComboOrganization->setModel(modelOrganizations);
    ui->filterComboContract->setModel(modelContracts);
    if (modelOrganizations->rowCount() > 20){
        ui->filterComboOrganization->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->filterComboOrganization->setStyleSheet("combobox-popup: 0;");
        ui->filterComboOrganization->setMaxVisibleItems(15);
    }

    ui->filterComboContract->setEnabled(false); // initial
    ui->groupPeriodFilter->setVisible(false);

    ui->groupBox_table_order->setHidden(true);
    ui->groupBox_table_report->setHidden(true);

    initBtnToolBar();
    initBtnFilter();

    // Conectăm scroll-ul
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &ListDocReportOrder::onScroll);

    connect(ui->filterComboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&ListDocReportOrder::indexChangedComboOrganization));
    connect(ui->filterComboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&ListDocReportOrder::indexChangedComboContract));

    connect(ui->filterStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onStartDateTimeChanged);
    connect(ui->filterEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onEndDateTimeChanged);

    connect(this, &ListDocReportOrder::IdOrganizationChanged, this, &ListDocReportOrder::slot_IdOrganizationChanged);
    connect(this, &ListDocReportOrder::IdContractChanged, this, &ListDocReportOrder::slot_IdContractChanged);
    connect(this, &ListDocReportOrder::TypeDocChanged, this, &ListDocReportOrder::slot_TypeDocChanged);

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &ListDocReportOrder::onDoubleClickedTable);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked), this, &ListDocReportOrder::onClickedTable);
    connect(ui->tableView, &QWidget::customContextMenuRequested, this, &ListDocReportOrder::slotContextMenuRequested);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ListDocReportOrder::updateMainTableByTimer);  // pu aprecierea daca e deschisa forma MainWindow
}

ListDocReportOrder::~ListDocReportOrder()
{
    delete timer;
    delete menu;
    delete setUpMenu_order;
    delete setUpMenu_report;
    delete popUp;
    delete proxyTable;
    delete modelTable;
    delete modelContracts;
    delete modelOrganizations;
    delete model_view_table_order;
    delete model_view_table_report;
    delete customPeriod;
    delete db;
    delete ui;
}

void ListDocReportOrder::onScroll(int value)
{
    if (loadDocumentsFull){ // se seteaza la enableLineEditSearch()
        if (timer->isActive())
            timer->stop();  // oprim timer sa nu actualizeze tabela
        return;             // nu mai incarcam date
    }

    if (value > 0 && timer->isActive()) {
        // oprim timer principal
        timer->stop();
        qInfo(logInfo()) << "Detectat scroll - timer principal pentru actualizarea listei de documente este oprit !!!";

        // Pornim un timer temporar pentru a restarta timer-ul principal
        QTimer::singleShot(2 * 60 * 1000, this, [this]() {
            timer->start(globals().updateIntervalListDoc * 1000);
            qInfo(logInfo()) << "Timer principal pentru actualizarea documentelor startat din nou !!!";
        });
    }

    int maxScroll = ui->tableView->verticalScrollBar()->maximum();
    if (value == maxScroll) {
        qInfo(logInfo()) << "Scroll detectat la capăt, se încărca mai multe date...";
        modelTable->fetchMoreData();
    }
}

void ListDocReportOrder::updateMainTableByTimer()
{
    updateTableView();
}

void ListDocReportOrder::enableLineEditSearch()
{
    if (ui->editSearch->isEnabled()){
        ui->editSearch->setEnabled(false);
    } else {
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Set\304\203ri de c\304\203utare a pa cientului"),
                                 tr("1.Pentru c\304\203utarea documentelor pacientului dup\304\203 nume, prenume sau IDNP este necesar "
                                    "de \303\256nc\304\203rcat toate documentele de la ini\310\233ierea programei.<br>"
                                    "2.Paginarea formei de prezentare a documentelor o s\304\203 fie dezactivat\304\203.<br>"
                                    "3.Dac\304\203 documente sunt \303\256n cantitate mare, procesul de \303\256nc\304\203rcare poate dura timp \303\256ndelungat.<br><br>"
                                    "Dori\310\233i s\304\203 continua\310\233i ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton){
            if (m_typeDoc == orderEcho) {
                ui->filterStartDateTime->setDateTime(QDateTime(QDate(2021,01,01), QTime(00,00,00)));
                ui->filterEndDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 12,31), QTime(23,59,59)));
                updateTextPeriod();
                connect(modelTable, &PaginatedSqlModel::updateProgress, this, &ListDocReportOrder::updateProgress);
                connect(modelTable, &PaginatedSqlModel::finishedProgress, this, &ListDocReportOrder::finishedProgress);
                updateTableViewOrderEchoFull();
                loadDocumentsFull = true;
            } else if (m_typeDoc == reportEcho) {
                ui->filterStartDateTime->setDateTime(QDateTime(QDate(2021,01,01), QTime(00,00,00)));
                ui->filterEndDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 12,31), QTime(23,59,59)));
                updateTextPeriod();
                connect(modelTable, &PaginatedSqlModel::updateProgress, this, &ListDocReportOrder::updateProgress);
                connect(modelTable, &PaginatedSqlModel::finishedProgress, this, &ListDocReportOrder::finishedProgress);
                updateTableViewReportEchoFull();
                loadDocumentsFull = true;
            }
            ui->editSearch->setEnabled(true);
        } else if (messange_box.clickedButton() == noButton) {

        }
    }
}

// **********************************************************************************
// --- functiile filtrului

void ListDocReportOrder::onClosePeriodFilter()
{
    if (ui->groupPeriodFilter->isVisible())
        ui->groupPeriodFilter->setVisible(false);
    else
        ui->groupPeriodFilter->setVisible(true);
}

void ListDocReportOrder::onStartDateTimeChanged()
{
    updateTextPeriod();
}

void ListDocReportOrder::onEndDateTimeChanged()
{
    updateTextPeriod();
    if (ui->filterStartDateTime->dateTime() > ui->filterEndDateTime->dateTime()){
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data finis\304\203rii perioadei nu poate fi mai mic\304\203 dec\303\242t \ndata lans\304\203rii perioadei !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
    }
}

void ListDocReportOrder::indexChangedComboOrganization(const int index)
{
    int id_organization = ui->filterComboOrganization->itemData(index, Qt::UserRole).toInt();
    setIdOrganization(id_organization);
}

void ListDocReportOrder::indexChangedComboContract(const int index)
{
    int id_contract = ui->filterComboContract->itemData(index, Qt::UserRole).toInt();
    setIdContract(id_contract);
}

// **********************************************************************************
// --- procesarea slot-urilor

void ListDocReportOrder::slot_TypeDocChanged()
{
    if (globals().updateIntervalListDoc > 0)
        timer->start(globals().updateIntervalListDoc * 1000);

    if (m_typeDoc == reportEcho){
        ui->btnReport->setDisabled(true);
        ui->filterComboOrganization->setEnabled(false);
        ui->filterComboContract->setEnabled(false);
        ui->btnOpenCatContract->setEnabled(false);
        ui->btnOpenCatOrganization->setEnabled(false);
        ui->view_table->setHidden(true);
        setWindowTitle(tr("Lista documentelor: Raport ecografic"));
        setWindowIcon(QIcon(":/img/examenEcho.png"));
        qInfo(logInfo()) << "Deschisa lista documentelor: Raport ecografic.";
    } else {
        setWindowTitle(tr("Lista documentelor: Comanda ecografic\304\203"));
        setWindowIcon(QIcon(":/img/orderEcho_x32.png"));
        qInfo(logInfo()) << "Deschisa lista documentelor: Comanda ecografică.";
    }
    loadSizeSectionPeriodTable(true); // only period
    updateTableView();
    loadSizeSectionPeriodTable(); // without period
}

void ListDocReportOrder::slot_IdOrganizationChanged()
{
    if (m_idOrganization == Enums::IDX::IDX_UNKNOW || m_idOrganization == 0)
        return;

    auto indexOrganization = modelOrganizations->match(modelOrganizations->index(0,0), Qt::UserRole, m_idOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty())
        ui->filterComboOrganization->setCurrentIndex(indexOrganization.first().row());

    QMap<QString, QString> items;
    if (db->getObjectDataById("organizations", m_idOrganization, items)){
        const int id_contract = items.constFind("id_contracts").value().toInt();
        if (id_contract > 0){
            ui->filterComboContract->setEnabled(true);
            setIdContract(id_contract);
        }
    }
    updateTextPeriod();
}

void ListDocReportOrder::slot_IdContractChanged()
{
    if (m_idContract == Enums::IDX::IDX_UNKNOW || m_idContract == 0)
        return;
    auto indexContract = modelContracts->match(modelContracts->index(0,0), Qt::UserRole, m_idContract, 1, Qt::MatchExactly);
    if (!indexContract.isEmpty())
        ui->filterComboContract->setCurrentIndex(indexContract.first().row());
    updateTextPeriod();
}

// **********************************************************************************
// --- procesarea butoanelor filtrului

void ListDocReportOrder::onClickBtnFilterApply()
{
    setNumberDoc(ui->filterEditNumberDoc->text()); // setam variabila m_numberDoc pu solicitarea corecta

    updateTableView();
    onClosePeriodFilter();
}

void ListDocReportOrder::onClickBtnFilterClear()
{
    ui->filterEditNumberDoc->clear();
    ui->filterComboOrganization->setCurrentIndex(0);
    ui->filterComboContract->setCurrentIndex(0);

    m_idOrganization = Enums::IDX::IDX_UNKNOW;
    m_idContract     = Enums::IDX::IDX_UNKNOW;
    updateTableView();
    updateTextPeriod();
}

void ListDocReportOrder::onClickBtnFilterClose()
{
    onClosePeriodFilter();
}

// **********************************************************************************
// --- procesarea butoanelor tool bar-ului

void ListDocReportOrder::onClickBtnAdd()
{
    if (m_typeDoc == orderEcho){
        docOrderEcho = new DocOrderEcho(this);
        docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
        docOrderEcho->setProperty("ItNew", true);
        docOrderEcho->setGeometry(250, 150, 1400, 800);
        docOrderEcho->show();
        connect(docOrderEcho, &DocOrderEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
        onClickedTable(ui->tableView->currentIndex());
    } else {
        CustomDialogInvestig* dialogInvestig = new CustomDialogInvestig(this);
        if (dialogInvestig->exec() != QDialog::Accepted)
            return;
        DocReportEcho* docReport = new DocReportEcho(this);
        docReport->setAttribute(Qt::WA_DeleteOnClose);
        docReport->set_t_organs_internal(dialogInvestig->get_t_organs_internal());
        docReport->set_t_urinary_system(dialogInvestig->get_t_urinary_system());
        docReport->set_t_prostate(dialogInvestig->get_t_prostate());
        docReport->set_t_gynecology(dialogInvestig->get_t_gynecology());
        docReport->set_t_breast(dialogInvestig->get_t_breast());
        docReport->set_t_thyroide(dialogInvestig->get_t_thyroide());
        docReport->set_t_gestation0(dialogInvestig->get_t_gestation0());
        docReport->set_t_gestation1(dialogInvestig->get_t_gestation1());
        docReport->set_t_gestation2(dialogInvestig->get_t_gestation2());
        docReport->setProperty("ItNew", true);
        docReport->show();
        connect(docReport, &DocReportEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
    }
}

void ListDocReportOrder::onClickBtnEdit()
{
    if (ui->tableView->currentIndex().row() == Enums::IDX::IDX_UNKNOW){
        QMessageBox::warning(this,
                             tr("Informa\310\233ie"),
                             tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    if (m_typeDoc == orderEcho){
        const int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
        docOrderEcho = new DocOrderEcho(this);
        docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
        docOrderEcho->setProperty("ItNew", false);
        docOrderEcho->setProperty("Id", _id);
        docOrderEcho->setGeometry(250, 150, 1400, 800);
        docOrderEcho->show();
        connect(docOrderEcho, &DocOrderEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
    } else {
        const int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id), Qt::DisplayRole).toInt();
        DocReportEcho* docReport = new DocReportEcho(this);
        docReport->setAttribute(Qt::WA_DeleteOnClose);
        docReport->setProperty("ItNew", false);
        docReport->setProperty("Id", _id);
        docReport->show();
        connect(docReport, &DocReportEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
    }
}

void ListDocReportOrder::onClickBtnDeletion()
{
    const int _row = ui->tableView->currentIndex().row();
    if (_row == Enums::IDX::IDX_UNKNOW){
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat r\303\242ndul !!!."), QMessageBox::Ok);
        return;
    }
    const int _id = proxyTable->index(_row, section_id).data(Qt::DisplayRole).toInt();

    QSqlQuery qry;
    if (m_typeDoc == orderEcho){
        qry.exec("PRAGMA foreign_keys = ON;");
        qry.prepare("SELECT "
                    "  reportEchoPresentation.docPresentation "
                    "FROM "
                    "  reportEchoPresentation "
                    "WHERE "
                    "  reportEchoPresentation.id_reportEcho IN ("
                    "    SELECT reportEcho.id "
                    "    FROM reportEcho "
                    "    WHERE id_orderEcho = :id)"
                    ";");
        qry.bindValue(":id", _id);
        if (qry.exec()){
            if (qry.next()){
                const QString doc_presentation = qry.value(0).toString();
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Eliminarea documentului."));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Exist\304\203 documente subordonate. Dori\310\233i s\304\203 continua\310\233i ?"));
                msgBox.setDetailedText(tr("Va fi eliminat documentul subordonat:\n%1").arg(doc_presentation));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setStyleSheet("QPushButton{width:120px;}");
                if (msgBox.exec() == QMessageBox::Yes){
                    qry.exec("PRAGMA foreign_keys = ON;");
                    qry.prepare("DELETE FROM orderEcho WHERE id = :id;");
                    qry.bindValue(":id", _id);
                    if (qry.exec()){
                        popUp->setPopupText(tr("Documentul a fost eliminat<br>"
                                               "cu succes din baza de date."));
                        popUp->show();
                        updateTableView();
                    } else {
                        msgBox.setWindowTitle(tr("Eliminarea documentului."));
                        msgBox.setIcon(QMessageBox::Warning);
                        msgBox.setText(tr("Documentul nu a fost eliminat din baza de date !!!"));
                        msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
                        msgBox.setStandardButtons(QMessageBox::Ok);
                        msgBox.setStyleSheet("QPushButton{width:120px;}");
                        msgBox.exec();
                    }
                }
            } else {
                qry.exec("PRAGMA foreign_keys = ON;");
                qry.prepare("DELETE FROM orderEcho WHERE id = :id;");
                qry.bindValue(":id", _id);
                if (qry.exec()){
                    popUp->setPopupText(tr("Documentul a fost eliminat<br>"
                                           "cu succes din baza de date."));
                    popUp->show();
                    updateTableView();
                } else {
                    QMessageBox msgBox;
                    msgBox.setWindowTitle(tr("Eliminarea documentului."));
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText(tr("Documentul nu a fost eliminat din baza de date !!!"));
                    msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
                    msgBox.setStandardButtons(QMessageBox::Ok);
                    msgBox.setStyleSheet("QPushButton{width:120px;}");
                    msgBox.exec();
                }
            }
        } else {
            qWarning(logWarning()) << tr("Solicitarea nereusita - %1").arg(qry.lastError().text());
        }
    }
}

void ListDocReportOrder::onClickBtnAddFilter()
{
    int _row = ui->tableView->currentIndex().row();
    m_currentRow = _row;
    int id_organization = proxyTable->data(proxyTable->index(_row, section_idOrganization), Qt::DisplayRole).toInt();
    setIdOrganization(id_organization);
    onClosePeriodFilter();
}

void ListDocReportOrder::onClickBtnFilterByIdOrganization()
{
    int _row = ui->tableView->currentIndex().row();
    m_currentRow = _row;
    int id_organization = proxyTable->data(proxyTable->index(_row, section_idOrganization), Qt::DisplayRole).toInt();
    setIdOrganization(id_organization);
    updateTableView();
}

void ListDocReportOrder::onClickBtnFilterRemove()
{
    ui->filterEditNumberDoc->clear();
    ui->filterComboOrganization->setCurrentIndex(0);
    ui->filterComboContract->setCurrentIndex(0);

    m_idOrganization = Enums::IDX::IDX_UNKNOW;
    m_idContract     = Enums::IDX::IDX_UNKNOW;
    updateTableView();
    updateTextPeriod();
}

void ListDocReportOrder::onClickBtnUpdateTable()
{
    updateTableView();
}

void ListDocReportOrder::onClickBtnHideShowColumn()
{
    columns = new ChoiceColumns(this);
    columns->set_attachedImages(! ui->tableView->isColumnHidden(section_attachedImages));
    columns->set_cardPayment(! ui->tableView->isColumnHidden(section_cardPayment));
    columns->set_numberDoc(! ui->tableView->isColumnHidden(section_numberDoc));
    columns->set_dateDoc(! ui->tableView->isColumnHidden(section_dateDoc));
    columns->set_idOrganization(! ui->tableView->isColumnHidden(section_idOrganization));
    columns->set_Organization(! ui->tableView->isColumnHidden(section_Organization));
    columns->set_idContract(! ui->tableView->isColumnHidden(section_idContract));
    columns->set_Contract(! ui->tableView->isColumnHidden(section_Contract));
    columns->set_idPacient(! ui->tableView->isColumnHidden(section_idPacient));
    columns->set_IDNP(! ui->tableView->isColumnHidden(section_IDNP));
    columns->set_pacient(! ui->tableView->isColumnHidden(section_pacient));
    columns->set_idUser(! ui->tableView->isColumnHidden(section_idUser));
    columns->set_user(! ui->tableView->isColumnHidden(section_user));
    columns->set_sum(! ui->tableView->isColumnHidden(section_sum));
    columns->set_comment(! ui->tableView->isColumnHidden(section_comment));
    columns->setListWidget();
    connect(columns, &ChoiceColumns::saveData, this, &ListDocReportOrder::showHideColums);
    columns->exec();
}

void ListDocReportOrder::onClickBtnPrint()
{
    if (ui->tableView->currentIndex().row() == Enums::IDX::IDX_UNKNOW){
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }
    if (m_typeDoc == orderEcho){
        int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
        docOrderEcho = new DocOrderEcho(this);
        docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
        docOrderEcho->setProperty("ItNew", false);
        docOrderEcho->setProperty("Id", _id);
        QString filePDF;
        docOrderEcho->onPrintDocument(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
        docOrderEcho->close();
    } else {
        int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id), Qt::DisplayRole).toInt();
        DocReportEcho* docReport = new DocReportEcho(this);
        docReport->setAttribute(Qt::WA_DeleteOnClose);
        docReport->setProperty("ItNew", false);
        docReport->setProperty("Id", _id);
        QString filePDF;
        docReport->onPrintDocument(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
        docReport->close();
    }
}

void ListDocReportOrder::onClickBtnSendEmail()
{
    // 📌 1 Lansam loader-ul
    loader = new ProcessingAction(this);
    loader->setAttribute(Qt::WA_DeleteOnClose);
    loader->setProperty("txtInfo", "Se descarca documentele în formatul PDF ...");
    loader->show();

    // 📌 2 verificam daca este directoriu tmp/USG
    if (! QFile(globals().main_path_save_documents).exists()) {
        QDir().mkpath(globals().main_path_save_documents);
    }

    // 📌 3 determinam id comenzii ecografice
    int _id_order = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();

    // 📌 4 Creăm thread-ul pentru trimiterea emailului
    QThread *thread = new QThread();
    HandlerFunctionThread *handler_functionThread = new HandlerFunctionThread();

    // 📌 5 Mutăm `handler_functionThread` în thread-ul nou
    handler_functionThread->moveToThread(thread);

    // 📌 6 setam `handler_functionThread` cu variabile necesrare
    handler_functionThread->setRequiredVariabile(globals().idUserApp,
                                                 globals().c_id_organizations,
                                                 globals().c_id_doctor);
    handler_functionThread->setRequiredVariableForExportDocuments(globals().thisMySQL,
                                                                  _id_order,
                                                                  -1,
                                                                  globals().unitMeasure,
                                                                  globals().c_logo_byteArray,
                                                                  globals().main_stamp_organization,
                                                                  globals().stamp_main_doctor,
                                                                  globals().signature_main_doctor,
                                                                  globals().pathTemplatesDocs,
                                                                  globals().main_path_save_documents);

    // 📌 7 Conectăm semnalele și sloturile
    connect(thread, &QThread::started, handler_functionThread, &HandlerFunctionThread::exportDocumentsToPDF);
    connect(handler_functionThread, &HandlerFunctionThread::setTextInfo, this, [this](QString txtInfo)
            {
                loader->setProperty("txtInfo", txtInfo);
            });
    connect(handler_functionThread, &HandlerFunctionThread::finishExportDocumenstToPDF, this, [this](QVector<ExportData> data_agentEmail)
            {
                agent_sendEmail = new AgentSendEmail(this);
                agent_sendEmail->setAttribute(Qt::WA_DeleteOnClose);
                for (const auto &data : data_agentEmail) {
                    agent_sendEmail->setProperty("NrOrder",     data.nr_order);
                    agent_sendEmail->setProperty("NrReport",    data.nr_report);
                    agent_sendEmail->setProperty("EmailFrom",   globals().main_email_organization);
                    agent_sendEmail->setProperty("EmailTo",     data.emailTo);
                    agent_sendEmail->setProperty("NamePatient", data.name_patient);
                    agent_sendEmail->setProperty("NameDoctor",  data.name_doctor_execute);
                    QDate _date_investigation = QDate::fromString(data.str_dateInvestigation.left(10), "dd.MM.yyyy");
                    agent_sendEmail->setProperty("DateInvestigation", _date_investigation);
                }
                agent_sendEmail->show();

                if (loader) {
                    loader->close();
                }
            });
    connect(handler_functionThread, &HandlerFunctionThread::finishExportDocumenstToPDF, thread, &QThread::quit);
    // connect(handler_functionThread, &HandlerFunctionThread::finishExportDocumenstToPDF, handler_functionThread, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // 📌 8 Pornim thread-ul
    thread->start();

}

void ListDocReportOrder::onClickBtnReport()
{
    if (ui->tableView->currentIndex().row() == Enums::IDX::IDX_UNKNOW){
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    int _id = Enums::IDX::IDX_UNKNOW;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - onClickBtnReport()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == Enums::IDX::IDX_UNKNOW || _id == Enums::IDX::IDX_WRITE){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }

    int _id_report = 0;
    QSqlQuery qry;
    qry.prepare("SELECT id FROM reportEcho WHERE id_orderEcho = :id_orderEcho;");
    qry.bindValue(":id_orderEcho", _id);
    if (qry.exec() && qry.next())
        _id_report = qry.value(0).toInt();

    if (_id_report != 0){
        DocReportEcho *report = new DocReportEcho(this);
        report->setAttribute(Qt::WA_DeleteOnClose);
        report->setProperty("ItNew", false);
        report->setProperty("Id", _id_report);
        report->setProperty("IdDocOrderEcho", _id);
        report->show();
        connect(report, &DocReportEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
    } else {

        const int _idPacient = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_idPacient), Qt::DisplayRole).toInt();

        // daca nu afost gasit documentul subaltern -> deschidem selectarea investigatiilor   
        CustomDialogInvestig* dialogInvestig = new CustomDialogInvestig(this);
        qry.prepare("SELECT cod FROM orderEchoTable WHERE id_orderEcho = :idOrderEcho;");
        qry.bindValue(":idOrderEcho", _id);
        if (qry.exec()){
            while (qry.next()) {
                QString _cod = qry.value(0).toString();

                if (! dialogInvestig->get_t_organs_internal())
                    dialogInvestig->set_t_organs_internal(code_organs_internal.contains(_cod));
                if (! dialogInvestig->get_t_urinary_system())
                    dialogInvestig->set_t_urinary_system(code_urynari_system.contains(_cod));
                if (! dialogInvestig->get_t_prostate())
                    dialogInvestig->set_t_prostate(code_prostate.contains(_cod));
                if (! dialogInvestig->get_t_gynecology())
                    dialogInvestig->set_t_gynecology(code_gynecologie.contains(_cod));
                if (! dialogInvestig->get_t_breast())
                    dialogInvestig->set_t_breast(code_breast.contains(_cod));
                if (! dialogInvestig->get_t_thyroide())
                    dialogInvestig->set_t_thyroide(code_thyroide.contains(_cod));
                if(! dialogInvestig->get_t_gestation0())
                    dialogInvestig->set_t_gestation0(code_gestation0.contains(_cod));
                if (! dialogInvestig->get_t_gestation1())
                    dialogInvestig->set_t_gestation1(code_gestation1.contains(_cod));
                if (! dialogInvestig->get_t_gestation2())
                    dialogInvestig->set_t_gestation2(code_gestation2.contains(_cod));
            }
        }
        if (dialogInvestig->exec() != QDialog::Accepted)
            return;

        // deschidem documentul -> 'raport ecografic' nou
        DocReportEcho* doc_report = new DocReportEcho(this);
        doc_report->setAttribute(Qt::WA_DeleteOnClose);
        doc_report->set_t_organs_internal(dialogInvestig->get_t_organs_internal());
        doc_report->set_t_urinary_system(dialogInvestig->get_t_urinary_system());
        doc_report->set_t_prostate(dialogInvestig->get_t_prostate());
        doc_report->set_t_gynecology(dialogInvestig->get_t_gynecology());
        doc_report->set_t_breast(dialogInvestig->get_t_breast());
        doc_report->set_t_thyroide(dialogInvestig->get_t_thyroide());
        doc_report->set_t_gestation0(dialogInvestig->get_t_gestation0());
        doc_report->set_t_gestation1(dialogInvestig->get_t_gestation1());
        doc_report->set_t_gestation2(dialogInvestig->get_t_gestation2());
        doc_report->setProperty("ItNew", true);
        doc_report->setProperty("IdPacient", _idPacient);
        doc_report->setProperty("IdDocOrderEcho", _id);
        doc_report->show();
        connect(doc_report, &DocReportEcho::PostDocument, this, &ListDocReportOrder::updateTableView);
    }
}

void ListDocReportOrder::onClickBtnShowHideViewTab()
{
    if (pressed_btn_viewTab == -1){
        pressed_btn_viewTab = 1;
        ui->groupBox_table_order->setHidden(false);
        ui->groupBox_table_report->setHidden(false);
        QModelIndex index = ui->tableView->currentIndex();
        onClickedTable(index);
        ui->splitter->resize(1518, 190);
    } else {
        pressed_btn_viewTab = -1;
        ui->groupBox_table_order->setHidden(true);
        ui->groupBox_table_report->setHidden(true);
    }
}

void ListDocReportOrder::openHistoryPatients()
{
    int _row = ui->tableView->currentIndex().row();
    const int id_patient = proxyTable->index(_row, section_idPacient).data(Qt::DisplayRole).toInt();

    patient_history = new PatientHistory(this);
    patient_history->setAttribute(Qt::WA_DeleteOnClose);
    if (id_patient > 0)
        patient_history->setProperty("IdPatient", id_patient);
    patient_history->show();
}

// **********************************************************************************
// ---

void ListDocReportOrder::filterRegExpChanged()
{
    timer->stop();
    if (m_typeDoc == orderEcho)
        proxyTable->setFilterKeyColumn(section_searchPacient);
    else
        proxyTable->setFilterKeyColumn(report_search_patient);

    QRegularExpression regExp(ui->editSearch->text(), QRegularExpression::CaseInsensitiveOption);
    proxyTable->setFilterRegularExpression(regExp);
}

void ListDocReportOrder::onClickedTable(const QModelIndex &index)
{
    // verificam indexul si daca este apasat btn 'btnViewTab'
    if (! index.isValid() || pressed_btn_viewTab == -1)
        return;

    // determinam 'id'
    int row = index.row();
    int _id = proxyTable->data(proxyTable->index(row, section_id), Qt::DisplayRole).toInt();

    // verificam 'id'
    if (_id == Enums::IDX::IDX_UNKNOW || _id == 0)
        return;

    if (m_typeDoc == TypeDoc::orderEcho){

        //-------------------------------------------
        if (model_view_table_order->rowCount() > 0)
            model_view_table_order->clear();
        QString str_order = QString("SELECT * FROM orderEchoTable WHERE id_orderEcho = '%1' AND deletionMark = '2';").arg(_id);
        model_view_table_order->setQuery(str_order);
        ui->view_table->setModel(model_view_table_order);

        ui->view_table->hideColumn(0);
        ui->view_table->hideColumn(1);
        ui->view_table->hideColumn(2);
        ui->view_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
        ui->view_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->view_table->verticalHeader()->setDefaultSectionSize(16);
        ui->view_table->setColumnWidth(3, 70);  // codul
        ui->view_table->setColumnWidth(4, 480); // denuimirea investigatiei

        //-------------------------------------------
        if (model_view_table_report->rowCount() > 0)
            model_view_table_report->clear();
        QString str_report = QString("SELECT id,concluzion FROM reportEcho WHERE id_orderEcho = '%1' AND deletionMark = '2';").arg(_id);
        model_view_table_report->setQuery(str_report);
        if (model_view_table_report->rowCount() > 0){
            if (ui->view_table_report->isHidden()){
                ui->view_table_report->setHidden(false);
                ui->btn_open_report->setHidden(false);
                ui->btn_print_report->setHidden(false);
            }
            ui->view_table_report->setModel(model_view_table_report);
            ui->text_empty_report->setHidden(true);
        } else {
            ui->view_table_report->setHidden(true);
            ui->btn_open_report->setHidden(true);
            ui->btn_print_report->setHidden(true);
            ui->text_empty_report->setHidden(false);
        }
        ui->view_table_report->hideColumn(0);
        ui->view_table_report->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
        ui->view_table_report->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->view_table_report->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei

    } else if (m_typeDoc == TypeDoc::reportEcho) {

        //-------------------------------------------
        if (model_view_table_order->rowCount() > 0)
            model_view_table_order->clear();
        QString str_order = QString("SELECT * FROM orderEchoTable WHERE id_orderEcho = '%1' AND deletionMark = '2';")
                .arg(proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt());
        model_view_table_order->setQuery(str_order);
        ui->view_table->setModel(model_view_table_order);

        ui->view_table->hideColumn(0);
        ui->view_table->hideColumn(1);
        ui->view_table->hideColumn(2);
        ui->view_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
        ui->view_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->view_table->verticalHeader()->setDefaultSectionSize(16);
        ui->view_table->setColumnWidth(3, 70);  // codul
        ui->view_table->setColumnWidth(4, 480); // denuimirea investigatiei
        ui->view_table->setHidden(false);

        //-------------------------------------------
        if (model_view_table_report->rowCount() > 0)
            model_view_table_report->clear();
        QString str_report = QString("SELECT concluzion FROM reportEcho WHERE id = '%1' AND deletionMark = '2';").arg(_id);
        model_view_table_report->setQuery(str_report);
        if (model_view_table_report->rowCount() > 0){
            if (ui->view_table_report->isHidden()){
                ui->view_table_report->setHidden(false);
                ui->btn_open_report->setHidden(false);
                ui->btn_print_report->setHidden(false);
            }
            ui->view_table_report->setModel(model_view_table_report);
            ui->text_empty_report->setHidden(true);
        } else {
            ui->view_table_report->setHidden(true);
            ui->btn_open_report->setHidden(true);
            ui->btn_print_report->setHidden(true);
            ui->text_empty_report->setHidden(false);
        }
        ui->view_table_report->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
        ui->view_table_report->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->view_table_report->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei

    } else {
        qWarning(logWarning()) << QString("%1 - onClickedTable()").arg(metaObject()->className())
                               << tr("Nu este setata(sau setata incorect) variabila 'm_typeDoc' !!!");
    }
}

void ListDocReportOrder::onDoubleClickedTable(const QModelIndex &index)
{
    Q_UNUSED(index);
    onClickBtnEdit();
}

// **********************************************************************************
// --- meniu contextual si actiunile

void ListDocReportOrder::slotContextMenuRequested(QPoint pos)
{
    int _row = ui->tableView->currentIndex().row();
    const QString _nameOrganization = proxyTable->index(_row, section_Organization).data(Qt::DisplayRole).toString();
    const QString _nameContract     = proxyTable->index(_row, section_Contract).data(Qt::DisplayRole).toString();
    const QString _namePacient      = proxyTable->index(_row, section_pacient).data(Qt::DisplayRole).toString();
    const int _id                   = proxyTable->index(_row, section_id).data(Qt::DisplayRole).toInt();
    int _id_report                  = Enums::IDX::IDX_UNKNOW;

    menu->clear();

    QAction* actionOpenDoc         = new QAction(QIcon(":/img/open-search.png"), tr("Deschide documentul"), this);
    QAction* actionPrintDoc        = new QAction(QIcon(":/img/print.png"), tr("Printare documentului"), this);
    QAction* actionDeletionMarkDoc = new QAction(QIcon(":/img/clear_x32.png"), tr("Eliminare documentului"), this);

    QAction* actionEditOrganization = new QAction(QIcon(":/img/company_x32.png"), tr("Editeaza persoana juridica '%1'.").arg(_nameOrganization), this);
    QAction* actionEditContract     = new QAction(QIcon(":/img/company_x32.png"), tr("Editeaza contractul '%1'.").arg(_nameContract), this);
    QAction* actionEditPacient      = new QAction(QIcon(":/img/pacient_x32.png"), tr("Editeaza datele pacientului '%1'.").arg(_namePacient), this);

    QAction* actionHistoryPatient   = new QAction(QIcon(":/img/medical-history.png"), tr("Deschide 'Istoria pacientului' - '%1;.").arg(_namePacient), this);

    connect(actionOpenDoc, &QAction::triggered, this, &ListDocReportOrder::onClickBtnEdit);
    connect(actionPrintDoc, &QAction::triggered, this, &ListDocReportOrder::onClickBtnPrint);
    connect(actionDeletionMarkDoc, &QAction::triggered, this, &ListDocReportOrder::onClickBtnDeletion);

    connect(actionEditOrganization, &QAction::triggered, this, &ListDocReportOrder::onActionEditOrganization);
    connect(actionEditContract, &QAction::triggered, this, &ListDocReportOrder::onActionEditContract);
    connect(actionEditPacient, &QAction::triggered, this, &ListDocReportOrder::onActionEditPacient);

    connect(actionHistoryPatient, &QAction::triggered, this, &ListDocReportOrder::openHistoryPatients);

    menu->addAction(actionOpenDoc);
    menu->addAction(actionPrintDoc);
    menu->addAction(actionDeletionMarkDoc);
    menu->addSeparator();
    menu->addAction(actionEditOrganization);
    menu->addAction(actionEditContract);
    menu->addSeparator();
    menu->addAction(actionEditPacient);
    menu->addSeparator();
    menu->addAction(actionHistoryPatient);

    if (m_typeDoc == orderEcho){
        QString presentationDoc = nullptr;
        QSqlQuery qry;
        qry.prepare("SELECT id, numberDoc, dateDoc "
                    "FROM reportEcho "
                    "WHERE id_orderEcho = :id_orderEcho;");
        qry.bindValue(":id_orderEcho", _id);
        if (qry.exec()){
            qry.next();
            _id_report = qry.value(0).toInt();
            if (globals().thisMySQL){
                static const QRegularExpression replaceT("T");
                static const QRegularExpression removeMilliseconds("\\.000");
                presentationDoc = tr("Raport ecografic nr.%1 din %2")
                                      .arg(qry.value(1).toString(),
                                           qry.value(2).toString()
                                               .replace(replaceT, " ")
                                               .replace(removeMilliseconds,""));
            } else {
                presentationDoc = tr("Raport ecografic nr.%1 din %2").arg(qry.value(1).toString(), qry.value(2).toString());
            }
        } else {
            qDebug() << qry.lastError().text();
        }

        if (_id_report != Enums::IDX::IDX_UNKNOW && presentationDoc != nullptr){
            QMenu* menuDocSubordonat = new QMenu(menu);
            menuDocSubordonat->setTitle(tr("Documente subordonate"));
            QAction* actionOpenDocReport = new QAction(QIcon(":/img/open-search.png"), tr("Editare - %1").arg(presentationDoc), menu);
            QAction* actionPrintDocReport = new QAction(QIcon(":/img/print.png"), tr("Printare - %1").arg(presentationDoc), menu);
            connect(actionOpenDocReport, &QAction::triggered, this, &ListDocReportOrder::onClickBtnReport);
            connect(actionPrintDocReport, &QAction::triggered, this, [=]()
                    {
                        DocReportEcho* docReport = new DocReportEcho(this);
                        docReport->setAttribute(Qt::WA_DeleteOnClose);
                        docReport->setProperty("ItNew", false);
                        docReport->setProperty("Id",    _id_report);
                        QString filePDF;
                        docReport->onPrintDocument(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
                    });
            menuDocSubordonat->addAction(actionOpenDocReport);
            menuDocSubordonat->addAction(actionPrintDocReport);
            menu->addSeparator();
            menu->addMenu(menuDocSubordonat);
        }
    }
    menu->popup(ui->tableView->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

void ListDocReportOrder::onActionEditOrganization()
{
    int _row = ui->tableView->currentIndex().row();
    int _id  = proxyTable->index(_row, section_idOrganization).data(Qt::DisplayRole).toInt();

    CatOrganizations* catOrganization = new CatOrganizations(this);
    catOrganization->setAttribute(Qt::WA_DeleteOnClose);
    catOrganization->setProperty("ItNew", false);
    catOrganization->setProperty("Id", _id);
    catOrganization->show();
}

void ListDocReportOrder::onActionEditContract()
{
    int _row = ui->tableView->currentIndex().row();
    int _id  = proxyTable->index(_row, section_idContract).data(Qt::DisplayRole).toInt();
    int _idOrganization = proxyTable->index(_row, section_idOrganization).data(Qt::DisplayRole).toInt();

    CatContracts* catContract = new CatContracts(this);
    catContract->setAttribute(Qt::WA_DeleteOnClose);
    catContract->setProperty("ItNew", false);
    catContract->setProperty("Id", _id);
    catContract->setProperty("IdOrganization", -_idOrganization);
    catContract->show();
}

void ListDocReportOrder::onActionEditPacient()
{
    int _row = ui->tableView->currentIndex().row();
    int _id  = proxyTable->index(_row, section_idPacient).data(Qt::DisplayRole).toInt();

    CatGeneral* catPacient = new CatGeneral(this);
    catPacient->setAttribute(Qt::WA_DeleteOnClose);
    catPacient->setProperty("itNew", false);
    catPacient->setProperty("typeCatalog", CatGeneral::TypeCatalog::Pacients);
    catPacient->setProperty("Id", _id);
    catPacient->show();
}

void ListDocReportOrder::showHideColums()
{
    timer->stop();

    if (columns->get_attachedImages()){
        ui->tableView->showColumn(section_attachedImages);
        ui->tableView->setColumnWidth(section_attachedImages, 15);
    } else {
        ui->tableView->hideColumn(section_attachedImages);
    }

    if (columns->get_cardPayment()){
        ui->tableView->showColumn(section_cardPayment);
        ui->tableView->setColumnWidth(section_cardPayment, 15);
    } else {
        ui->tableView->hideColumn(section_cardPayment);
    }

    if (columns->get_numberDoc()){
        ui->tableView->showColumn(section_numberDoc);
        ui->tableView->setColumnWidth(section_numberDoc, 70);
    } else {
        ui->tableView->hideColumn(section_numberDoc);
    }

    if (columns->get_dateDoc()){
        ui->tableView->showColumn(section_dateDoc);
        ui->tableView->setColumnWidth(section_dateDoc, 153);
    } else {
        ui->tableView->hideColumn(section_dateDoc);
    }

    if (columns->get_idOrganization()){
        ui->tableView->showColumn(section_idOrganization);
        ui->tableView->setColumnWidth(section_idOrganization, 15);
    } else {
        ui->tableView->hideColumn(section_idOrganization);
    }

    if (columns->get_Organization()){
        ui->tableView->showColumn(section_Organization);
        ui->tableView->setColumnWidth(section_Organization, 136);
    } else {
        ui->tableView->hideColumn(section_Organization);
    }

    if (columns->get_idContract()){
        ui->tableView->showColumn(section_idContract);
        ui->tableView->setColumnWidth(section_idContract, 15);
    } else {
        ui->tableView->hideColumn(section_idContract);
    }

    if (columns->get_Contract()){
        ui->tableView->showColumn(section_Contract);
        ui->tableView->setColumnWidth(section_Contract, 132);
    } else {
        ui->tableView->hideColumn(section_Contract);
    }

    if (columns->get_idPacient()){
        ui->tableView->showColumn(section_idPacient);
        ui->tableView->setColumnWidth(section_idPacient, 15);
    } else {
        ui->tableView->hideColumn(section_idPacient);
    }

    if (columns->get_IDNP()){
        ui->tableView->showColumn(section_IDNP);
        ui->tableView->setColumnWidth(section_IDNP, 155);
    } else {
        ui->tableView->hideColumn(section_IDNP);
    }

    if (columns->get_pacient()){
        ui->tableView->showColumn(section_pacient);
        ui->tableView->setColumnWidth(section_pacient, 160);
    } else {
        ui->tableView->hideColumn(section_pacient);
    }

    if (columns->get_idUser()){
        ui->tableView->showColumn(section_idUser);
        ui->tableView->setColumnWidth(section_idUser, 15);
    } else {
        ui->tableView->hideColumn(section_idUser);
    }

    if (columns->get_user()){
        ui->tableView->showColumn(section_user);
        ui->tableView->setColumnWidth(section_user, 64);
    } else {
        ui->tableView->hideColumn(section_user);
    }

    if (columns->get_sum()){
        ui->tableView->showColumn(section_sum);
        ui->tableView->setColumnWidth(section_sum, 65);
    } else {
        ui->tableView->hideColumn(section_sum);
    }

    if (columns->get_comment()){
        ui->tableView->showColumn(section_comment);
        ui->tableView->setColumnWidth(section_comment, 300);
    } else {
        ui->tableView->hideColumn(section_comment);
    }

//    timer->start(globals().updateIntervalListDoc * 1000);

    columns->close();
}

QString ListDocReportOrder::enumToString(TypeDoc typeDoc)
{
    switch (typeDoc) {
    case TypeDoc::orderEcho:  return "orderEcho";
    case TypeDoc::reportEcho: return "reportEcho";
    default: return "unknow";
    }
}

// **********************************************************************************
// --- initierea butoanelor tool bar-ului + butoanele filtrului

void ListDocReportOrder::initBtnToolBar()
{

    // ----------------------------------------------------------------------------
    // instalam filtrul evenimentelor pu prezentarea indiciilor (подсказки)

    ui->btnAdd->setMouseTracking(true);
    ui->btnAdd->installEventFilter(this);
    ui->btnEdit->setMouseTracking(true);
    ui->btnEdit->installEventFilter(this);
    ui->btnDeletion->setMouseTracking(true);
    ui->btnDeletion->installEventFilter(this);
    ui->btnAddFilter->setMouseTracking(true);
    ui->btnAddFilter->installEventFilter(this);
    ui->btnFilter->setMouseTracking(true);
    ui->btnFilter->installEventFilter(this);
    ui->btnFilterRemove->setMouseTracking(true);
    ui->btnFilterRemove->installEventFilter(this);
    ui->btnUpdateTable->setMouseTracking(true);
    ui->btnUpdateTable->installEventFilter(this);
    ui->btnPrint->setMouseTracking(true);
    ui->btnPrint->installEventFilter(this);
    ui->btnSendEmail->setMouseTracking(true);
    ui->btnSendEmail->installEventFilter(this);
    ui->btnReport->setMouseTracking(true);
    ui->btnReport->installEventFilter(this);
    ui->btnViewTab->setMouseTracking(true);
    ui->btnViewTab->installEventFilter(this);
    ui->btnPeriodDate->setMouseTracking(true);
    ui->btnPeriodDate->installEventFilter(this);
    ui->btnSearch->setMouseTracking(true);
    ui->btnSearch->installEventFilter(this);
    ui->btnHideShowColumn->setMouseTracking(true);
    ui->btnHideShowColumn->installEventFilter(this);

    QString style_toolButton = db->getStyleForToolButton();
    ui->btnReport->setStyleSheet(style_toolButton);
    ui->btnViewTab->setStyleSheet(style_toolButton);

    QString style_btnToolBar = db->getStyleForButtonToolBar();
    ui->btnAdd->setStyleSheet(style_btnToolBar);
    ui->btnEdit->setStyleSheet(style_btnToolBar);
    ui->btnDeletion->setStyleSheet(style_btnToolBar);
    ui->btnAddFilter->setStyleSheet(style_btnToolBar);
    ui->btnFilter->setStyleSheet(style_btnToolBar);
    ui->btnFilterRemove->setStyleSheet(style_btnToolBar);
    ui->btnUpdateTable->setStyleSheet(style_btnToolBar);
    ui->btnPrint->setStyleSheet(style_btnToolBar);
    ui->btnSendEmail->setStyleSheet(style_btnToolBar);
    ui->btnPeriodDate->setStyleSheet(style_btnToolBar);
    ui->btnSearch->setStyleSheet(style_btnToolBar);
    ui->btnHideShowColumn->setStyleSheet(style_btnToolBar);

    ui->editSearch->setEnabled(false);
    ui->editSearch->setPlaceholderText(tr("... c\304\203utarea dup\304\203 nume, prenume sau IDNP a pacientului"));

    updateTextPeriod();

    connect(ui->btnAdd, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnAdd);
    connect(ui->btnEdit, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnEdit);
    connect(ui->btnDeletion, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnDeletion);
    connect(ui->btnAddFilter, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnAddFilter);
    connect(ui->btnFilter, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnFilterByIdOrganization);
    connect(ui->btnFilterRemove, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnFilterRemove);
    connect(ui->btnUpdateTable, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnUpdateTable);
    connect(ui->btnHideShowColumn, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnHideShowColumn);
    connect(ui->btnPrint, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnPrint);
    connect(ui->btnSendEmail, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnSendEmail);
    connect(ui->btnReport, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnReport);

    connect(ui->btnSearch, &QToolButton::clicked, this, &ListDocReportOrder::enableLineEditSearch);
    connect(ui->btnPeriodDate, &QToolButton::clicked, this, &ListDocReportOrder::onClosePeriodFilter);

    connect(ui->editSearch, &QLineEdit::textChanged, this, &ListDocReportOrder::filterRegExpChanged);
    connect(ui->editSearch, &QLineEdit::textEdited, this, [this]()
            {
                timer->start(globals().updateIntervalListDoc * 1000);
            });
    connect(ui->btnViewTab, &QToolButton::clicked, this, &ListDocReportOrder::onClickBtnShowHideViewTab);

    // ----------------------------------------------------------------------------
    // meniu btn print order

    formationPrinMenuForOrder();
    formationPrinMenuForReport();

}

void ListDocReportOrder::initBtnFilter()
{
    QString style_toolButton = db->getStyleForToolButton();
    ui->btnChoicePeriod->setStyleSheet(style_toolButton);
    ui->btnOpenCatContract->setStyleSheet(style_toolButton);
    ui->btnOpenCatOrganization->setStyleSheet(style_toolButton);

    connect(ui->btnChoicePeriod, &QPushButton::clicked, this, [this]()
    {
        customPeriod->setDateStart(ui->filterStartDateTime->date());
        customPeriod->setDateEnd(ui->filterEndDateTime->date());
        customPeriod->show();

        timer->stop(); // la prezentarea alegerii perioadei oprim timer actualizarii listei de documente
        disconnect(ui->filterStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onStartDateTimeChanged); // deconectam conexiunile pu a exclude controlul perioadei
        disconnect(ui->filterEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onEndDateTimeChanged);     // vezi function - onStartDateTimeChanged()

        connect(customPeriod, &CustomPeriod::mChangePeriod, this, [this]()
        {
            QDateTime _startDate = customPeriod->getDateStart();
            QDateTime _endDate   = customPeriod->getDateEnd();

            ui->filterStartDateTime->setDateTime(_startDate);
            ui->filterEndDateTime->setDateTime(_endDate);
            updateTextPeriod();

            connect(ui->filterStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onStartDateTimeChanged);
            connect(ui->filterEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &ListDocReportOrder::onEndDateTimeChanged);
            timer->start(globals().updateIntervalListDoc * 1000);
        });

    });

    connect(ui->btnOpenCatOrganization, &QPushButton::clicked, this, [this]()
    {
        int id_organization = ui->filterComboOrganization->itemData(ui->filterComboOrganization->currentIndex(), Qt::UserRole).toInt();
        if (id_organization == 0)
            return;
        CatOrganizations* catOrganization = new CatOrganizations(this);
        catOrganization->setAttribute(Qt::WA_DeleteOnClose);
        catOrganization->setProperty("ItNew", false);
        catOrganization->setProperty("Id", id_organization);
        catOrganization->show();
    });

    connect(ui->btnOpenCatContract, &QPushButton::clicked, this, [this]()
    {
        int id_organization = ui->filterComboOrganization->itemData(ui->filterComboOrganization->currentIndex(), Qt::UserRole).toInt();
        int id_contract = ui->filterComboContract->itemData(ui->filterComboContract->currentIndex(), Qt::UserRole).toInt();
        if (id_contract == 0)
            return;
        CatContracts* catContract = new CatContracts(this);
        catContract->setAttribute(Qt::WA_DeleteOnClose);
        catContract->setProperty("ItNew", false);
        catContract->setProperty("Id", id_contract);
        catContract->setProperty("IdOrganization", id_organization);
        catContract->show();
    });

    connect(ui->btnFilterApply, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnFilterApply);
    connect(ui->btnFilterClear, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnFilterClear);
    connect(ui->btnFilterClose, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnFilterClose);
}

// **********************************************************************************
// --- procesarea perioadei

void ListDocReportOrder::updateTextPeriod()
{
    ui->lablePeriodDate->setText(tr("Perioada: ") +
                             ui->filterStartDateTime->dateTime().toString("dd.MM.yyyy") + " - " +
                             ui->filterEndDateTime->dateTime().toString("dd.MM.yyyy"));
    if (ui->filterComboOrganization->currentIndex() > 0){
        QString str = ui->lablePeriodDate->text();
        str = str + tr("; filtru: ") + ui->filterComboOrganization->currentText();
        ui->lablePeriodDate->setText(str);
    }
    if (ui->filterComboContract->currentIndex() > 0){
        QString str = ui->lablePeriodDate->text();
        str = str + " (" + ui->filterComboContract->currentText() +")";
        ui->lablePeriodDate->setText(str);
    }
}

void ListDocReportOrder::openDocOrderEchoByClickBtnTable()
{
    int _id = Enums::IDX::IDX_UNKNOW;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == Enums::IDX::IDX_UNKNOW || _id == Enums::IDX::IDX_WRITE){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }

    docOrderEcho = new DocOrderEcho(this);
    docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
    docOrderEcho->setProperty("ItNew", false);
    docOrderEcho->setProperty("Id", _id);
    docOrderEcho->show();
}

// **********************************************************************************
// --- procesarea printarii

void ListDocReportOrder::openPrintDesignerPreviewOrder(bool preview)
{
    int _id = Enums::IDX::IDX_UNKNOW;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == Enums::IDX::IDX_UNKNOW || _id == Enums::IDX::IDX_WRITE){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }
    docOrderEcho = new DocOrderEcho(this);
    docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
    docOrderEcho->setProperty("ItNew", false);
    docOrderEcho->setProperty("Id", _id);
    QString filePDF;
    if (preview)
        docOrderEcho->onPrintDocument(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
    else
        docOrderEcho->onPrintDocument(Enums::TYPE_PRINT::OPEN_DESIGNER, filePDF);
}

void ListDocReportOrder::openPrintDesignerPreviewReport(bool preview)
{
    int id_doc = Enums::IDX::IDX_UNKNOW;
    if (m_typeDoc == TypeDoc::orderEcho)
        id_doc = model_view_table_report->data(model_view_table_report->index(0, 0), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        id_doc = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewReport()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (id_doc == Enums::IDX::IDX_UNKNOW || id_doc == Enums::IDX::IDX_WRITE){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewReport()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }
    DocReportEcho* docReport = new DocReportEcho(this);
    docReport->setAttribute(Qt::WA_DeleteOnClose);
    docReport->setProperty("ItNew", false);
    docReport->setProperty("Id",    id_doc);
    QString filePDF;
    if (preview)
        docReport->onPrintDocument(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
    else
        docReport->onPrintDocument(Enums::TYPE_PRINT::OPEN_DESIGNER, filePDF);
}

void ListDocReportOrder::formationPrinMenuForOrder()
{
    db->updateVariableFromTableSettingsUser();

    if (ui->btn_print_order->menu() != nullptr) // daca a fost setat meniu
        ui->btn_print_order->menu()->clear();   // golim

    if (globals().showDesignerMenuPrint){
        QAction *openDesignerOrder = new QAction(setUpMenu_order);
        openDesignerOrder->setIcon(QIcon(":/images/design.png"));
        openDesignerOrder->setText(tr("Deschide designer"));

        QAction *openPreviewOrder  = new QAction(setUpMenu_order);
        openPreviewOrder->setIcon(QIcon(":/images/Print.png"));
        openPreviewOrder->setText(tr("Deschide preview"));

        setUpMenu_order->clear();
        setUpMenu_order->addAction(openDesignerOrder);
        setUpMenu_order->addAction(openPreviewOrder);
        setUpMenu_order->setWindowFlags(setUpMenu_report->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu_order->setAttribute(Qt::WA_TranslucentBackground);
        setUpMenu_order->setStyleSheet("QMenu "
                                       "{"
                                       "  border-radius: 6px; "
                                       "  font-family: 'Segoe UI', 'San Francisco'; "
                                       "  font-size: 14px;"
                                       "}"
                                       "QMenu::item "
                                       "{"
                                       "  height: 25px; "
                                       "  width: 150px; "
                                       "  border: none;"
                                       "  padding-left: 4px 8px;" // Adaugă un pic de spațiere internă
                                       "}"
                                       "QMenu::item:selected "
                                       "{"
                                       "  background-color: #0078d7; " // Fundal albastru
                                       "  color: white; "              // Text alb
                                       "  border-radius: 4px;"         // Colțuri rotunjite pentru element
                                       "}");

        ui->btn_print_order->setMenu(setUpMenu_order);

        connect(openDesignerOrder, &QAction::triggered, this, [this]()
                {
                    openPrintDesignerPreviewOrder(false);
                });

        connect(openPreviewOrder, &QAction::triggered, this, [this]()
                {
                    openPrintDesignerPreviewOrder(true);
                });
    } else {
        connect(ui->btn_print_order, &QPushButton::clicked, this, [this]()
                {
                    openPrintDesignerPreviewOrder(true);
                });
    }

    connect(ui->btn_open_order, &QPushButton::clicked, this, &ListDocReportOrder::openDocOrderEchoByClickBtnTable);
}

void ListDocReportOrder::formationPrinMenuForReport()
{
    db->updateVariableFromTableSettingsUser();

    if (ui->btn_open_report->menu() != nullptr) // daca este setata menii
        ui->btn_open_report->menu()->clear();   // golim

    if (globals().showDesignerMenuPrint){
        QAction *openDesignerReport = new QAction(setUpMenu_report);
        openDesignerReport->setIcon(QIcon(":/images/design.png"));
        openDesignerReport->setText(tr("Deschide designer"));

        QAction *openPreviewReport  = new QAction(setUpMenu_report);
        openPreviewReport->setIcon(QIcon(":/images/Print.png"));
        openPreviewReport->setText(tr("Deschide preview"));

        setUpMenu_report->clear();
        setUpMenu_report->addAction(openDesignerReport);
        setUpMenu_report->addAction(openPreviewReport);
        setUpMenu_report->setWindowFlags(setUpMenu_report->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu_report->setAttribute(Qt::WA_TranslucentBackground);
        setUpMenu_report->setStyleSheet("QMenu "
                                        "{"
                                        "  border-radius: 6px; "
                                        "  font-family: 'Segoe UI', 'San Francisco'; "
                                        "  font-size: 14px;"
                                        "}"
                                        "QMenu::item "
                                        "{"
                                        "  height: 25px; "
                                        "  width: 150px; "
                                        "  border: none;"
                                        "  padding: 4px 8px;" // Adaugă un pic de spațiere internă
                                        "}"
                                        "QMenu::item:selected "
                                        "{"
                                        "  background-color: #0078d7; " // Fundal albastru
                                        "  color: white; "              // Text alb
                                        "  border-radius: 4px;"         // Colțuri rotunjite pentru element
                                        "}");

        ui->btn_print_report->setMenu(setUpMenu_report);

        connect(openDesignerReport, &QAction::triggered, this, [this]()
                {
                    openPrintDesignerPreviewReport(false);
                });

        connect(openPreviewReport, &QAction::triggered, this, [this]()
                {
                    openPrintDesignerPreviewReport();
                });
    } else {
        connect(ui->btn_print_report, &QPushButton::clicked, this, [this]()
                {
                    openPrintDesignerPreviewReport();
                });
    }

    connect(ui->btn_open_report, &QPushButton::clicked, this, &ListDocReportOrder::onClickBtnReport);
}

// **********************************************************************************
// --- actualizarea solicitarilor tabelelor, textului

void ListDocReportOrder::updateTableView()
{
    if (m_typeDoc == unknowDoc){
        qWarning(logWarning()) << this->metaObject()->className()
                               << ": nu este determinata proprietatea 'typeDoc' !!! Completarea tabelei nu este posibila.";
        return;
    }
    if (m_typeDoc == orderEcho)
        updateTableViewOrderEcho();
     else if (m_typeDoc == reportEcho)
        updateTableViewReportEcho();
}

void ListDocReportOrder::updateTableViewOrderEcho()
{
    m_currentRow = ui->tableView->currentIndex().row();

    QString strQry;
    if (globals().thisMySQL)
        strQry = "SELECT orderEcho.id,"
                 "orderEcho.deletionMark,"
                 "orderEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "orderEcho.numberDoc,"
                 "orderEcho.dateDoc,"
                 "organizations.id AS idOrganization,"
                 "organizations.name AS organization,"
                 "contracts.id AS idContract,"
                 "contracts.name AS Contract,"
                 "pacients.id AS idPacient,"
                 "fullNamePacients.nameBirthday AS searchPacients,"
                 "fullNamePacients.name AS pacient,"
                 "pacients.IDNP,"
                 "fullNameDoctors.nameAbbreviated AS doctor,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                 "INNER JOIN fullNamePacients ON orderEcho.id_pacients = fullNamePacients.id_pacients "
                 "LEFT JOIN fullNameDoctors ON orderEcho.id_doctors = fullNameDoctors.id "
                 "INNER JOIN users ON orderEcho.id_users = users.id";
    else
        strQry = "SELECT orderEcho.id,"
                 "orderEcho.deletionMark,"
                 "orderEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "orderEcho.numberDoc,"
                 "orderEcho.dateDoc,"
                 "organizations.id AS idOrganization,"
                 "organizations.name AS organization,"
                 "contracts.id AS idContract,"
                 "contracts.name AS Contract,"
                 "pacients.id AS idPacient,"
                 "pacients.name ||' '|| pacients.fName ||' '|| pacients.mName ||', '|| "
                 "substr(pacients.birthday, 9, 2) ||'.'|| "
                 "substr(pacients.birthday, 6, 2) ||'.'|| "
                 "substr(pacients.birthday, 1, 4) AS searchPacients,"
                 "pacients.name ||' '|| pacients.fName AS pacient,"
                 "pacients.IDNP,"
                 "fullNameDoctors.nameAbbreviated AS doctor,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                 "LEFT JOIN fullNameDoctors ON orderEcho.id_doctors = fullNameDoctors.id "
                 "INNER JOIN users ON orderEcho.id_users = users.id";
    if (! m_numberDoc.isEmpty() || m_idOrganization > 0 || m_idContract > 0)
        strQry = strQry + " WHERE ";
    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString("orderEcho.numberDoc = '%1'").arg(m_numberDoc);
    if (m_idOrganization > 0){
        if (! m_numberDoc.isEmpty())
            strQry = strQry + " AND ";
        strQry = strQry + QString("orderEcho.id_organizations = '%1'").arg(m_idOrganization);
    }
    if (m_idContract > 0){
        if (! m_numberDoc.isEmpty() || m_idOrganization > 0)
            strQry = strQry + " AND ";
        strQry = strQry + QString("orderEcho.id_contracts = '%1'").arg(m_idContract);
    }

    QString startDate = ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDate   = ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (! m_numberDoc.isEmpty() || m_idOrganization > 0 || m_idContract > 0)
        strQry = strQry + QString(" AND orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);
    else
        strQry = strQry + QString(" WHERE orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);

    strQry = strQry + " ORDER BY orderEcho.dateDoc DESC "
                      " LIMIT :limit OFFSET :offset;";

#ifdef QT_DEBUG
    qDebug() << "----------------------------------------------------------------------------------------------------------'";
    qDebug() << "Solicitarea pentru popularea listei de documente pe perioada:" + startDate + " - " + endDate;
    qDebug() << strQry;
#else
    if (QCoreApplication::arguments().count() > 1
        && QCoreApplication::arguments()[1].contains("/debug")){
        qDebug() << strQry;
    }
#endif

    int scrollPosition = ui->tableView->verticalScrollBar()->value();
    int maxScroll = ui->tableView->verticalScrollBar()->maximum();

    int rowHeight = ui->tableView->rowHeight(0); // Calculăm înălțimea rândului
    int startRow = 0;
    if (rowHeight > 0)
        startRow = scrollPosition / rowHeight;
    else
        Q_UNUSED(startRow)

    if (scrollPosition == 0) {
        modelTable->resetPagination();    // resetarea
        modelTable->setPagination(50, 0); // setarea initiala paginarii
        modelTable->setStrQuery(strQry);  // setarea textului solicitarii
        modelTable->fetchMoreData();      // prelucrarea solicitarii si prezentarea datelor
    } else if (scrollPosition < maxScroll) {
        modelTable->updateVisibleData(scrollPosition, rowHeight);
    }

    proxyTable->setSourceModel(modelTable);
    ui->tableView->setModel(proxyTable);
    ui->tableView->hideColumn(section_id);
    ui->tableView->hideColumn(section_idOrganization);
    ui->tableView->hideColumn(section_idContract);
    ui->tableView->hideColumn(section_idPacient);
    ui->tableView->hideColumn(section_searchPacient);
    ui->tableView->hideColumn(section_idUser);    
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu); // initializam meniu contextual

    if (m_currentRow != Enums::IDX::IDX_UNKNOW)
        ui->tableView->selectRow(m_currentRow);
    else
        ui->tableView->selectRow(0);
    updateHeaderTableOrderEcho();
}

void ListDocReportOrder::updateTableViewOrderEchoFull()
{
    QString strQry;
    if (globals().thisMySQL)
        strQry = "SELECT orderEcho.id,"
                 "orderEcho.deletionMark,"
                 "orderEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "orderEcho.numberDoc,"
                 "orderEcho.dateDoc,"
                 "organizations.id AS idOrganization,"
                 "organizations.name AS organization,"
                 "contracts.id AS idContract,"
                 "contracts.name AS Contract,"
                 "pacients.id AS idPacient,"
                 "fullNamePacients.nameBirthday AS searchPacients,"
                 "fullNamePacients.name AS pacient,"
                 "pacients.IDNP,"
                 "fullNameDoctors.nameAbbreviated AS doctor,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                 "INNER JOIN fullNamePacients ON orderEcho.id_pacients = fullNamePacients.id_pacients "
                 "LEFT JOIN fullNameDoctors ON orderEcho.id_doctors = fullNameDoctors.id "
                 "INNER JOIN users ON orderEcho.id_users = users.id";
    else
        strQry = "SELECT orderEcho.id,"
                 "orderEcho.deletionMark,"
                 "orderEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "orderEcho.numberDoc,"
                 "orderEcho.dateDoc,"
                 "organizations.id AS idOrganization,"
                 "organizations.name AS organization,"
                 "contracts.id AS idContract,"
                 "contracts.name AS Contract,"
                 "pacients.id AS idPacient,"
                 "pacients.name ||' '|| pacients.fName ||' '|| pacients.mName ||', '|| "
                 "substr(pacients.birthday, 9, 2) ||'.'|| "
                 "substr(pacients.birthday, 6, 2) ||'.'|| "
                 "substr(pacients.birthday, 1, 4) AS searchPacients,"
                 "pacients.name ||' '|| pacients.fName AS pacient,"
                 "pacients.IDNP,"
                 "fullNameDoctors.nameAbbreviated AS doctor,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                 "LEFT JOIN fullNameDoctors ON orderEcho.id_doctors = fullNameDoctors.id "
                 "INNER JOIN users ON orderEcho.id_users = users.id";
    if (! m_numberDoc.isEmpty() || m_idOrganization > 0 || m_idContract > 0)
        strQry = strQry + " WHERE ";
    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString("orderEcho.numberDoc = '%1'").arg(m_numberDoc);
    if (m_idOrganization > 0){
        if (! m_numberDoc.isEmpty())
            strQry = strQry + " AND ";
        strQry = strQry + QString("orderEcho.id_organizations = '%1'").arg(m_idOrganization);
    }
    if (m_idContract > 0){
        if (! m_numberDoc.isEmpty() || m_idOrganization > 0)
            strQry = strQry + " AND ";
        strQry = strQry + QString("orderEcho.id_contracts = '%1'").arg(m_idContract);
    }

    QString startDate = ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDate   = ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (! m_numberDoc.isEmpty() || m_idOrganization > 0 || m_idContract > 0)
        strQry = strQry + QString(" AND orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);
    else
        strQry = strQry + QString(" WHERE orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);

    strQry = strQry + " ORDER BY orderEcho.dateDoc DESC;";

#ifdef QT_DEBUG
    qDebug() << "----------------------------------------------------------------------------------------------------------'";
    qDebug() << "Solicitarea pentru popularea listei de documente pe perioada:" + startDate + " - " + endDate;
    qDebug() << strQry;
#else
    if (QCoreApplication::arguments().count() > 1
        && QCoreApplication::arguments()[1].contains("/debug")){
        qDebug() << strQry;
    }
#endif

    modelTable->setStrQuery(strQry);
    modelTable->fetchMoreDataFull();

    proxyTable->setSourceModel(modelTable);
    ui->tableView->setModel(proxyTable);
    ui->tableView->hideColumn(section_id);
    ui->tableView->hideColumn(section_idOrganization);
    ui->tableView->hideColumn(section_idContract);
    ui->tableView->hideColumn(section_idPacient);
    ui->tableView->hideColumn(section_searchPacient);
    ui->tableView->hideColumn(section_idUser);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu); // initializam meniu contextual

    ui->tableView->selectRow(0);
    updateHeaderTableOrderEcho();
}

void ListDocReportOrder::updateTableViewReportEcho()
{
    m_currentRow = ui->tableView->currentIndex().row();

    QString strQry;
    if (globals().thisMySQL)
        strQry = "SELECT reportEcho.id,"
                 "reportEcho.deletionMark,"
                 "reportEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "reportEcho.numberDoc,"
                 "reportEcho.dateDoc,"
                 "reportEcho.id_orderEcho,"
                 "pacients.id AS idPacient,"
                 "CONCAT(pacients.name,' ', pacients.fName,' ', pacients.mName,', ', "
                 "SUBSTRING(pacients.birthday, 9, 2),'.', "
                 "SUBSTRING(pacients.birthday, 6, 2),'.', "
                 "SUBSTRING(pacients.birthday, 1, 4)) AS searchPacients,"
                 "CONCAT(pacients.name,' ', pacients.fName) AS pacient,"
                 "pacients.IDNP,"
                 "CONCAT('Raport ecografic nr.', orderEcho.numberDoc,' din ',"
                 "SUBSTRING(orderEcho.dateDoc,9,2),'.', SUBSTRING(orderEcho.dateDoc,6,2),'.', SUBSTRING(orderEcho.dateDoc,1,4),' ',SUBSTRING(orderEcho.dateDoc,12,8)) AS DocOrder,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "reportEcho.concluzion,"
                 "reportEcho.comment "
                 "FROM reportEcho "
                 "INNER JOIN pacients ON reportEcho.id_pacients = pacients.id "
                 "INNER JOIN orderEcho ON reportEcho.id_orderEcho = orderEcho.id "
                 "INNER JOIN users ON reportEcho.id_users = users.id";
    else
        strQry = "SELECT reportEcho.id,"
                 "reportEcho.deletionMark,"
                 "reportEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "reportEcho.numberDoc,"
                 "reportEcho.dateDoc,"
                 "reportEcho.id_orderEcho,"
                 "pacients.id AS idPacient,"
                 "pacients.name ||' '|| pacients.fName ||' '|| pacients.mName ||', '|| "
                 "substr(pacients.birthday, 9, 2) ||'.'|| "
                 "substr(pacients.birthday, 6, 2) ||'.'|| "
                 "substr(pacients.birthday, 1, 4) AS searchPacients,"
                 "pacients.name ||' '|| pacients.fName AS pacient,"
                 "pacients.IDNP,"
                 "'Raport ecografic nr.' || orderEcho.numberDoc ||' din '"
                 "|| substr(orderEcho.dateDoc,9,2) ||'.'|| substr(orderEcho.dateDoc,6,2) ||'.'|| substr(orderEcho.dateDoc,1,4) ||' '|| substr(orderEcho.dateDoc,12,8) AS DocOrder,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "reportEcho.concluzion,"
                 "reportEcho.comment "
                 "FROM reportEcho "
                 "INNER JOIN pacients ON reportEcho.id_pacients = pacients.id "
                 "INNER JOIN orderEcho ON reportEcho.id_orderEcho = orderEcho.id "
                 "INNER JOIN users ON reportEcho.id_users = users.id";

    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString(" WHERE orderEcho.numberDoc = '%1'").arg(m_numberDoc);

    QString startDate = ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDate   = ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString(" AND orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);
    else
        strQry = strQry + QString(" WHERE orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);

    strQry = strQry + " ORDER BY orderEcho.dateDoc DESC "
                      " LIMIT :limit OFFSET :offset;";

#ifdef QT_DEBUG
    qDebug() << "----------------------------------------------------------------------------------------------------------'";
    qDebug() << "Solicitarea pentru popularea listei de documente pe perioada:" + startDate + " - " + endDate;
    qDebug() << strQry;
#else
    if (QCoreApplication::arguments().count() > 1
        && QCoreApplication::arguments()[1].contains("/debug")){
        qDebug() << strQry;
    }
#endif

    int scrollPosition = ui->tableView->verticalScrollBar()->value();
    int maxScroll = ui->tableView->verticalScrollBar()->maximum();

    int rowHeight = ui->tableView->rowHeight(0); // Calculăm înălțimea rândului
    int startRow = 0;
    if (rowHeight > 0)
        startRow = scrollPosition / rowHeight;
    else
        Q_UNUSED(startRow)

    if (scrollPosition == 0) {
        modelTable->resetPagination();    // resetarea
        modelTable->setPagination(50, 0); // setarea initiala paginarii
        modelTable->setStrQuery(strQry);  // setarea textului solicitarii
        modelTable->fetchMoreData();      // prelucrarea solicitarii si prezentarea datelor
    } else if (scrollPosition < maxScroll) {
        modelTable->updateVisibleData(scrollPosition, rowHeight);
    }

    proxyTable->setSourceModel(modelTable);
    ui->tableView->setModel(proxyTable);
    ui->tableView->hideColumn(report_id);
    ui->tableView->hideColumn(report_id_orderEcho);
    ui->tableView->hideColumn(report_id_patient);
    ui->tableView->hideColumn(report_search_patient);
    ui->tableView->hideColumn(report_id_user);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);      // initializam meniu contextual
    if (m_currentRow != Enums::IDX::IDX_UNKNOW)
        ui->tableView->selectRow(m_currentRow);
    else
        ui->tableView->selectRow(0);
    updateHeaderTableReportEcho();
}

void ListDocReportOrder::updateTableViewReportEchoFull()
{
    m_currentRow = ui->tableView->currentIndex().row();

    QString strQry;
    if (globals().thisMySQL)
        strQry = "SELECT reportEcho.id,"
                 "reportEcho.deletionMark,"
                 "reportEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "reportEcho.numberDoc,"
                 "reportEcho.dateDoc,"
                 "reportEcho.id_orderEcho,"
                 "pacients.id AS idPacient,"
                 "CONCAT(pacients.name,' ', pacients.fName,' ', pacients.mName,', ', "
                 "SUBSTRING(pacients.birthday, 9, 2),'.', "
                 "SUBSTRING(pacients.birthday, 6, 2),'.', "
                 "SUBSTRING(pacients.birthday, 1, 4)) AS searchPacients,"
                 "CONCAT(pacients.name,' ', pacients.fName) AS pacient,"
                 "pacients.IDNP,"
                 "CONCAT('Raport ecografic nr.', orderEcho.numberDoc,' din ',"
                 "SUBSTRING(orderEcho.dateDoc,9,2),'.', SUBSTRING(orderEcho.dateDoc,6,2),'.', SUBSTRING(orderEcho.dateDoc,1,4),' ',SUBSTRING(orderEcho.dateDoc,12,8)) AS DocOrder,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "reportEcho.concluzion,"
                 "reportEcho.comment "
                 "FROM reportEcho "
                 "INNER JOIN pacients ON reportEcho.id_pacients = pacients.id "
                 "INNER JOIN orderEcho ON reportEcho.id_orderEcho = orderEcho.id "
                 "INNER JOIN users ON reportEcho.id_users = users.id";
    else
        strQry = "SELECT reportEcho.id,"
                 "reportEcho.deletionMark,"
                 "reportEcho.attachedImages,"
                 "orderEcho.cardPayment,"
                 "reportEcho.numberDoc,"
                 "reportEcho.dateDoc,"
                 "reportEcho.id_orderEcho,"
                 "pacients.id AS idPacient,"
                 "pacients.name ||' '|| pacients.fName ||' '|| pacients.mName ||', '|| "
                 "substr(pacients.birthday, 9, 2) ||'.'|| "
                 "substr(pacients.birthday, 6, 2) ||'.'|| "
                 "substr(pacients.birthday, 1, 4) AS searchPacients,"
                 "pacients.name ||' '|| pacients.fName AS pacient,"
                 "pacients.IDNP,"
                 "'Raport ecografic nr.' || orderEcho.numberDoc ||' din '"
                 "|| substr(orderEcho.dateDoc,9,2) ||'.'|| substr(orderEcho.dateDoc,6,2) ||'.'|| substr(orderEcho.dateDoc,1,4) ||' '|| substr(orderEcho.dateDoc,12,8) AS DocOrder,"
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "reportEcho.concluzion,"
                 "reportEcho.comment "
                 "FROM reportEcho "
                 "INNER JOIN pacients ON reportEcho.id_pacients = pacients.id "
                 "INNER JOIN orderEcho ON reportEcho.id_orderEcho = orderEcho.id "
                 "INNER JOIN users ON reportEcho.id_users = users.id";

    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString(" WHERE orderEcho.numberDoc = '%1'").arg(m_numberDoc);

    QString startDate = ui->filterStartDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDate   = ui->filterEndDateTime->dateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (! m_numberDoc.isEmpty())
        strQry = strQry + QString(" AND orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);
    else
        strQry = strQry + QString(" WHERE orderEcho.dateDoc BETWEEN '%1' AND '%2'").arg(startDate, endDate);

    strQry = strQry + " ORDER BY orderEcho.dateDoc DESC;";

#ifdef QT_DEBUG
    qDebug() << "----------------------------------------------------------------------------------------------------------'";
    qDebug() << "Solicitarea pentru popularea listei de documente pe perioada:" + startDate + " - " + endDate;
    qDebug() << strQry;
#else
    if (QCoreApplication::arguments().count() > 1
        && QCoreApplication::arguments()[1].contains("/debug")){
        qDebug() << strQry;
    }
#endif

    modelTable->setStrQuery(strQry);
    modelTable->fetchMoreDataFull();

    proxyTable->setSourceModel(modelTable);
    ui->tableView->setModel(proxyTable);
    ui->tableView->hideColumn(report_id);
    ui->tableView->hideColumn(report_id_orderEcho);
    ui->tableView->hideColumn(report_id_patient);
    ui->tableView->hideColumn(report_search_patient);
    ui->tableView->hideColumn(report_id_user);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(true);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);      // initializam meniu contextual

    ui->tableView->selectRow(0);
    updateHeaderTableReportEcho();
}

void ListDocReportOrder::updateHeaderTableOrderEcho()
{
    modelTable->setHeaderData(section_deletionMark, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(section_attachedImages, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(section_attachedImages, Qt::Horizontal, QIcon(":img/image-files.png"), Qt::DecorationRole);
    modelTable->setHeaderData(section_cardPayment, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(section_cardPayment, Qt::Horizontal, QIcon(":img/master_card.png"), Qt::DecorationRole);
    modelTable->setHeaderData(section_numberDoc, Qt::Horizontal, tr("Num\304\203r"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organiza\310\233ia (id)"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organiza\310\233ia"));
    modelTable->setHeaderData(section_dateDoc, Qt::Horizontal, tr("Data"));
    modelTable->setHeaderData(section_idContract, Qt::Horizontal, tr("Contract (id)"));
    modelTable->setHeaderData(section_Contract, Qt::Horizontal, tr("Contract"));
    modelTable->setHeaderData(section_idPacient, Qt::Horizontal, tr("Pacient (id)"));
    modelTable->setHeaderData(section_searchPacient, Qt::Horizontal, tr("Pacient (cautare)"));
    modelTable->setHeaderData(section_pacient, Qt::Horizontal, tr("Pacient"));
    modelTable->setHeaderData(section_IDNP, Qt::Horizontal, tr("IDNP"));
    modelTable->setHeaderData(section_doctor, Qt::Horizontal, tr("Trimis de ..."));
    modelTable->setHeaderData(section_idUser, Qt::Horizontal, tr("Autor (id)"));
    modelTable->setHeaderData(section_user, Qt::Horizontal, tr("Autor"));
    modelTable->setHeaderData(section_sum, Qt::Horizontal, tr("Total (MDL)"));
    modelTable->setHeaderData(section_comment, Qt::Horizontal, tr("Comentariu"));
}

void ListDocReportOrder::updateHeaderTableReportEcho()
{
    modelTable->setHeaderData(report_deletionMark, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(report_attachedImages, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(report_attachedImages, Qt::Horizontal, QIcon(":img/image-files.png"), Qt::DecorationRole);
    modelTable->setHeaderData(report_cardPayment, Qt::Horizontal, tr(""));
    modelTable->setHeaderData(report_cardPayment, Qt::Horizontal, QIcon(":img/master_card.png"), Qt::DecorationRole);
    modelTable->setHeaderData(report_numberDoc, Qt::Horizontal, tr("Num\304\203r"));
    modelTable->setHeaderData(report_dateDoc, Qt::Horizontal, tr("Data"));
    modelTable->setHeaderData(report_id_orderEcho, Qt::Horizontal, tr("Comanda ecografica (id)"));
    modelTable->setHeaderData(report_id_patient, Qt::Horizontal, tr("Pacient (id)"));
    modelTable->setHeaderData(report_search_patient, Qt::Horizontal, tr("Pacient (cautare)"));
    modelTable->setHeaderData(report_name_patient, Qt::Horizontal, tr("Pacientul"));
    modelTable->setHeaderData(report_idnp_patient, Qt::Horizontal, tr("IDNP"));
    modelTable->setHeaderData(report_name_DocOrder, Qt::Horizontal, tr("Comanda ecografica"));
    modelTable->setHeaderData(report_id_user, Qt::Horizontal, tr("Autor (id)"));
    modelTable->setHeaderData(report_name_user, Qt::Horizontal, tr("Autor"));
    modelTable->setHeaderData(report_concluzion, Qt::Horizontal, tr("Concluzia"));
    modelTable->setHeaderData(report_comment, Qt::Horizontal, tr("Comentariu"));
}

// **********************************************************************************
// --- salvarea si setarea dimensiunilor(sortarea) sectiilor + alte

QString ListDocReportOrder::getNameTableForSettings()
{
    QString nameTable;
    switch (m_typeDoc) {
    case orderEcho:
        nameTable = "orderEcho";
        break;
    case reportEcho:
        nameTable = "reportEcho";
        break;
    default:
        qWarning(logWarning()) << tr("Nu este determinat proprietatea 'm_typeDoc' !!!");
    }
    return nameTable;
}

void ListDocReportOrder::loadSizeSectionPeriodTable(bool only_period)
{
    QSqlQuery qry;

    //*****************************************************
    //---------- setarea perioadei
    if (only_period){
        QString str = "SELECT dateStart, dateEnd "
                      "FROM settingsForms "
                      "WHERE typeForm = :typeForm AND numberSection = 0;";
        qry.prepare(str);
        qry.bindValue(":typeForm", (m_typeDoc == orderEcho) ? "orderEcho" : "reportEcho");
        if (qry.exec() && qry.next()){
            QString str_date_start;
            QString str_date_end;
            if (globals().thisMySQL){
                static const QRegularExpression replaceT("T");
                static const QRegularExpression removeMilliseconds("\\.000");
                str_date_start = qry.value(0).toString().replace(replaceT, " ").replace(removeMilliseconds,"");
                str_date_end   = qry.value(1).toString().replace(replaceT, " ").replace(removeMilliseconds,"");
            } else {
                str_date_start = qry.value(0).toString();
                str_date_end   = qry.value(1).toString();
            }
            if (str_date_start.isEmpty() && str_date_end.isEmpty()){
                ui->filterStartDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 01, 01), QTime(00,00,00)));
                ui->filterEndDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 12, 31), QTime(23,59,59)));
            } else {
                ui->filterStartDateTime->setDateTime(QDateTime::fromString(str_date_start, "yyyy-MM-dd hh:mm:ss"));
                ui->filterEndDateTime->setDateTime(QDateTime::fromString(str_date_end, "yyyy-MM-dd hh:mm:ss"));
            }
        } else {
            ui->filterStartDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 01, 01), QTime(00,00,00)));
            ui->filterEndDateTime->setDateTime(QDateTime(QDate(QDate::currentDate().year(), 12, 31), QTime(23,59,59)));

            //---------- prezentarea erorii
            if (! qry.lastError().text().isEmpty()){
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Determinarea perioadei"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Nu a fost extras\304\203 perioada din set\304\203rile formei !!!"));
                msgBox.setDetailedText(qry.lastError().text());
                msgBox.setStandardButtons(QMessageBox::Ok);
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
                        "WHERE typeForm LIKE %1;").arg((m_typeDoc == orderEcho) ? "'%orderEcho%'" : "'%reportEcho%'"));
    if (qry.exec()){
        int num = -1;
        while (qry.next()) {
            num ++;
            const int current_size = qry.value(1).toInt();
            const int direction_sorting = qry.value(2).toInt();

            //---------- setarea dimensiunilor sectiilor
            if (current_size >= 0)
                ui->tableView->setColumnWidth(num, qry.value(1).toInt());
            else
                ui->tableView->setColumnWidth(num, sizeSectionDefault(num));

            //---------- setarea directiei sortarii
            if (direction_sorting > 0)
                ui->tableView->horizontalHeader()->setSortIndicator(num, (direction_sorting == 0) ? Qt::AscendingOrder : Qt::DescendingOrder);
        }
    } else {

        //---------- setarea dimensiunilor sectiilor
        for (int numSection = 0; numSection < ui->tableView->horizontalHeader()->count(); ++numSection){
            ui->tableView->setColumnWidth(numSection, sizeSectionDefault(numSection));
        }

        //---------- setarea directiei sortarii
        ui->tableView->horizontalHeader()->setSortIndicator(section_numberDoc, Qt::DescendingOrder);

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
        ui->tableView->selectRow(0);
}

void ListDocReportOrder::saveSizeSectionTable()
{
    const QString type_doc = enumToString(m_typeDoc);

        // salvam dimensiunile setiilor
    for (int numSection = 0; numSection < ui->tableView->horizontalHeader()->count(); ++numSection) {

        // determinarea si setarea variabilei directiei de sortare
        int directionSorting = -1;
        if (numSection == ui->tableView->horizontalHeader()->sortIndicatorSection())
            directionSorting = ui->tableView->horizontalHeader()->sortIndicatorOrder();

        // variabile pu inserarea sau actualizarea datelor in tabela
        const int find_id = db->findIdFromTableSettingsForm(type_doc, numSection);
        const int size_section = ui->tableView->horizontalHeader()->sectionSize(numSection);

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

int ListDocReportOrder::sizeSectionDefault(const int numberSection)
{
    if (m_typeDoc == unknowDoc)
        return Enums::IDX::IDX_UNKNOW;

    if (m_typeDoc == orderEcho){

        switch (numberSection) {
        case section_id:
            return sz_id;
        case section_deletionMark:
            return sz_deletionMark;
        case section_attachedImages:
            return sz_attachedImages;
        case section_cardPayment:
            return sz_cardPayment;
        case section_numberDoc:
            return sz_numberDoc;
        case section_dateDoc:
            return sz_dateDoc;
        case section_idOrganization:
            return sz_id_organization;
        case section_Organization:
            return sz_organization;
        case section_idContract:
            return sz_id_contract;
        case section_Contract:
            return sz_contract;
        case section_idPacient:
            return sz_id_pacient;
        case section_searchPacient:
            return sz_search_pacient;
        case section_pacient:
            return sz_pacient;
        case section_IDNP:
            return sz_idnp;
        case section_doctor:
            return sz_doctor;
        case section_idUser:
            return sz_id_user;
        case section_user:
            return sz_user;
        case section_sum:
            return sz_sum;
        default:
            return 0;
        }

    } else {

        switch (numberSection) {
        case report_id:
            return sz_id;
        case report_deletionMark:
            return sz_deletionMark;
        case report_attachedImages:
            return sz_attachedImages;
        case report_cardPayment:
            return sz_cardPayment;
        case report_numberDoc:
            return sz_numberDoc;
        case report_dateDoc:
            return sz_dateDoc;
        case report_id_orderEcho:
            return sz_id;
        case report_id_patient:
            return sz_id_pacient;
        case report_search_patient:
            return sz_search_pacient;
        case report_name_patient:
            return sz_pacient;
        case report_idnp_patient:
            return sz_idnp;
        case report_name_DocOrder:
            return 400;
        case report_id_user:
            return sz_id_pacient;
        case report_name_user:
            return sz_user;
        default:
            return 0;
        }
    }
}

// **********************************************************************************
// --- evenimente

void ListDocReportOrder::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSizeSectionTable();
    }
}

bool ListDocReportOrder::eventFilter(QObject *obj, QEvent *event)
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
    } else if (obj == ui->btnFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnFilter->pos().x() - 56, ui->btnFilter->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Filtrare dup\304\203<br>%1 (Ctrl + F2)")
                                .arg(proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_Organization), Qt::DisplayRole).toString())); // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnFilterRemove){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnFilterRemove->pos().x() - 50, ui->btnFilterRemove->pos().y() + 30)); // determinam parametrii globali
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
    } else if (obj == ui->btnHideShowColumn){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnUpdateTable->pos().x() - 35, ui->btnUpdateTable->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Ascunde/afi\310\231eaz\304\203 <br>coloni\310\233e (Ctrl + H)"));  // setam textul
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
    } else if (obj == ui->btnSendEmail){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnSendEmail->pos().x() - 30, ui->btnSendEmail->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Trimite scrisoarea<br>"
                                   "prin e-mail"));   // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnReport){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnReport->pos().x() - 64, ui->btnReport->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Crearea/deschiderea<br>"
                                   "documentului 'Raport ecografic' (Ctrl + R)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnViewTab){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnViewTab->pos().x() - 58, ui->btnViewTab->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Vizualizarea<br>"
                                   "p\304\203r\310\233ii tabelare sau concluziei (Ctrl + T)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnPeriodDate){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnPeriodDate->pos().x() - 60, ui->btnPeriodDate->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Perioada \310\231i filtru (Ctrl + Shift + P)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnSearch){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnSearch->pos().x() - 104, ui->btnSearch->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("C\304\203utarea pacientului<br>"
                                   "dup\304\203: nume/prenume sau IDNP (Ctrl + S)"));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    }

    return false;
}

void ListDocReportOrder::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        switch (m_typeDoc) {
        case orderEcho:
            setWindowTitle(tr("Lista documentelor: Comanda ecografic\304\203"));
            updateHeaderTableOrderEcho();
            break;
        case reportEcho:
            setWindowTitle(tr("Lista documentelor: Raport ecografic"));
            updateHeaderTableReportEcho();
            break;
        default:
            qWarning(logWarning()) << tr("Nu este determinat proprietatea 'm_typeDoc' !!!");
        }
    }
}

void ListDocReportOrder::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End){
        ui->tableView->selectRow(modelTable->rowCount() - 1);
    } else if (event->key() == Qt::Key_Home){
        ui->tableView->selectRow(0);
    }

    if (pressed_btn_viewTab == -1)
        return;

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down){
        QModelIndex index = ui->tableView->currentIndex();
        onClickedTable(index);
    }
}


