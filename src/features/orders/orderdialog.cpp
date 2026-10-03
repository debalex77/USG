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

#include "orderdialog.h"
#include "ui_orderdialog.h"
#include "common/sessioncontext.h"
#include "common/maindatabaseconnectioncontext.h"
#include "settings/settingsservice.h"

#include <common/applicationpathscontext.h>
#include <features/printing/orderprintservice.h>

#include <algorithm>

OrderDialog::OrderDialog(DataBase &db, QWidget *parent)
    : OrderDialog(db, 0, parent)
{
}

OrderDialog::OrderDialog(DataBase &db, int orderId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OrderDialog) // initial
    , m_settings(ApplicationPathsContext::instance().tableSettingsFilePath())
    , m_db(db)
    , m_currentDB(m_db.getDatabase())
    , popUp(new PopUp(this))
    , timer(new QTimer(this))
    , modelPatients(new QStandardItemModel(this))
    , completerPatients(new QCompleter(this))
    , timerPatientSearch(new QTimer(this))
    , modelTableSource(new OrderInvestigationModel(OrderInvestigationModel::Kind::Available, this))
    , modelTableOrder(new OrderInvestigationModel(OrderInvestigationModel::Kind::Selected, this))
    , proxy(new SortModel(this))
    , toolButtonStyleForText(m_db.toolButtonStyleForText())
    , styleForButtonMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    loadLayoutSizes();

    // separam fereastra
    if (SettingsService::instance().user().openDocumentsInSeparateWindows)
        setWindowFlags(Qt::Window);

    // setam titlu
    setWindowTitle(tr("Comanda ecografica %1").arg("[*]"));

    setupDateDocFormat();

    // setam datele combo
    updateModelOrganizations();
    updateModelContracts();
    updateModelTypesPrices();
    updateModelPerformingDoctors();
    updateModelNurses();
    updateModelRefferingDoctors();

    setStyleMaxVisibleItemsComboBox();
    initConnections();

    setupMaxLengthForDataPatient();
    initSetCompleter();

    initSetCompleterAddress();

    // tabele
    initTableSource();
    initTableOrder();

    initFooterDoc();

    if (globals().isSystemThemeDark) {
        ui->frame_table->setObjectName("customFrame");
    }

    if (m_layoutSizes.window.isValid() && !m_layoutSizes.window.isEmpty())
        resize(m_layoutSizes.window);

    if (orderId > 0) {
        setOrderId(orderId);
        loadOrder();
    } else {
        initializeNewOrder();
    }
}

OrderDialog::~OrderDialog()
{
    // Acoperă toate căile de închidere, inclusiv acceptarea documentului
    // și distrugerea ferestrei odată cu aplicația.
    saveLayoutSizes();
    delete ui;
}

void OrderDialog::onPrintDocument(PrintType::Column type_print, const QString &filePDF)
{
    onPrint(type_print, filePDF);
}

bool OrderDialog::exportToPdf(const QString &filePDF, QString *error)
{
    OrderPrintService service(m_db, m_currentDB);
    OrderPrintService::Request request;
    request.orderId = orderId();
    request.mode = PrintType::ExportToPDF;
    request.pdfFile = filePDF;
    request.reportParent = this;
    const OrderPrintService::Result result = service.print(request);
    if (error)
        *error = result.error;
    return result.success;
}

bool OrderDialog::extPostDocument()
{
    return onPost();
}

void OrderDialog::setSuggestedPatientName(const QString &fullName)
{
    const QString patientName = fullName.trimmed();
    if (patientName.isEmpty())
        return;

    ui->comboPatient->setCurrentText(patientName);
}

bool OrderDialog::applyPrefillData(const PrefillData &data, QString *errorText)
{
    if (errorText)
        errorText->clear();

    if (data.organizationId > 0)
        setIdOrganization(data.organizationId);
    if (data.referringDoctorId > 0)
        setIdRefferingDoctor(data.referringDoctorId);
    if (data.patientId > 0)
        setIdPatient(data.patientId);
    else
        setSuggestedPatientName(data.patientText);

    if (data.investigationIds.isEmpty())
        return true;

    QStringList unavailable;
    for (int investigationId : data.investigationIds) {
        if (investigationId <= 0)
            continue;
        QSqlQuery investigationQuery(m_currentDB);
        investigationQuery.prepare(QStringLiteral(
            "SELECT cod, name FROM investigations WHERE id = :id"));
        investigationQuery.bindValue(QStringLiteral(":id"), investigationId);
        if (!investigationQuery.exec() || !investigationQuery.next()) {
            unavailable.append(tr("ID %1 (inexistentă)").arg(investigationId));
            continue;
        }

        const QString code = investigationQuery.value(0).toString();
        const QString name = investigationQuery.value(1).toString();
        bool found = false;
        for (int row = 0; row < modelTableSource->rowCount(); ++row) {
            const QVariantMap source = modelTableSource->rowDataByRow(row);
            if (source.value(QStringLiteral("cod")).toString() != code)
                continue;
            QVariantMap destination;
            destination[QStringLiteral("id")]           = m_tempOrderRowId--;
            destination[QStringLiteral("deletionMark")] = documentStatus();
            destination[QStringLiteral("id_orderEcho")] = orderId();
            destination[QStringLiteral("cod")]          = source.value(QStringLiteral("cod"));
            destination[QStringLiteral("name")]         = source.value(QStringLiteral("name"));
            destination[QStringLiteral("price")]        = source.value(QStringLiteral("price"));
            found = modelTableOrder->addRow(destination);
            break;
        }
        if (!found)
            unavailable.append(QStringLiteral("%1 - %2").arg(code, name));
    }

    updateDocumentSumText();
    if (modelTableOrder->rowCount() > 0)
        dataWasModified();
    if (unavailable.isEmpty())
        return true;
    if (errorText) {
        *errorText = tr("Următoarele investigații nu sunt disponibile în lista de prețuri "
                        "pentru organizația și contractul selectate:\n%1")
                         .arg(unavailable.join(QStringLiteral("\n")));
    }
    return false;
}

void OrderDialog::initializeNewOrder()
{
    m_documentContext.clear();
    setPost(DocStatus::Unknow);

    // data si ora atuala
    ui->dateTimeDoc->setDateTime(QDateTime::currentDateTime());
    connect(timer, &QTimer::timeout,
            this, &OrderDialog::updateTimerDateDoc, Qt::UniqueConnection);
    timer->start(1000);

    // numar documentului
    ui->numberDoc->setEnabled(false);
    ui->patientBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd"));
    setPatientDataEnabled(false);

    // setam date din variabile globale
    const Settings::OrganizationSettings &organization = SettingsService::instance().organization();
    setIdPerformingDoctor(organization.defaultDoctorId);
    setIdNurse(organization.defaultNurseId);
    setIdUser(SessionContext::instance().userId());

    ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_organization);
    changeIconForItemToolBox(OrderToolBoxIdx::Box_organization);

    m_attachedImages = StatusObject::ZeroWrite;

    ui->comboOrganization->setFocus();
}

void OrderDialog::loadOrder()
{
    if (orderId() <= 0)
        return;

    // blocam signale ca sa nu fie modificarea formei
    QList<QComboBox*> combos = findChildren<QComboBox*>();
    std::vector<QSignalBlocker> comboBlockers; // vector STL care va contine obiecte QSignalBlocker
    comboBlockers.reserve(combos.size());      // rezervarea de memorie pentru elemente

    for (QComboBox *combo : std::as_const(combos))
        comboBlockers.emplace_back(combo); // vectorul creeaza obiectul in interiorul sau = QSignalBlocker(combo)

    QSignalBlocker b_dateTime(ui->dateTimeDoc);
    QSignalBlocker c_comment(ui->editComment);
    QSignalBlocker b_card(ui->cardPayment);

    QSqlQuery qry(m_currentDB);
    qry.prepare(R"(
        SELECT
            id,
            deletionMark,
            numberDoc,
            dateDoc,
            id_organizations,
            id_contracts,
            id_typesPrices,
            id_doctors,
            id_doctors_execute,
            id_nurses,
            patient_id,
            id_users, sum, comment,
            cardPayment, attachedImages, uuid
        FROM
            orderEcho
        WHERE
            id = :id
    )");
    qry.bindValue(":id", orderId());
    if (qry.exec() && qry.next()){

        qInfo(logInfo()) << "OrderDialog: vizualizarea comenzii, id=" << orderId();

        OrderDocumentContextData context;
        context.orderId           = qry.value("id").toInt();
        context.organizationId    = qry.value("id_organizations").toInt();
        context.contractId        = qry.value("id_contracts").toInt();
        context.priceTypeId       = qry.value("id_typesPrices").toInt();
        context.referringDoctorId = qry.value("id_doctors").toInt();
        context.executingDoctorId = qry.value("id_doctors_execute").toInt();
        context.nurseId           = qry.value("id_nurses").toInt();
        context.patientId         = qry.value("patient_id").toInt();
        context.authorUserId      = qry.value("id_users").toInt();
        context.deletionMark      = qry.value("deletionMark").toInt();
        m_documentContext.setData(context);

        ui->numberDoc->setText(qry.value(OrderSections::NumberDoc).toString());
        ui->numberDoc->setEnabled(false);

        const QVariant dateDocValue = qry.value(OrderSections::DateDoc);
        QDateTime dateDoc = dateDocValue.toDateTime();
        if (!dateDoc.isValid())
            dateDoc = QDateTime::fromString(dateDocValue.toString(),
                                            QStringLiteral("yyyy-MM-dd hh:mm:ss"));
        if (dateDoc.isValid())
            ui->dateTimeDoc->setDateTime(dateDoc);

        setPost(qry.value(OrderSections::DeletionMark).toInt());
        setIdOrganization(qry.value(OrderSections::Id_Organizations).toInt());

        // La deschiderea unui document semnalele comboboxurilor sunt blocate,
        // deci schimbarea organizatiei nu poate reincarca automat contractele.
        // Modelul initial contine contractele tuturor organizatiilor si poate
        // sa nu contina un contract CNAM istoric/inactiv.
        updateModelContracts();
        setIdContract(context.contractId);
        setIdTypePrice(context.priceTypeId);
        setIdRefferingDoctor(context.referringDoctorId);
        setIdPerformingDoctor(context.executingDoctorId);
        setIdNurse(context.nurseId);
        setIdPatient(context.patientId);
        setIdUser(context.authorUserId);

        updateTableSource();
        updateTableOrder();

        ui->editComment->setPlainText(qry.value(OrderSections::Comment).toString());

        // determinam cash sau card
        if (qry.value(OrderSections::CardPayment).toInt() == PaymentMethod::Card)
            ui->cardPayment->setCheckState(Qt::Checked);
        else
            ui->cardPayment->setCheckState(Qt::Unchecked);

        // determinam daca sunt atasate imaginile
        if (qry.value(OrderSections::AttachedImg).toInt() == 0)
            m_attachedImages = 0;
        else
            m_attachedImages = 1;
    }
    ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
    changeIconForItemToolBox(OrderToolBoxIdx::Box_patient);
}

