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

#include "reportdialog.h"
#include "features/orders/orderdialog.h"
#include "features/patients/patienthistory.h"
#include "ui_reportdialog.h"
#include "common/sessioncontext.h"
#include "common/maindatabaseconnectioncontext.h"

#include <features/catalogs/normograms.h>
#include <ui/dialogs/customdialoginvestig.h>
#include <ui/dialogs/custommessage.h>
#include <ui/widgets/balloontip.h>
#include <infrastructure/sync/syncreportworker.h>
#include <settings/settingsservice.h>
#include <common/applicationpathscontext.h>
#include <features/printing/printimagesservice.h>
#include <features/printing/reportprintservice.h>

#include <QThread>
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QImage>
#include <QMenu>
#include <QSqlError>
#include <QScopeGuard>
#include <QVBoxLayout>

namespace {
    constexpr auto SettingsKey = "ReportDialog";
}

ReportDialog::ReportDialog(DataBase &db,
                           const ReportDialogParameters &parameters,
                           QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDialog)
    , m_settings(ApplicationPathsContext::instance().tableSettingsFilePath())
    , m_db(db)
    , m_currentDB(m_db.getDatabase())
    , popUp(new PopUp(this))
    , timer(new QTimer(this))
    , m_params(parameters)
    , m_statusDoc(m_params.status)
    , modelPatients(new QStandardItemModel(this))
    , completerPatients(new QCompleter(this))
    , timerPatientSearch(new QTimer(this))
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
    , styleForButtonMessageBox(m_db.getStyleForButtonMessageBox())
    , m_post(DocStatus::Unknow) // initial
{
    ui->setupUi(this);

    setStatusDcument(m_statusDoc); // dortam initial
    QSqlDatabase currentDB = m_db.getDatabase();
    style_pressed = globals().isSystemThemeDark
            ? R"(QCommandLinkButton { background:#5b5b5b; border:1px inset #00baff; color:#fff; })"
            : R"(background:#C2C2C3; border:1px inset navy;)";

    style_unpressed = globals().isSystemThemeDark
            ? R"(QCommandLinkButton { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #4b4b4b, stop:1 #3c3c3c); border:0; color:#fff; })"
            : R"(background-color:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f6f7fa, stop:1 #dadbde); border:0;)";

    initSetCompleter();
    applyParameters();

    setupOptionsMenu();
    initConnections();

    updateStyleBtnNavigation();
    ui->frame_table->resize(900, ui->frame_btn->height()); /** initial */
    initSetStyleFrame();

    initFooterDoc();
}

ReportDialog::~ReportDialog()
{
    // Acoperă toate căile de închidere, inclusiv acceptarea documentului
    // și distrugerea ferestrei odată cu aplicația.
    saveWindowSize();
    delete ui;
}

QDate ReportDialog::documentDate() const
{
    return ui->dateTimeDoc->date();
}

int ReportDialog::getId() const
{
    return m_params.id;
}

int ReportDialog::getIdOrder() const
{
    return m_params.idOrder;
}

int ReportDialog::getIdPatient() const
{
    return m_params.idPatient;
}

int ReportDialog::getIdUser() const
{
    return m_params.idUser;
}

bool ReportDialog::documentModified() const
{
    return isWindowModified();
}

bool ReportDialog::documentIsNew() const
{
    return m_params.isNew;
}

void ReportDialog::onPrintDocument(PrintType::Column type_print, const QString &filePDF)
{
    onPrint(type_print, filePDF);
}

QStringList ReportDialog::exportToPdf(const QString &fileBase,
                                      bool showDoctorStamp,
                                      bool showDoctorSignature,
                                      QString *error)
{
    ReportPrintService service(m_db, m_currentDB);
    ReportPrintService::Request request;
    request.reportId = m_params.id;
    request.mode = PrintType::ExportToPDF;
    request.pdfBase = fileBase;
    request.showDoctorStamp = showDoctorStamp;
    request.showDoctorSignature = showDoctorSignature;
    request.reportParent = this;
    const ReportPrintService::Result result = service.print(request);
    if (error)
        *error = result.error;
    return result.success ? result.files : QStringList{};
}

bool ReportDialog::extPostDocument()
{
    return onPost();
}

void ReportDialog::dataWasModified()
{
    setWindowModified(true);
}

void ReportDialog::updateTimerDateDoc()
{
    timer->stop();
}

void ReportDialog::onDateTimeChanged()
{
    const QDateTime now = QDateTime::currentDateTime();

    QSignalBlocker b(ui->dateTimeDoc);
    ui->dateTimeDoc->setDateTime(now);

    setWindowTitle(tr("Raport ecografic (creare) nr.%1 din %2[*]")
                       .arg(ui->numberDoc->text(),
                            now.toString("dd.MM.yyyy hh:mm:ss")));
}

void ReportDialog::onChooseInvestigations()
{
    CustomDialogInvestig dlg(this);
    dlg.setSystems(m_params.systems);
    if (dlg.exec() != QDialog::Accepted)
        return;
    m_systems = dlg.selectedSystems();
    m_systems |= ReportSections::ReportSystem::Images |
                 ReportSections::ReportSystem::Video;

    m_params.systems = m_systems;

    buildSections();
    setupVisibleSections();
    setupNavigationConnections();
    setupPageSignals();
    showFirstAvailablePage();
    updateStyleBtnNavigation();

    setWindowModified(true);
}

