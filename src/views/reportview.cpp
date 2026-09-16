#include "reportview.h"
#include "ui_reportview.h"

#include <catalogs/contractdialog.h>
#include <catalogs/organizationdialog.h>
#include <catalogs/userdialog.h>

ReportView::ReportView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Form)
    , m_settings(globals().pathSettingsCommon)
    , m_db(db)
    , m_toolBar(new ToolBarCustom(this,
                                  ToolBarCustom::AddEditDelete |
                                  ToolBarCustom::Filter |
                                  ToolBarCustom::UpdateColumn |
                                  ToolBarCustom::PrintEmail |
                                  ToolBarCustom::ReportViewTab |
                                  ToolBarCustom::PeriodSearch))
    , m_popup(new PopUp(this))
    , m_previewModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Lista documentelor: Rapoarte ecografice"));
    setWindowIcon(QIcon(":/img/documents/reportEcho.png"));
    setMinimumSize(900, 520);

    m_filter.startDate = QDateTime(QDate(2021, 1, 1), QTime(0, 0));
    m_filter.endDate = QDateTime(QDate::currentDate(), QTime(23, 59, 59));
    loadTableSettings();

    buildUi();
    initFilterModels();
    restoreFilterControls();
    initTable();
    initConnections();
    restoreTableSections();
    qInfo(logInfo()) << "ReportView: inițializarea listei rapoartelor ecografice.";
    reload();
}

ReportView::~ReportView()
{
    delete ui;
}

void ReportView::buildUi()
{
    m_toolBar->setStyles(m_db.toolButtonStyleForIcon(),
                         m_db.toolButtonStyleForText());

    m_toolBar->getBtnSendEmail()->hide();
    m_toolBar->getBtnSaecrPacient()->hide();
    ui->layoutToolBar->addWidget(m_toolBar);

    const QList<QToolButton *> popupButtons = {
        m_toolBar->getBtnAddDoc(),
        m_toolBar->getBtnDeletDoc(),
        m_toolBar->getBtnEditDoc(),
        m_toolBar->getBtnAddFilter(),
        m_toolBar->getBtnSetFilter(),
        m_toolBar->getBtnDeleteFilter(),
        m_toolBar->getBtnUpdateTable(),
        m_toolBar->getBtnHideShowColumn(),
        m_toolBar->getBtnPrintDoc(),
        m_toolBar->getBtnViewTabOrder(),
        m_toolBar->getBtnOpenPeriod()
    };
    m_toolBar->getBtnCreateReport()->hide();
    m_toolBar->getBtnViewTabOrder()->setText(tr("Preview concluzion"));
    for (QToolButton *button : popupButtons) {
        if (button)
            button->installEventFilter(this);
    }

    ui->dateStart->setDateTime(m_filter.startDate);
    ui->dateEnd->setDateTime(m_filter.endDate);
    ui->groupBoxFilter->hide();
    ui->groupBox_table_report->hide();

    const QString iconButtonStyle = m_db.toolButtonStyleForIcon();
    ui->btnSelectPeriod->setStyleSheet(iconButtonStyle);
    ui->openCatOrganization->setStyleSheet(iconButtonStyle);
    ui->openCatContract->setStyleSheet(iconButtonStyle);
    ui->openCatUser->setStyleSheet(iconButtonStyle);
    ui->btnOpenReport->setStyleSheet(
        globals().isSystemThemeDark ? "color: #fff;" : "color: #000;");
    configurePrintButton();


    m_previewModel->setHorizontalHeaderLabels({tr("Concluzie")});
    ui->tableReport->setModel(m_previewModel);
    ui->tableReport->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableReport->setWordWrap(true);
    ui->tableReport->horizontalHeader()->setStretchLastSection(true);
    ui->tableReport->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void ReportView::initTable()
{
    m_model = new ReportJournalModel(m_db, this);
    m_model->setBatchSize(100);
    m_proxy = new SortModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(ReportJournalModel::SortRole);
    m_proxy->setDynamicSortFilter(false);
    ui->tableView->setModel(m_proxy);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setWordWrap(false);
    ui->tableView->setTextElideMode(Qt::ElideRight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->viewport()->setMouseTracking(true);
    ui->tableView->viewport()->installEventFilter(this);

    const QSet<int> hiddenColumns = {
        ReportJournal::Id, ReportJournal::OrderId, ReportJournal::PatientId,
        ReportJournal::UserId, ReportJournal::Conclusion,
        ReportJournal::PatientSearch, ReportJournal::Uuid
    };
    for (const int column : hiddenColumns)
        ui->tableView->hideColumn(column);

    ui->tableView->setItemDelegateForColumn(
        ReportJournal::DeletionMark, new CenterIconDelegate(ui->tableView));
    ui->tableView->setItemDelegateForColumn(
        ReportJournal::AttachedImages, new CenterIconDelegate(ui->tableView));

    m_columns = new TableColumnsController(ui->tableView, this);
    m_columns->setFixedHiddenColumns(hiddenColumns);
    m_columns->setExcludedFromMenuColumns({
        ReportJournal::DeletionMark, ReportJournal::AttachedImages
    });
    connect(m_columns, &TableColumnsController::columnsChanged,
            this, &ReportView::onColumnsChanged);
}

void ReportView::initConnections()
{
    connect(m_toolBar, &ToolBarCustom::addDoc,
            this, &ReportView::addReport);
    connect(m_toolBar, &ToolBarCustom::deleteDoc,
            this, &ReportView::removeReport);
    connect(m_toolBar, &ToolBarCustom::editDoc,
            this, &ReportView::editReport);

    connect(m_toolBar, &ToolBarCustom::addFilter,
            this, &ReportView::toggleFilter);
    connect(m_toolBar, &ToolBarCustom::setFilter,
            this, &ReportView::applyFilter);
    connect(m_toolBar, &ToolBarCustom::deleteFilter,
            this, &ReportView::clearFilter);

    connect(m_toolBar, &ToolBarCustom::updateTable,
            this, &ReportView::reload);

    connect(m_toolBar, &ToolBarCustom::hideShowColumn,
            this, &ReportView::showColumnsMenu);

    connect(m_toolBar, &ToolBarCustom::printDoc,
            this, &ReportView::printReport);

    connect(m_toolBar, &ToolBarCustom::openPeriod,
            this, &ReportView::choosePeriod);
    connect(ui->btnApplyFilter, &QPushButton::clicked,
            this, &ReportView::applyFilter);
    connect(ui->btnClearFilter, &QPushButton::clicked,
            this, &ReportView::clearFilter);
    connect(ui->btnCloseFilter, &QPushButton::clicked,
            this, &ReportView::toggleFilter);
    connect(ui->btnSelectPeriod, &QToolButton::clicked,
            this, &ReportView::choosePeriod);
    connect(ui->openCatOrganization, &QToolButton::clicked, this, [this]() {
        const int id = ui->comboOrganizations->currentData(
            QueryRolesModel::roleForColumn(m_organizations->columnIndex("id"))).toInt();
        if (id <= 0)
            return;
        auto *dialog = new OrganizationDialog(m_db, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setProperty("isNew", false);
        dialog->setProperty("id", id);
        dialog->show();
    });
    connect(ui->openCatContract, &QToolButton::clicked, this, [this]() {
        const int organizationId = ui->comboOrganizations->currentData(
            QueryRolesModel::roleForColumn(m_organizations->columnIndex("id"))).toInt();
        const int contractId = ui->comboContracts->currentData(
            QueryRolesModel::roleForColumn(m_contracts->columnIndex("id"))).toInt();
        if (contractId <= 0)
            return;
        auto *dialog = new ContractDialog(m_db, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setProperty("isNew", false);
        dialog->setProperty("id", contractId);
        dialog->setProperty("idOrganization", organizationId);
        dialog->show();
    });
    connect(ui->openCatUser, &QToolButton::clicked, this, [this]() {
        const int id = ui->comboUsers->currentData(
            QueryRolesModel::roleForColumn(m_users->columnIndex("id"))).toInt();
        if (id <= 0)
            return;
        auto *dialog = new UserDialog(m_db, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setProperty("isNew", false);
        dialog->setProperty("id", id);
        dialog->show();
    });
    connect(ui->btnOpenReport, &QPushButton::clicked,
            this, &ReportView::editReport);
    connect(m_toolBar, &ToolBarCustom::viewTabOrder,
            this, &ReportView::toggleReportPreview);
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportView::organizationChanged);
    connect(ui->tableView, &QTableView::clicked,
            this, &ReportView::updateReportPreview);
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &ReportView::editReport);
    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &ReportView::showContextMenu);
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ReportView::fetchNextBatch);
}

const ReportJournal::Item *ReportView::currentItem(bool showWarning) const
{
    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        if (showWarning)
            QMessageBox::information(const_cast<ReportView *>(this),
                                     tr("Informație"), tr("Nu este marcat rândul."));
        return nullptr;
    }
    const QModelIndex sourceIndex = m_proxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return nullptr;
    return &m_model->itemAt(sourceIndex.row());
}

