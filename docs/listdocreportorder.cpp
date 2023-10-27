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

    QString strQuery;
    modelTable       = new BaseSqlQueryModel(strQuery, ui->tableView); // model principal pu solicitarea din BD
    modelTable->setProperty("modelParent", BaseSqlQueryModel::ModelParent::ListOrderReport);

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
    delete toolBar;
    delete customPeriod;
    delete db;
    delete ui;
}

void ListDocReportOrder::updateMainTableByTimer()
{
    updateTableView();
}

void ListDocReportOrder::enableLineEditSearch()
{
    if (editSearch->isEnabled()){
        editSearch->setEnabled(false);
        editSearch->setPlaceholderText(tr(""));
    } else {
        editSearch->setEnabled(true);
#if defined(Q_OS_LINUX)
        editSearch->setPlaceholderText(tr("... căutarea după nume, prenume sau IDNP a pacientului"));
#elif defined(Q_OS_MACOS)
        editSearch->setPlaceholderText(tr("... căutarea după nume, prenume sau IDNP a pacientului"));
#elif defined(Q_OS_WIN)
        editSearch->setPlaceholderText(tr("... c\304\203utarea dup\304\203 nume, prenume sau IDNP a pacientului"));
#endif
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
#if defined(Q_OS_LINUX)
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data finisării perioadei nu poate fi mai mică decât \ndata lansării perioadei !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
#elif defined(Q_OS_MACOS)
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data finisării perioadei nu poate fi mai mică decât \ndata lansării perioadei !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
#elif defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data finis\304\203rii perioadei nu poate fi mai mic\304\203 dec\303\242t \ndata lans\304\203rii perioadei !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
#endif
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
    if (globals::updateIntervalListDoc > 0)
        timer->start(globals::updateIntervalListDoc * 1000);

    if (m_typeDoc == reportEcho){
        btnReport->setDisabled(true);
        ui->filterComboOrganization->setEnabled(false);
        ui->filterComboContract->setEnabled(false);
        ui->btnOpenCatContract->setEnabled(false);
        ui->btnOpenCatOrganization->setEnabled(false);
        ui->view_table->setHidden(true);
        setWindowTitle(tr("Lista documentelor: Raport ecografic"));
        setWindowIcon(QIcon(":/img/examenEcho.png"));
    } else {
#if defined(Q_OS_LINUX)
        setWindowTitle(tr("Lista documentelor: Comanda ecografică"));
#elif defined(Q_OS_MACOS)
        setWindowTitle(tr("Lista documentelor: Comanda ecografică"));
#elif defined(Q_OS_WIN)
        setWindowTitle(tr("Lista documentelor: Comanda ecografic\304\203"));
#endif
        setWindowIcon(QIcon(":/img/orderEcho_x32.png"));
    }
    loadSizeSectionPeriodTable(true); // only period
    updateTableView();
    loadSizeSectionPeriodTable(); // without period
}

void ListDocReportOrder::slot_IdOrganizationChanged()
{
    if (m_idOrganization == idx_unknow || m_idOrganization == 0)
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
    if (m_idContract == idx_unknow || m_idContract == 0)
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

    m_idOrganization = idx_unknow;
    m_idContract     = idx_unknow;
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
    if (ui->tableView->currentIndex().row() == idx_unknow){
#if defined(Q_OS_LINUX)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_MACOS)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#endif
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
    if (_row == idx_unknow){
#if defined(Q_OS_LINUX)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_MACOS)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#endif
        return;
    }
    const int _id = proxyTable->index(_row, section_id).data(Qt::DisplayRole).toInt();

    QSqlQuery qry;
    if (m_typeDoc == orderEcho){
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
#if defined(Q_OS_LINUX)
                msgBox.setText(tr("Există documente subordonate. Doriți să continuați ?"));
#elif defined(Q_OS_MACOS)
                msgBox.setText(tr("Există documente subordonate. Doriți să continuați ?"));
#elif defined(Q_OS_WIN)
                msgBox.setText(tr("Exist\304\203 documente subordonate. Dori\310\233i s\304\203 continua\310\233i ?"));
#endif
                msgBox.setDetailedText(tr("Va fi eliminat documentul subordonat:\n%1").arg(doc_presentation));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                msgBox.setButtonText(QMessageBox::Yes, tr("Da"));
                msgBox.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//                msgBox.addButton(tr("Da"), QMessageBox::YesRole);
//                msgBox.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
                msgBox.setStyleSheet("QPushButton{width:120px;}");
                if (msgBox.exec() == QMessageBox::Yes){
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

    m_idOrganization = idx_unknow;
    m_idContract     = idx_unknow;
    updateTableView();
    updateTextPeriod();
}

void ListDocReportOrder::onClickBtnUpdateTable()
{
    updateTableView();
}

void ListDocReportOrder::onClickBtnPrint()
{
    if (ui->tableView->currentIndex().row() == idx_unknow){
#if defined(Q_OS_LINUX)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_MACOS)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#endif
        return;
    }
    if (m_typeDoc == orderEcho){
        int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
        docOrderEcho = new DocOrderEcho(this);
        docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
        docOrderEcho->setProperty("ItNew", false);
        docOrderEcho->setProperty("Id", _id);
        docOrderEcho->onPrintDocument(DocOrderEcho::TypePrint::openPreview);
        docOrderEcho->close();
    } else {
        int _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id), Qt::DisplayRole).toInt();
        DocReportEcho* docReport = new DocReportEcho(this);
        docReport->setAttribute(Qt::WA_DeleteOnClose);
        docReport->setProperty("ItNew", false);
        docReport->setProperty("Id", _id);
        docReport->onPrintDocument(docReport->openPreview);
        docReport->close();
    }
}

void ListDocReportOrder::onClickBtnReport()
{
    if (ui->tableView->currentIndex().row() == idx_unknow){
#if defined(Q_OS_LINUX)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_MACOS)
        QMessageBox::warning(this, tr("Informație"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#elif defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Informa\310\233ie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
#endif
        return;
    }

    int _id = idx_unknow;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - onClickBtnReport()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == idx_unknow || _id == idx_write){
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
    } else {

        const int _idPacient = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_idPacient), Qt::DisplayRole).toInt();

        // daca nu afost gasit documentul subaltern -> deschidem selectarea investigatiilor
        CustomDialogInvestig* dialogInvestig = new CustomDialogInvestig(this);
        qry.prepare("SELECT cod FROM orderEchoTable WHERE id_orderEcho = :idOrderEcho;");
        qry.bindValue(":idOrderEcho", _id);
        if (qry.exec()){
            while (qry.next()) {
                QString _cod = qry.value(0).toString();
                if (_cod == "1021" || _cod == "1022" || _cod == "1023" || _cod == "1050.61" || _cod == "1050.62" || _cod == "1050.63" ||
                    _cod == "1050.62.1" || _cod == "1050.62.2" || _cod == "1050.63.1")
                    dialogInvestig->set_t_organs_internal(true);
                if (_cod == "1021" || _cod == "1022" || _cod == "1024" || _cod == "1050.19" || _cod == "1050.20" || _cod == "1050.61" || _cod == "1050.62" || _cod == "1050.63" ||
                    _cod == "1050.62.1" || _cod == "1050.62.2" || _cod == "1050.63.1")
                    dialogInvestig->set_t_urinary_system(true);
                if (_cod == "1025" || _cod == "1038" || _cod == "1050.55" || _cod == "1050.62.1" || _cod == "1050.62.2")
                    dialogInvestig->set_t_prostate(true);
                if (_cod == "1026" || _cod == "1033" || _cod == "1050.22" || _cod == "1050.23" || _cod == "1050.25" || _cod == "1050.26" || _cod == "1050.62" || _cod == "1050.63"||
                    _cod == "1050.63.1" || _cod == "1050.69")
                    dialogInvestig->set_t_gynecology(true);
                if (_cod == "1037" || _cod == "1037.1" || _cod == "1050.34" || _cod == "1050.35" || _cod == "1050.63.1")
                    dialogInvestig->set_t_breast(true);
                if (_cod == "1036" || _cod == "1036.1" || _cod == "1050.31" || _cod == "1050.32" || _cod == "1050.62.2" || _cod == "1050.63.1")
                    dialogInvestig->set_t_thyroide(true);
                if (_cod == "1027" || _cod == "1027.1" || _cod == "1028" || _cod == "1028.1" || _cod == "1050.66" || _cod == "1050.67")
                    dialogInvestig->set_t_gestation0(true);
                if (_cod == "1028.4.1" || _cod == "1028.4.2")
                    dialogInvestig->set_t_gestation1(true);
                if (_cod == "1029.1.1" || _cod == "1029.1.2" || _cod == "1029.2" || _cod == "1029.21" || _cod == "1029.3" || _cod == "1029.31")
                    dialogInvestig->set_t_gestation2(true);
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QRegExp regExp(editSearch->text(), Qt::CaseInsensitive, QRegExp::RegExp);
    proxyTable->setFilterRegExp(regExp);
#else
    QRegularExpression regExp(editSearch->text(), QRegularExpression::CaseInsensitiveOption);
    proxyTable->setFilterRegularExpression(regExp);
#endif
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
    if (_id == idx_unknow || _id == 0)
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
    int _id_report                  = idx_unknow;

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
            if (globals::thisMySQL)
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                presentationDoc = tr("Raport ecografic nr.%1 din %2").arg(qry.value(1).toString(), qry.value(2).toString().replace(QRegExp("T"), " ").replace(".000",""));
#else
                presentationDoc = tr("Raport ecografic nr.%1 din %2").arg(qry.value(1).toString(), qry.value(2).toString().replace(QRegularExpression("T"), " ").replace(".000",""));
#endif
            else
                presentationDoc = tr("Raport ecografic nr.%1 din %2").arg(qry.value(1).toString(), qry.value(2).toString());
        } else {
            qDebug() << qry.lastError().text();
        }

        if (_id_report != idx_unknow && presentationDoc != nullptr){
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
                        docReport->onPrintDocument(docReport->openPreview);
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
    // alocarea memoriei

    toolBar         = new QToolBar(this);
    btnAdd          = new QToolButton(this);
    btnEdit         = new QToolButton(this);
    btnDeletion     = new QToolButton(this);
    btnFilter       = new QToolButton(this);
    btnAddFilter    = new QToolButton(this);
    btnFilterRemove = new QToolButton(this);
    btnUpdateTable  = new QToolButton(this);
    btnPrint        = new QToolButton(this);
    btnReport       = new QToolButton(this);
    btnViewTab      = new QToolButton(this);
    btnPeriodDate   = new QToolButton(this);
    btnSearch       = new QToolButton(this);

    // ----------------------------------------------------------------------------
    // setarea stilului btn

    btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    btnAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnAdd->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));
    btnAdd->setObjectName("toolBtn_add"); // pu adresare din css

    btnEdit->setIcon(QIcon(":/img/edit_x32.png"));
    btnEdit->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnEdit->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnEdit->setShortcut(QKeySequence(Qt::Key_F2));
    btnEdit->setObjectName("toolBtn_edit"); // pu adresare din css

    btnDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    btnDeletion->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnDeletion->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
    btnDeletion->setObjectName("toolBtn_remove"); // pu adresare din css

    btnAddFilter->setIcon(QIcon(":/img/filter-add-icon.png"));
    btnAddFilter->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnAddFilter->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnAddFilter->setObjectName("toolBtn_add_filter"); // pu adresare din css

    btnFilter->setIcon(QIcon(":/img/filter-icon.png"));
    btnFilter->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFilter->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnFilter->setObjectName("toolBtn_filter"); // pu adresare din css

    btnFilterRemove->setIcon(QIcon(":/img/filter-delete-icon.png"));
    btnFilterRemove->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFilterRemove->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnFilterRemove->setObjectName("toolBtn_filter_remove"); // pu adresare din css

    btnUpdateTable->setIcon(QIcon(":/img/update_x32.png"));
    btnUpdateTable->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnUpdateTable->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnUpdateTable->setShortcut(QKeySequence(Qt::Key_F5));
    btnUpdateTable->setObjectName("toolBtn_update"); // pu adresare din css

    btnPrint->setIcon(QIcon(":/img/print.png"));//.pixmap(14, 14));
    btnPrint->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnPrint->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnPrint->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F5));

    btnReport->setIcon(QIcon(":/img/examenEcho.png").pixmap(14, 14));
    btnReport->setText(tr("Raport eco."));
    btnReport->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnReport->setObjectName("tool_btn_report");

    btnViewTab->setIcon(QIcon(":/img/view_table.png").pixmap(14, 14));
    btnViewTab->setText(tr("Tabela"));
    btnViewTab->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnViewTab->setObjectName("tool_btn_report");

    btnPeriodDate->setIcon(QIcon(":/img/date_period.png"));
    btnPeriodDate->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnPeriodDate->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnPeriodDate->setObjectName("toolBtn_period"); // pu adresare din css

    btnSearch->setIcon(QIcon(":/img/search_pacients.png"));
    btnSearch->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnSearch->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnSearch->setObjectName("toolBtn_search"); // pu adresare din css

    editSearch = new QLineEdit(this);
    editSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    editSearch->setClearButtonEnabled(true);
    editSearch->setEnabled(false);

    // ----------------------------------------------------------------------------
    // instalam filtrul evenimentelor pu prezentarea indiciilor (подсказки)

    btnAdd->setMouseTracking(true);
    btnAdd->installEventFilter(this);
    btnEdit->setMouseTracking(true);
    btnEdit->installEventFilter(this);
    btnDeletion->setMouseTracking(true);
    btnDeletion->installEventFilter(this);
    btnAddFilter->setMouseTracking(true);
    btnAddFilter->installEventFilter(this);
    btnFilter->setMouseTracking(true);
    btnFilter->installEventFilter(this);
    btnFilterRemove->setMouseTracking(true);
    btnFilterRemove->installEventFilter(this);
    btnUpdateTable->setMouseTracking(true);
    btnUpdateTable->installEventFilter(this);
    btnPrint->setMouseTracking(true);
    btnPrint->installEventFilter(this);
    btnReport->setMouseTracking(true);
    btnReport->installEventFilter(this);
    btnViewTab->setMouseTracking(true);
    btnViewTab->installEventFilter(this);
    btnPeriodDate->setMouseTracking(true);
    btnPeriodDate->installEventFilter(this);
    btnSearch->setMouseTracking(true);
    btnSearch->installEventFilter(this);

    // ----------------------------------------------------------------------------
    // crearea toolBar-ului

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
    toolBar->addWidget(btnPrint);
    toolBar->addSeparator();
    toolBar->addWidget(btnReport);
    toolBar->addSeparator();
    toolBar->addWidget(btnViewTab);
    toolBar->addSeparator();
    toolBar->addWidget(btnPeriodDate);
    toolBar->addSeparator();
    toolBar->addWidget(btnSearch);
    toolBar->addWidget(editSearch);

    QSpacerItem *itemSpacer = new QSpacerItem(0,0, QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addSpacerItem(itemSpacer);

    lablePeriodDate = new QLabel(this);
    lablePeriodDate->setStyleSheet("color: blue; font-weight: bold;");
    ui->layoutToolBar->addWidget(lablePeriodDate);
    updateTextPeriod();

    connect(btnAdd, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnAdd);
    connect(btnEdit, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnEdit);
    connect(btnDeletion, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnDeletion);
    connect(btnAddFilter, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnAddFilter);
    connect(btnFilter, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnFilterByIdOrganization);
    connect(btnFilterRemove, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnFilterRemove);
    connect(btnUpdateTable, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnUpdateTable);
    connect(btnPrint, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnPrint);
    connect(btnReport, &QAbstractButton::clicked, this, &ListDocReportOrder::onClickBtnReport);

    connect(btnSearch, &QAbstractButton::clicked, this, &ListDocReportOrder::enableLineEditSearch);
    connect(btnPeriodDate, &QAbstractButton::clicked, this, &ListDocReportOrder::onClosePeriodFilter);

    connect(editSearch, &QLineEdit::textChanged, this, &ListDocReportOrder::filterRegExpChanged);
    connect(editSearch, &QLineEdit::textEdited, this, [this]()
            {
                timer->start(globals::updateIntervalListDoc * 1000);
            });
    connect(btnViewTab, &QAbstractButton::clicked, this, [this]()
    {
        if (pressed_btn_viewTab == -1){
            pressed_btn_viewTab = 1;
            ui->groupBox_table_order->setHidden(false);
            ui->groupBox_table_report->setHidden(false);
            QModelIndex index = ui->tableView->currentIndex();
            onClickedTable(index);
            btnViewTab->setStyleSheet("background-color: #dadbde");
            ui->splitter->resize(1518, 190);
        } else {
            pressed_btn_viewTab = -1;
            ui->groupBox_table_order->setHidden(true);
            ui->groupBox_table_report->setHidden(true);
            btnViewTab->setStyleSheet("background-color: #f6f7fa");
        }
    });

    // ----------------------------------------------------------------------------
    // meniu btn print order

    db->updateVariableFromTableSettingsUser();
    if (globals::showDesignerMenuPrint){
        QAction *openDesignerOrder = new QAction(setUpMenu_order);
        openDesignerOrder->setIcon(QIcon(":/images/design.png"));
        openDesignerOrder->setText(tr("Deschide designer"));

        QAction *openPreviewOrder  = new QAction(setUpMenu_order);
        openPreviewOrder->setIcon(QIcon(":/images/Print.png"));
        openPreviewOrder->setText(tr("Deschide preview"));

        setUpMenu_order->addAction(openDesignerOrder);
        setUpMenu_order->addAction(openPreviewOrder);
        setUpMenu_order->setWindowFlags(setUpMenu_report->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu_order->setAttribute(Qt::WA_TranslucentBackground);
        setUpMenu_order->setStyleSheet(" QMenu {border-radius:5px; font-family:'Arial'; font-size:14px;}"
                                       " QMenu::item {height:25px; width:150px; border: 1px solid none;}");

        ui->btn_print_order->setMenu(setUpMenu_order);
        ui->btn_open_order->setStyleSheet("width: 110px; height: 20px;");
        ui->btn_print_order->setStyleSheet("width: 110px; height: 20px;");

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

    // ----------------------------------------------------------------------------
    // meniu btn print report

    db->updateVariableFromTableSettingsUser();
    if (globals::showDesignerMenuPrint){
        QAction *openDesignerReport = new QAction(setUpMenu_report);
        openDesignerReport->setIcon(QIcon(":/images/design.png"));
        openDesignerReport->setText(tr("Deschide designer"));

        QAction *openPreviewReport  = new QAction(setUpMenu_report);
        openPreviewReport->setIcon(QIcon(":/images/Print.png"));
        openPreviewReport->setText(tr("Deschide preview"));

        setUpMenu_report->addAction(openDesignerReport);
        setUpMenu_report->addAction(openPreviewReport);
        setUpMenu_report->setWindowFlags(setUpMenu_report->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu_report->setAttribute(Qt::WA_TranslucentBackground);
        setUpMenu_report->setStyleSheet(" QMenu {border-radius:5px; font-family:'Arial'; font-size:14px;}"
                                        " QMenu::item {height:25px; width:150px; border: 1px solid none;}");

        ui->btn_print_report->setMenu(setUpMenu_report);
        ui->btn_print_report->setStyleSheet("width: 110px; height: 20px;");
        ui->btn_open_report->setStyleSheet("width: 110px; height: 20px;");

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

void ListDocReportOrder::initBtnFilter()
{
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
            timer->start(globals::updateIntervalListDoc * 1000);
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
// --- actualizarea solicitarilor tabelelor, textului

void ListDocReportOrder::updateTextPeriod()
{
    lablePeriodDate->setText(tr("Perioada: ") +
                             ui->filterStartDateTime->dateTime().toString("dd.MM.yyyy") + " - " +
                             ui->filterEndDateTime->dateTime().toString("dd.MM.yyyy"));
    if (ui->filterComboOrganization->currentIndex() > 0){
        QString str = lablePeriodDate->text();
        str = str + tr("; filtru: ") + ui->filterComboOrganization->currentText();
        lablePeriodDate->setText(str);
    }
    if (ui->filterComboContract->currentIndex() > 0){
        QString str = lablePeriodDate->text();
        str = str + " (" + ui->filterComboContract->currentText() +")";
        lablePeriodDate->setText(str);
    }
}

void ListDocReportOrder::openDocOrderEchoByClickBtnTable()
{
    int _id = idx_unknow;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == idx_unknow || _id == idx_write){
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

void ListDocReportOrder::openPrintDesignerPreviewOrder(bool preview)
{
    int _id = idx_unknow;
    if (m_typeDoc == TypeDoc::orderEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_id), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        _id = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id_orderEcho), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (_id == idx_unknow || _id == idx_write){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewOrder()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }
    docOrderEcho = new DocOrderEcho(this);
    docOrderEcho->setAttribute(Qt::WA_DeleteOnClose);
    docOrderEcho->setProperty("ItNew", false);
    docOrderEcho->setProperty("Id", _id);
    if (preview)
        docOrderEcho->onPrintDocument(DocOrderEcho::TypePrint::openPreview);
    else
        docOrderEcho->onPrintDocument(DocOrderEcho::TypePrint::openDesigner);
}

void ListDocReportOrder::openPrintDesignerPreviewReport(bool preview)
{
    int id_doc = idx_unknow;
    if (m_typeDoc == TypeDoc::orderEcho)
        id_doc = model_view_table_report->data(model_view_table_report->index(0, 0), Qt::DisplayRole).toInt();
    else if (m_typeDoc == TypeDoc::reportEcho)
        id_doc = proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), report_id), Qt::DisplayRole).toInt();
    else
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewReport()").arg(metaObject()->className())
                            << tr("Nu a fost setata(sau setata incorect) variabila 'm_typeDoc'.");

    if (id_doc == idx_unknow || id_doc == idx_write){
        qInfo(logWarning()) << tr("%1 - openPrintDesignerPreviewReport()").arg(metaObject()->className())
                            << tr("Nu a fost identificat 'id' documentului pentru printare.");
        return;
    }
    DocReportEcho* docReport = new DocReportEcho(this);
    docReport->setAttribute(Qt::WA_DeleteOnClose);
    docReport->setProperty("ItNew", false);
    docReport->setProperty("Id",    id_doc);
    if (preview)
        docReport->onPrintDocument(docReport->openPreview);
    else
        docReport->onPrintDocument(docReport->openDesigner);
}

void ListDocReportOrder::updateTableView()
{
    if (m_typeDoc == unknowDoc){
        qWarning(logWarning()) << tr("Nu este determinata proprietatea 'typeDoc' !!! Completarea tabelei nu este posibila.");
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
     if (globals::thisMySQL)
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
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                 "INNER JOIN fullNamePacients ON orderEcho.id_pacients = fullNamePacients.id_pacients "
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
                 "users.id AS idUser,"
                 "users.name AS user,"
                 "orderEcho.sum,"
                 "orderEcho.comment "
                 "FROM orderEcho "
                 "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                 "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                 "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
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

    modelTable->setQuery(strQry);

    while (modelTable->canFetchMore())
        modelTable->fetchMore();

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
    ui->tableView->verticalHeader()->setDefaultSectionSize(10);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);      // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu); // initializam meniu contextual
    if (m_currentRow != idx_unknow)
        ui->tableView->selectRow(m_currentRow);
    else
        ui->tableView->selectRow(0);
    updateHeaderTableOrderEcho();
}

void ListDocReportOrder::updateTableViewReportEcho()
{
    m_currentRow = ui->tableView->currentIndex().row();

    QString strQry;
    if (globals::thisMySQL)
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

    modelTable->setQuery(strQry);

    while (modelTable->canFetchMore())
        modelTable->fetchMore();

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
    ui->tableView->verticalHeader()->setDefaultSectionSize(10);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);      // initializam meniu contextual
    if (m_currentRow != idx_unknow)
        ui->tableView->selectRow(m_currentRow);
    else
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
#if defined(Q_OS_LINUX)
    modelTable->setHeaderData(section_numberDoc, Qt::Horizontal, tr("Număr"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația (id)"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația"));
#elif defined(Q_OS_MACOS)
    modelTable->setHeaderData(section_numberDoc, Qt::Horizontal, tr("Număr"));
        modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația (id)"));
        modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organizația"));
#elif defined(Q_OS_WIN)
    modelTable->setHeaderData(section_numberDoc, Qt::Horizontal, tr("Num\304\203r"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organiza\310\233ia (id)"));
    modelTable->setHeaderData(section_Organization, Qt::Horizontal, tr("Organiza\310\233ia"));
#endif
    modelTable->setHeaderData(section_dateDoc, Qt::Horizontal, tr("Data"));
    modelTable->setHeaderData(section_idContract, Qt::Horizontal, tr("Contract (id)"));
    modelTable->setHeaderData(section_Contract, Qt::Horizontal, tr("Contract"));
    modelTable->setHeaderData(section_idPacient, Qt::Horizontal, tr("Pacient (id)"));
    modelTable->setHeaderData(section_searchPacient, Qt::Horizontal, tr("Pacient (cautare)"));
    modelTable->setHeaderData(section_pacient, Qt::Horizontal, tr("Pacient"));
    modelTable->setHeaderData(section_IDNP, Qt::Horizontal, tr("IDNP"));
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
#if defined(Q_OS_LINUX)
    modelTable->setHeaderData(report_numberDoc, Qt::Horizontal, tr("Număr"));
#elif defined(Q_OS_WIN)
    modelTable->setHeaderData(report_numberDoc, Qt::Horizontal, tr("Num\304\203r"));
#endif
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
        QString str = QString("SELECT "
                              "dateStart,"
                              "dateEnd "
                              "FROM settingsForms "
                              "WHERE typeForm LIKE %1 AND dateStart %2 AND dateEnd %2;")
                          .arg((m_typeDoc == orderEcho) ? "'%orderEcho%'" : "'%reportEcho%'",
                               (globals::thisMySQL) ? "IS NOT Null" : "NOT NULL");
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
#if defined(Q_OS_LINUX)
                msgBox.setText(tr("Nu a fost extrasă perioada din setările formei !!!"));
#elif defined(Q_OS_WIN)
                msgBox.setText(tr("Nu a fost extras\304\203 perioada din set\304\203rile formei !!!"));
#endif
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
        return idx_unknow;

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
    if (obj == btnAdd){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnAdd->pos().x() - 20, btnAdd->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Adaugă."));         // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Adaugă."));         // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Adaug\304\203."));  // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnEdit){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnEdit->pos().x() - 20, btnEdit->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Editează."));       // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Editează."));       // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Editeaz\304\203.")); // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnDeletion){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnDeletion->pos().x() - 46, btnDeletion->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Eliminare."));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnAddFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnAddFilter->pos().x() - 50, btnAddFilter->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Adaugă filtru."));  // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Adaugă filtru."));  // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Adaug\304\203 filtru."));  // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnFilter){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnFilter->pos().x() - 56, btnFilter->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Filtrare după<br>%1.")
                                .arg(proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_Organization), Qt::DisplayRole).toString())); // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Filtrare după<br>%1.")
                                .arg(proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_Organization), Qt::DisplayRole).toString())); // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Filtrare dup\304\203<br>%1.")
                                .arg(proxyTable->data(proxyTable->index(ui->tableView->currentIndex().row(), section_Organization), Qt::DisplayRole).toString())); // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnFilterRemove){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnFilterRemove->pos().x() - 50, btnFilterRemove->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Golește filtru."));       // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Golește filtru."));       // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Gole\310\231te filtru."));       // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnUpdateTable){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnUpdateTable->pos().x() - 60, btnUpdateTable->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Actualizează tabela."));  // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Actualizează tabela."));  // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Actualizeaz\304\203 tabela."));  // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnPrint){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnPrint->pos().x() - 30, btnPrint->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Printare."));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnReport){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnReport->pos().x() - 64, btnReport->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Crearea/deschiderea<br>"
                                   "documentului 'Raport ecografic'."));       // setam textul
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnViewTab){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnViewTab->pos().x() - 58, btnViewTab->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Vizualizarea<br>"
                                   "părții tabelare sau concluziei."));       // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Vizualizarea<br>"
                                   "părții tabelare sau concluziei."));       // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Vizualizarea<br>"
                                   "p\304\203r\310\233ii tabelare sau concluziei."));       // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnPeriodDate){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnPeriodDate->pos().x() - 60, btnPeriodDate->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Perioada și filtrului."));       // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Perioada și filtrului."));       // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Perioada \310\231i filtrului."));       // setam textul
#endif
            popUp->showFromGeometryTimer(p);            // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == btnSearch){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnSearch->pos().x() - 104, btnSearch->pos().y() + 30)); // determinam parametrii globali
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Căutarea pacientului<br>"
                                   "după: nume/prenume sau IDNP."));       // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Căutarea pacientului<br>"
                                   "după: nume/prenume sau IDNP."));       // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("C\304\203utarea pacientului<br>"
                                   "dup\304\203: nume/prenume sau IDNP."));       // setam textul
#endif
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
#if defined(Q_OS_LINUX)
            setWindowTitle(tr("Lista documentelor: Comanda ecografică"));
#elif defined(Q_OS_MACOS)
            setWindowTitle(tr("Lista documentelor: Comanda ecografică"));
#elif defined(Q_OS_WIN)
            setWindowTitle(tr("Lista documentelor: Comanda ecografic\304\203"));
#endif
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