void ReportDialog::onOpenPrintParameters()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Parametrii de printare"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *showDoctorStamp = new QCheckBox(tr("Prezintă ștampila doctorului"), &dialog);
    auto *showDoctorSignature = new QCheckBox(tr("Prezintă semnătura doctorului"), &dialog);

    const int printDoctorId =
        SettingsService::instance().organization().defaultDoctorId;
    const PrintImagesService::DoctorPrintImages doctor =
        PrintImagesService::loadDoctorPrintImages(
            m_currentDB,
            printDoctorId);

    const bool stampAvailable = !doctor.stamp.isEmpty();
    const bool signatureAvailable = !doctor.signature.isEmpty();

    showDoctorStamp->setChecked(m_showDoctorStamp && stampAvailable);
    showDoctorStamp->setEnabled(stampAvailable);
    if (!stampAvailable)
        showDoctorStamp->setText(tr("Prezintă ștampila doctorului (imaginea lipsește)"));

    showDoctorSignature->setChecked(m_showDoctorSignature && signatureAvailable);
    showDoctorSignature->setEnabled(signatureAvailable);
    if (!signatureAvailable) {
        showDoctorSignature->setText(
            tr("Prezintă semnătura doctorului (imaginea lipsește)"));
    }

    layout->addWidget(showDoctorStamp);
    layout->addWidget(showDoctorSignature);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                         QDialogButtonBox::Cancel,
                                         Qt::Horizontal,
                                         &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
        return;

    m_showDoctorStamp = showDoctorStamp->isChecked();
    m_showDoctorSignature = showDoctorSignature->isChecked();
}

void ReportDialog::onOpenOrder()
{
    if (m_params.idOrder <= 0)
        return;

    OrderDialog order(m_db, m_params.idOrder, this);
    connect(&order, &OrderDialog::PostDocument, this,
            [this](){
                ui->labelOrderEcho->setText(m_params.orderDisplayText);
            });
    order.exec();
}

void ReportDialog::onOpenPatient()
{
    if (m_params.idPatient <= 0)
        return;

    CatalogDialog catalog(m_db, CatalogType::Type::Patients, this);
    catalog.setProperty("isNew", false);
    catalog.setProperty("id", m_params.idPatient);
    connect(&catalog, &CatalogDialog::catalogDialogChanged,
            this, [this](){
                setIdPatient(m_params.idPatient);
            });
    catalog.exec();
}

void ReportDialog::onOpenPatientHistory()
{
    if (m_params.idPatient <= 0)
        return;

    PatientHistory patientHistory(m_db, this);
    patientHistory.setIdPatient(m_params.idPatient);
    patientHistory.exec();
}

void ReportDialog::slotPatientTextChanged(const QString &text)
{
    Q_UNUSED(text);
    timerPatientSearch->start();
}

void ReportDialog::updateModelPatientsByText()
{
    const QString text = ui->comboPatient->lineEdit()->text().trimmed();

    modelPatients->clear();
    modelPatients->setColumnCount(1);

    if (text.length() < 2)
        return;

    QSqlQuery qry(m_currentDB);
    QString sql = MainDatabaseConnectionContext::instance().isMariaDb()
                      ? m_db.getTextSQL(":/sql/queries_doc/searchPatientByText_mariadb.sql")
                      : m_db.getTextSQL(":/sql/queries_doc/searchPatientByText_sqlite.sql");

    qry.prepare(sql);

    const QString prefixText   = text + "%";
    const QString containsText = "%" + text + "%";

    qry.addBindValue(prefixText);
    qry.addBindValue(prefixText);
    qry.addBindValue(containsText);
    qry.addBindValue(containsText);

    if (!qry.exec()) {
        qWarning() << "Eroare exec query patients:"
                   << qry.lastError().text();
        return;
    }

    while (qry.next()) {
        auto *item = new QStandardItem(qry.value(1).toString()); // FullName
        item->setData(qry.value(0).toInt(), Qt::UserRole);       // id
        modelPatients->appendRow(item);
    }

    if (modelPatients->rowCount() > 0)
        completerPatients->complete();
}

void ReportDialog::activatedItemCompleter(const QModelIndex &index)
{
    timerPatientSearch->stop();

    const int current_id = index.data(Qt::UserRole).toInt();
    if (current_id <= 0)
        return;

    setIdPatient(current_id);
}

void ReportDialog::updateTextConcluzionBySystem()
{
    QSignalBlocker b(ui->concluzion);
    ui->concluzion->clear();

    const QList<ReportSections::ReportSystem> order = ReportSections::allSystems();

    for (ReportSections::ReportSystem system : order) {
        if (!m_systems.testFlag(system))
            continue;

        if (!m_sections.contains(system))
            continue;

        ReportPageBase *page = m_sections.value(system).page;
        if (!page)
            continue;

        const QString text = page->concluzionText().trimmed();
        if (text.isEmpty())
            continue;

        ui->concluzion->appendPlainText(text);
    }
}

void ReportDialog::openNormograms()
{
    isOpenNormogram = true;
    updateStyleBtnNavigation();

    Normograms normogram(this);
    normogram.exec();

    isOpenNormogram = false;
    updateStyleBtnNavigation();
}