void ReportView::reload()
{
    qInfo(logInfo()).noquote()
        << QString("ReportView: încărcare rapoarte pentru perioada %1 - %2.")
               .arg(m_filter.startDate.toString("dd.MM.yyyy HH:mm:ss"),
                    m_filter.endDate.toString("dd.MM.yyyy HH:mm:ss"));

    m_model->setFilter(m_filter);
    m_model->reload();
    if (!m_model->lastError().isEmpty()) {
        CustomMessage message(this);
        message.setWindowTitle(tr("Încărcarea rapoartelor ecografice"));
        message.setTextTitle(tr("Lista rapoartelor nu a putut fi încărcată."));
        message.setDetailedText(m_model->lastError());
        message.exec();
    }
    ui->tableView->sortByColumn(m_filter.sortSection, m_filter.sortOrder);
    if (m_proxy->rowCount() > 0)
        ui->tableView->selectRow(0);
    updateReportPreview();
    updatePeriodText();

    qInfo(logInfo()) << "ReportView: rapoarte încărcate în primul lot:"
                     << m_proxy->rowCount();
}

void ReportView::addReport()
{
    CustomDialogInvestig dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    ReportDialog::ReportDialogParameters params;
    params.isNew      = true;
    params.idPatient  = 0;
    params.idOrder    = 0;
    params.status     = DocStatus::Unknow;
    params.systems    = dlg.selectedSystems();

    auto *report = new ReportDialog(m_db, params, this);
    report->setAttribute(Qt::WA_DeleteOnClose);
    connect(report, &ReportDialog::reportCreated,
            this, &ReportView::reload);
    connect(report, &ReportDialog::reportChanged,
            this, &ReportView::reload);
    connect(report, &ReportDialog::reportPost, this,
            [this](){
                reload();
                m_popup->setPopupText(tr("Documentul a fost salvat cu succes<br> in baza de date."));
                m_popup->show();
            });
    report->show();

    QTimer::singleShot(2000, this, [this]() {
        QMessageBox::information(this,
                                 tr("Crearea raportului."),
                                 tr("Crearea raportului ecografic este în stânsă legătură cu documentul <u>Comanda ecografică</u> !!! <br><br>"
                                    "Pentru salvarea raportului trebuie să existe o <b>Comandă ecografică</b> validă și pacientul asociat acesteia.<br><br>"
                                    "Pentru formarea corectă a rapoartelor statistice este necesar de urmat ordinea creării documentelor:<br>"
                                    "1. Comanda ecografică<br>"
                                    "2. Raport ecografic."),
                                 QMessageBox::Ok);
    });
}

