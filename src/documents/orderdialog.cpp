#include "orderdialog.h"
#include "ui_orderdialog.h"

OrderDialog::OrderDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , m_post(DocStatus::Unknow)
    , ui(new Ui::OrderDialog) // initial
    , m_settings(globals().pathSettingsCommon)
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
    if (globals().showDocumentsInSeparatWindow)
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
}

OrderDialog::~OrderDialog()
{
    delete ui;
}

void OrderDialog::onPrintDocument(PrintType::Column type_print, const QString &filePDF)
{
    onPrint(type_print, filePDF);
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

    setIsNew(true);

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
            "SELECT cod, name FROM investigations WHERE id=:id"));
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
            destination[QStringLiteral("id")] = m_tempOrderRowId--;
            destination[QStringLiteral("deletionMark")] = m_post;
            destination[QStringLiteral("id_orderEcho")] = m_id;
            destination[QStringLiteral("cod")] = source.value(QStringLiteral("cod"));
            destination[QStringLiteral("name")] = source.value(QStringLiteral("name"));
            destination[QStringLiteral("price")] = source.value(QStringLiteral("price"));
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

void OrderDialog::slot_IsNewChanged()
{
    if (m_isNew) {
        slot_PostChanged(); // fortam setarea titlului

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
        setIdPerformingDoctor(globals().c_id_doctor);
        setIdNurse(globals().c_id_nurse);
        setIdUser(globals().idUserApp);

        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_organization);
        changeIconForItemToolBox(OrderToolBoxIdx::Box_organization);

        m_attachedImages = StatusObject::ZeroWrite;

        ui->comboOrganization->setFocus();

    } else {
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        changeIconForItemToolBox(OrderToolBoxIdx::Box_patient);
    }
}

void OrderDialog::slot_IdChanged()
{
    if (m_id <= 0)
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
            id, deletionMark, numberDoc, dateDoc,
            id_organizations, id_contracts, id_typesPrices,
            id_doctors, id_doctors_execute, id_nurses,
            patient_id, id_users, sum, comment,
            cardPayment, attachedImages, uuid
        FROM orderEcho
        WHERE id = :id
    )");
    qry.bindValue(":id", m_id);
    if (qry.exec() && qry.next()){
        qInfo(logInfo()) << "OrderDialog: vizualizarea comenzii, id=" << m_id;

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
        // deci schimbarea organizației nu poate reîncărca automat contractele.
        // Modelul inițial conține contractele tuturor organizațiilor și poate
        // să nu conțină un contract CNAM istoric/inactiv.
        updateModelContracts();
        setIdContract(qry.value(OrderSections::Id_Contracts).toInt());
        setIdTypePrice(qry.value(OrderSections::Id_TypesPrices).toInt());
        setIdRefferingDoctor(qry.value(OrderSections::Id_Doctors).toInt());
        setIdPerformingDoctor(qry.value(OrderSections::Id_Doctors_exec).toInt());
        setIdNurse(qry.value(OrderSections::Id_Nurses).toInt());
        setIdPatient(qry.value(OrderSections::Id_Patients).toInt());
        setIdUser(qry.value(OrderSections::Id_Users).toInt());

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
    changeIconForItemToolBox(OrderToolBoxIdx::Box_patient);
}

void OrderDialog::slot_IdOrganizationChanged()
{
    if (m_idOrganization < 0)
        return;

    ui->comboOrganization->setCurrentIndex(modelOrganizations->rowById("id", m_idOrganization));
}

void OrderDialog::slot_IdContractChanged()
{
    if (m_idContract < 0)
        return;

    ui->comboContract->setCurrentIndex(modelContracts->rowById("id", m_idContract));
}

void OrderDialog::slot_IdTypePriceChanged()
{
    if (m_idTypePrice < 0)
        return;

    ui->comboTypePrices->setCurrentIndex(modelTypePrices->rowById("id", m_idTypePrice));

    if (m_idOrganization < 0 ||
        m_idContract < 0)
        return;

    updateTableSource();
}