bool ReportDialog::controlRequiredObjects()
{
    if (ui->comboPatient->currentText().trimmed().isEmpty() || m_params.idPatient <= 0){
        BalloonTip::showBalloonFor(ui->comboPatient,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este selectat <b>'Pacientul'</b> !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    return true;
}

void ReportDialog::onPrint(PrintType::Column type_print, const QString &filePDF)
{
    if (m_params.isNew) {
        QMessageBox::warning(this,
                             tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nPrintare nu este posibila."),
                             QMessageBox::Ok);
        return;
    }

    const bool restoreVisibility = isVisible();
    if (restoreVisibility)
        hide();

    ReportPrintService service(m_db, m_currentDB);
    ReportPrintService::Request request;
    request.reportId = m_params.id;
    request.mode = type_print;
    request.pdfBase = filePDF;
    request.showDoctorStamp = m_showDoctorStamp;
    request.showDoctorSignature = m_showDoctorSignature;
    request.reportParent = this;

    const ReportPrintService::Result result = service.print(request);

    if (restoreVisibility)
        show();

    if (!result.success) {
        CustomMessage message(this);
        message.setWindowTitle(QGuiApplication::applicationDisplayName());
        message.setTextTitle(tr("Printare nu este posibilă !!!"));
        message.setDetailedText(result.error);
        message.exec();
    }
}

bool ReportDialog::onSave()
{
    if (!controlRequiredObjects())
        return false;

    const bool originallyModified = isWindowModified();
    const int originalId          = m_params.id;
    const QString originalNumber  = ui->numberDoc->text();
    const auto originalPost       = m_post;

    // restauram valorile initiale
    auto restoreState = qScopeGuard([&] {
        m_params.id = originalId;
        ui->numberDoc->setText(originalNumber);
        setPost(originalPost);
        setWindowModified(originallyModified);
    });

    // SQLite images live in another file. ATTACH lets the report transaction
    // include their writes on the same connection.
    bool imagesAttached = false;
    auto detachImageDatabase = [&] {
        if (imagesAttached) {
            QSqlQuery detach(m_currentDB);
            if (!detach.exec("DETACH DATABASE report_save_images"))
                qWarning(logWarning()) << "Image database detach:" << detach.lastError().text();
        }
    };
    auto detachImages = qScopeGuard(detachImageDatabase);

    if (MainDatabaseConnectionContext::instance().isSqlite() && m_systems.testFlag(ReportSections::ReportSystem::Images)) {
        QSqlQuery attach(m_currentDB);
        attach.prepare("ATTACH DATABASE :path AS report_save_images");
        attach.bindValue(":path", m_db.getDatabaseImage().databaseName());
        if (!attach.exec()) {
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Nu s-a putut include baza imaginilor în tranzacție."));
            msg.setDetailedText(attach.lastError().text());
            msg.exec();
            return false;
        }
        imagesAttached = true;
    }

    if (m_post == DocStatus::Unknow)
        setPost(DocStatus::Write);

    if (!m_currentDB.transaction()) {
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Nu s-a putut porni tranzactia."));
        msg.setDetailedText(m_currentDB.lastError().text());
        msg.exec();
        return false;
    }

    auto rollback = [&] {
        if (!m_currentDB.rollback())
            qCritical(logCritical()) << "Report rollback:" << m_currentDB.lastError().text();
    };

    auto transactionGuard = qScopeGuard(rollback); // -- guard-ul executa rollback()

    QString details_error;

    // validarea tabelei principale cu determinarea ID
    if (m_params.isNew) {

        // numarul documentului
        if (ui->numberDoc->text().trimmed().isEmpty()) {
            const int year = ui->dateTimeDoc->date().year();
            const int nextNumber = m_db.getNextNumberDoc("reportEcho", year, &details_error);

            if (nextNumber <= 0) {
                rollback();
                transactionGuard.dismiss();

                CustomMessage msg(this);
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(tr("Nu s-a putut genera numărul documentului."));
                msg.setDetailedText(details_error);
                msg.exec();

                return false;
            }
            ui->numberDoc->setText(QStringLiteral("%1/%2")
                                       .arg(nextNumber)
                                       .arg(year));
        }

        // inseram datele
        if (!insertData()) {

            const QString error = m_currentDB.lastError().text();
            rollback();
            transactionGuard.dismiss();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Nu s-a putut insera datele documentului în baza de date."));
            msg.setDetailedText(error);
            msg.exec();

            return false;
        }
    } else {

        if (!updateData()) {

            const QString error = m_currentDB.lastError().text();
            rollback();
            transactionGuard.dismiss();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Nu s-a putut actualiza datele documentului în baza de date."));
            msg.setDetailedText(error);
            msg.exec();

            return false;
        }

    }

    // validam datele pe fiecare pagina
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        if (!m_systems.testFlag(it.key()))
            continue;

        const auto &section = it.value();
        if (!section.page)
            continue;

        auto *imagePage = qobject_cast<ReportPageImage*>(section.page);
        const bool saved = imagePage && imagesAttached
            ? imagePage->saveData(m_params.id, m_currentDB,
                                  QStringLiteral("report_save_images.imagesReports"))
            : section.page->saveData(m_params.id);
        if (!saved) {

            const QString error = m_currentDB.lastError().text();
            rollback();
            transactionGuard.dismiss();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Nu s-a putut insera/actualiza datele documentului în baza de date."));
            msg.setDetailedText(error);
            msg.exec();

            return false;
        }

    }

    QString parentOrderError;
    if (!updateParentOrderAttachedMedia(&parentOrderError)) {
        rollback();
        transactionGuard.dismiss();

        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Nu s-a putut actualiza indicatorul atașamentelor în comanda ecografică."));
        msg.setDetailedText(parentOrderError);
        msg.exec();
        return false;
    }

    if (!m_currentDB.commit()) {
        const QString commitError = m_currentDB.lastError().text();
        rollback();
        transactionGuard.dismiss();

        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Salvarea documentului nu s-a finalizat."));
        msg.setDetailedText(commitError);
        msg.exec();
        return false;
    }

    transactionGuard.dismiss();
    restoreState.dismiss();
    detachImageDatabase();
    detachImages.dismiss();

    for (auto it = m_sections.cbegin(); it != m_sections.cend(); ++it) {
        if (m_systems.testFlag(it.key()) && it.value().page)
            qInfo(logInfo()) << "ReportDialog: salvare confirmată; id=" << m_params.id
                             << "sistem=" << it.value().page->metaObject()->className()
                             << "operație=" << (m_params.isNew ? "creare" : "actualizare")
                             << "validare=" << m_postInProgress;
    }

    if (m_params.isNew)
        emit reportCreated();
    else
        emit reportChanged();

    if (m_params.isNew) {
        m_params.isNew = false;
        qInfo(logInfo()) << QStringLiteral("Documentul 'Raport ecografic' nr.='%1' creat cu succes in baza de date.")
                                .arg(ui->numberDoc->text());
    } else {
        qInfo(logInfo()) << QStringLiteral("Documentul 'Raport ecografic' nr.='%1' modificat cu succes in baza de date.")
        .arg(ui->numberDoc->text());
    }

    popUp->setPopupText(tr("Documentul a fost %1 cu succes<br> in baza de date.")
                            .arg(m_postInProgress ? tr("validat") : tr("salvat")));
    popUp->show();

    setWindowModified(false);

    // sincronizarea
    if (MainDatabaseConnectionContext::instance().isSqlite()
        && SettingsService::instance().synchronization().enabled)
        initSync();

    return true;
}

bool ReportDialog::onPost()
{
    DocStatus::Column oldPost = m_post;
    setPost(DocStatus::Post);
    m_postInProgress = true;

    if (!onSave()) {
        m_postInProgress = false;
        setPost(oldPost);
        return false;
    }
    m_postInProgress = false;

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Printarea documentului"),
                             tr("Doriți să printați documentul ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(styleForButtonMessageBox);
    noButton->setStyleSheet(styleForButtonMessageBox);
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton)
        onPrint(PrintType::Preview, QString());

    emit reportPost();
    accept();
    return true;
}

void ReportDialog::applyParameters()
{
    if (!m_params.orderDisplayText.isEmpty())
        ui->labelOrderEcho->setText(m_params.orderDisplayText);
    else
        ui->labelOrderEcho->setText(QString{});

    if (m_params.isNew) {

        // adaugam sectiile implicite
        m_params.systems |= ReportSections::ReportSystem::Images |
                            ReportSections::ReportSystem::Video;
        m_systems = m_params.systems;

        // constrium paginile
        buildSections();
        setupVisibleSections();
        setupNavigationConnections();
        showFirstAvailablePage();
        setupPageSignals();

        // completam datele pacientului
        if (m_params.idPatient > 0)
            setIdPatient(m_params.idPatient);

        // datele user
        m_params.idUser   = SessionContext::instance().userId();
        m_params.nameUser = globals().nameUserApp;

        // citim contextul - organizationId, userId, doctorExecuteId, nurseId
        // ID-le ce au fost introduse in order
        if (m_params.idOrder > 0)
            loadDocumentContext();

        connect(timer, &QTimer::timeout,
                this, &ReportDialog::onDateTimeChanged, Qt::UniqueConnection);

        connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
                this, &ReportDialog::updateTimerDateDoc, Qt::UniqueConnection);

        onDateTimeChanged();

        timer->start(1000);
    }

    // incarcam datele
    loadReport();

    // restauram inaltimea si latimea
    restoreWindowSize();
}