void ReportView::removeReport()
{
    const auto *item = currentItem();
    if (!item)
        return;

    const int reportId = item->id;
    const int orderId = item->orderId;
    const QString reportNumber = item->numberDoc.trimmed();
    const QByteArray reportUuid = item->uuid;

    QMessageBox confirmation(QMessageBox::Question,
                             tr("Eliminarea raportului ecografic"),
                             tr("Doriți să fie eliminată și Comanda ecografică asociată?"),
                             QMessageBox::NoButton, this);

    confirmation.setInformativeText(
        tr("Da — se elimină Raportul nr.%1 și Comanda ecografică.\n"
           "Nu — se elimină numai Raportul ecografic.")
            .arg(reportNumber));

    QPushButton *yesButton = confirmation.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton = confirmation.addButton(tr("Nu"), QMessageBox::NoRole);
    QPushButton *cancelButton = confirmation.addButton(tr("Anulare"), QMessageBox::RejectRole);
    yesButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    noButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    cancelButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    confirmation.setDefaultButton(cancelButton);
    confirmation.exec();
    if (confirmation.clickedButton() == cancelButton)
        return;
    const bool deleteAssociatedOrder = confirmation.clickedButton() == yesButton;

    bool deleteFromCloud = false;
    if (globals().thisSqlite && globals().cloud_srv_exist) {
        QMessageBox cloudConfirmation(
            QMessageBox::Question,
            tr("Eliminarea documentelor din cloud"),
            tr("Doriți ca documentele selectate să fie eliminate și din baza de date cloud?"),
            QMessageBox::NoButton,
            this);
        cloudConfirmation.setInformativeText(
            tr("Șterge și din cloud — elimină documentele local și din MariaDB.\n"
               "Șterge numai local — documentele din MariaDB sunt păstrate."));
        QPushButton *cloudButton = cloudConfirmation.addButton(
            tr("Șterge și din cloud"), QMessageBox::YesRole);
        QPushButton *localButton = cloudConfirmation.addButton(
            tr("Șterge numai local"), QMessageBox::NoRole);
        QPushButton *cancelCloudButton = cloudConfirmation.addButton(
            tr("Anulare"), QMessageBox::RejectRole);
        cloudButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
        localButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
        cancelCloudButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
        cloudConfirmation.setDefaultButton(cancelCloudButton);
        cloudConfirmation.exec();

        if (cloudConfirmation.clickedButton() == cancelCloudButton)
            return;
        deleteFromCloud = cloudConfirmation.clickedButton() == cloudButton;
    }

    QSqlDatabase database = m_db.getDatabase();
    QString errorText;
    QString failedQuery;

    if (!database.transaction()) {
        errorText = database.lastError().text();
    } else {
        // În MariaDB, reportVideo are istoric ON DELETE RESTRICT; îl eliminăm
        // explicit înaintea raportului. În celelalte tabele acționează CASCADE.
        if (database.tables(QSql::Tables).contains(
                QStringLiteral("reportVideo"), Qt::CaseInsensitive)) {
            QSqlQuery deleteVideo(database);
            deleteVideo.prepare(
                QStringLiteral("DELETE FROM reportVideo WHERE id_reportEcho = :id"));
            deleteVideo.bindValue(QStringLiteral(":id"), reportId);
            if (!deleteVideo.exec()) {
                errorText = deleteVideo.lastError().text();
                failedQuery = deleteVideo.lastQuery();
            }
        }

        if (errorText.isEmpty()) {
            QSqlQuery deleteDocument(database);
            deleteDocument.prepare(
                deleteAssociatedOrder
                    ? QStringLiteral("DELETE FROM orderEcho WHERE id = :id")
                    : QStringLiteral("DELETE FROM reportEcho WHERE id = :id"));
            deleteDocument.bindValue(QStringLiteral(":id"),
                                     deleteAssociatedOrder ? orderId : reportId);
            if (!deleteDocument.exec() || deleteDocument.numRowsAffected() != 1) {
                errorText = deleteDocument.lastError().text();
                if (errorText.isEmpty())
                    errorText = deleteAssociatedOrder
                                    ? tr("Comanda asociată nu mai există sau nu a fost eliminată.")
                                    : tr("Raportul nu mai există sau nu a fost eliminat.");
                failedQuery = deleteDocument.lastQuery();
            }
        }

        if (!errorText.isEmpty()) {
            database.rollback();
        } else if (!database.commit()) {
            errorText = database.lastError().text();
        }
    }

    if (!errorText.isEmpty()) {
        CustomMessage message(this);
        message.setWindowTitle(tr("Eliminarea raportului ecografic"));
        message.setTextTitle(
            tr("Raportul ecografic nr.%1 nu a fost eliminat.")
                .arg(reportNumber));
        message.setDetailedText(tr("Eroare SQL: %1\nInterogare: %2")
                                    .arg(errorText,
                                         failedQuery.isEmpty()
                                             ? tr("tranzacție bază de date")
                                             : failedQuery));
        message.exec();
        qCritical(logCritical()).noquote()
            << "ReportView removeReport failed:" << errorText
            << "\nLast query:" << failedQuery;
        return;
    }

    // La SQLite imaginile sunt păstrate într-o bază separată, fără FK către
    // reportEcho, de aceea curățarea lor se face explicit după commit.
    if (globals().thisSqlite) {
        QSqlDatabase imageDatabase = m_db.getDatabaseImage();
        if (imageDatabase.isOpen() &&
            imageDatabase.tables(QSql::Tables).contains(
                QStringLiteral("imagesReports"), Qt::CaseInsensitive)) {
            QSqlQuery deleteImages(imageDatabase);
            deleteImages.prepare(
                deleteAssociatedOrder
                    ? QStringLiteral("DELETE FROM imagesReports WHERE id_orderEcho = :id")
                    : QStringLiteral("DELETE FROM imagesReports WHERE id_reportEcho = :id"));
            deleteImages.bindValue(QStringLiteral(":id"),
                                   deleteAssociatedOrder ? orderId : reportId);
            if (!deleteImages.exec()) {
                qWarning(logWarning()).noquote()
                    << "ReportView: raportul a fost eliminat, dar imaginile locale nu au putut fi curățate:"
                    << deleteImages.lastError().text();
            }
        }
    }

    QString cloudDeleteError;
    if (deleteFromCloud &&
        !removeCloudDocuments(reportUuid, deleteAssociatedOrder, &cloudDeleteError)) {
        CustomMessage message(this);
        message.setWindowTitle(tr("Sincronizarea eliminării"));
        message.setTextTitle(
            tr("Documentele au fost eliminate local, dar eliminarea din cloud a eșuat."));
        message.setDetailedText(cloudDeleteError);
        message.exec();
        qCritical(logCritical()).noquote()
            << "ReportView cloud delete failed:" << cloudDeleteError;
    }

    if (deleteAssociatedOrder) {
        qInfo(logInfo()).noquote()
            << QString("Raportul ecografic nr.%1 cu ID=%2 și comanda asociată ID=%3 au fost eliminate.")
                   .arg(reportNumber).arg(reportId).arg(orderId);
        m_popup->setPopupText(
            tr("Raportul și comanda asociată au fost eliminate<br>cu succes din baza de date."));
    } else {
        qInfo(logInfo()).noquote()
            << QString("Raportul ecografic nr.%1 cu ID=%2 a fost eliminat; comanda ID=%3 a fost păstrată.")
                   .arg(reportNumber).arg(reportId).arg(orderId);
        m_popup->setPopupText(
            tr("Raportul ecografic a fost eliminat.<br>Comanda asociată a fost păstrată."));
    }
    m_popup->show();
    reload();
}