bool OrderDialog::isNewDocument() const
{
    return orderId() <= 0;
}

int OrderDialog::orderId() const { return m_documentContext.data().orderId; }
int OrderDialog::organizationId() const { return m_documentContext.data().organizationId; }
int OrderDialog::contractId() const { return m_documentContext.data().contractId; }
int OrderDialog::priceTypeId() const { return m_documentContext.data().priceTypeId; }
int OrderDialog::patientId() const { return m_documentContext.data().patientId; }
int OrderDialog::nurseId() const { return m_documentContext.data().nurseId; }
int OrderDialog::performingDoctorId() const { return m_documentContext.data().executingDoctorId; }
int OrderDialog::referringDoctorId() const { return m_documentContext.data().referringDoctorId; }
int OrderDialog::authorUserId() const { return m_documentContext.data().authorUserId; }
int OrderDialog::documentStatus() const { return m_documentContext.data().deletionMark; }

void OrderDialog::setOrderId(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.orderId = value;
    m_documentContext.setData(context);
}

void OrderDialog::setIdOrganization(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.organizationId = value;
    m_documentContext.setData(context);

    if (organizationId() < 0)
        return;

    ui->comboOrganization->setCurrentIndex(modelOrganizations->rowById("id", organizationId()));
}

void OrderDialog::setIdContract(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.contractId = value;
    m_documentContext.setData(context);

    if (contractId() < 0)
        return;

    ui->comboContract->setCurrentIndex(modelContracts->rowById("id", contractId()));
}

void OrderDialog::setIdTypePrice(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.priceTypeId = value;
    m_documentContext.setData(context);

    if (priceTypeId() < 0)
        return;

    ui->comboTypePrices->setCurrentIndex(modelTypePrices->rowById("id", priceTypeId()));

    if (organizationId() < 0 || contractId() < 0)
        return;

    updateTableSource();
}

void OrderDialog::setIdPatient(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.patientId = value;
    m_documentContext.setData(context);

    if (patientId() <= 0) {
        QSignalBlocker blocker(ui->comboPatient->lineEdit());
        ui->comboPatient->setEditText(QString());
        ui->patientBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd"));
        ui->patientIDNP->clear();
        ui->patientAddress->clear();
        ui->patientMedicalPolicy->clear();
        ui->patientPhone->clear();
        ui->patientEmail->clear();
        ui->newPatient->setChecked(false);
        return;
    }

    loadPatientDetails();
    setPatientDataEnabled(false);
}

void OrderDialog::setIdNurse(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.nurseId = value;
    m_documentContext.setData(context);

    if (nurseId() < 0)
        return;

    ui->comboNurse->setCurrentIndex(modelNurses->rowById("id", nurseId()));
}

void OrderDialog::setIdPerformingDoctor(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.executingDoctorId = value;
    m_documentContext.setData(context);

    if (performingDoctorId() < 0)
        return;

    ui->comboPerformingDoctor->setCurrentIndex(modelPerformingDoctors->rowById("id", performingDoctorId()));
}

void OrderDialog::setIdRefferingDoctor(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.referringDoctorId = value;
    m_documentContext.setData(context);

    if (!modelRefferingDoctors)
        return;

    const int row = referringDoctorId() > 0
                        ? modelRefferingDoctors->rowById("id", referringDoctorId())
                        : 0;
    QSignalBlocker blocker(ui->comboReferringDoctor);
    ui->comboReferringDoctor->setCurrentIndex(row >= 0 ? row : 0);
}

void OrderDialog::setIdUser(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.authorUserId = value;
    m_documentContext.setData(context);
}

void OrderDialog::setPost(int value)
{
    OrderDocumentContextData context = m_documentContext.data();
    context.deletionMark = value;
    m_documentContext.setData(context);

    if (documentStatus() == DocStatus::Unknow)
        setWindowTitle(tr("Comanda ecografica (crearea) %1").arg("[*]"));
    else if (documentStatus() == DocStatus::Write)
        setWindowTitle(tr("Comanda ecografica (salvata) %1").arg("[*]"));
    else if (documentStatus() == DocStatus::DeletionMark)
        setWindowTitle(tr("Comanda ecografica (marcata pentru eliminare) %1").arg("[*]"));
    else if (documentStatus() == DocStatus::Post)
        setWindowTitle(tr("Comanda ecografica (validata) %1").arg("[*]"));
}

void OrderDialog::dataWasModified()
{
    setWindowModified(true);
}