void ReportDialog::buildSections()
{
    using namespace ReportSections;

    if (m_systems.testFlag(ReportSystem::OrgansInternal)
        && !m_sections.contains(ReportSystem::OrgansInternal)
        && ui->btnOrgansInternal) {

        auto *page = new ReportPageOrgansInternal(m_db, m_currentDB, this);
        addSection(ReportSystem::OrgansInternal, page, ui->btnOrgansInternal);
    }

    if (m_systems.testFlag(ReportSystem::UrinarySystem)
        && !m_sections.contains(ReportSystem::UrinarySystem)
        && ui->btnUrinarySystem) {

        auto *page = new ReportPageUrinarySystem(m_db, m_currentDB, this);
        addSection(ReportSystem::UrinarySystem, page, ui->btnUrinarySystem);
    }

    if (m_systems.testFlag(ReportSystem::Prostate)
        && !m_sections.contains(ReportSystem::Prostate)
        && ui->btnProstate) {

        auto *page = new ReportPageProstate(m_db, m_currentDB, this);
        addSection(ReportSystem::Prostate, page, ui->btnProstate);
    }

    if (m_systems.testFlag(ReportSystem::Gynecology)
        && !m_sections.contains(ReportSystem::Gynecology)
        && ui->btnGynecology) {

        auto *page = new ReportPageGynecology(m_db, m_currentDB, this);
        addSection(ReportSystem::Gynecology, page, ui->btnGynecology);
    }

    if (m_systems.testFlag(ReportSystem::Breast)
        && !m_sections.contains(ReportSystem::Breast)
        && ui->btnBreast) {

        auto *page = new ReportPageBreast(m_db, m_currentDB, this);
        addSection(ReportSystem::Breast, page, ui->btnBreast);
    }

    if (m_systems.testFlag(ReportSystem::Thyroid)
        && !m_sections.contains(ReportSystem::Thyroid)
        && ui->btnThyroid) {

        auto *page = new ReportPageThyroid(m_db, m_currentDB, this);
        addSection(ReportSystem::Thyroid, page, ui->btnThyroid);
    }

    if (m_systems.testFlag(ReportSystem::Gestation0)
        && !m_sections.contains(ReportSystem::Gestation0)
        && ui->btnGestation0) {

        auto *page = new ReportPageGestation0(m_db, m_currentDB, this);
        addSection(ReportSystem::Gestation0, page, ui->btnGestation0);
    }

    if (m_systems.testFlag(ReportSystem::Gestation1)
        && !m_sections.contains(ReportSystem::Gestation1)
        && ui->btnGestation1) {

        auto *page = new ReportPageGestation1(m_db, m_currentDB, this);
        addSection(ReportSystem::Gestation1, page, ui->btnGestation1);
    }

    if (m_systems.testFlag(ReportSystem::Gestation2)
        && !m_sections.contains(ReportSystem::Gestation2)
        && ui->btnGestation2) {

        auto *page = new ReportPageGestation2(m_db, m_currentDB, this);
        addSection(ReportSystem::Gestation2, page, ui->btnGestation2);
    }

    if (m_systems.testFlag(ReportSystem::LymphNodes)
        && !m_sections.contains(ReportSystem::LymphNodes)
        && ui->btnLymphNodes) {

        auto *page = new ReportPageLymphNodes(m_db, m_currentDB, this);
        addSection(ReportSystem::LymphNodes, page, ui->btnLymphNodes);
    }

    if (m_systems.testFlags(ReportSystem::Images)
        && !m_sections.contains(ReportSystem::Images)
        && ui->btnImages) {

        QSqlDatabase db_img = MainDatabaseConnectionContext::instance().isSqlite()
                                  ? m_db.getDatabaseImage()
                                  : m_currentDB;

        auto *page = new ReportPageImage(m_db, db_img, this);
        addSection(ReportSystem::Images, page, ui->btnImages);
    }

    if (m_systems.testFlags(ReportSystem::Video)
        && !m_sections.contains(ReportSystem::Video)
        && ui->btnVideo) {

        auto *page = new ReportPageVideo(m_db, m_currentDB, this);
        addSection(ReportSystem::Video, page, ui->btnVideo);
    }

}