bool ReportView::removeCloudDocuments(const QByteArray &reportUuid,
                                      bool removeAssociatedOrder,
                                      QString *error)
{
    if (reportUuid.isEmpty()) {
        if (error)
            *error = tr("Raportul local nu are UUID; documentul cloud nu poate fi identificat sigur.");
        return false;
    }

    const QString connectionName = QStringLiteral("report_delete_cloud_%1")
                                       .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool success = false;
    QString failure;
    {
        QSqlDatabase cloud = m_db.getDatabaseCloudThread(connectionName);
        if (!cloud.isOpen()) {
            failure = cloud.lastError().text();
        } else if (!cloud.transaction()) {
            failure = cloud.lastError().text();
        } else {
            QSqlQuery findReport(cloud);
            findReport.prepare(QStringLiteral(
                "SELECT id, id_orderEcho FROM reportEcho WHERE uuid = :uuid"));
            findReport.bindValue(QStringLiteral(":uuid"), reportUuid, QSql::Binary);
            if (!findReport.exec()) {
                failure = findReport.lastError().text();
            } else if (!findReport.next()) {
                // Copia cloud lipsește deja: starea dorită este realizată.
                success = cloud.commit();
                if (!success)
                    failure = cloud.lastError().text();
            } else {
                const qint64 cloudReportId = findReport.value(0).toLongLong();
                const qint64 cloudOrderId = findReport.value(1).toLongLong();

                if (cloud.tables(QSql::Tables).contains(
                        QStringLiteral("reportVideo"), Qt::CaseInsensitive)) {
                    QSqlQuery deleteVideo(cloud);
                    deleteVideo.prepare(removeAssociatedOrder
                        ? QStringLiteral("DELETE FROM reportVideo WHERE id_orderEcho = :id")
                        : QStringLiteral("DELETE FROM reportVideo WHERE id_reportEcho = :id"));
                    deleteVideo.bindValue(QStringLiteral(":id"),
                                          removeAssociatedOrder ? cloudOrderId : cloudReportId);
                    if (!deleteVideo.exec())
                        failure = deleteVideo.lastError().text();
                }

                if (failure.isEmpty()) {
                    QSqlQuery deleteDocument(cloud);
                    deleteDocument.prepare(removeAssociatedOrder
                        ? QStringLiteral("DELETE FROM orderEcho WHERE id = :id")
                        : QStringLiteral("DELETE FROM reportEcho WHERE id = :id"));
                    deleteDocument.bindValue(QStringLiteral(":id"),
                                             removeAssociatedOrder ? cloudOrderId : cloudReportId);
                    if (!deleteDocument.exec())
                        failure = deleteDocument.lastError().text();
                }

                if (failure.isEmpty()) {
                    success = cloud.commit();
                    if (!success)
                        failure = cloud.lastError().text();
                } else {
                    cloud.rollback();
                }
            }
        }
    }
    m_db.removeDatabaseThread(connectionName);

    if (!success && error)
        *error = failure.isEmpty() ? tr("Eroare necunoscută la eliminarea din cloud.")
                                  : failure;
    return success;
}