void OrderDialog::updateTimerDateDoc()
{
    QDateTime now = QDateTime::currentDateTime();

    {
        QSignalBlocker b(ui->dateTimeDoc);
        ui->dateTimeDoc->setDateTime(now);
    }

    setWindowTitle(tr("Comanda ecografica (crearea) %1 %2")
                       .arg(" nr." + ui->numberDoc->text() + " din " +
                                now.toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
}

void OrderDialog::onDateTimeChanged()
{
    timer->stop();
}

void OrderDialog::onCreateNewDoctor()
{
    CatalogDialog *catalog = new CatalogDialog(m_db, CatalogType::Type::Doctors, this);
    catalog->setAttribute(Qt::WA_DeleteOnClose);
    catalog->setProperty("isNew", true);
    catalog->setWindowModality(Qt::ApplicationModal);
    connect(catalog, &CatalogDialog::catalogDialogCreatedReturnID,
            this, [this](int id_doctor)
            {
                if (id_doctor <= 0)
                    return;

                updateModelRefferingDoctors();
                const int row = modelRefferingDoctors
                                    ? modelRefferingDoctors->rowById("id", id_doctor)
                                    : -1;
                if (row < 0) {
                    qWarning(logWarning())
                        << "OrderDialog: doctorul nou creat nu a fost găsit în model, id="
                        << id_doctor;
                    QMessageBox::warning(
                        this,
                        tr("Doctori"),
                        tr("Doctorul a fost creat, dar nu a putut fi selectat în listă."));
                    return;
                }

                setIdRefferingDoctor(id_doctor);
                ui->comboReferringDoctor->setCurrentIndex(row);
            });
    catalog->show();
}

void OrderDialog::onOpenCatalogDoctor()
{
    if (referringDoctorId() <= 0)
        return;
    CatalogDialog *catalog = new CatalogDialog(m_db, CatalogType::Type::Doctors, this);
    catalog->setAttribute(Qt::WA_DeleteOnClose);
    catalog->setProperty("isNew", false);
    catalog->setProperty("id", referringDoctorId());
    catalog->setWindowModality(Qt::ApplicationModal);
    connect(catalog, &CatalogDialog::catalogDialogChanged,
            this, [this]()
            {
                const int editedDoctorId = referringDoctorId();
                updateModelRefferingDoctors();

                const int row = modelRefferingDoctors
                                    ? modelRefferingDoctors->rowById("id", editedDoctorId)
                                    : -1;
                if (row >= 0)
                    ui->comboReferringDoctor->setCurrentIndex(row);
            });
    catalog->show();
}

void OrderDialog::indexChangedCombo(int index)
{
    Q_UNUSED(index);

    QComboBox *combo = qobject_cast<QComboBox*>(sender());

    if (combo == ui->comboOrganization) {
        // determinam rolul
        auto roleID            = modelOrganizations->roleForColumn("id");
        auto role_id_contract  = modelOrganizations->roleForColumn("id_contracts");
        auto role_id_typePrice = modelOrganizations->roleForColumn("id_typePrice");
        // variabile
        const int id_organization = ui->comboOrganization->currentData(roleID).toInt();
        const int id_contracts    = ui->comboOrganization->currentData(role_id_contract).toInt();
        const int id_typePrice    = ui->comboOrganization->currentData(role_id_typePrice).toInt();

        // setam ID organizatiei
        if (id_organization > 0)
            setIdOrganization(id_organization);

        // actualizam modelul contracte, apoi setam ID contractului
        updateModelContracts();

        // Modelul contractelor tocmai a fost recreat. Selectam explicit randul
        // contractului implicit chiar daca proprietatea avea deja acelasi ID.
        if (id_contracts > 0) {
            setIdContract(id_contracts);
            const int contractRow = modelContracts->rowById("id", id_contracts);
            ui->comboContract->setCurrentIndex(contractRow >= 0 ? contractRow : 0);
        } else {
            setIdContract(0);
            ui->comboContract->setCurrentIndex(0);
        }

        // setam ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);

        dataWasModified(); // modificam forma

        // setam focusul cursorului
        if (ui->comboTypePrices->currentIndex() > 0)
            ui->comboReferringDoctor->setFocus();

    } else if (combo == ui->comboContract) {
        // determinam rolul
        auto roleID            = modelContracts->roleForColumn("id");
        auto role_id_typePrice = modelContracts->roleForColumn("id_typesPrices");
        // ID necesare
        const int id_contracts = ui->comboContract->currentData(roleID).toInt();
        const int id_typePrice = ui->comboContract->currentData(role_id_typePrice).toInt();

        // setam ID contractului
        if (id_contracts > 0)
            setIdContract(id_contracts);

        // ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);
        else
            ui->comboTypePrices->setCurrentIndex(0);

        dataWasModified(); // modificarea formei

    } else if (combo == ui->comboTypePrices) {
        // rolul si ID necesare
        auto roleID = modelTypePrices->roleForColumn("id");
        const int id_typePrice = ui->comboTypePrices->currentData(roleID).toInt();

        // setam ID typePrice
        if (id_typePrice > 0)
            setIdTypePrice(id_typePrice);

        dataWasModified(); // modificam forma

    } else if (combo == ui->comboReferringDoctor) {
        // rolul si ID
        auto roleID = modelRefferingDoctors->roleForColumn("id");
        const int id_doctor = ui->comboReferringDoctor->currentData(roleID).toInt();

        // setam ID doctorului
        setIdRefferingDoctor(id_doctor > 0 ? id_doctor : 0);

        dataWasModified(); // modificam forma

        // schimbam indexul toolBoxului si focusul cursorului
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        ui->comboPatient->setFocus();

    } else if (combo == ui->comboPerformingDoctor) {
        // rolul si ID necesar
        auto roleID = modelPerformingDoctors->roleForColumn("id");
        const int id_doctor_exec = ui->comboPerformingDoctor->currentData(roleID).toInt();

        // setam ID
        if (id_doctor_exec)
            setIdPerformingDoctor(id_doctor_exec);

        dataWasModified(); // modifcarea formei

    } else if (combo == ui->comboNurse) {
        // rolul si ID necesar
        auto roleID = modelNurses->roleForColumn("id");
        const int id_nurse = ui->comboNurse->currentData(roleID).toInt();

        // setam ID nurse
        if (id_nurse > 0)
            setIdNurse(id_nurse);

        dataWasModified(); // modificarea formei
    }
}

void OrderDialog::newPatientStateChanged(const int value)
{
    setPatientDataEnabled(value == Qt::Checked);
}

bool OrderDialog::splitFullNamePatient(QString &name, QString &fName)
{
    const QString text = ui->comboPatient->currentText().trimmed();

    if (text.isEmpty())
        return false;

    // partea pana la prima virgula (nume complet)
    QString fullName = text.section(',', 0, 0).trimmed();

    if (fullName.isEmpty())
        return false;

    // split robust dupa spatii multiple
    QStringList parts = fullName.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    if (parts.size() < 2)
        return false;

    name = parts.first();       // nume
    parts.removeFirst();
    fName = parts.join(' ');    // prenume (poate avea mai multe)

    return true;
}

void OrderDialog::onValidateDataPatient()
{
    /** 1. Verificam daca este completat combo pacientului */
    if (ui->comboPatient->currentText().isEmpty()){
        BalloonTip::showBalloonFor(ui->comboPatient,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Nu sunt determinate datele pacientului !!!"),
                                   4000,
                                   true,
                                   BalloonTip::TopCenter);
        return;
    }

    /** 2. Anuntam variabile necesare */
    QString lastName;
    QString firstName;

    /** 3. Despartim nume, prenume */
    if (! splitFullNamePatient(lastName, firstName))
        return;

    /** 4. Verificam daca sunt completate variabile */
    if (lastName.isEmpty() || firstName.isEmpty())
        return;

    /** 5. setam structura */
    PatientDataStructure patientData;
    patientData.id            = patientId() <= 0 ? 0 : patientId();
    patientData.deletionMark  = StatusObject::ZeroWrite;
    patientData.idnp          = ui->patientIDNP->text();
    patientData.name          = lastName;
    patientData.firstName     = firstName;
    patientData.middleName    = QString(); // nu se completează în interfața curentă
    patientData.medicalPolicy = ui->patientMedicalPolicy->text();
    patientData.birthday      = ui->patientBirthday->date();
    patientData.address       = ui->patientAddress->text();
    patientData.phone         = ui->patientPhone->text();
    patientData.email         = ui->patientEmail->text();
    patientData.comment       = QString();
    // patientData.uuid - setarea se petrece in PatientSaverWorker, apoi se
    // transmite in initSyncPatientData(SyncPatientWorker)

    /** 6. Creăm thread-ul pentru trimiterea */
    QThread *thread = new QThread();

    /** 7. alocam memoria worker-lui si mutam in flux nou */
    auto worker = new PatientSaverWorker(dbProvider(), patientData);
    worker->moveToThread(thread);

    /** 8. conectarea - lansarea procesului inserarii sau actualizarii datelor */
    if (ui->newPatient->isChecked() && patientId() <= 0){
        connect(thread, &QThread::started,
                worker, &PatientSaverWorker::processInsert, Qt::UniqueConnection);
    } else {
        connect(thread, &QThread::started,
                worker, &PatientSaverWorker::processUpdate, Qt::UniqueConnection);
    }

    /** 9. conectarea - procesarea daca exista pacient in bd si daca exista erori la inserare sau actualizare datelor */
    connect(worker, &PatientSaverWorker::finishedPatientExistInBD,
            this, [this](const PatientDataStructure savedPatient)
            {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle(tr("Varificarea datelor"));
                msgBox.setText(tr("Pacientul(a) exista in baza da date:"
                                  "<br>%1")
                                   .arg(" - nume, prenume: " + ui->comboPatient->currentText() + "<br>"
                                        " - anul nasterii: " + savedPatient.birthday.toString("dd.MM.yyyy") + "<br>"
                                        " - idnp: " + savedPatient.idnp)
                               );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            });
    connect(worker, &PatientSaverWorker::finishedError,
            this, [this](const QStringList &listErr)
            {
                if (listErr.isEmpty())
                    return;

                CustomMessage msg(this);
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(tr("Inserarea/modificarea datelor pacientului %1 nu s-a efectuat !!!")
                                      .arg(ui->comboPatient->currentText()));
                msg.setDetailedText(listErr.join("\n"));
                msg.exec();
            });
    connect(worker, &PatientSaverWorker::finished,
            this, [this](const PatientDataStructure savedPatient)
            {
                /** prezentam mesaj ca au fost salvate datele pacientului */
                popUp->setPopupText(tr("Datele pacientului <b>%1</b><br> "
                                       "au fost inserate/modificate in baza de date cu succes.")
                                        .arg(ui->comboPatient->currentText()));
                popUp->show();

                /** setam ID pacientului si emitem signal */
                if (savedPatient.id > 0) {
                    setIdPatient(savedPatient.id); // setam 'id' pacientului
                    emit createNewPacient(); // emitem signal pu conectarea din alte clase

                    /** aplicam checkBox */
                    if (ui->newPatient->isChecked())        // dupa validarea datelor pacientului
                        ui->newPatient->setChecked(false);  // obiectul(pacientul) nu este nou
                }

                /** inchidem acces la campuri */
                setPatientDataEnabled(false);

                /** instalam focusul */
                ui->editFilterPattern->setFocus();

                /** initierea syncronizarii */
                if (MainDatabaseConnectionContext::instance().isSqlite()
                    && SettingsService::instance().synchronization().enabled)
                    initSyncPatientData(savedPatient);

            });

    /** 10. conectarea - distrugerea daca a fost emis ca exista pacient */
    connect(worker, &PatientSaverWorker::finishedPatientExistInBD,
            thread, &QThread::quit);
    connect(worker, &PatientSaverWorker::finishedPatientExistInBD,
            worker, &PatientSaverWorker::deleteLater);

    /** 11. conectarea - distrugerea daca a fost emis ca sunt erori */
    connect(worker, &PatientSaverWorker::finishedError,
            thread, &QThread::quit);
    connect(worker, &PatientSaverWorker::finishedError,
            worker, &PatientSaverWorker::deleteLater);

    /** 12. conectarea - distrugerea daca inserate/actualizate datele cu succes */
    connect(worker, &PatientSaverWorker::finished,
            thread, &QThread::quit);
    connect(worker, &PatientSaverWorker::finished,
            worker, &PatientSaverWorker::deleteLater);

    /** 13. conectarea distrugerea thread-lui */
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    /** 14. start thread */
    thread->start();
}

void OrderDialog::onEditDataPatient()
{
    setPatientDataEnabled(true);
}

void OrderDialog::onClearDataPatient()
{
    setPatientDataEnabled(true);
    setIdPatient(-1);

    {
        QSignalBlocker blocker(ui->comboPatient->lineEdit());
        ui->comboPatient->setEditText(QString());
    }

    ui->patientBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd"));
    ui->patientIDNP->clear();
    ui->patientAddress->clear();
    ui->patientMedicalPolicy->clear();
    ui->patientPhone->clear();
    ui->patientEmail->clear();
    ui->newPatient->setChecked(false);
}

void OrderDialog::onOpenPatientHistory()
{
    if (patientId() <= 0)
        return;

    PatientHistory patientHistory(m_db, this);
    patientHistory.setIdPatient(patientId());
    hide();
    patientHistory.exec();
    show();
    ui->editFilterPattern->setFocus();
}

void OrderDialog::slotPatientTextChanged(const QString &text)
{
    Q_UNUSED(text);
    timerPatientSearch->start();
}

void OrderDialog::updateModelPatientsByText()
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

void OrderDialog::activatedItemCompleter(const QModelIndex &index)
{
    timerPatientSearch->stop();

    const int current_id = index.data(Qt::UserRole).toInt();
    if (current_id <= 0)
        return;

    setIdPatient(current_id);

    if (!ui->newPatient->isChecked())
        ui->editFilterPattern->setFocus();
}

void OrderDialog::filterRegExpChanged()
{
    proxy->setFilterRegularExpression(
        QRegularExpression(ui->editFilterPattern->text(),
                           QRegularExpression::CaseInsensitiveOption));
}

void OrderDialog::onDoubleClickedTableSource(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = proxy->mapToSource(index);
    if (!sourceIndex.isValid())
        return;

    const QVariantMap itemSource = modelTableSource->rowDataByRow(sourceIndex.row());
    const QString codSource = itemSource["cod"].toString();

    // verificam daca exista in tabela investigatia
    for (int n = 0; n < modelTableOrder->rowCount(); ++n) {
        if (ui->tableViewOrder->isRowHidden(n))
            continue;
        const QVariantMap itemOrder = modelTableOrder->rowDataByRow(n);
        const QString codOrder = itemOrder["cod"].toString();
        if (codOrder == codSource) {
            QMessageBox::warning(this, tr("Atentie"),
                                 tr("Investigatia <b>'%1 - %2'</b> exista in tabel.")
                                     .arg(codOrder, itemOrder["name"].toString()),
                                 QMessageBox::Ok);
            return;
        }
    }

    QVariantMap rowData;
    rowData["id"]           = m_tempOrderRowId--; // seteaza initial -1, -2, -3 etc.
    rowData["deletionMark"] = documentStatus();
    rowData["id_orderEcho"] = orderId();
    rowData["cod"]          = codSource;
    rowData["name"]         = itemSource["name"];
    rowData["price"]        = itemSource["price"];

    if (modelTableOrder->addRow(rowData)) {
        updateDocumentSumText();
        dataWasModified();
    }

    // dupa alegerea investigatiei curatim 'editFilterPattern'
    if (! ui->editFilterPattern->text().isEmpty())
        ui->editFilterPattern->setText(""); // pu actualizarea TableSource
}

void OrderDialog::onDoubleClickedTableOrder(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int row = index.row();
    QModelIndex idx = modelTableOrder->index(row, OrderTableSections::Price);
    ui->tableViewOrder->setCurrentIndex(idx);
    ui->tableViewOrder->edit(idx);
}

void OrderDialog::onPrint(PrintType::Column type_print, const QString &filePDF)
{
    if (isNewDocument()) {
        QMessageBox::warning(this,
                             tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nPrintare nu este posibila."),
                             QMessageBox::Ok);
        return;
    }

    OrderPrintService service(m_db, m_currentDB);
    OrderPrintService::Request request;
    request.orderId      = orderId();
    request.mode         = type_print;
    request.pdfFile      = filePDF;
    request.reportParent = this;

    const OrderPrintService::Result result = service.print(request);
    if (!result.success) {
        CustomMessage message(this);
        message.setWindowTitle(QGuiApplication::applicationDisplayName());
        message.setTextTitle(tr("Printare nu este posibilă !!!"));
        message.setDetailedText(result.error);
        message.exec();
    }
}

int OrderDialog::valuePaymentOrder() const
{
    bool noncomercial = ui->comboTypePrices->currentData(modelTypePrices->roleForColumn("noncomercial")).toBool();

    if (noncomercial) {
        return PaymentMethod::Transfer;
    } else {
        if (ui->cardPayment->isChecked())
            return PaymentMethod::Card;
        else
            return PaymentMethod::Cash;
    }
}

bool OrderDialog::insertDataOrder(QString &details_error)
{
    QVector<QVariant> data;
    data.append(documentStatus());
    data.append(ui->numberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(organizationId());
    data.append(contractId());
    data.append(priceTypeId());
    data.append((referringDoctorId() <= 0)
                    ? QVariant()
                    : referringDoctorId());
    data.append((performingDoctorId() <= 0)
                    ? QVariant()
                    : performingDoctorId());
    data.append((nurseId() <= 0)
                    ? QVariant()
                    : nurseId());
    data.append(patientId());
    data.append(authorUserId());
    data.append(documentSum());
    data.append((ui->editComment->toPlainText().isEmpty())
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append(valuePaymentOrder());
    data.append(m_attachedImages);

    QUuid uuid = QUuid::createUuid();
    data.append(uuid.toRfc4122());

    QVariant newId;
    if (! m_db.execPreparedFromFileReturnID(m_db.getDatabase(),
                                           ":/sql/queries_doc/order_insert.sql",
                                           data,
                                           &newId,
                                           &details_error))
    {
        return false;
    } else {
        setOrderId(newId.toInt());
        return true;
    }
}

bool OrderDialog::updateDataOrder(QString &details_error)
{
    QVector<QVariant> data;
    data.append(documentStatus());
    data.append(ui->numberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(organizationId());
    data.append(contractId());
    data.append(priceTypeId());
    data.append((referringDoctorId() <= 0)
                    ? QVariant()
                    : referringDoctorId());
    data.append((performingDoctorId() <= 0)
                    ? QVariant()
                    : performingDoctorId());
    data.append((nurseId() <= 0)
                    ? QVariant()
                    : nurseId());
    data.append(patientId());
    data.append(authorUserId());
    data.append(documentSum());
    data.append((ui->editComment->toPlainText().isEmpty())
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append(valuePaymentOrder());
    data.append(m_attachedImages);
    data.append(orderId());

    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   ":/sql/queries_doc/order_update.sql",
                                   data,
                                   &details_error))
    {
        return false;
    } else {
        return true;
    }
}

void OrderDialog::editRowTableOrder()
{
    const QModelIndex idx = ui->tableViewOrder->currentIndex();
    if (!idx.isValid())
        return;
    onDoubleClickedTableOrder(idx);
}

void OrderDialog::removeRowTableOrder()
{
    const QModelIndex idx = ui->tableViewOrder->currentIndex();
    if (!idx.isValid())
        return;

    modelTableOrder->removeRowAt(idx.row());
    updateDocumentSumText();
    dataWasModified();
}

void OrderDialog::slotContextMenuRequested(const QPoint &pos)
{
    const QModelIndex idx = ui->tableViewOrder->indexAt(pos);
    if (!idx.isValid())
        return;

    ui->tableViewOrder->setCurrentIndex(idx);
    ui->tableViewOrder->selectRow(idx.row());

    QMenu menu(this);

    QAction *actionEditRow = menu.addAction(QIcon(":/img/toolBar/edit.png"),
                                            tr("Editează rândul."));
    QAction *actionRemoveRow = menu.addAction(QIcon(":/img/toolBar/delete.png"),
                                              tr("Șterge rândul."));

    QAction *chosen = menu.exec(ui->tableViewOrder->viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == actionEditRow) {
        editRowTableOrder();
    } else if (chosen == actionRemoveRow) {
        removeRowTableOrder();
    }
}

bool OrderDialog::deleteRowsOrderTable(QString &details_error)
{
    QSqlQuery qry(m_currentDB);
    qry.prepare(QStringLiteral("DELETE FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho"));
    qry.bindValue(":id_orderEcho", orderId());

    if (!qry.exec()) {
        details_error = qry.lastError().text();
        return false;
    }

    return true;
}

bool OrderDialog::reinsertAllOrderRows(QString &details_error)
{
    QList<QVariantMap> rows;
    rows.reserve(modelTableOrder->rowCount());

    int tempId = -1;

    // resetam ID (important negativ -1, -2 etc), deletionMark, ID(order)
    for (int i = 0; i < modelTableOrder->rowCount(); ++i) {
        QVariantMap rowData = modelTableOrder->rowDataByRow(i);
        if (rowData.isEmpty()) {
            details_error = tr("Rând invalid în modelul detaliilor.");
            return false;
        }

        rowData["id"]           = tempId--; // -1, -2, -3 etc
        rowData["deletionMark"] = documentStatus();
        rowData["id_orderEcho"] = orderId();

        rows.append(rowData);
    }

    // reconstruim modelul cu ID-uri temporare corecte
    modelTableOrder->setRows(rows);

    // inseram toate liniile
    for (int i = 0; i < modelTableOrder->rowCount(); ++i) {
        if (!modelTableOrder->insertRowToDatabase(m_db, "orderEchoTable", i, &details_error))
            return false;
    }

    return true;
}

bool OrderDialog::controlRequiredObjects()
{
    if (ui->comboOrganization->currentIndex() <= StatusObject::ZeroWrite){
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_organization);
        BalloonTip::showBalloonFor(ui->comboOrganization,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este selectată <b>'Organizația'</b> !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    if (ui->comboContract->currentIndex() <= StatusObject::ZeroWrite){
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_organization);
        BalloonTip::showBalloonFor(ui->comboContract,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este selectat <b>'Contractul'</b> !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    if (ui->comboTypePrices->currentIndex() <= StatusObject::ZeroWrite){
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_organization);
        BalloonTip::showBalloonFor(ui->comboTypePrices,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este selectat <b>'Tipul prețului'</b> !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    if (ui->comboPatient->currentText().isEmpty() || patientId() <= 0){
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        BalloonTip::showBalloonFor(ui->comboPatient,
                                   QMessageBox::Information,
                                   tr("Verificarea datelor"),
                                   tr("Nu este selectat <b>'Pacientul'</b> !!!"),
                                   4000,
                                   true,
                                   BalloonTip::BottomCenter);
        return false;
    }
    QSqlQuery patientQuery(m_currentDB);
    patientQuery.prepare(QStringLiteral("SELECT 1 FROM patients WHERE id = ? LIMIT 1"));
    patientQuery.addBindValue(patientId());
    if (!patientQuery.exec() || !patientQuery.next()) {
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        BalloonTip::showBalloonFor(ui->comboPatient,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Pacientul selectat (ID %1) nu există în baza de date. Selectați din nou pacientul din listă.")
                                       .arg(patientId()),
                                   6000,
                                   true,
                                   BalloonTip::BottomCenter);
        qWarning(logWarning()).noquote()
            << QStringLiteral("OrderDialog: patient_id=%1 nu există în patients; SQL: %2")
                   .arg(patientId())
                   .arg(patientQuery.lastError().text());
        return false;
    }
    if (ui->tableViewOrder->model()->rowCount() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este aleasa nici o investigatie !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    return true;
}

void OrderDialog::onOpenReport()
{
    // controlam daca documentul este validat
    if (isNewDocument()){
        QMessageBox::warning(this, tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nRaportul ecografic nu poate fi format."),
                             QMessageBox::Ok);
        return;
    }


    this->hide();

    const QString orderDisplayText = QStringLiteral("Comanda ecografică nr.%1 din %2")
                                         .arg(ui->numberDoc->text().trimmed(), ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));

    ReportDialog::ReportDialogParameters params;

    // determinam daca este salvat document 'Report', deschidem document
    QSqlQuery q(m_currentDB);
    q.prepare("SELECT id, deletionMark FROM reportEcho WHERE id_orderEcho = :id_orderEcho");
    q.bindValue(":id_orderEcho", orderId());

    if (!q.exec()) {
        this->show();
        QMessageBox::critical(this, tr("Eroare SQL"),
                              tr("Nu s-a putut verifica raportul existent:\n%1")
                                  .arg(q.lastError().text()));
        return;
    }

    if (q.next()) {
        params.isNew     = false;
        params.id        = q.value("id").toInt();
        params.idOrder   = orderId();
        params.idPatient = patientId();
        params.status    = DocStatus::determineStatusDoc(q.value("deletionMark").toInt());
        params.orderDisplayText = orderDisplayText;

        auto *report = new ReportDialog(m_db, params, this);
        report->setAttribute(Qt::WA_DeleteOnClose);

        connect(report, &QObject::destroyed, this, [this]() {
            this->show();
        });

        report->show();
        return;
    }

    // determinam investigatii din 'modelTableOrder'
    QStringList codes;
    for (int n = 0; n < modelTableOrder->rowCount(); ++n) {
        if (ui->tableViewOrder->isRowHidden(n))
            continue;
        const QVariantMap itemOrder = modelTableOrder->rowDataByRow(n);
        const QString cod = itemOrder.value("cod").toString().trimmed();

        if (!cod.isEmpty()) // verificam coduri goale
            codes << cod;
    }

    CustomDialogInvestig dlg(this);
    dlg.setCodes(codes);

    if (dlg.exec() != QDialog::Accepted) {
        this->show();
        return;
    }

    params.isNew     = true;
    params.idPatient = patientId();
    params.idOrder   = orderId();
    params.status    = DocStatus::determineStatusDoc(documentStatus());
    params.systems   = dlg.selectedSystems();
    params.orderDisplayText = orderDisplayText;

    auto *report = new ReportDialog(m_db, params, nullptr);
    report->setAttribute(Qt::WA_DeleteOnClose);
    connect(report, &QObject::destroyed, this, [this]() {
        this->close();
    });
    report->show();
}

bool OrderDialog::onSave()
{
    if (!controlRequiredObjects())
        return false;

    // Pentru documentul nou, initializeNewOrder() pornește actualizarea orei.
    // La prima salvare/validare oprim timerul ca data documentului să rămână fixă.
    if (timer->isActive())
        timer->stop();

    const int initialId = orderId();
    const int initialPost = documentStatus();
    const bool wasNew = isNewDocument();
    const int initialAttachedImages = m_attachedImages;
    const QString initialNumberDoc = ui->numberDoc->text();
    QList<QVariantMap> initialOrderRows;
    initialOrderRows.reserve(modelTableOrder->rowCount());
    for (int row = 0; row < modelTableOrder->rowCount(); ++row)
        initialOrderRows.append(modelTableOrder->rowDataByRow(row));

    if (documentStatus() == DocStatus::Unknow)
        setPost(DocStatus::Write);

    QString details_error;
    QSqlDatabase current_db = m_db.getDatabase();

    if (!current_db.transaction()) {
        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Nu s-a putut porni tranzactia."));
        msg.setDetailedText(current_db.lastError().text());
        msg.exec();
        setPost(initialPost);
        return false;
    }

    const auto rollbackAndRestore = [&]() {
        if (!current_db.rollback()) {
            qCritical(logCritical()).noquote()
            << QStringLiteral("Rollback-ul salvării comenzii a eșuat: %1")
                   .arg(current_db.lastError().text());
        }

        setOrderId(initialId);
        m_attachedImages = initialAttachedImages;
        setPost(initialPost);
        ui->numberDoc->setText(initialNumberDoc);
        modelTableOrder->setRows(initialOrderRows);
    };

    if (wasNew) {

        if (m_attachedImages == StatusObject::Unknow)
            m_attachedImages = 0;

        // generam/setam nr.documentului
        if (ui->numberDoc->text().trimmed().isEmpty()) {
            const int year = ui->dateTimeDoc->date().year();
            const int nextNumber = m_db.getNextNumberDoc("orderEcho", year, &details_error);

            if (nextNumber <= 0) {
                rollbackAndRestore();

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

        // inseram header-ul
        if (!insertDataOrder(details_error)) {
            rollbackAndRestore();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Validarea documentului nu s-a efectuat."));
            msg.setDetailedText(details_error);
            msg.exec();

            return false;
        }

        // inseram toate liniile
        for (int i = 0; i < modelTableOrder->rowCount(); ++i) {
            modelTableOrder->setFieldValueInternal(i, "deletionMark", documentStatus());
            modelTableOrder->setFieldValueInternal(i, "id_orderEcho", orderId());

            if (!modelTableOrder->insertRowToDatabase(m_db, "orderEchoTable", i, &details_error)) {
                rollbackAndRestore();

                CustomMessage msg(this);
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(tr("Validarea documentului nu s-a efectuat."));
                msg.setDetailedText(details_error);
                msg.exec();

                return false;
            }
        }

    } else {

        // actualizam header-ul
        if (!updateDataOrder(details_error)) {
            rollbackAndRestore();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msg.setDetailedText(details_error);
            msg.exec();

            return false;
        }

        // stergem toate liniile vechi
        if (!deleteRowsOrderTable(details_error)) {
            rollbackAndRestore();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Nu s-au putut șterge liniile vechi ale documentului."));
            msg.setDetailedText(details_error);
            msg.exec();

            return false;
        }

        // reinseream toate liniile curente
        if (!reinsertAllOrderRows(details_error)) {
            rollbackAndRestore();

            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msg.setDetailedText(details_error);
            msg.exec();

            return false;
        }
    }

    if (!current_db.commit()) {
        const QString commitError = current_db.lastError().text();
        rollbackAndRestore();

        CustomMessage msg(this);
        msg.setWindowTitle(QGuiApplication::applicationDisplayName());
        msg.setTextTitle(tr("Salvarea documentului nu s-a finalizat."));
        msg.setDetailedText(commitError);
        msg.exec();
        return false;
    }

    if (wasNew) {
        qInfo(logInfo()) << QStringLiteral("Documentul 'Comanda ecografica' nr.='%1' creat cu succes in baza de date.")
                                .arg(ui->numberDoc->text());
    } else {
        qInfo(logInfo()) << QStringLiteral("Documentul 'Comanda ecografica' nr.='%1' modificat cu succes in baza de date.")
        .arg(ui->numberDoc->text());
    }

    popUp->setPopupText(tr("Documentul a fost %1 cu succes<br> in baza de date.")
                            .arg(m_postInProgress ? tr("validat") : tr("salvat")));
    popUp->show();

    setWindowModified(false);

    if (!m_postInProgress)
        emit SaveDocument();

    if (MainDatabaseConnectionContext::instance().isSqlite()
        && SettingsService::instance().synchronization().enabled)
        initSyncOrderData();

    return true;
}

bool OrderDialog::onPost()
{
    const int oldPost = documentStatus();
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
    yesButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    noButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton) {
        QString str;
        onPrint(PrintType::Preview, str);
    }

    emit PostDocument();
    accept();
    return true;
}

void OrderDialog::setupDateDocFormat()
{
    ui->dateTimeDoc->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
    ui->dateTimeDoc->setCalendarPopup(true);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &OrderDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged,
            this, &OrderDialog::onDateTimeChanged, Qt::UniqueConnection);
}

void OrderDialog::updateModelOrganizations()
{
    if (modelOrganizations)
        delete modelOrganizations;

    QString str = m_db.getTextSQL(":/sql/queries/organizations_combo_view.sql");
    modelOrganizations = new QueryRolesModel(str, ui->comboOrganization);
    modelOrganizations->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboOrganization->setModel(modelOrganizations);
    ui->comboOrganization->setModelColumn(modelOrganizations->columnIndex("name"));
}

void OrderDialog::updateModelContracts()
{
    QueryRolesModel *newModel = nullptr;

    if (m_documentContext.data().organizationId <= 0) {
        QString str = m_db.getTextSQL(":/sql/queries/contracts_view.sql");
        newModel = new QueryRolesModel(str, ui->comboContract);
        newModel->setEmptyRowEnabled(true);
    } else {
        QSqlQuery qry(m_currentDB);
        qry.prepare(m_db.getTextSQL(
            MainDatabaseConnectionContext::instance().isSqlite()
                ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
                : ":/sql/queries/contracts_select_by_organization_mysql.sql"));

        qry.addBindValue(m_documentContext.data().organizationId);
        qry.addBindValue(m_documentContext.data().contractId);

        if (!qry.exec()) {
            qCritical(logCritical()).noquote()
            << "SQL error:" << qry.lastError().text()
            << "Last query:" << qry.lastQuery();
            ui->comboContract->addItem(tr("<<-- contract indisponibil -->>"), 0);
            return;
        }

        newModel = new QueryRolesModel(QString(), ui->comboContract);
        newModel->setQuery(std::move(qry));
        newModel->setEmptyRowEnabled(true);
    }

    if (modelContracts)
        delete modelContracts;

    modelContracts = newModel;
    ui->comboContract->setModel(modelContracts);
    ui->comboContract->setModelColumn(
        modelContracts->columnIndex(m_documentContext.data().organizationId <= 0 ? "contract_owner" : "name"));
}

void OrderDialog::updateModelTypesPrices()
{
    if (modelTypePrices)
        delete modelTypePrices;

    QString str = m_db.getTextSQL(":/sql/queries/typePrices_combo_view.sql");
    modelTypePrices = new QueryRolesModel(str, ui->comboTypePrices);
    modelTypePrices->setEmptyRowEnabled(true); // <<- Selecteaza ->>
    ui->comboTypePrices->setModel(modelTypePrices);
    ui->comboTypePrices->setModelColumn(modelTypePrices->columnIndex("name"));
}

void OrderDialog::updateModelPerformingDoctors()
{
    if (modelPerformingDoctors)
        delete modelPerformingDoctors;

    QString str = m_db.getTextSQL(":/sql/queries/doctors_combo_view.sql");
    modelPerformingDoctors = new QueryRolesModel(str, ui->comboPerformingDoctor);
    modelPerformingDoctors->setEmptyRowEnabled(true);
    ui->comboPerformingDoctor->setModel(modelPerformingDoctors);
    ui->comboPerformingDoctor->setModelColumn(modelPerformingDoctors->columnIndex("display"));
}

void OrderDialog::updateModelNurses()
{
    if (modelNurses)
        delete modelNurses;

    QString str = m_db.getTextSQL(":/sql/queries/nurses_combo_view.sql");
    modelNurses = new QueryRolesModel(str, ui->comboNurse);
    modelNurses->setEmptyRowEnabled(true);
    ui->comboNurse->setModel(modelNurses);
    ui->comboNurse->setModelColumn(modelNurses->columnIndex("display"));
}

void OrderDialog::updateModelRefferingDoctors()
{
    const int selectedDoctorId = referringDoctorId();
    QSignalBlocker blocker(ui->comboReferringDoctor);

    if (modelRefferingDoctors)
        delete modelRefferingDoctors;

    QString str = m_db.getTextSQL(":/sql/queries/doctors_combo_view.sql");
    modelRefferingDoctors = new QueryRolesModel(str, ui->comboReferringDoctor);
    modelRefferingDoctors->setEmptyRowEnabled(true);
    ui->comboReferringDoctor->setModel(modelRefferingDoctors);
    ui->comboReferringDoctor->setModelColumn(modelRefferingDoctors->columnIndex("display"));

    const int row = selectedDoctorId > 0
                        ? modelRefferingDoctors->rowById("id", selectedDoctorId)
                        : 0;
    ui->comboReferringDoctor->setCurrentIndex(row >= 0 ? row : 0);
}

void OrderDialog::setStyleMaxVisibleItemsComboBox()
{
    const auto comboBoxes = findChildren<QComboBox*>();
    for (QComboBox *combo : comboBoxes) {
        if (!combo)
            continue;
        combo->setStyleSheet(QStringLiteral("combobox-popup: 0;"));
        combo->setMaxVisibleItems(15);

        if (combo->view())
            combo->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
}

void OrderDialog::initConnections()
{
    // --- combo
    auto signal = QOverload<int>::of(&QComboBox::currentIndexChanged);
    auto slot   = &OrderDialog::indexChangedCombo;

    connect(ui->comboOrganization, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboContract, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboTypePrices, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboPerformingDoctor, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboReferringDoctor, signal, this, slot, Qt::UniqueConnection);
    connect(ui->comboNurse, signal, this, slot, Qt::UniqueConnection);

    // --- new patient
    connect(ui->newPatient, &QCheckBox::checkStateChanged,
            this, &OrderDialog::newPatientStateChanged, Qt::UniqueConnection);

    // --- card & cash
    connect(ui->cardPayment, &QCheckBox::checkStateChanged,
            this, &OrderDialog::dataWasModified, Qt::UniqueConnection);

    // --- toolbutton
    const QList<QToolButton*> tbs = findChildren<QToolButton*>();
    for (QToolButton *tb : std::as_const(tbs))
        tb->setStyleSheet(toolButtonStyleForText);

    connect(ui->btnValidatePatient, &QToolButton::clicked,
            this, &OrderDialog::onValidateDataPatient, Qt::UniqueConnection);
    connect(ui->btnEditPatient, &QToolButton::clicked,
            this, &OrderDialog::onEditDataPatient, Qt::UniqueConnection);
    connect(ui->btnClearPatient, &QToolButton::clicked,
            this, &OrderDialog::onClearDataPatient, Qt::UniqueConnection);
    connect(ui->btnPatientHistory, &QToolButton::clicked,
            this, &OrderDialog::onOpenPatientHistory, Qt::UniqueConnection);
    connect(ui->btnCreateNewDoctor, &QToolButton::clicked,
            this, &OrderDialog::onCreateNewDoctor, Qt::UniqueConnection);
    connect(ui->btnOpenDoctor, &QToolButton::clicked,
            this, &OrderDialog::onOpenCatalogDoctor, Qt::UniqueConnection);

    // --- toolBox
    connect(ui->toolBox, QOverload<int>::of(&QToolBox::currentChanged),
            this, QOverload<int>::of(&OrderDialog::changeIconForItemToolBox), Qt::UniqueConnection);

    connect(ui->btnReport, &QAbstractButton::clicked,
            this, &OrderDialog::onOpenReport, Qt::UniqueConnection);
    connect(ui->btnOk, &QAbstractButton::clicked,
            this, &OrderDialog::onPost, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &OrderDialog::onSave, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &OrderDialog::close, Qt::UniqueConnection);
}

void OrderDialog::setupMaxLengthForDataPatient()
{
    ui->patientIDNP->setMaxLength(20);           // limitarea caracterilor
    ui->patientMedicalPolicy->setMaxLength(20);
    ui->patientAddress->setMaxLength(255);
    ui->patientPhone->setMaxLength(100);
    ui->editFilterPattern->setPlaceholderText(tr("...căutare după denumirea investigației sau după cuvânt cheie/model"));
}

void OrderDialog::initSetCompleter()
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
            this, &OrderDialog::slotPatientTextChanged,
            Qt::UniqueConnection);

    connect(timerPatientSearch, &QTimer::timeout,
            this, &OrderDialog::updateModelPatientsByText,
            Qt::UniqueConnection);

    connect(completerPatients, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex &>::of(&OrderDialog::activatedItemCompleter));
}

void OrderDialog::ensurePatientInCompleterModel(int idPatient, const QString &fullName)
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

void OrderDialog::loadPatientDetails()
{
    if (patientId() <= 0)
        return;

    QSqlQuery qry(m_currentDB);
    qry.prepare(MainDatabaseConnectionContext::instance().isSqlite()
                    ? m_db.getTextSQL(":/sql/queries_doc/patients_byID_sqlite.sql")
                    : m_db.getTextSQL(":/sql/queries_doc/patients_byID_mariadb.sql"));
    qry.addBindValue(patientId());

    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
        << "SQL error:" << qry.lastError().text();
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    if (!qry.next()) {
        qCritical(logCritical()).noquote()
            << "Pacientul cu id =" << patientId() << " nu a fost găsit";
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    const QString fullName = qry.value(PatientSearchColumns::FullName).toString().trimmed();

    // ne asiguram ca pacientul exista in modelul completerului
    ensurePatientInCompleterModel(patientId(), fullName);

    // sincronizam textul din combo
    {
        QSignalBlocker blocker(ui->comboPatient->lineEdit());
        ui->comboPatient->setEditText(fullName);
    }

    // data nasterii
    {
        const QVariant vBirthday = qry.value(PatientSearchColumns::Birthday);
        QDate birthday = vBirthday.toDate();

        if (!birthday.isValid()) {
            const QString s = vBirthday.toString().trimmed();
            birthday = QDate::fromString(s, "dd.MM.yyyy");
            if (!birthday.isValid())
                birthday = QDate::fromString(s, "yyyy-MM-dd");
            if (!birthday.isValid())
                birthday = QDate::fromString(s, Qt::ISODate);
        }

        if (birthday.isValid())
            ui->patientBirthday->setDate(birthday);
        else
            ui->patientBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd"));
    }

    ui->patientIDNP->setText(qry.value(PatientSearchColumns::IDNP).toString());
    ui->patientMedicalPolicy->setText(qry.value(PatientSearchColumns::MedicalPolicy).toString());
    ui->patientAddress->setText(qry.value(PatientSearchColumns::Address).toString());
    ui->patientPhone->setText(qry.value(PatientSearchColumns::Telephone).toString());
    ui->patientEmail->setText(qry.value(PatientSearchColumns::Email).toString());
}

void OrderDialog::initSyncPatientData(PatientDataStructure patientData)
{
    // 1. Creăm thread-ul pentru trimiterea
    QThread *thread = new QThread();

    // 2. alocam memoria worker-lui si mutam in flux nou
    auto worker = new SyncPatientWorker(dbProvider(), patientData);
    worker->moveToThread(thread);

    // 3. conectarea - lansarea procesului de syncronizare
    connect(thread, &QThread::started,
            worker, &SyncPatientWorker::process);

    connect(worker, &SyncPatientWorker::syncError,
            this, [this](const QString &error)
            {
                CustomMessage msg(this);
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(tr("Datele pacientului au fost salvate local, dar sincronizarea cloud a eșuat."));
                msg.setDetailedText(error);
                msg.exec();
            });

    // 4. conectarea - distrugea workerului
    connect(worker, &SyncPatientWorker::finished,
            thread, &QThread::quit);
    connect(worker, &SyncPatientWorker::finished,
            worker, &SyncPatientWorker::deleteLater);

    // 5. distrugerea thread-lui
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    // 6. lansarea thread-lui
    thread->start();
}

void OrderDialog::initSyncOrderData()
{
    PatientDataStructure patientData;
    patientData.id = patientId();

    OrderDataStructure orderData;
    orderData.id = orderId();

    QThread *thread = new QThread();
    auto *worker = new SyncOrderWorker(dbProvider(), patientData, orderData);
    worker->moveToThread(thread);

    connect(thread, &QThread::started,
            worker, &SyncOrderWorker::process);
    connect(worker, &SyncOrderWorker::syncError,
            this, [this](const QString &error)
            {
                CustomMessage msg(this);
                msg.setWindowTitle(QGuiApplication::applicationDisplayName());
                msg.setTextTitle(tr("Comanda a fost salvată local, dar sincronizarea cloud a eșuat."));
                msg.setDetailedText(error);
                msg.exec();
            });
    connect(worker, &SyncOrderWorker::finished,
            thread, &QThread::quit);
    connect(worker, &SyncOrderWorker::finished,
            worker, &SyncOrderWorker::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);

    thread->start();
}

void OrderDialog::handleCompleterAddress(const QString &text)
{
    if (text.startsWith("m.", Qt::CaseInsensitive) ||
        text.startsWith("s.", Qt::CaseInsensitive) ||
        text.startsWith("or.", Qt::CaseInsensitive)) {
        ui->patientAddress->setCompleter(completerCity);

    } else {
        ui->patientAddress->setCompleter(nullptr);

    }
}

QStringList OrderDialog::loadDataFromXml(const QString &filePath, const QString &tagName)
{
    QStringList dataList;
    QFile file(filePath);
    if (! file.open(QIODevice::ReadOnly)) {
        qCritical(logCritical()) << "Nu se poate deschide fișierul:" << filePath;
        return dataList;
    }

    QDomDocument doc;
    if (! doc.setContent(&file)) {
        qCritical(logCritical()) << "Eroare la parsarea fișierului XML:" << filePath;
        return dataList;
    }

    QDomNodeList elements = doc.elementsByTagName(tagName);
    for (int i = 0; i < elements.count(); ++i) {
        QDomNode node = elements.at(i);
        if (node.isElement()) {
            dataList << node.toElement().text();
        }
    }

    return dataList;
}

void OrderDialog::initSetCompleterAddress()
{
    cityList = loadDataFromXml(":/xmls/city.xml", "city");

    completerCity = new QCompleter(cityList, this);
    completerCity->setCaseSensitivity(Qt::CaseInsensitive);
    completerCity->setCompletionMode(QCompleter::PopupCompletion);

    connect(ui->patientAddress, &QLineEdit::textChanged,
            this, &OrderDialog::handleCompleterAddress, Qt::UniqueConnection);
}

void OrderDialog::initTableSource()
{
    proxy->setSourceModel(modelTableSource);
    proxy->setFilterKeyColumn(OrderTableSections::Name);

    ui->tableView->setModel(proxy);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tableView->hideColumn(PricingsTableSection::Id);
    ui->tableView->hideColumn(PricingsTableSection::DeletionMark);
    ui->tableView->hideColumn(PricingsTableSection::Id_Pricings);

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSortIndicator(
        PricingsTableSection::Cod, Qt::AscendingOrder);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);

    ui->tableView->setColumnWidth(PricingsTableSection::Cod, 70);
    ui->tableView->setColumnWidth(PricingsTableSection::Name, 500);

    for (auto it = m_layoutSizes.sourceTable.begin(); it != m_layoutSizes.sourceTable.end(); ++it)
        ui->tableView->setColumnWidth(it.key(), it.value());

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &OrderDialog::onDoubleClickedTableSource, Qt::UniqueConnection);

    connect(ui->editFilterPattern, &QLineEdit::textChanged,
            this, &OrderDialog::filterRegExpChanged, Qt::UniqueConnection);
}

void OrderDialog::updateTableSource()
{
    int currentId = -1;

    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);

    if (sourceIndex.isValid())
        currentId = modelTableSource->idByRow(sourceIndex.row());

    QSqlQuery q(m_currentDB);
    q.prepare(m_db.getTextSQL(":/sql/queries_doc/orderPopulateTableSource.sql"));
    q.bindValue(":id_organization", m_documentContext.data().organizationId);
    q.bindValue(":id_contract",     m_documentContext.data().contractId);
    q.bindValue(":id_typePrices",   m_documentContext.data().priceTypeId);
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "updateTableSource error:"
            << q.lastError().text();
        modelTableSource->clear();
        return;
    }

    QList<QVariantMap> rows;
    QSqlRecord rec = q.record();

    while (q.next()) {
        QVariantMap row;
        for (int i = 0; i < rec.count(); ++i)
            row[rec.fieldName(i)] = q.value(i);

        rows.append(row);
    }
    modelTableSource->setRows(rows);

    if (currentId >= 0) {
        const int row = modelTableSource->rowById(currentId);
        if (row >= 0) {
            QModelIndex sourceIdx = modelTableSource->index(row, 0);
            QModelIndex proxyIdx = proxy->mapFromSource(sourceIdx);
            if (proxyIdx.isValid()) {
                ui->tableView->selectRow(proxyIdx.row());
                ui->tableView->scrollTo(proxyIdx);
            }
        }
    }
}

void OrderDialog::initTableOrder()
{
    ui->tableViewOrder->setModel(modelTableOrder);
    ui->tableViewOrder->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewOrder->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableViewOrder->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tableViewOrder->hideColumn(OrderTableSections::Id);
    ui->tableViewOrder->hideColumn(OrderTableSections::DeletionMark);
    ui->tableViewOrder->hideColumn(OrderTableSections::Id_OrderEcho);

    ui->tableViewOrder->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewOrder->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableViewOrder->horizontalHeader()->setSortIndicator(
        OrderTableSections::Cod, Qt::AscendingOrder);

    ui->tableViewOrder->verticalHeader()->setDefaultSectionSize(30);

    ui->tableViewOrder->setColumnWidth(OrderTableSections::Cod, 70);
    ui->tableViewOrder->setColumnWidth(OrderTableSections::Name, 500);

    for (auto it = m_layoutSizes.orderTable.begin(); it != m_layoutSizes.orderTable.end(); ++it)
        ui->tableViewOrder->setColumnWidth(it.key(), it.value());

    connect(ui->tableViewOrder->model(), &QAbstractItemModel::dataChanged,
            this, &OrderDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->tableViewOrder, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &OrderDialog::onDoubleClickedTableOrder, Qt::UniqueConnection);

    connect(ui->tableViewOrder->model(), &QAbstractItemModel::dataChanged,
            this, &OrderDialog::updateDocumentSumText, Qt::UniqueConnection);

    connect(ui->tableViewOrder, &QWidget::customContextMenuRequested,
            this, &OrderDialog::slotContextMenuRequested, Qt::UniqueConnection);
}

void OrderDialog::updateTableOrder()
{
    int currentId = -1;

    const QModelIndex currentIndex = ui->tableViewOrder->currentIndex();
    if (currentIndex.isValid())
        currentId = modelTableOrder->idByRow(currentIndex.row());

    QSqlQuery q(m_currentDB);
    q.prepare("SELECT * FROM orderEchoTable WHERE id_orderEcho = :id_orderEcho");
    q.bindValue(":id_orderEcho", m_documentContext.data().orderId);
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "updateTableOrder error:"
            << q.lastError().text();
        modelTableOrder->clear();
        return;
    }

    QList<QVariantMap> rows;
    QSqlRecord rec = q.record();

    while (q.next()) {
        QVariantMap row;
        for (int i = 0; i < rec.count(); ++i)
            row[rec.fieldName(i)] = q.value(i);

        rows.append(row);
    }
    modelTableOrder->setRows(rows);

    if (currentId >= 0) {
        const int row = modelTableOrder->rowById(currentId);
        if (row >= 0) {
            ui->tableViewOrder->selectRow(row);
            ui->tableViewOrder->scrollTo(modelTableOrder->index(row, 0));
        }
    }

    updateDocumentSumText();
}

void OrderDialog::changeIconForItemToolBox(const int index)
{
    const QIcon expandedIcon = globals().isSystemThemeDark
                                   ? QIcon(QStringLiteral(":/img/common/arrow_down_dark.png"))
                                   : QIcon(QStringLiteral(":/img/common/arrow_down_light.png"));

    const QIcon collapsedIcon = globals().isSystemThemeDark
                                    ? QIcon(QStringLiteral(":/img/common/arrow_right_dark.png"))
                                    : QIcon(QStringLiteral(":/img/common/arrow_right_light.png"));

    for (int i = 0; i < ui->toolBox->count(); ++i) {
        ui->toolBox->setItemIcon(i, (i == index) ? expandedIcon : collapsedIcon);
    }
}

void OrderDialog::setPatientDataEnabled(bool enabled)
{
    ui->patientBirthday->setEnabled(enabled);
    ui->patientIDNP->setEnabled(enabled);
    ui->patientMedicalPolicy->setEnabled(enabled);
    ui->patientAddress->setEnabled(enabled);
    ui->patientPhone->setEnabled(enabled);
    ui->patientEmail->setEnabled(enabled);

    if (enabled)
        ui->patientBirthday->setFocus(); // setam focusul
}

double OrderDialog::documentSum() const
{
    double sum = 0.0;
    for (int n = 0; n < modelTableOrder->rowCount(); ++n) {
        if (ui->tableViewOrder->isRowHidden(n))
            continue;
        sum += modelTableOrder->rowDataByRow(n).value("price").toDouble();
    }
    return sum;
}

void OrderDialog::updateDocumentSumText()
{
    ui->labelSumOrder->setText(QString("%1 MDL")
                                   .arg(documentSum(), 0, 'f', 2));
}

void OrderDialog::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/catalogs/user.png").pixmap(18,18);
    auto labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    auto labelAuthor = new QLabel(this);
    labelAuthor->setText(globals().nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    ui->btnPrint->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_P));
    ui->btnOk->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_Return));
    ui->btnWrite->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_S));
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
}