void ReportDialog::setupVisibleSections()
{
    using namespace ReportSections;

    ui->btnOrgansInternal->setVisible(m_systems.testFlag(ReportSystem::OrgansInternal));
    ui->btnUrinarySystem->setVisible(m_systems.testFlag(ReportSystem::UrinarySystem));
    ui->btnProstate->setVisible(m_systems.testFlag(ReportSystem::Prostate));
    ui->btnGynecology->setVisible(m_systems.testFlag(ReportSystem::Gynecology));
    ui->btnBreast->setVisible(m_systems.testFlag(ReportSystem::Breast));
    ui->btnThyroid->setVisible(m_systems.testFlag(ReportSystem::Thyroid));
    ui->btnGestation0->setVisible(m_systems.testFlag(ReportSystem::Gestation0));
    ui->btnGestation1->setVisible(m_systems.testFlag(ReportSystem::Gestation1));
    ui->btnGestation2->setVisible(m_systems.testFlag(ReportSystem::Gestation2));
    ui->btnLymphNodes->setVisible(m_systems.testFlag(ReportSystem::LymphNodes));

    ui->btnNormograms->setVisible(m_systems.testFlags(ReportSystem::Gestation0) ||
                                  m_systems.testFlags(ReportSystem::Gestation1) ||
                                  m_systems.testFlags(ReportSystem::Gestation2));

    ui->btnImages->setVisible(m_systems.testFlags(ReportSystem::Images));
    ui->btnVideo->setVisible(m_systems.testFlags(ReportSystem::Video));

    ui->comment->setVisible(!ui->comment->toPlainText().isEmpty());
}

void ReportDialog::setupNavigationConnections()
{
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const ReportSections::ReportSystem system = it.key();
        const SectionUi &section = it.value();

        if (!section.button || !section.page)
            continue;

        if (system == ReportSections::ReportSystem::Gestation0 ||
            system == ReportSections::ReportSystem::Gestation1 ||
            system == ReportSections::ReportSystem::Gestation2)
            connect(ui->btnNormograms, &QCommandLinkButton::clicked,
                    this, &ReportDialog::openNormograms, Qt::UniqueConnection);

        connect(section.button, &QCommandLinkButton::clicked,
                this, [this, system]()
                {
                    const SectionUi section = m_sections.value(system);
                    if (!section.page)
                        return;

                    ui->stackedWidget->setCurrentWidget(section.page);
                    updateStyleBtnNavigation();
                });
    }

    connect(ui->btnComment, &QCommandLinkButton::clicked,this,
            [this](){
                ui->comment->setVisible(!ui->comment->isVisible());
                updateStyleBtnNavigation();
            });
}

void ReportDialog::showFirstAvailablePage()
{
    using namespace ReportSections;

    const QList<ReportSystem> order = allSystems();

    for (ReportSystem system : order) {
        if (!m_sections.contains(system))
            continue;

        const SectionUi section = m_sections.value(system);
        if (!section.page)
            continue;

        ui->stackedWidget->setCurrentWidget(section.page);
        return;
    }
}

void ReportDialog::addSection(ReportSections::ReportSystem system,
                              ReportPageBase *page,
                              QCommandLinkButton *button)
{
    Q_ASSERT(page);
    Q_ASSERT(button);
    Q_ASSERT(!m_sections.contains(system));

    SectionUi sectionUi;
    sectionUi.page = page;
    sectionUi.button = button;

    m_sections.insert(system, sectionUi);
    ui->stackedWidget->addWidget(page);
}

void ReportDialog::loadDataSection()
{
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const auto &section = it.value();

        if (!section.page)
            continue;
        if (section.page->loadData(m_params.id))
            qInfo(logInfo()) << "ReportDialog: vizualizarea raportului; id=" << m_params.id
                             << "sistem=" << section.page->metaObject()->className();
    }
}

void ReportDialog::setupPageSignals()
{
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const auto &section = it.value();

        if (!section.page)
            continue;

        connect(section.page, &ReportPageBase::dataWasModified,
                this, &ReportDialog::dataWasModified, Qt::UniqueConnection);

        connect(section.page, &ReportPageBase::concluzionTextChanged,
                this, &ReportDialog::updateTextConcluzionBySystem, Qt::UniqueConnection);

        connect(section.page, &ReportPageBase::showPopUp, this,
                [this](const QString &text){
                    popUp->setPopupText(text);
                    popUp->show();
                });

        connect(section.page, &ReportPageBase::countImagesBtn, this,
                [this](const int &imagesLoaded){
                    ui->btnImages->setText(tr("Imagini (atașate %1)").arg(imagesLoaded));
                    m_attachedImages = imagesLoaded;
                });

        connect(section.page, &ReportPageBase::countVideoBtn, this,
                [this](const int &countVideo){
                    ui->btnVideo->setText(tr("Video (atașate %1)").arg(countVideo));
                    m_attachedVideos = countVideo;
                });
    }
}

void ReportDialog::initConnections()
{
    const QList<QToolButton*> btns = this->findChildren<QToolButton*>();
    for (QToolButton *btn : btns)
        btn->setStyleSheet(toolButtonStyleForText);

    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &ReportDialog::dataWasModified, Qt::UniqueConnection);

    const QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    for (QLineEdit *item : items)
        connect(item, &QLineEdit::textChanged,
                this, &ReportDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->btnOpenPatient, &QAbstractButton::clicked,
            this, &ReportDialog::onOpenPatient, Qt::UniqueConnection);
    connect(ui->btnOpenOrder, &QAbstractButton::clicked,
            this, &ReportDialog::onOpenOrder, Qt::UniqueConnection);
    connect(ui->btnPatientHistory, &QAbstractButton::clicked,
            this, &ReportDialog::onOpenPatientHistory, Qt::UniqueConnection);

    connect(ui->comment, &QPlainTextEdit::textChanged,
                this, &ReportDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->btnPrint, &QAbstractButton::clicked, this, [this]()
            {
                onPrint(PrintType::Column::Preview, QString());
            });
    connect(ui->btnOk, &QAbstractButton::clicked,
            this, &ReportDialog::onPost, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &ReportDialog::onSave, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &ReportDialog::close, Qt::UniqueConnection);

}