void ReportView::editReport()
{
    const auto *item = currentItem();
    if (!item)
        return;

    ReportDialog::ReportDialogParameters params;
    params.isNew            = false;
    params.id               = item->id;
    params.idOrder          = item->orderId;
    params.idPatient        = item->patientId;
    params.idUser           = item->userId;
    params.status           = DocStatus::determineStatusDoc(item->deletionMark);
    params.orderDisplayText = item->orderDescription;

    auto *dialog = new ReportDialog(m_db, params, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &ReportDialog::reportChanged, this, &ReportView::reload);
    connect(dialog, &ReportDialog::reportPost, this, &ReportView::reload);
    dialog->show();
}

void ReportView::printReport()
{
    printReportDocument(PrintType::Preview);
}

void ReportView::printReportDocument(PrintType::Column typePrint)
{
    const auto *item = currentItem();
    if (!item)
        return;

    ReportDialog::ReportDialogParameters params;
    params.isNew            = false;
    params.id               = item->id;
    params.idOrder          = item->orderId;
    params.idPatient        = item->patientId;
    params.idUser           = item->userId;
    params.status           = DocStatus::determineStatusDoc(item->deletionMark);
    params.orderDisplayText = item->orderDescription;
    ReportDialog dialog(m_db, params, this);
    dialog.onPrintDocument(typePrint);
}