void OrderDialog::slot_IdPatientChanged()
{
    if (m_idPatient <= 0) {
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

void OrderDialog::slot_IdNurseChanged()
{
    if (m_idNurse < 0)
        return;

    ui->comboNurse->setCurrentIndex(modelNurses->rowById("id", m_idNurse));
}

void OrderDialog::slot_IdPerformingDoctorChanged()
{
    if (m_idPerformingDoctor < 0)
        return;

    ui->comboPerformingDoctor->setCurrentIndex(modelPerformingDoctors->rowById("id", m_idPerformingDoctor));
}

void OrderDialog::slot_IdRefferingDoctorChanged()
{
    if (m_idRefferingDoctor < 0)
        return;

    ui->comboReferringDoctor->setCurrentIndex(modelRefferingDoctors->rowById("id", m_idRefferingDoctor));
}

void OrderDialog::slot_IdUserChanged()
{
    if (m_idUser < 0)
        return;
}

void OrderDialog::slot_PostChanged()
{
    if (m_post == DocStatus::Unknow)
        setWindowTitle(tr("Comanda ecografica (crearea) %1").arg("[*]"));
    else if (m_post == DocStatus::Write)
        setWindowTitle(tr("Comanda ecografica (salvata) %1").arg("[*]"));
    else if (m_post == DocStatus::DeletionMark)
        setWindowTitle(tr("Comanda ecografica (marcata pentru eliminare) %1").arg("[*]"));
    else if (m_post == DocStatus::Post)
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
                setIdRefferingDoctor(id_doctor);
            });
    catalog->show();
}