void ReportDialog::setupOptionsMenu()
{
    auto *menu = new QMenu(ui->btnOptions);
    QAction *chooseInvestigations = menu->addAction(QIcon(QStringLiteral(":/img/catalogs/investigations.png")),
                                                    tr("Alegerea investigațiilor"));

    QAction *printParameters = menu->addAction(QIcon(QStringLiteral(":/img/actions/print.png")),
                                               tr("Parametrii de printare"));

    connect(chooseInvestigations, &QAction::triggered,
            this, &ReportDialog::onChooseInvestigations);

    connect(printParameters, &QAction::triggered,
            this, &ReportDialog::onOpenPrintParameters);

    ui->btnOptions->setMenu(menu);
}

void ReportDialog::setStatusDcument(DocStatus::Column status)
{
    m_statusDoc = status;

    switch (m_statusDoc) {
    case DocStatus::Unknow: setWindowTitle(tr("Raport ecografic (crearea) %1").arg("[*]"));
        break;
    case DocStatus::Write: setWindowTitle(tr("Raport ecografic (salvat) %1").arg("[*]"));
        break;
    case DocStatus::Post: setWindowTitle(tr("Raport ecografic (validat) %1").arg("[*]"));
        break;
    default: setWindowTitle(tr("Raport ecografic (crearea) %1").arg("[*]"));; break;
    }
}

void ReportDialog::setIdPatient(const int id)
{
    if (id <= 0) {
        ui->comboPatient->clear();
        m_params.idPatient = id;
        return;
    }
    m_params.idPatient = id;
    loadPatientDetails();
}

void ReportDialog::setIdUser(const int id)
{
    if (id <= 0)
        return;
    m_params.idUser = id;
    // de completat footer
}

void ReportDialog::ensurePatientInCompleterModel(int idPatient, const QString &fullName)
{
    if (idPatient <= 0 || fullName.trimmed().isEmpty())
        return;

    for (int row = 0; row < modelPatients->rowCount(); ++row) {
        const QModelIndex idx = modelPatients->index(row, 0);
        if (idx.data(Qt::UserRole).toInt() == idPatient)
            return; // exista deja
    }

    auto *item = new QStandardItem(fullName);
    item->setData(idPatient, Qt::UserRole);
    modelPatients->appendRow(item);
}

void ReportDialog::loadPatientDetails()
{
    if (m_params.idPatient <= 0)
        return;

    QSqlQuery qry(m_currentDB);
    qry.prepare(MainDatabaseConnectionContext::instance().isSqlite()
                    ? m_db.getTextSQL(":/sql/queries_doc/patients_byID_sqlite.sql")
                    : m_db.getTextSQL(":/sql/queries_doc/patients_byID_mariadb.sql"));
    qry.addBindValue(m_params.idPatient);

    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
        << "SQL error:" << qry.lastError().text();
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    if (!qry.next()) {
        qCritical(logCritical()).noquote()
        << "Pacientul cu id =" << m_params.idPatient << " nu a fost găsit";
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    const QString fullName = qry.value(PatientSearchColumns::FullName).toString().trimmed();

    // ne asiguram ca pacientul exista in modelul completerului
    ensurePatientInCompleterModel(m_params.idPatient, fullName);

    // sincronizam textul din combo
    {
        QSignalBlocker blocker(ui->comboPatient->lineEdit());
        ui->comboPatient->setEditText(fullName);
    }
}

void ReportDialog::loadReport()
{
    using namespace ReportSections;

    if (m_params.id <= 0)
        return;

    // solicitarea
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT
            r.*,
            u.name AS nameUser
        FROM
            reportEcho r
        INNER JOIN
            users u on u.id = r.id_users
        WHERE r.id = :id
    )");
    q.bindValue(":id", m_params.id);
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "loadReport sql error:" << q.lastError().text()
            << "\nLast query:" <<q.lastQuery();
        return;
    }

    if (!q.next())
        return;

    //--- status documentului
    const auto status = DocStatus::determineStatusDoc(q.value("deletionMark").toInt());
    setStatusDcument(status);
    setPost(status);

    //--- sisteme
    if (q.value("t_organs_internal").toInt() == 1)
        m_systems |= ReportSystem::OrgansInternal;
    if (q.value("t_urinary_system").toInt() == 1)
        m_systems |= ReportSystem::UrinarySystem;
    if (q.value("t_prostate").toInt() == 1)
        m_systems |= ReportSystem::Prostate;
    if (q.value("t_gynecology").toInt() == 1)
        m_systems |= ReportSystem::Gynecology;
    if (q.value("t_breast").toInt() == 1)
        m_systems |= ReportSystem::Breast;
    if (q.value("t_thyroid").toInt() == 1)
        m_systems |= ReportSystem::Thyroid;
    if (q.value("t_gestation0").toInt() == 1)
        m_systems |= ReportSystem::Gestation0;
    if (q.value("t_gestation1").toInt() == 1)
        m_systems |= ReportSystem::Gestation1;
    if (q.value("t_gestation2").toInt() == 1)
        m_systems |= ReportSystem::Gestation2;
    if (q.value("t_lymphNodes").toInt() == 1)
        m_systems |= ReportSystem::LymphNodes;

    // adaugam sectiile implicite
    m_systems |= ReportSections::ReportSystem::Images |
                 ReportSections::ReportSystem::Video;

    m_params.systems = m_systems;

    // construim butoane de navigare
    buildSections();
    setupVisibleSections();
    setupNavigationConnections();
    showFirstAvailablePage();

    // blocam ca sa evitam modificarea formei
    QSignalBlocker b(ui->comboPatient);
    QSignalBlocker bc(ui->comment);
    QSignalBlocker bd(ui->dateTimeDoc);

    QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : std::as_const(items))
        itemsBlockers.emplace_back(item);

    //--- nr.documentului
    ui->numberDoc->setText(q.value("numberDoc").toString());
    ui->numberDoc->setEnabled(!ui->numberDoc->text().isEmpty());
    //--- data
    ui->dateTimeDoc->setDateTime(q.value("dateDoc").toDateTime());
    //--- comanda părinte
    // Parametrul poate lipsi când raportul este deschis direct din
    // PatientHistory. La salvare trebuie folosită relația persistentă din BD,
    // nu valoarea implicită 0, altfel MariaDB respinge cheia externă.
    m_params.idOrder = q.value("id_orderEcho").toInt();
    //--- pacient
    setIdPatient(q.value("patient_id").toInt());
    //--- concluzia
    ui->concluzion->setPlainText(q.value("concluzion").toString());
    //--- comenatriu
    ui->comment->setPlainText(q.value("comment").toString());
    ui->comment->setHidden(ui->comment->toPlainText().isEmpty());
    //--- datele user
    m_params.idUser   = q.value("id_users").toInt();
    m_params.nameUser = q.value("nameUser").toString();

    loadDocumentContext();

    setupPageSignals();
    loadDataSection();
}