void OrderDialog::saveLayoutSizes()
{
    const QString className = metaObject()->className();
    m_settings.setValue(className, "window/width", width());
    m_settings.setValue(className, "window/height", height());

    // table source
    const int ctSectionsSource = ui->tableView->horizontalHeader()->count();
    for (int section = 0; section < ctSectionsSource; ++section) {
        const int w = ui->tableView->horizontalHeader()->sectionSize(section);
        m_settings.setValue(className, QString("tableSource/%1").arg(section), w);
    }

    // table order
    const int ctSectOrder = ui->tableViewOrder->horizontalHeader()->count();
    for (int section = 0; section < ctSectOrder; ++section) {
        const int w = ui->tableViewOrder->horizontalHeader()->sectionSize(section);
        m_settings.setValue(className, QString("tableOrder/%1").arg(section), w);
    }
    m_settings.save();
}

void OrderDialog::loadLayoutSizes()
{
    const QJsonObject objRoot = m_settings.getJsonObject("OrderDialog");
    const QJsonObject winObj = objRoot.value("window").toObject();
    m_layoutSizes.window = QSize(winObj.value("width").toInt(),
                                 winObj.value("height").toInt());

    const QJsonObject sourceObj = objRoot.value("tableSource").toObject();
    for (auto it = sourceObj.begin(); it != sourceObj.end(); ++it) {
        const int index = it.key().toInt();
        m_layoutSizes.sourceTable[index] = it.value().toInt();
    }

    const QJsonObject orderObj = objRoot.value("tableOrder").toObject();
    for (auto it = orderObj.begin(); it != orderObj.end(); ++it) {
        const int index = it.key().toInt();
        m_layoutSizes.orderTable[index] = it.value().toInt();
    }
}