void ReportView::openOrder()
{
    const auto *item = currentItem();
    if (!item || item->orderId <= 0)
        return;
    auto *dialog = new OrderDialog(m_db, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setProperty("isNew", false);
    dialog->setProperty("id", item->orderId);
    dialog->show();
}

void ReportView::applyFilter()
{
    if (ui->dateStart->dateTime() > ui->dateEnd->dateTime()) {
        QMessageBox::warning(this, tr("Verificarea perioadei"),
                             tr("Data de sfârșit nu poate fi mai mică decât data de început."),
                             QMessageBox::Ok);
        return;
    }
    m_filter.startDate = ui->dateStart->dateTime();
    m_filter.endDate = ui->dateEnd->dateTime();
    m_filter.nrDoc = ui->numberDoc->text().trimmed();
    m_filter.idOrganization = ui->comboOrganizations->currentData(
        QueryRolesModel::roleForColumn(m_organizations->columnIndex("id"))).toInt();
    m_filter.idContract = ui->comboContracts->currentData(
        QueryRolesModel::roleForColumn(m_contracts->columnIndex("id"))).toInt();
    m_filter.idUser = ui->comboUsers->currentData(
        QueryRolesModel::roleForColumn(m_users->columnIndex("id"))).toInt();
    reload();
    saveTableSettings();
    ui->groupBoxFilter->hide();
}

void ReportView::clearFilter()
{
    ui->numberDoc->clear();
    ui->comboOrganizations->setCurrentIndex(0);
    ui->comboContracts->setCurrentIndex(0);
    ui->comboUsers->setCurrentIndex(0);
    m_filter.nrDoc.clear();
    m_filter.idOrganization = 0;
    m_filter.idContract = 0;
    m_filter.idUser = 0;
    reload();
}

void ReportView::toggleFilter()
{
    ui->groupBoxFilter->setVisible(!ui->groupBoxFilter->isVisible());
}

void ReportView::choosePeriod()
{
    CustomPeriod dialog(this);
    dialog.setDateStart(ui->dateStart->date());
    dialog.setDateEnd(ui->dateEnd->date());
    if (dialog.exec() != QDialog::Accepted)
        return;
    ui->dateStart->setDateTime(dialog.getDateStart());
    ui->dateEnd->setDateTime(dialog.getDateEnd());
    applyFilter();
}

void ReportView::showColumnsMenu()
{
    if (!m_columns)
        return;
    QToolButton *button = m_toolBar->getBtnHideShowColumn();
    m_columns->showMenu(button->mapToGlobal(QPoint(0, button->height())));
}

void ReportView::showContextMenu(const QPoint &pos)
{
    if (!currentItem(false))
        return;
    QMenu menu(this);
    QAction *openReportAction = menu.addAction(
        QIcon(":/img/catalogs/open_catalog.png"), tr("Deschide raportul"));
    QAction *openOrderAction = menu.addAction(
        QIcon(":/img/documents/orderEcho.png"), tr("Deschide comanda asociată"));
    QAction *printAction = menu.addAction(
        QIcon(":/img/actions/print.png"), tr("Printează raportul"));
    QAction *selected = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    if (selected == openReportAction)
        editReport();
    else if (selected == openOrderAction)
        openOrder();
    else if (selected == printAction)
        printReport();
}

void ReportView::fetchNextBatch(int value)
{
    QScrollBar *bar = ui->tableView->verticalScrollBar();
    if (bar && value >= bar->maximum() - 8 && m_model->canFetchMore())
        m_model->fetchMore();
}

void ReportView::updatePeriodText()
{
    QString text = tr("Perioada: %1 - %2")
                       .arg(m_filter.startDate.toString("dd.MM.yyyy"),
                            m_filter.endDate.toString("dd.MM.yyyy"));
    if (!m_filter.nrDoc.isEmpty())
        text += tr("; număr: %1").arg(m_filter.nrDoc);
    if (m_filter.idOrganization > 0)
        text += tr("; organizație: %1").arg(ui->comboOrganizations->currentText());
    if (m_filter.idContract > 0)
        text += tr("; contract: %1").arg(ui->comboContracts->currentText());
    if (m_filter.idUser > 0)
        text += tr("; autor: %1").arg(ui->comboUsers->currentText());
    m_toolBar->setTextPeriod(text);
}

void ReportView::initFilterModels()
{
    m_organizations = new QueryRolesModel(
        m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql"),
        ui->comboOrganizations);
    m_organizations->setEmptyRowEnabled(true);
    ui->comboOrganizations->setModel(m_organizations);
    ui->comboOrganizations->setModelColumn(m_organizations->columnIndex("name"));

    m_users = new QueryRolesModel(
        m_db.getTextSQL(":/sql/queries/users_combo_view.sql"),
        ui->comboUsers);
    m_users->setEmptyRowEnabled(true);
    ui->comboUsers->setModel(m_users);
    ui->comboUsers->setModelColumn(m_users->columnIndex("name"));

    updateContractsModel();
}

void ReportView::restoreFilterControls()
{
    ui->saveFilter->setChecked(m_filter.saveFilter);
    if (!m_filter.saveFilter)
        return;

    ui->numberDoc->setText(m_filter.nrDoc);
    if (m_filter.idOrganization > 0) {
        ui->comboOrganizations->setCurrentIndex(
            m_organizations->rowById("id", m_filter.idOrganization));
        updateContractsModel();
    }
    if (m_filter.idContract > 0)
        ui->comboContracts->setCurrentIndex(
            m_contracts->rowById("id", m_filter.idContract));
    if (m_filter.idUser > 0)
        ui->comboUsers->setCurrentIndex(
            m_users->rowById("id", m_filter.idUser));
}

void ReportView::updateContractsModel()
{
    const int organizationId = m_organizations
        ? ui->comboOrganizations->currentData(QueryRolesModel::roleForColumn(
              m_organizations->columnIndex("id"))).toInt()
        : 0;

    delete m_contracts;
    m_contracts = nullptr;
    if (organizationId <= 0) {
        m_contracts = new QueryRolesModel(
            m_db.getTextSQL(":/sql/queries/contracts_view.sql"), ui->comboContracts);
        m_contracts->setEmptyRowEnabled(true);
        ui->comboContracts->setModel(m_contracts);
        ui->comboContracts->setModelColumn(m_contracts->columnIndex("contract_owner"));
        return;
    }

    QSqlQuery query(m_db.getDatabase());
    query.prepare(m_db.getTextSQL(
        globals().thisSqlite
            ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
            : ":/sql/queries/contracts_select_by_organization_mysql.sql"));
    query.addBindValue(organizationId);
    if (!query.exec()) {
        qWarning(logWarning()).noquote()
            << "ReportView contracts filter error:" << query.lastError().text();
        return;
    }
    m_contracts = new QueryRolesModel(QString(), ui->comboContracts);
    m_contracts->setQuery(std::move(query));
    m_contracts->setEmptyRowEnabled(true);
    ui->comboContracts->setModel(m_contracts);
    ui->comboContracts->setModelColumn(m_contracts->columnIndex("name"));
}

void ReportView::organizationChanged(int)
{
    updateContractsModel();
}

void ReportView::updateReportPreview()
{
    m_previewModel->removeRows(0, m_previewModel->rowCount());
    if (!ui->groupBox_table_report->isVisible())
        return;

    const auto *item = currentItem(false);
    if (!item)
        return;
    auto *conclusion = new QStandardItem(item->conclusion);
    conclusion->setEditable(false);
    m_previewModel->appendRow(conclusion);
}

void ReportView::toggleReportPreview()
{
    ui->groupBox_table_report->setVisible(!ui->groupBox_table_report->isVisible());
    if (ui->groupBox_table_report->isVisible())
        updateReportPreview();
}

void ReportView::configurePrintButton()
{
    ui->btnPrintReport->setStyleSheet(
        globals().isSystemThemeDark ? "color: #fff;" : "color: #000;");
    disconnect(ui->btnPrintReport, &QAbstractButton::clicked, nullptr, nullptr);

    if (QMenu *oldMenu = ui->btnPrintReport->menu()) {
        ui->btnPrintReport->setMenu(nullptr);
        oldMenu->deleteLater();
    }

    if (!globals().showDesignerMenuPrint) {
        connect(ui->btnPrintReport, &QPushButton::clicked,
                this, &ReportView::printReport, Qt::UniqueConnection);
        return;
    }

    auto *menu = new QMenu(ui->btnPrintReport);
    QAction *preview = menu->addAction(QIcon(":/img/actions/print.png"), tr("Preview"));
    QAction *designer = menu->addAction(tr("Designer"));
    connect(preview, &QAction::triggered, this,
            [this]() { printReportDocument(PrintType::Preview); });
    connect(designer, &QAction::triggered, this,
            [this]() { printReportDocument(PrintType::Designer); });
    ui->btnPrintReport->setMenu(menu);
}

void ReportView::loadTableSettings()
{
    const QJsonObject object = m_settings.getJsonObject(m_settingsGroup);

    const QDateTime savedStart = QDateTime::fromString(
        object.value("startDate").toString(), Qt::ISODateWithMs);
    const QDateTime savedEnd = QDateTime::fromString(
        object.value("endDate").toString(), Qt::ISODateWithMs);
    if (savedStart.isValid())
        m_filter.startDate = savedStart;
    if (savedEnd.isValid())
        m_filter.endDate = savedEnd;

    // Nu permitem restaurarea unei perioade invalide dintr-un fișier vechi
    // sau modificat manual.
    if (m_filter.startDate > m_filter.endDate) {
        m_filter.startDate = QDateTime(QDate(2021, 1, 1), QTime(0, 0));
        m_filter.endDate = QDateTime(QDate::currentDate(), QTime(23, 59, 59));
    }

    const QJsonObject filter = object.value("filter").toObject();
    m_filter.saveFilter = filter.value("saveFilter").toBool(false);
    if (m_filter.saveFilter) {
        m_filter.nrDoc = filter.value("nr_doc").toString();
        m_filter.idOrganization = filter.value("id_organization").toInt();
        m_filter.idContract = filter.value("id_contract").toInt();
        m_filter.idUser = filter.value("id_user").toInt();
    }

    const QJsonObject sections = object.value("sections").toObject();
    for (auto it = sections.constBegin(); it != sections.constEnd(); ++it) {
        bool valid = false;
        const int section = it.key().toInt(&valid);
        if (valid)
            m_filter.sectionSizes.insert(section, it.value().toInt());
    }

    const QJsonObject hidden = object.value("hide_show_sections").toObject();
    for (auto it = hidden.constBegin(); it != hidden.constEnd(); ++it) {
        bool valid = false;
        const int section = it.key().toInt(&valid);
        if (valid)
            m_filter.hiddenSections.insert(section, it.value().toInt() != 0);
    }

    const QJsonObject sort = object.value("sort").toObject();
    m_filter.sortSection = sort.value("section").toInt(ReportJournal::DateDoc);
    m_filter.sortOrder = sort.value("direction").toInt(
                             static_cast<int>(Qt::DescendingOrder)) ==
                             static_cast<int>(Qt::AscendingOrder)
                         ? Qt::AscendingOrder
                         : Qt::DescendingOrder;
}

void ReportView::restoreTableSections()
{
    QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    ui->tableView->setUpdatesEnabled(false);
    const bool stretchLast = header->stretchLastSection();
    header->setStretchLastSection(false);

    for (int section = 0; section < header->count(); ++section) {
        int width = m_filter.sectionSizes.value(section, 0);
        if (width <= 0)
            width = qMax(header->sectionSizeHint(section), header->defaultSectionSize());
        header->resizeSection(section, width);
    }

    if (m_columns)
        m_columns->setHiddenSections(m_filter.hiddenSections);

    header->setStretchLastSection(stretchLast);
    if (m_filter.sortSection >= 0 &&
        m_filter.sortSection < ui->tableView->model()->columnCount()) {
        ui->tableView->sortByColumn(m_filter.sortSection, m_filter.sortOrder);
    }
    ui->tableView->setUpdatesEnabled(true);
}

int ReportView::lastVisibleSection() const
{
    const QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return -1;
    for (int section = header->count() - 1; section >= 0; --section) {
        if (!header->isSectionHidden(section))
            return section;
    }
    return -1;
}

bool ReportView::previewImagesDocs(QEvent *event)
{
    auto *helpEvent = static_cast<QHelpEvent *>(event);
    const QModelIndex index = ui->tableView->indexAt(helpEvent->pos());

    if (!index.isValid())
        return false;

    const QModelIndex sourceIndex = m_proxy->mapToSource(index);
    if (!sourceIndex.isValid())
        return false;
    const int idReport = m_model->itemAt(sourceIndex.row()).id;
    if (idReport <= 0)
        return false;

    QPixmap outPixmap1, outPixmap2, outPixmap3;

    QSqlQuery q(globals().thisSqlite ? m_db.getDatabaseImage() : m_db.getDatabase());
    q.prepare(R"(
        SELECT
            image_1,
            image_2,
            image_3
        FROM
            imagesReports
        WHERE
            id_reportEcho = :id_reportEcho
    )");
    q.bindValue(":id_reportEcho", idReport);

    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "ReportView preview images error:" << q.lastError().text()
            << "| report_id=" << idReport;
        return false;
    }
    if (!q.next())
        return false;

    auto loadPixmap = [](const QVariant &value, QPixmap &pixmap) {
        const QByteArray storedData = value.toByteArray();
        if (storedData.isEmpty())
            return;

        const QByteArray decodedData = QByteArray::fromBase64(storedData);
        if (!decodedData.isEmpty() && pixmap.loadFromData(decodedData))
            return;

        // Compatibilitate cu înregistrările în care imaginea este BLOB binar,
        // nu text codificat Base64.
        pixmap.loadFromData(storedData);
    };
    loadPixmap(q.value(0), outPixmap1);
    loadPixmap(q.value(1), outPixmap2);
    loadPixmap(q.value(2), outPixmap3);

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

    if (!outPixmap1.isNull() || !outPixmap2.isNull() || !outPixmap3.isNull()) {
        QToolTip::showText(helpEvent->globalPos(),
                           html, ui->tableView->viewport());
        return true;
    }

    return false;
}