bool ReportDialog::loadDocumentContext()
{
    QString error;
    if (m_documentContext.loadFromOrder(m_currentDB, m_params.idOrder, &error))
        return true;

    qWarning(logWarning()).noquote()
        << QStringLiteral("ReportDialog: contextul comenzii nu a putut fi încărcat: %1")
               .arg(error);
    return false;
}

void ReportDialog::initSetCompleter()
{
    modelPatients->clear();
    modelPatients->setColumnCount(1);

    completerPatients->setModel(modelPatients);
    completerPatients->setCompletionColumn(0);
    completerPatients->setCaseSensitivity(Qt::CaseInsensitive);
    completerPatients->setCompletionMode(QCompleter::PopupCompletion);
    completerPatients->setFilterMode(Qt::MatchContains);
    completerPatients->setModelSorting(QCompleter::UnsortedModel);

    ui->comboPatient->setEditable(true);
    ui->comboPatient->lineEdit()->setCompleter(completerPatients);

    timerPatientSearch->setSingleShot(true);
    timerPatientSearch->setInterval(250);

    connect(ui->comboPatient->lineEdit(), &QLineEdit::textEdited,
            this, &ReportDialog::slotPatientTextChanged,
            Qt::UniqueConnection);

    connect(timerPatientSearch, &QTimer::timeout,
            this, &ReportDialog::updateModelPatientsByText,
            Qt::UniqueConnection);

    connect(completerPatients, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex &>::of(&ReportDialog::activatedItemCompleter));
}

void ReportDialog::initSetStyleFrame()
{
    if (globals().isSystemThemeDark){
        const QString style_frame = R"(
            QFrame#customFrame
            {
                background-color: #2b2b2b;
                border: 1px solid #555; /* Linie subțire gri */
                border-radius: 5px;
            }
        )";
        ui->frame_btn->setStyleSheet(R"(
            background-color: #2b2b2b;
            border: 1px solid #555;
            border-radius: 5px;
        )");
        ui->frame_table->setObjectName("customFrame");
        ui->frame_table->setStyleSheet(style_frame);
    }
}

void ReportDialog::updateStyleBtnNavigation()
{
    // determinam pagina curenta
    auto currentPage = ui->stackedWidget->currentWidget();

    // setam stilul
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const ReportSections::ReportSystem system = it.key();
        const auto &section = it.value();

        if (!section.button || !section.page)
            continue;

        if (system == ReportSections::ReportSystem::Gestation0 ||
            system == ReportSections::ReportSystem::Gestation1 ||
            system == ReportSections::ReportSystem::Gestation2)
            ui->btnNormograms->setStyleSheet(isOpenNormogram ? style_pressed : style_unpressed);

        section.button->setStyleSheet(
            section.page == currentPage ? style_pressed : style_unpressed
            );
    }

    // Comment are logic aparte: pressed dacă e vizibil
    ui->btnComment->setStyleSheet(ui->comment->isVisible()
                                    ? style_pressed
                                    : style_unpressed);
}

void ReportDialog::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/catalogs/user.png").pixmap(18,18);
    QLabel *labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    auto labelAuthor = new QLabel(this);
    labelAuthor->setText(m_params.nameUser);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void ReportDialog::setPost(DocStatus::Column post)
{
    m_post = post;
}