DatabaseProvider *OrderDialog::dbProvider()
{
    return &m_provider;
}

void OrderDialog::closeEvent(QCloseEvent *event)
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
            const bool ok = (documentStatus() == DocStatus::Post) ? onPost() : onSave();
            if (ok) {
                saveLayoutSizes();
                event->accept();
            } else {
                event->ignore();
            }
            return;
        }

        if (messange_box.clickedButton() == noButton) {
            saveLayoutSizes();
            event->accept();
            return;
        }

        event->ignore();
        return;
    }

    saveLayoutSizes();
    event->accept();
}

void OrderDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void OrderDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {

        if (ui->tableViewOrder->hasFocus() || ui->tableViewOrder->viewport()->hasFocus()) {
            const int row = ui->tableViewOrder->currentIndex().row();
            if (row >= 0) {
                const QModelIndex idx = modelTableOrder->index(row, OrderTableSections::Price);
                onDoubleClickedTableOrder(idx);
            }
            event->accept();
            return;
        }

        if (ui->comboOrganization->hasFocus()) {
            ui->comboOrganization->showPopup();
            event->accept();
            return;
        }

        if (ui->comboContract->hasFocus() && ui->comboReferringDoctor->currentIndex() == 0) {
            ui->comboReferringDoctor->setFocus();
            ui->comboReferringDoctor->showPopup();
            event->accept();
            return;
        }

        if (ui->comboReferringDoctor->hasFocus()) {
            ui->comboReferringDoctor->showPopup();
            event->accept();
            return;
        }

        focusNextChild();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_F5) {
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        ui->comboPatient->setFocus();
        event->accept();
        return;
    }

    QDialog::keyPressEvent(event);
}