void OrderDialog::onOpenCatalogDoctor()
{
    if (m_idRefferingDoctor <= 0)
        return;
    CatalogDialog *catalog = new CatalogDialog(m_db, CatalogType::Type::Doctors, this);
    catalog->setAttribute(Qt::WA_DeleteOnClose);
    catalog->setProperty("isNew", false);
    catalog->setProperty("id", m_idRefferingDoctor);
    catalog->setWindowModality(Qt::ApplicationModal);
    connect(catalog, &CatalogDialog::catalogDialogChanged,
            this, &OrderDialog::updateModelRefferingDoctors, Qt::UniqueConnection);
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
        if (id_doctor > 0)
            setIdRefferingDoctor(id_doctor);

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
    patientData.id            = m_idPatient <= 0 ? 0 : m_idPatient;
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
    if (ui->newPatient->isChecked() && m_idPatient <= 0){
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

                setPatientDataEnabled(false);

                /** initierea syncronizarii */
                if (globals().cloud_srv_exist)
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
    PatientHistory *patient_history = new PatientHistory(m_db, this);
    patient_history->setAttribute(Qt::WA_DeleteOnClose);
    patient_history->setProperty("IdPatient", m_idPatient);
    this->hide();
    patient_history->exec();
    this->show();
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
    QString sql = globals().thisMySQL
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
    rowData["deletionMark"] = m_post;
    rowData["id_orderEcho"] = m_id;
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

void OrderDialog::setImageForDocPrint()
{
    exist_logo = 0;
    exist_stamp = 0;
    exist_stamp_doctor = 0;
    exist_signature = 0;

    //------------------------------------------------------------------------------------------------------
    // ----- 1. logotipul
    QPixmap pix_logo = QPixmap();
    QStandardItem* img_item_logo = new QStandardItem();
    QString name_key_logo = "logo_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_logo, &pix_logo)){
        if (! globals().c_logo_byteArray.isEmpty() && pix_logo.loadFromData(globals().c_logo_byteArray)){
            globals().cache_img.insert(name_key_logo, pix_logo);
        }
    }

    // --- setam logotipul
    if (! pix_logo.isNull()) {
        img_item_logo->setData(pix_logo.scaled(300,50, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_logo = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 2. stampila organizatiei
    QPixmap pix_stamp_organization = QPixmap();
    QStandardItem* img_item_stamp_organization = new QStandardItem();
    QString name_key_stamp_organization = "stamp_organization_id-" + QString::number(globals().c_id_organizations) + "_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_stamp_organization, &pix_stamp_organization)) {
        if (! globals().main_stamp_organization.isEmpty() && pix_stamp_organization.loadFromData(globals().main_stamp_organization)){
            globals().cache_img.insert(name_key_stamp_organization, pix_stamp_organization);
        }
    }

    // --- setam stampila
    if (! pix_stamp_organization.isNull()) {
        img_item_stamp_organization->setData(pix_stamp_organization.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stamp = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 3. stampila doctorului
    QPixmap pix_stamp_doctor = QPixmap();
    QStandardItem* img_item_stamp_doctor = new QStandardItem();
    QString name_key_stamp_doctor = "stamp_doctor_id-" + QString::number(globals().c_id_doctor) + "_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_stamp_doctor, &pix_stamp_doctor)) {
        if (! globals().stamp_main_doctor.isEmpty() && pix_stamp_doctor.loadFromData(globals().stamp_main_doctor)){
            globals().cache_img.insert(name_key_stamp_doctor, pix_stamp_doctor);
        }
    }

    // --- setam stampila
    if (! pix_stamp_doctor.isNull()) {
        img_item_stamp_doctor->setData(pix_stamp_doctor.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stamp_doctor = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 4. semnatura doctorului
    QPixmap pix_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    QString name_key_signature = "signature_doctor_id-" + QString::number(globals().c_id_doctor) + "_" + globals().nameUserApp;

    // --- verificam cache
    if (! globals().cache_img.find(name_key_signature, &pix_signature)) {
        if(! globals().signature_main_doctor.isEmpty() && pix_signature.loadFromData(globals().signature_main_doctor)) {
            globals().cache_img.insert(name_key_signature, pix_signature);
        }
    }

    // --- setam semnatura
    if (! pix_signature.isNull()) {
        img_item_signature->setData(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_signature = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp_organization);
    items_img.append(img_item_stamp_doctor);
    items_img.append(img_item_signature);

    if (model_img) {
        delete model_img;
        model_img = nullptr;
    }

    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(4);
    model_img->appendRow(items_img);
}

void OrderDialog::onPrint(PrintType::Column type_print, const QString &filePDF)
{
    if (m_isNew){
        QMessageBox::warning(this, tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nPrintare nu este posibila."),
                             QMessageBox::Ok);
        return;
    }

    if (type_print == PrintType::ExportToPDF && filePDF.trimmed().isEmpty())
        return;

    // *************************************************************************************
    // alocam memoria
    LimeReport::ReportEngine *m_report = new LimeReport::ReportEngine(this);
    QSqlQueryModel *print_model_organization = new QSqlQueryModel(this);
    QSqlQueryModel *print_model_patient      = new QSqlQueryModel(this);
    QSqlQueryModel *print_model_table        = new QSqlQueryModel(this);

    // *************************************************************************************
    // verificam daca este complectata variabila 'noncomercial_price'
    auto roleNoncomecial = modelTypePrices->roleForColumn("noncomercial");
    bool noncomercial = ui->comboTypePrices ->currentData(roleNoncomecial).toBool();

    // *************************************************************************************
    // logotipul, semnaturile
    setImageForDocPrint();
    m_report->dataManager()->addModel("table_img", model_img, true);

    // *************************************************************************************
    // setam solicitarile in model
    m_db.setModelQuery(*print_model_organization,
                       m_db.getDatabase(),
                       m_db.getTextSQL(":/sql/queries_print/tableConstants.sql"),
                       {m_idUser});

    m_db.setModelQuery(*print_model_patient,
                       m_db.getDatabase(),
                       m_db.getTextSQL(":/sql/queries_print/tablePatientByID.sql"),
                       {m_idPatient});

    QVariantMap map;                    // adaugam variabila pu
    map["noncomercial"] = noncomercial; // conditia: noncomercial = '0-00' else price
    m_db.setModelQuery(*print_model_table,
                       m_db.getDatabase(),
                       m_db.getTextSQL(":/sql/queries_print/orderTable.sql"),
                       {m_id},
                       map);

    // *************************************************************************************
    // transmitem variabile si modelurile generatorului de rapoarte
    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("sume_total", QString("%1").arg(noncomercial == 0
                                                                                   ? documentSum()
                                                                                   : 0, 0, 'f', 2));
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("v_exist_stamp", exist_stamp);
    m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
    m_report->dataManager()->setReportVariable("v_exist_signature", exist_signature);

    m_report->dataManager()->addModel("main_organization", print_model_organization, false);
    m_report->dataManager()->addModel("table_pacient", print_model_patient, false);
    m_report->dataManager()->addModel("table_table", print_model_table, false);
    m_report->setShowProgressDialog(true);

    m_report->setPreviewWindowTitle(tr("Comanda ecografică nr.") +
                                    ui->numberDoc->text() + tr(" din ") +
                                    ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss") +
                                    tr(" (printare)"));

    // *************************************************************************************
    // verificam drumul spre forme de tipar
    QDir dir;
    if (! QFile(dir.toNativeSeparators(globals().pathTemplatesDocs + "/Order.lrxml")).exists()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Printarea documentului"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Documentul nu poate fi printat."));
        msgBox.setDetailedText(tr("Nu a fost gasit fisierul sablon formei de tipar:\n%1")
                                   .arg(dir.toNativeSeparators(globals().pathTemplatesDocs + "/Order.lrxml")));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet(m_db.getStyleForButtonMessageBox());
        msgBox.exec();

        delete print_model_organization;
        delete print_model_patient;
        delete print_model_table;
        delete m_report;

        return;
    }
    m_report->loadFromFile(dir.toNativeSeparators(globals().pathTemplatesDocs + "/Order.lrxml"));

    // *************************************************************************************
    // prezentam forma de tipar
    if (type_print == PrintType::Designer){
        qInfo(logInfo()) << QStringLiteral("Printare (designer) - document 'Comanda ecografica' nr.%1")
        .arg(ui->numberDoc->text());
        m_report->designReport();
    } else if (type_print == PrintType::Preview){
        qInfo(logInfo()) << QStringLiteral("Printare (preview) - document 'Comanda ecografica' nr.%1")
        .arg(ui->numberDoc->text());
        m_report->previewReport();
    } else if (type_print == PrintType::ExportToPDF){
        qInfo(logInfo()) << QStringLiteral("Printare (export pdf) - document 'Comanda ecografica' nr.%1")
        .arg(ui->numberDoc->text());
        m_report->printToPDF(filePDF);
        emit printToPdfFinished();
    }

    // *************************************************************************************
    // elibiram memoria
    print_model_organization->deleteLater();
    print_model_patient->deleteLater();
    print_model_table->deleteLater();
    m_report->deleteLater();
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
    data.append(m_post);
    data.append(ui->numberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(m_idOrganization);
    data.append(m_idContract);
    data.append(m_idTypePrice);
    data.append((m_idRefferingDoctor <= 0)
                    ? QVariant()
                    : m_idRefferingDoctor);
    data.append((m_idPerformingDoctor <= 0)
                    ? QVariant()
                    : m_idPerformingDoctor);
    data.append((m_idNurse <= 0)
                    ? QVariant()
                    : m_idNurse);
    data.append(m_idPatient);
    data.append(m_idUser);
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
        m_id = newId.toInt();
        return true;
    }
}

bool OrderDialog::updateDataOrder(QString &details_error)
{
    QVector<QVariant> data;
    data.append(m_post);
    data.append(ui->numberDoc->text());
    data.append(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    data.append(m_idOrganization);
    data.append(m_idContract);
    data.append(m_idTypePrice);
    data.append((m_idRefferingDoctor <= 0)
                    ? QVariant()
                    : m_idRefferingDoctor);
    data.append((m_idPerformingDoctor <= 0)
                    ? QVariant()
                    : m_idPerformingDoctor);
    data.append((m_idNurse <= 0)
                    ? QVariant()
                    : m_idNurse);
    data.append(m_idPatient);
    data.append(m_idUser);
    data.append(documentSum());
    data.append((ui->editComment->toPlainText().isEmpty())
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append(valuePaymentOrder());
    data.append(m_attachedImages);
    data.append(m_id);

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
    qry.bindValue(":id_orderEcho", m_id);

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
        rowData["deletionMark"] = m_post;
        rowData["id_orderEcho"] = m_id;

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
    if (ui->comboPatient->currentText().isEmpty() || m_idPatient <= 0){
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
    patientQuery.addBindValue(m_idPatient);
    if (!patientQuery.exec() || !patientQuery.next()) {
        ui->toolBox->setCurrentIndex(OrderToolBoxIdx::Box_patient);
        BalloonTip::showBalloonFor(ui->comboPatient,
                                   QMessageBox::Warning,
                                   tr("Verificarea datelor"),
                                   tr("Pacientul selectat (ID %1) nu există în baza de date. Selectați din nou pacientul din listă.")
                                       .arg(m_idPatient),
                                   6000,
                                   true,
                                   BalloonTip::BottomCenter);
        qWarning(logWarning()).noquote()
            << QStringLiteral("OrderDialog: patient_id=%1 nu există în patients; SQL: %2")
                   .arg(m_idPatient)
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
    if (m_isNew){
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
    q.bindValue(":id_orderEcho", m_id);

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
        params.idOrder   = m_id;
        params.idPatient = m_idPatient;
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
    params.idPatient = m_idPatient;
    params.idOrder   = m_id;
    params.status    = DocStatus::determineStatusDoc(m_post);
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

    /** in slot_IsNewChanged daca isNew am pus focus pe comboOrganization
     *  ca urmare la validarea/salvarea documentului e necesar de oprit
     *  timer pu dateTimeDoc
     ********************************************************************/
    if (timer->isActive())
        timer->stop();

    const int initialId = m_id;
    const int initialPost = m_post;
    const int initialAttachedImages = m_attachedImages;
    const QString initialNumberDoc = ui->numberDoc->text();
    QList<QVariantMap> initialOrderRows;
    initialOrderRows.reserve(modelTableOrder->rowCount());
    for (int row = 0; row < modelTableOrder->rowCount(); ++row)
        initialOrderRows.append(modelTableOrder->rowDataByRow(row));

    if (m_post == DocStatus::Unknow)
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

        m_id = initialId;
        m_attachedImages = initialAttachedImages;
        setPost(initialPost);
        ui->numberDoc->setText(initialNumberDoc);
        modelTableOrder->setRows(initialOrderRows);
    };

    if (m_isNew) {

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
            modelTableOrder->setFieldValueInternal(i, "deletionMark", m_post);
            modelTableOrder->setFieldValueInternal(i, "id_orderEcho", m_id);

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

    if (m_isNew) {
        setIsNew(false);
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

    if (globals().cloud_srv_exist)
        initSyncOrderData();

    return true;
}

bool OrderDialog::onPost()
{
    const int oldPost = m_post;
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

    if (m_idOrganization <= 0) {
        QString str = m_db.getTextSQL(":/sql/queries/contracts_view.sql");
        newModel = new QueryRolesModel(str, ui->comboContract);
        newModel->setEmptyRowEnabled(true);
    } else {
        QSqlQuery qry(m_currentDB);
        qry.prepare(m_db.getTextSQL(
            globals().thisSqlite
                ? ":/sql/queries/contracts_select_by_organization_sqlite.sql"
                : ":/sql/queries/contracts_select_by_organization_mysql.sql"));

        qry.addBindValue(m_idOrganization);
        qry.addBindValue(m_idContract);

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
        modelContracts->columnIndex(m_idOrganization <= 0 ? "contract_owner" : "name"));
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
    if (modelRefferingDoctors)
        delete modelRefferingDoctors;

    QString str = m_db.getTextSQL(":/sql/queries/doctors_combo_view.sql");
    modelRefferingDoctors = new QueryRolesModel(str, ui->comboReferringDoctor);
    modelRefferingDoctors->setEmptyRowEnabled(true);
    ui->comboReferringDoctor->setModel(modelRefferingDoctors);
    ui->comboReferringDoctor->setModelColumn(modelRefferingDoctors->columnIndex("display"));
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
    if (m_idPatient <= 0)
        return;

    QSqlQuery qry(m_currentDB);
    qry.prepare(globals().thisSqlite
                    ? m_db.getTextSQL(":/sql/queries_doc/patients_byID_sqlite.sql")
                    : m_db.getTextSQL(":/sql/queries_doc/patients_byID_mariadb.sql"));
    qry.addBindValue(m_idPatient);

    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
        << "SQL error:" << qry.lastError().text();
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    if (!qry.next()) {
        qCritical(logCritical()).noquote()
            << "Pacientul cu id =" << m_idPatient << " nu a fost găsit";
        qCritical(logCritical()).noquote()
            << "SQL query:" << qry.lastQuery();
        return;
    }

    const QString fullName = qry.value(PatientSearchColumns::FullName).toString().trimmed();

    // ne asiguram ca pacientul exista in modelul completerului
    ensurePatientInCompleterModel(m_idPatient, fullName);

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
    patientData.id = m_idPatient;

    OrderDataStructure orderData;
    orderData.id = m_id;

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
    q.bindValue(":id_organization", m_idOrganization);
    q.bindValue(":id_contract",     m_idContract);
    q.bindValue(":id_typePrices",   m_idTypePrice);
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
    q.bindValue(":id_orderEcho", m_id);
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
            const bool ok = (m_post == DocStatus::Post) ? onPost() : onSave();
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
