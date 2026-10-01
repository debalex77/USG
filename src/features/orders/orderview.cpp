/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "orderview.h"
#include "ui_orderview.h"
#include "settings/settingsservice.h"

#include <common/applicationpathscontext.h>
#include <QScopeGuard>
#include <features/printing/orderprintservice.h>
#include <features/printing/reportprintservice.h>

OrderView::OrderView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OrderView)
    , m_settings(ApplicationPathsContext::instance().tableSettingsFilePath())
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
    updateModelContracts(m_filter.idOrganization);
    updateModelUsers();

    initTableView();
    loadFilterData();
    updateTableView();
    updateTextPeriod();

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
    saveSettingsJournal();
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

    if (modelTable->canFetchMore()) {
        modelTable->fetchMore();
        showJournalLoadError();
    }
}

// **********************************************************************************
// --- procesarea butoanelor toolBar-lui

void OrderView::onAddDoc()
{
    OrderDialog *doc = new OrderDialog(m_db, this);
    doc->setAttribute(Qt::WA_DeleteOnClose);
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

    OrderDialog *doc = new OrderDialog(m_db, item.id, this);
    doc->setAttribute(Qt::WA_DeleteOnClose);
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
    // Copiem datele: dialogurile de confirmare rulează bucle de evenimente
    // în care jurnalul se poate reîncărca.
    const OrderJournal::Item item = modelTable->itemAt(sourceIndex.row());
    const int id_order = item.id;
    const QString orderNumber = item.numberDoc.trimmed();

    QSqlQuery qCheck(m_db.getDatabase());
    qCheck.prepare(
        MainDatabaseConnectionContext::instance().isMariaDb()
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

    // Confirmarea este cerută întotdeauna, ca în ReportView::removeReport.
    QMessageBox messageBox(
        QMessageBox::Question,
        tr("Eliminarea documentului."),
        exist
            ? tr("Există documente subordonate care vor fi eliminate.<br>Doriți să continuați?")
            : tr("Doriți să eliminați Comanda ecografică nr.%1?").arg(orderNumber.toHtmlEscaped()),
        QMessageBox::NoButton,
        this
        );
    if (exist)
        messageBox.setDetailedText(tr("Va fi eliminat documentul subordonat:\n%1")
                                       .arg(qCheck.value("report").toString()));
    QPushButton *yesButton    = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton     = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);
    QPushButton *cancelButton = messageBox.addButton(tr("Anulare"), QMessageBox::RejectRole);

    yesButton->setStyleSheet(styleBtnMessageBox);
    noButton->setStyleSheet(styleBtnMessageBox);
    cancelButton->setStyleSheet(styleBtnMessageBox);
    messageBox.setDefaultButton(cancelButton);

    messageBox.exec();

    if (messageBox.clickedButton() != yesButton)
        return;

    bool deleteFromCloud = false;
    if (MainDatabaseConnectionContext::instance().isSqlite()
        && SettingsService::instance().synchronization().enabled) {
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
        cloudButton->setStyleSheet(styleBtnMessageBox);
        localButton->setStyleSheet(styleBtnMessageBox);
        cancelCloudButton->setStyleSheet(styleBtnMessageBox);
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
        // explicit înaintea comenzii. În celelalte tabele acționează CASCADE.
        if (database.tables(QSql::Tables).contains(
                QStringLiteral("reportVideo"), Qt::CaseInsensitive)) {
            QSqlQuery deleteVideo(database);
            deleteVideo.prepare(
                QStringLiteral("DELETE FROM reportVideo WHERE id_orderEcho = :id"));
            deleteVideo.bindValue(QStringLiteral(":id"), id_order);
            if (!deleteVideo.exec()) {
                errorText = deleteVideo.lastError().text();
                failedQuery = deleteVideo.lastQuery();
            }
        }

        if (errorText.isEmpty()) {
            QSqlQuery deleteDocument(database);
            deleteDocument.prepare(QStringLiteral("DELETE FROM orderEcho WHERE id = :id"));
            deleteDocument.bindValue(QStringLiteral(":id"), id_order);
            if (!deleteDocument.exec() || deleteDocument.numRowsAffected() != 1) {
                errorText = deleteDocument.lastError().text();
                if (errorText.isEmpty())
                    errorText = tr("Comanda nu mai există sau nu a fost eliminată.");
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
        message.setWindowTitle(tr("Eliminarea documentului"));
        message.setTextTitle(tr("Nu s-a putut de eliminat documentul nr.%1 din baza de date")
                                 .arg(orderNumber));
        message.setDetailedText(tr("Eroare SQL: %1\nInterogare: %2")
                                    .arg(errorText,
                                         failedQuery.isEmpty()
                                             ? tr("tranzacție bază de date")
                                             : failedQuery));
        message.exec();
        qCritical(logCritical()).noquote()
            << "OrderView onDeleteDoc failed:" << errorText
            << "\nLast query:" << failedQuery;
        return;
    }

    // La SQLite imaginile sunt păstrate într-o bază separată, fără FK către
    // orderEcho, de aceea curățarea lor se face explicit după commit.
    if (MainDatabaseConnectionContext::instance().isSqlite()) {
        QSqlDatabase imageDatabase = m_db.getDatabaseImage();
        if (imageDatabase.isOpen() &&
            imageDatabase.tables(QSql::Tables).contains(
                QStringLiteral("imagesReports"), Qt::CaseInsensitive)) {
            QSqlQuery deleteImages(imageDatabase);
            deleteImages.prepare(
                QStringLiteral("DELETE FROM imagesReports WHERE id_orderEcho = :id"));
            deleteImages.bindValue(QStringLiteral(":id"), id_order);
            if (!deleteImages.exec()) {
                qWarning(logWarning()).noquote()
                    << "OrderView: comanda a fost eliminată, dar imaginile locale nu au putut fi curățate:"
                    << deleteImages.lastError().text();
            }
        }
    }

    QString cloudDeleteError;
    if (deleteFromCloud && !removeCloudOrder(item.uuid, &cloudDeleteError)) {
        CustomMessage message(this);
        message.setWindowTitle(tr("Sincronizarea eliminării"));
        message.setTextTitle(
            tr("Documentele au fost eliminate local, dar eliminarea din cloud a eșuat."));
        message.setDetailedText(cloudDeleteError);
        message.exec();
        qCritical(logCritical()).noquote()
            << "OrderView cloud delete failed:" << cloudDeleteError;
    }

    popUp->setPopupText(tr("Documentul este eliminat<br> cu succes din baza de date."));
    popUp->show();

    qInfo(logInfo())
        << QString("Eliminat documentul 'Comanda ecografica nr.%1' cu ID='%2' din baza de date.")
               .arg(orderNumber)
               .arg(id_order);

    updateTableView();
}

bool OrderView::removeCloudOrder(const QByteArray &orderUuid, QString *error)
{
    if (orderUuid.isEmpty()) {
        if (error)
            *error = tr("Comanda locală nu are UUID; documentul cloud nu poate fi identificat sigur.");
        return false;
    }

    // Ștergerea din cloud rulează pe firul GUI și poate dura (rețea lentă,
    // server indisponibil): utilizatorul vede de ce fereastra nu răspunde.
    ProcessingAction progress(this);
    progress.setTxtInfo(tr("Se elimină documentele din cloud ..."));
    progress.show();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto restoreCursor = qScopeGuard([] { QApplication::restoreOverrideCursor(); });
    // desenează dialogul înainte de blocare; fără input, ca să nu pornească alte acțiuni
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    const QString connectionName = QStringLiteral("order_delete_cloud_%1")
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
            QSqlQuery findOrder(cloud);
            findOrder.prepare(QStringLiteral("SELECT id FROM orderEcho WHERE uuid = :uuid"));
            findOrder.bindValue(QStringLiteral(":uuid"), orderUuid, QSql::Binary);
            if (!findOrder.exec()) {
                failure = findOrder.lastError().text();
            } else if (!findOrder.next()) {
                // Copia cloud lipsește deja: starea dorită este realizată.
                success = cloud.commit();
                if (!success)
                    failure = cloud.lastError().text();
            } else {
                const qint64 cloudOrderId = findOrder.value(0).toLongLong();

                if (cloud.tables(QSql::Tables).contains(
                        QStringLiteral("reportVideo"), Qt::CaseInsensitive)) {
                    QSqlQuery deleteVideo(cloud);
                    deleteVideo.prepare(
                        QStringLiteral("DELETE FROM reportVideo WHERE id_orderEcho = :id"));
                    deleteVideo.bindValue(QStringLiteral(":id"), cloudOrderId);
                    if (!deleteVideo.exec())
                        failure = deleteVideo.lastError().text();
                }

                if (failure.isEmpty()) {
                    QSqlQuery deleteDocument(cloud);
                    deleteDocument.prepare(QStringLiteral("DELETE FROM orderEcho WHERE id = :id"));
                    deleteDocument.bindValue(QStringLiteral(":id"), cloudOrderId);
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

void OrderView::onAddFilter()
{
    const bool hide = !ui->groupBoxFilter->isHidden();
    // Închiderea fără „Aplică” renunță la valorile modificate în panou.
    if (hide)
        syncFilterControls();
    ui->groupBoxFilter->setHidden(hide);
}

void OrderView::onSetFilter()
{
    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!isValidIndex(proxyIndex))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(proxyIndex);
    // copie: meniul rulează o buclă de evenimente
    const OrderJournal::Item item = modelTable->itemAt(sourceIndex.row());

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
        m_filter.idOrganization = item.idOrganizations;
        m_filter.idContract     = 0; // contractul anterior poate aparține altei organizații

    } else if (selectedAction == filterPacient) {
        m_filter.patientId = item.patientId;
        m_filter.patientName = item.patientName;

    } else if (selectedAction == filterAuthor) {
        m_filter.idUser = item.idUsers;

    } else {
        return;
    }

    syncFilterControls();
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
    printOrder(PrintType::Preview);
}

void OrderView::onSendEmail()
{
    // LimeReport poate procesa evenimente în timpul exportului; o a doua
    // pornire ar reutiliza conexiunea DB a exportului curent (același fir).
    if (m_emailExportRunning)
        return;

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

    // Fiecare trimitere primește propriul director temporar. Astfel două
    // exporturi simultane nu își pot șterge sau amesteca documentele.
    QString exportDirectory;
    QString exportError;
    if (!AgentSendEmail::prepareExportDirectory(&exportDirectory, &exportError)) {
        QMessageBox::warning(this,
                             tr("Transmiterea prin e-mail"),
                             exportError,
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
    data.thisMySQL                    = MainDatabaseConnectionContext::instance().isMariaDb();
    data.id_order                     = item.id;
    data.id_report                    = -1;
    data.id_patient                   = item.patientId;
    const Settings::OrganizationSettings &printSettings =
        SettingsService::instance().organization();
    data.printOrganizationId          = printSettings.organizationId;
    data.printDoctorId                = printSettings.defaultDoctorId;
    data.printNurseId                 = printSettings.defaultNurseId;
    data.unitMeasure                  = globals().unitMeasure;
    data.pathTemplatesDocs            = ApplicationPathsContext::instance().data().templatesDirectory;
    data.filePDF                      = exportDirectory;

    // LimeReport utilizeaza un ScriptEngineManager global bazat pe QJSEngine.
    // Crearea/distrugerea motoarelor de raport in thread-uri diferite corupe
    // starea singletonului si provoaca abort la inchiderea aplicatiei.
    // Exportul LimeReport trebuie executat in thread-ul GUI, unde este folosit
    // si in restul aplicatiei.
    auto *worker = new DocEmailExporterWorker(m_db, &m_dbProvider, data, this);

    connect(worker, &DocEmailExporterWorker::setTextInfo,
            this, &OrderView::updateEmailExportProgress, Qt::QueuedConnection);
    connect(worker, &DocEmailExporterWorker::finished,
            this, &OrderView::launchEmailAgent, Qt::QueuedConnection);
    connect(worker, &DocEmailExporterWorker::finished,
            worker, &QObject::deleteLater);

    m_emailExportRunning = true;
    QTimer::singleShot(0, worker, &DocEmailExporterWorker::process);
}

void OrderView::updateEmailExportProgress(const QString &text)
{
    if (loader)
        loader->setProperty("txtInfo", text);
}

void OrderView::launchEmailAgent(const QVector<DatesForAgentEmail> &exportedData)
{
    m_emailExportRunning = false;
    if (loader)
        loader->close();

    if (exportedData.isEmpty() || !exportedData.constFirst().success) {
        const QString errorText = exportedData.isEmpty()
            ? tr("Exportul documentelor nu s-a finalizat. Verificați jurnalul aplicației.")
            : exportedData.constFirst().errorText;
        QMessageBox::critical(this,
                              tr("Transmiterea prin e-mail"),
                              errorText,
                              QMessageBox::Ok);
        if (!exportedData.isEmpty())
            AgentSendEmail::removeExportDirectory(exportedData.constFirst().exportDirectory);
        return;
    }

    const DatesForAgentEmail &data = exportedData.constFirst();
    AgentSendEmail::MailContext context;
    context.organizationId    = data.organizationId;
    context.thisReports       = false;
    context.nrOrder           = data.nr_order;
    context.nrReport          = data.nr_report;
    context.emailTo           = data.emailTo;
    context.namePatient       = data.name_patient;
    context.nameDoctor        = data.name_doctor_execute;
    context.attachments       = data.attachments;
    context.exportDirectory   = data.exportDirectory;
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

    // Dacă raportul există deja, îl deschidem; un raport marcat ca șters nu
    // este considerat document activ asociat comenzii.
    if (reportParametersForOrder(item, params)) {
        openReportDocument(params);
        return;
    }

    // determinam investigatii din 'Order'
    QStringList codes;
    QSqlQuery q(m_db.getDatabase());
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

        updatePrintButtons(
            SettingsService::instance().user().printMenuMode
            == Settings::PrintMenuMode::PreviewAndDesigner);

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
            MainDatabaseConnectionContext::instance().isMariaDb()
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
        {
            // Ambele date se schimbă împreună; validarea per câmp ar compara
            // începutul nou cu sfârșitul vechi și ar respinge perioada.
            const QSignalBlocker blockerStart(ui->dateStart);
            const QSignalBlocker blockerEnd(ui->dateEnd);
            ui->dateStart->setDateTime(dlg.getDateStart());
            ui->dateEnd->setDateTime(dlg.getDateEnd());
        }
        validatePeriodAndUpdate();
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

    Settings::UserPreferencesData preferences = SettingsService::instance().user();
    preferences.printMenuMode = showDesignerMenuPrint
                                    ? Settings::PrintMenuMode::PreviewAndDesigner
                                    : Settings::PrintMenuMode::Standard;
    SettingsService::instance().setUser(preferences);

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

    if (SettingsService::instance().user().printMenuMode
        == Settings::PrintMenuMode::Standard) {
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

    const int orderId = static_cast<int>(modelTable->itemAt(sourceIndex.row()).id);
    if (orderId <= 0)
        return;

    // Printarea nu are nevoie de formularul documentului; serviciul citește
    // direct datele comenzii.
    OrderPrintService service(m_db, m_db.getDatabase());
    OrderPrintService::Request request;
    request.orderId      = orderId;
    request.mode         = typePrint;
    request.reportParent = this;

    const OrderPrintService::Result result = service.print(request);
    if (!result.success)
        showPrintError(result.error);
}

void OrderView::printReport(PrintType::Column typePrint)
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxyTable->mapToSource(idx);
    if (!sourceIndex.isValid())
        return;

    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    ReportDialog::ReportDialogParameters params;
    if (!reportParametersForOrder(item, params))
        return;

    printReport(params, typePrint);
}

void OrderView::printReport(
    const ReportDialog::ReportDialogParameters &params,
    const PrintType::Column typePrint)
{
    if (params.id <= 0)
        return;

    // Ca în ReportDialog la deschidere: ștampila și semnătura sunt implicit ascunse.
    ReportPrintService service(m_db, m_db.getDatabase());
    ReportPrintService::Request request;
    request.reportId     = params.id;
    request.mode         = typePrint;
    request.reportParent = this;

    const ReportPrintService::Result result = service.print(request);
    if (!result.success)
        showPrintError(result.error);
}

void OrderView::showJournalLoadError()
{
    if (!modelTable)
        return;

    const QString error = modelTable->takeLastError();
    if (error.isEmpty())
        return;

    CustomMessage message(this);
    message.setWindowTitle(windowTitle());
    message.setTextTitle(tr("Lista documentelor nu a putut fi încărcată complet."));
    message.setDetailedText(error);
    message.exec();
}

void OrderView::showPrintError(const QString &error)
{
    CustomMessage message(this);
    message.setWindowTitle(QGuiApplication::applicationDisplayName());
    message.setTextTitle(tr("Printare nu este posibilă !!!"));
    message.setDetailedText(error);
    message.exec();
}

bool OrderView::reportParametersForOrder(
    const OrderJournal::Item &order,
    ReportDialog::ReportDialogParameters &params)
{
    if (order.id <= 0)
        return false;

    QSqlQuery query(m_db.getDatabase());
    query.prepare(QStringLiteral(R"(
        SELECT
            id,
            deletionMark,
            id_users
        FROM
            reportEcho
        WHERE
            id_orderEcho = :id_orderEcho AND
            deletionMark <> :deleted
        ORDER BY
            id DESC
        LIMIT 1
    )"));
    query.bindValue(QStringLiteral(":id_orderEcho"), order.id);
    query.bindValue(QStringLiteral(":deleted"), DocStatus::DeletionMark);

    if (!query.exec()) {
        qWarning(logWarning()).noquote()
            << "OrderView: verificarea raportului asociat a eșuat:"
            << query.lastError().text()
            << "\nLast query:" << query.lastQuery();
        return false;
    }

    if (!query.next())
        return false;

    params = {};
    params.isNew = false;
    params.id = query.value(QStringLiteral("id")).toInt();
    params.idOrder = order.id;
    params.idPatient = order.patientId;
    params.idUser = query.value(QStringLiteral("id_users")).toInt();
    params.status = DocStatus::determineStatusDoc(
        query.value(QStringLiteral("deletionMark")).toInt());
    params.orderDisplayText = QStringLiteral("Comanda ecografică nr.%1 din %2")
                                  .arg(order.numberDoc.trimmed(), order.dateDocText);
    return params.id > 0;
}

void OrderView::openReportDocument(
    const ReportDialog::ReportDialogParameters &params)
{
    auto *report = new ReportDialog(m_db, params, this);
    report->setAttribute(Qt::WA_DeleteOnClose);
    connect(report, &ReportDialog::reportChanged,
            this, &OrderView::updateTableView, Qt::UniqueConnection);
    connect(report, &ReportDialog::reportPost,
            this, &OrderView::updateTableView, Qt::UniqueConnection);
    report->show();
}


// **********************************************************************************
// --- procesarea actiunilor cu tableView

void OrderView::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);

    // Orice schimbare a rândului curent (mouse, tastatură, reîncărcare)
    // actualizează previzualizarea.
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

    // Combo-urile modifică doar panoul; filtrul jurnalului (m_filter) se
    // schimbă numai la „Aplică” (onApplyFilter).
    if (qobject_cast<QComboBox*>(sender()) != ui->comboOrganizations)
        return;

    const int organizationId = comboCurrentId(ui->comboOrganizations, modelOrganizations);
    if (organizationId == m_contractsOrganizationId)
        return;

    // Lista contractelor depinde de organizație; se propune contractul
    // implicit al organizației alese.
    const int id_contracts = ui->comboOrganizations->currentData(
        modelOrganizations->roleForColumn("id_contracts")).toInt();

    const QSignalBlocker blocker(ui->comboContracts);
    updateModelContracts(organizationId);
    if (organizationId > 0 && id_contracts > 0 && modelContracts) {
        const int row = modelContracts->rowById("id", id_contracts);
        if (row >= 0)
            ui->comboContracts->setCurrentIndex(row);
    }
}

void OrderView::onApplyFilter()
{
    m_filter.nrDoc          = ui->numberDoc->text().trimmed();
    m_filter.idOrganization = comboCurrentId(ui->comboOrganizations, modelOrganizations);
    m_filter.idContract     = comboCurrentId(ui->comboContracts, modelContracts);
    m_filter.idUser         = comboCurrentId(ui->comboUsers, modelUsers);
    updateTableView();
    updateTextPeriod();
    ui->groupBoxFilter->setHidden(true);
    saveSettingsJournal();
}

void OrderView::slotContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = ui->tableView->indexAt(pos);
    if (!index.isValid())
        return;

    // Acțiunile trebuie aplicate rândului pe care s-a deschis meniul, nu unei
    // selecții anterioare din jurnal.
    ui->tableView->setCurrentIndex(index);
    ui->tableView->selectRow(index.row());

    const QModelIndex sourceIndex = proxyTable->mapToSource(index);
    if (!sourceIndex.isValid())
        return;

    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    ReportDialog::ReportDialogParameters reportParams;
    const bool hasReport = reportParametersForOrder(item, reportParams);

    QMenu menu(this);

    QAction *actionNewDoc = menu.addAction(QIcon(":/img/toolBar/add.png"),
                                           tr("Creează document nou."));
    QAction *actionEditDoc = menu.addAction(QIcon(":/img/toolBar/edit.png"),
                                            tr("Editează documentul."));
    QAction *actionDeleteDoc = menu.addAction(QIcon(":/img/toolBar/delete.png"),
                                              tr("Elimină documentul"));
    menu.addSeparator();
    QAction *actionPrintDoc = menu.addAction(
        QIcon(":/img/actions/print.png"), tr("Printează comanda"));

    QAction *actionOpenReport = nullptr;
    QAction *actionPrintReport = nullptr;
    if (hasReport) {
        menu.addSeparator();
        actionOpenReport = menu.addAction(
            QIcon(":/img/documents/reportEcho.png"), tr("Deschide raportul"));
        actionPrintReport = menu.addAction(
            QIcon(":/img/actions/print.png"), tr("Printează raportul"));
    }

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
    else if (selectedAction == actionOpenReport)
        openReportDocument(reportParams);
    else if (selectedAction == actionPrintReport)
        printReport(reportParams, PrintType::Preview);
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
    // Valorile filtrului se restabilesc numai dacă utilizatorul a ales
    // păstrarea lui; altfel jurnalul ar fi filtrat cu combo-urile pe „toate”.
    const QJsonObject filterObj = obj.value("filter").toObject();
    m_filter.saveFilter = filterObj.value("saveFilter").toBool(false);
    if (m_filter.saveFilter) {
        m_filter.nrDoc          = filterObj.value("nr_doc").toString();
        m_filter.idOrganization = filterObj.value("id_organization").toInt(0);
        m_filter.idContract     = filterObj.value("id_contract").toInt(0);
        m_filter.idUser         = filterObj.value("id_user").toInt(0);
    }

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

    // Textul descrie filtrul aplicat, nu valorile încă neaplicate din panou.
    // Organizația/autorul marcat ca șters lipsește din combo (filtru rapid);
    // atunci numele se ia din jurnal, unde toate rândurile au acel id.
    const bool hasRows = modelTable && modelTable->rowCount() > 0;

    QString organization = filterDisplayName(modelOrganizations, m_filter.idOrganization);
    if (organization.isEmpty() && m_filter.idOrganization > 0 && hasRows)
        organization = modelTable->itemAt(0).organizationName;
    if (!organization.isEmpty())
        str += tr("; filtru: ") + organization;
    if (!m_filter.patientName.isEmpty())
        str += tr("; pacient: ") + m_filter.patientName;
    QString author = filterDisplayName(modelUsers, m_filter.idUser);
    if (author.isEmpty() && m_filter.idUser > 0 && hasRows)
        author = modelTable->itemAt(0).userName;
    if (!author.isEmpty())
        str += tr("; autor: ") + author;

    toolBar->setTextPeriod(str);
}

void OrderView::loadFilterData()
{
    ui->dateStart->setDateTime(m_filter.startDate);
    ui->dateEnd->setDateTime(m_filter.endDate);

    ui->saveFilter->setChecked(m_filter.saveFilter);
    syncFilterControls();
}

void OrderView::syncFilterControls()
{
    // Panoul de filtru reflectă filtrul aplicat (m_filter).
    const QSignalBlocker blockerOrganizations(ui->comboOrganizations);
    const QSignalBlocker blockerContracts(ui->comboContracts);
    const QSignalBlocker blockerUsers(ui->comboUsers);

    ui->numberDoc->setText(m_filter.nrDoc);

    auto selectById = [](QComboBox *combo, const QueryRolesModel *model, int id) {
        const int row = (model && id > 0) ? model->rowById("id", id) : 0;
        combo->setCurrentIndex(qMax(row, 0));
    };

    selectById(ui->comboOrganizations, modelOrganizations, m_filter.idOrganization);
    if (m_contractsOrganizationId != m_filter.idOrganization)
        updateModelContracts(m_filter.idOrganization);
    selectById(ui->comboContracts, modelContracts, m_filter.idContract);
    selectById(ui->comboUsers, modelUsers, m_filter.idUser);
}

int OrderView::comboCurrentId(QComboBox *combo, const QueryRolesModel *model) const
{
    if (!combo || !model)
        return 0;

    // rândul gol „<<- Selectează ->>” nu are id; înseamnă fără filtru
    return qMax(combo->currentData(model->roleForColumn("id")).toInt(), 0);
}

QString OrderView::filterDisplayName(const QueryRolesModel *model, int id) const
{
    if (!model || id <= 0)
        return QString();

    const int row = model->rowById("id", id);
    if (row < 0)
        return QString();

    return model->index(row, model->columnIndex("name")).data().toString();
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

    // filtru: valorile se păstrează numai la cererea utilizatorului
    const bool saveFilter = ui->saveFilter->isChecked();
    m_settings.setValue(m_typeJournal, "filter/saveFilter", saveFilter);
    m_settings.setValue(m_typeJournal, "filter/id_organization", saveFilter ? m_filter.idOrganization : 0);
    m_settings.setValue(m_typeJournal, "filter/id_contract", saveFilter ? m_filter.idContract : 0);
    m_settings.setValue(m_typeJournal, "filter/id_user", saveFilter ? m_filter.idUser : 0);
    m_settings.setValue(m_typeJournal, "filter/nr_doc",
                        !saveFilter || m_filter.nrDoc.trimmed().isEmpty()
                            ? QVariant()
                            : QVariant(m_filter.nrDoc.trimmed()));

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

void OrderView::updateModelContracts(int organizationId)
{
    // Pointerul se anulează imediat: dacă interogarea de mai jos eșuează,
    // nu rămâne o referință spre modelul șters.
    delete modelContracts;
    modelContracts = nullptr;
    m_contractsOrganizationId = qMax(organizationId, 0);

    if (m_contractsOrganizationId <= 0) {
        QString str = m_db.getTextSQL(":/sql/queries/contracts_view.sql");
        modelContracts = new QueryRolesModel(str, ui->comboContracts);
        modelContracts->setEmptyRowEnabled(true);
        ui->comboContracts->setModel(modelContracts);
        ui->comboContracts->setModelColumn(modelContracts->columnIndex("contract_owner"));
    } else {
        QSqlQuery qry(m_db.getDatabase());
        qry.prepare(m_db.getTextSQL(
            MainDatabaseConnectionContext::instance().isSqlite()
                ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
                : ":/sql/queries/contracts_select_by_organization_mysql.sql")
                    );
        qry.addBindValue(m_contractsOrganizationId);
        qry.addBindValue(m_filter.idContract); // contractul din filtru rămâne vizibil chiar dacă e nevalid
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
    // selecția contractului o face apelantul (syncFilterControls / indexChangedCombo)
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
    // Modelul tabelului nu se înlocuiește, deci nici selectionModel().
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &OrderView::onCurrentRowChanged, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &OrderView::onDoubleClickedTableView, Qt::UniqueConnection);

    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &OrderView::slotContextMenuRequested, Qt::UniqueConnection);

    // Conectat după setSortingEnabled(true): rulează după sortarea proprie a
    // tabelului și o completează cu încărcarea întregului jurnal.
    connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &OrderView::onSortIndicatorChanged, Qt::UniqueConnection);
}

bool OrderView::sortRequiresFullJournal(int section, Qt::SortOrder order)
{
    // Loader-ul paginează numai în ordinea dateDoc DESC, id DESC; orice altă
    // sortare este corectă doar după încărcarea tuturor rândurilor din perioadă.
    return !(section == OrderJournal::DateDoc && order == Qt::DescendingOrder);
}

void OrderView::fetchAllJournalRows()
{
    if (!modelTable || !modelTable->canFetchMore())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int previousCount = -1;
    while (modelTable->canFetchMore() && modelTable->rowCount() != previousCount) {
        previousCount = modelTable->rowCount();
        modelTable->fetchMore();
    }
    QApplication::restoreOverrideCursor();

    showJournalLoadError();
}

void OrderView::onSortIndicatorChanged(int section, Qt::SortOrder order)
{
    if (!modelTable || !proxyTable)
        return;

    m_filter.sortSection = section;
    m_filter.sortOrder   = order;

    if (!sortRequiresFullJournal(section, order) || !modelTable->canFetchMore())
        return;

    // Rândurile adăugate nu sunt sortate de proxy (dynamicSortFilter = false),
    // de aceea sortarea se reaplică după încărcarea completă.
    fetchAllJournalRows();
    proxyTable->sort(section, order);
}

void OrderView::updateTableView()
{
    qInfo(logInfo()) << "OrderView: vizualizarea/actualizarea jurnalului";
    if (!modelTable || !proxyTable)
        return;

    // Documentul curent se reține după id: după reîncărcare (document nou,
    // ștergere, sortare) același număr de rând poate fi alt document.
    qint64 currentId = 0;
    int currentRow = -1;
    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        currentRow = currentIndex.row();
        currentId = proxyTable->index(currentRow, OrderJournal::Id)
                        .data(Qt::UserRole).toLongLong();
    }

    modelTable->setFilter(m_filter);
    modelTable->reload();

    if (m_filter.sortSection >= 0 &&
        m_filter.sortSection < proxyTable->columnCount())
    {
        if (sortRequiresFullJournal(m_filter.sortSection, m_filter.sortOrder))
            fetchAllJournalRows();

        auto *header = ui->tableView->horizontalHeader();
        if (header->sortIndicatorSection() != m_filter.sortSection ||
            header->sortIndicatorOrder() != m_filter.sortOrder) {
            // sortează prin proxy și emite sortIndicatorChanged
            ui->tableView->sortByColumn(m_filter.sortSection, m_filter.sortOrder);
        } else {
            // indicatorul neschimbat: sortByColumn nu ar resorta rândurile noi
            proxyTable->sort(m_filter.sortSection, m_filter.sortOrder);
        }
    }

    showJournalLoadError();

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

    int rowToSelect = -1;
    if (currentId > 0) {
        for (int row = 0; row < proxyTable->rowCount(); ++row) {
            if (proxyTable->index(row, OrderJournal::Id)
                    .data(Qt::UserRole).toLongLong() == currentId) {
                rowToSelect = row;
                break;
            }
        }
    }
    // documentul eliminat sau ieșit din filtru: rândul de pe aceeași poziție
    if (rowToSelect < 0)
        rowToSelect = qBound(0, currentRow, proxyTable->rowCount() - 1);

    // După reset-ul modelului indexul curent e invalid, deci selectRow emite
    // currentRowChanged -> updateDocumentPreview().
    ui->tableView->selectRow(rowToSelect);
}

void OrderView::initToolBar()
{
    toolBar->setStyles(m_db.toolButtonStyleForIcon(),
                       m_db.toolButtonStyleForText());

    ui->layoutToolBar->addWidget(toolBar);

    // Căutarea pacienților nu este implementată în jurnal (ca în ReportView);
    // butonul ascuns dezactivează și scurtătura Ctrl+F.
    toolBar->getBtnSaecrPacient()->hide();

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

    // fără organizație, lista contractelor revine la toate contractele
    syncFilterControls();

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

    updateTableView();
    updateTextPeriod(); // după încărcare: poate folosi numele din jurnal
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

    // Jurnalul știe deja dacă documentul are imagini; fără ele nu interogăm baza.
    const QModelIndex sourceIndex = proxyTable->mapToSource(index);
    if (!sourceIndex.isValid())
        return false;
    const OrderJournal::Item &item = modelTable->itemAt(sourceIndex.row());
    if (item.attachedImages <= 0)
        return false;

    const qint64 id_order = item.id;

    QPixmap outPixmap1, outPixmap2, outPixmap3;

    QSqlQuery qry(MainDatabaseConnectionContext::instance().isSqlite() ? m_db.getDatabaseImage() : m_db.getDatabase());
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

        // Imaginile se salvează cu octeții fișierului original (JPEG, PNG, ...):
        // formatul se detectează din conținut.
        QByteArray outByteArray1 = QByteArray::fromBase64(qry.value(0).toByteArray());
        if (! outByteArray1.isEmpty())
            outPixmap1.loadFromData(outByteArray1);

        QByteArray outByteArray2 = QByteArray::fromBase64(qry.value(1).toByteArray());
        if (! outByteArray2.isEmpty())
            outPixmap2.loadFromData(outByteArray2);

        QByteArray outByteArray3 = QByteArray::fromBase64(qry.value(2).toByteArray());
        if (! outByteArray3.isEmpty())
            outPixmap3.loadFromData(outByteArray3);

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

    if (! outPixmap1.isNull() || ! outPixmap2.isNull() || ! outPixmap3.isNull()) {
        QToolTip::showText(helpEvent->globalPos(),
                           html, ui->tableView->viewport());
        return true;
    }

    return false;
}

void OrderView::closeEvent(QCloseEvent *event)
{
    // Exportul LimeReport rulează în firul GUI și poate procesa evenimente
    // intern. Distrugerea ferestrei (și a workerului copil) în acel interval
    // ar invalida obiectele aflate încă în DocEmailExporterWorker::process().
    if (m_emailExportRunning) {
        qWarning(logWarning())
            << "OrderView nu poate fi închis cât timp se pregătesc documentele pentru e-mail.";
        popUp->setPopupText(tr("Se pregătesc documentele pentru e-mail.<br>"
                               "Fereastra poate fi închisă după finalizare."));
        popUp->show();
        event->ignore();
        return;
    }

    saveSettingsJournal();

    // Nu apelăm QDialog::closeEvent(): acesta invocă reject(), iar reject()
    // închide QMdiSubWindow-ul părinte, aflat deja în curs de închidere;
    // dialogul rămâne vizibil și QDialog ar ignora evenimentul Close.
    event->accept();
}

bool OrderView::eventFilter(QObject *obj, QEvent *event)
{
    // previzualizarea imaginelor atasate
    // Filtrul este instalat și pe butoanele toolBar-ului; poziția evenimentelor
    // lor nu este o poziție în tabel.
    if (event->type() == QEvent::ToolTip && obj == ui->tableView->viewport()) {
        return previewImagesDocs(event);
    }

    auto *button = qobject_cast<QToolButton *>(obj);
    if (!button || !popUp)
        return QDialog::eventFilter(obj, event);

    // Enter/Leave se transmit mai departe butonului: altfel QToolButton nu
    // își actualizează starea hover.
    if (event->type() == QEvent::Leave) {
        popUp->hidePop();
        return QDialog::eventFilter(obj, event);
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

    return QDialog::eventFilter(obj, event);
}

void OrderView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Lista documentelor: Comanda ecografică"));
    }

    QDialog::changeEvent(event);
}

void OrderView::keyReleaseEvent(QKeyEvent *event)
{
    // Previzualizarea se actualizează prin currentRowChanged.
    if (event->key() == Qt::Key_End) {
        // ultimul document din perioadă, nu ultimul rând încărcat; la
        // ordinea paginată (dateDoc DESC) rândurile noi se adaugă la final
        fetchAllJournalRows();
        if (proxyTable->rowCount() > 0)
            ui->tableView->selectRow(proxyTable->rowCount() - 1);
    } else if (event->key() == Qt::Key_Home) {
        if (proxyTable->rowCount() > 0)
            ui->tableView->selectRow(0);
    }

    QDialog::keyReleaseEvent(event);
}