void ReportView::saveTableSettings()
{
    QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    m_settings.setValue(m_settingsGroup, "startDate",
                        ui->dateStart->dateTime());
    m_settings.setValue(m_settingsGroup, "endDate",
                        ui->dateEnd->dateTime());
    m_settings.setValue(m_settingsGroup, "filter/saveFilter",
                        ui->saveFilter->isChecked());
    m_settings.setValue(m_settingsGroup, "filter/nr_doc",
                        ui->saveFilter->isChecked() ? ui->numberDoc->text().trimmed()
                                                    : QString());
    m_settings.setValue(m_settingsGroup, "filter/id_organization",
                        ui->saveFilter->isChecked() ? m_filter.idOrganization : 0);
    m_settings.setValue(m_settingsGroup, "filter/id_contract",
                        ui->saveFilter->isChecked() ? m_filter.idContract : 0);
    m_settings.setValue(m_settingsGroup, "filter/id_user",
                        ui->saveFilter->isChecked() ? m_filter.idUser : 0);

    m_settings.setValue(m_settingsGroup, "sort/section",
                        header->sortIndicatorSection());
    m_settings.setValue(m_settingsGroup, "sort/direction",
                        static_cast<int>(header->sortIndicatorOrder()));

    const int stretchedSection = lastVisibleSection();
    for (int section = 0; section < header->count(); ++section) {
        m_settings.setValue(m_settingsGroup,
                            QString("hide_show_sections/%1").arg(section),
                            header->isSectionHidden(section) ? 1 : 0);
        if (section == stretchedSection)
            continue;
        const int width = header->sectionSize(section);
        if (width > 0)
            m_settings.setValue(m_settingsGroup,
                                QString("sections/%1").arg(section), width);
    }
    m_settings.save();
}