void ReportDialog::bindValues(QSqlQuery &q)
{
    auto b = [=](bool val) {return MainDatabaseConnectionContext::instance().isMariaDb() ? val : int(val);};

    q.bindValue(":deletionMark",      static_cast<int>(m_post));
    q.bindValue(":numberDoc",         ui->numberDoc->text());
    q.bindValue(":dateDoc",           ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.bindValue(":patient_id",       m_params.idPatient);
    q.bindValue(":id_orderEcho",      m_params.idOrder);
    q.bindValue(":t_organs_internal", b(m_systems.testFlags(ReportSections::ReportSystem::OrgansInternal)));
    q.bindValue(":t_urinary_system",  b(m_systems.testFlags(ReportSections::ReportSystem::UrinarySystem)));
    q.bindValue(":t_prostate",        b(m_systems.testFlags(ReportSections::ReportSystem::Prostate)));
    q.bindValue(":t_gynecology",      b(m_systems.testFlags(ReportSections::ReportSystem::Gynecology)));
    q.bindValue(":t_breast",          b(m_systems.testFlags(ReportSections::ReportSystem::Breast)));
    q.bindValue(":t_thyroid",         b(m_systems.testFlags(ReportSections::ReportSystem::Thyroid)));
    q.bindValue(":t_gestation0",      b(m_systems.testFlags(ReportSections::ReportSystem::Gestation0)));
    q.bindValue(":t_gestation1",      b(m_systems.testFlags(ReportSections::ReportSystem::Gestation1)));
    q.bindValue(":t_gestation2",      b(m_systems.testFlags(ReportSections::ReportSystem::Gestation2)));
    q.bindValue(":t_gestation3",      QVariant()); // nu se foloseste
    q.bindValue(":t_lymphNodes",      b(m_systems.testFlags(ReportSections::ReportSystem::LymphNodes)));
    q.bindValue(":id_users",          m_params.idUser);
    q.bindValue(":concluzion",        ui->concluzion->toPlainText());
    q.bindValue(":comment",           ui->comment->toPlainText().isEmpty() ? QVariant() : ui->comment->toPlainText());
    q.bindValue(":attachedImages",    m_attachedImages);
}

bool ReportDialog::insertData()
{
    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        INSERT INTO reportEcho (
            deletionMark,
            numberDoc,
            dateDoc,
            patient_id,
            id_orderEcho,
            t_organs_internal,
            t_urinary_system,
            t_prostate,
            t_gynecology,
            t_breast,
            t_thyroid,
            t_gestation0,
            t_gestation1,
            t_gestation2,
            t_gestation3,
            t_lymphNodes,
            id_users,
            concluzion,
            comment,
            attachedImages,
            uuid
        ) VALUES (
            :deletionMark,
            :numberDoc,
            :dateDoc,
            :patient_id,
            :id_orderEcho,
            :t_organs_internal,
            :t_urinary_system,
            :t_prostate,
            :t_gynecology,
            :t_breast,
            :t_thyroid,
            :t_gestation0,
            :t_gestation1,
            :t_gestation2,
            :t_gestation3,
            :t_lymphNodes,
            :id_users,
            :concluzion,
            :comment,
            :attachedImages,
            :uuid
        )
    )");
    bindValues(q);
    q.bindValue(":uuid", QUuid::createUuid().toRfc4122());
    if (!q.exec()) {
        qCritical(logCritical()).noquote()
        << "ReportDialog (insertData) error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    m_params.id = q.lastInsertId().toInt();

    return true;
}

bool ReportDialog::updateData()
{
    if (m_params.id <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        UPDATE reportEcho SET
            deletionMark      = :deletionMark,
            numberDoc         = :numberDoc,
            dateDoc           = :dateDoc,
            patient_id       = :patient_id,
            id_orderEcho      = :id_orderEcho,
            t_organs_internal = :t_organs_internal,
            t_urinary_system  = :t_urinary_system,
            t_prostate        = :t_prostate,
            t_gynecology      = :t_gynecology,
            t_breast          = :t_breast,
            t_thyroid         = :t_thyroid,
            t_gestation0      = :t_gestation0,
            t_gestation1      = :t_gestation1,
            t_gestation2      = :t_gestation2,
            t_gestation3      = :t_gestation3,
            t_lymphNodes      = :t_lymphNodes,
            id_users          = :id_users,
            concluzion        = :concluzion,
            comment           = :comment,
            attachedImages    = :attachedImages
        WHERE
            id = :id
    )");
    bindValues(q);
    q.bindValue(":id", m_params.id);
    if (!q.exec()) {
        qCritical(logCritical()).noquote()
        << "ReportDialog (updateData) error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    return true;
}

bool ReportDialog::updateParentOrderAttachedMedia(QString *error)
{
    if (m_params.idOrder <= 0) {
        if (error)
            *error = tr("ID-ul comenzii ecografice asociate este invalid.");
        return false;
    }

    // Valoarea istorică folosită de OrderView:
    // 0 = fără atașamente, 1 = imagini, 2 = video (eventual și imagini).
    const int attachedMedia = m_attachedVideos > 0 ? 2
                                                   : (m_attachedImages > 0 ? 1 : 0);
    QSqlQuery query(m_currentDB);
    query.prepare(QStringLiteral(
        "UPDATE orderEcho SET attachedImages=:attachedImages WHERE id=:id"));
    query.bindValue(QStringLiteral(":attachedImages"), attachedMedia);
    query.bindValue(QStringLiteral(":id"), m_params.idOrder);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        qCritical(logCritical())
            << "ReportDialog: actualizarea orderEcho.attachedImages a eșuat:"
            << query.lastError().text();
        return false;
    }
    return true;
}

void ReportDialog::initSync()
{
    if (m_params.id <= 0)
        return;

    QThread *thread = new QThread();
    auto *worker = new SyncReportWorker(m_params.id);
    worker->moveToThread(thread);

    connect(thread, &QThread::started,
            worker, &SyncReportWorker::process);
    connect(worker, &SyncReportWorker::syncError,
            qApp, [](const QString &error)
            {
                CustomMessage msg(QApplication::activeWindow());
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(
                    QObject::tr("Raportul a fost salvat local, dar sincronizarea cloud a eșuat."));
                msg.setDetailedText(error);
                msg.exec();
            });
    connect(worker, &SyncReportWorker::finished,
            thread, &QThread::quit);
    connect(worker, &SyncReportWorker::finished,
            worker, &SyncReportWorker::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    thread->start();
}

void ReportDialog::saveWindowSize()
{
    m_settings.setValue(SettingsKey, "window/width", width());
    m_settings.setValue(SettingsKey, "window/height", height());
}

void ReportDialog::restoreWindowSize()
{
    const QJsonObject objRoot = m_settings.getJsonObject(SettingsKey);
    const QJsonObject winObj = objRoot.value("window").toObject();

    if (!winObj.contains("width") || !winObj.contains("height"))
        return;

    const int width  = winObj.value("width").toInt();
    const int height = winObj.value("height").toInt();

    if (width > 0 && height > 0)
        resize(width, height);
}

void ReportDialog::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()) {
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Doriți să salvați aceste modificări ?"),
                                 QMessageBox::NoButton, this);

        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);

        yesButton->setStyleSheet(styleForButtonMessageBox);
        noButton->setStyleSheet(styleForButtonMessageBox);
        cancelButton->setStyleSheet(styleForButtonMessageBox);

        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            const bool ok = (m_post == DocStatus::Post) ? onPost() : onSave();
            if (ok) {
                event->accept();
            } else {
                event->ignore();
            }
            return;
        }

        if (messange_box.clickedButton() == noButton) {
            event->accept();
            return;
        }

        event->ignore();
        return;
    }

    saveWindowSize();

    event->accept();
}

void ReportDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void ReportDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        focusNextChild();
        event->accept();
        return;
    }
    QDialog::keyPressEvent(event);
}