void ReportView::onColumnsChanged()
{
    if (m_columns)
        m_filter.hiddenSections = m_columns->hiddenSections();
    saveTableSettings();
}

bool ReportView::isValidIndex(const QModelIndex &index)
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

void ReportView::closeEvent(QCloseEvent *event)
{
    saveTableSettings();
    QDialog::closeEvent(event);
}

bool ReportView::eventFilter(QObject *watched, QEvent *event)
{
    // previzualizarea imaginelor atasate
    if (watched == ui->tableView->viewport() &&
        event->type() == QEvent::ToolTip) {
        return previewImagesDocs(event);
    }

    auto *button = qobject_cast<QToolButton *>(watched);
    if (!button || !m_popup)
        return QDialog::eventFilter(watched, event);

    if (event->type() == QEvent::Leave) {
        m_popup->hidePop();
        return true;
    }
    if (event->type() != QEvent::Enter)
        return QDialog::eventFilter(watched, event);

    QString text;
    if (button == m_toolBar->getBtnAddDoc())
        text = tr("Adaugă (Ins)");
    else if (button == m_toolBar->getBtnDeletDoc())
        text = tr("Elimină (Del)");
    else if (button == m_toolBar->getBtnEditDoc())
        text = tr("Editează (F2)");
    else if (button == m_toolBar->getBtnAddFilter())
        text = tr("Deschide filtru (Ctrl + F1)");
    else if (button == m_toolBar->getBtnSetFilter())
        text = tr("Filtru rapid (Ctrl + F2)");
    else if (button == m_toolBar->getBtnDeleteFilter())
        text = tr("Șterge filtru (Ctrl + F3)");
    else if (button == m_toolBar->getBtnUpdateTable())
        text = tr("Actualizează (F5)");
    else if (button == m_toolBar->getBtnHideShowColumn())
        text = tr("Ascunde/prezintă secții<br> (Ctrl + H)");
    else if (button == m_toolBar->getBtnPrintDoc())
        text = tr("Printare (Ctrl + P)");
    else if (button == m_toolBar->getBtnViewTabOrder())
        text = tr("Vizualizarea concluziei (Ctrl + T)");
    else if (button == m_toolBar->getBtnOpenPeriod())
        text = tr("Perioada (Ctrl + Shift + P)");

    if (text.isEmpty())
        return QDialog::eventFilter(watched, event);

    const QPoint position = button->mapToGlobal(QPoint(0, button->height()));
    m_popup->setPopupText(text);
    m_popup->showFromGeometryTimer(position);
    return true;
}
