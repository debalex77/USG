#include "docorderecho.h"
#include <data/mainwindow.h>
#include "ui_docorderecho.h"
#include "docs/docreportecho.h"
#include <QDesktopServices>

static const int max_length_comment = 255;

DocOrderEcho::DocOrderEcho(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocOrderEcho)
{
    ui->setupUi(this);

    setTitleDoc(); // setam titlu documentului

    if (globals::showDocumentsInSeparatWindow)
        setWindowFlags(Qt::Window);

    db            = new DataBase(this);
    popUp         = new PopUp(this);
    timer         = new QTimer(this); // alocam memoria
    menu          = new QMenu(this);
    setUpMenu     = new QMenu(this);
    completer     = new QCompleter(this);
    modelPacients = new QStandardItemModel(completer);

    initCompleterAddressPatients();

    ui->dateTimeDoc->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
    ui->dateTimeDoc->setCalendarPopup(true);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::onDateTimeChanged);

    ui->editIDNP->setMaxLength(20);           // limitarea caracterilor
    ui->editPolicyMedical->setMaxLength(20);
    ui->editAddress->setMaxLength(255);
    ui->editPhone->setMaxLength(100);
    ui->editFilterPattern->setPlaceholderText(tr("...căutare după denumirea investigației sau după cuvânt cheie/model"));

    QString strQryOrganizations = "SELECT id,name FROM organizations WHERE deletionMark = 0;";
    QString strQryContracts     = "SELECT id,name FROM contracts WHERE deletionMark = 0;";
    QString strQryTypesPrices   = "SELECT id,name FROM typesPrices WHERE deletionMark = 0;";

    QString strQryDoctors;
    strQryDoctors = "SELECT doctors.id, fullNameDoctors.name FROM doctors "
                    "INNER JOIN fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors "
                    "WHERE deletionMark = 0 ORDER BY fullNameDoctors.name;";

    QString strQryNurses;
    strQryNurses = "SELECT nurses.id, fullNameNurses.name FROM nurses "
                   "INNER JOIN fullNameNurses ON nurses.id = fullNameNurses.id_nurses "
                   "WHERE deletionMark = 0 ORDER BY fullNameNurses.name;";

    modelOrganizations  = new BaseSqlQueryModel(strQryOrganizations, ui->comboOrganization);  // alocarea memoriei
    modelContracts      = new BaseSqlQueryModel(strQryContracts, ui->comboContract);
    modelTypesPrices    = new BaseSqlQueryModel(strQryTypesPrices, ui->comboTypesPricing);
    modelNurses         = new BaseSqlQueryModel(strQryNurses, ui->comboNurse);
    modelDoctors        = new BaseSqlQueryModel(strQryDoctors, ui->comboDoctor);
    modelDoctorsExecute = new BaseSqlQueryModel(strQryDoctors, ui->comboDoctorExecute);
    modelTableSource    = new BaseSqlTableModel(this);
    modelTableOrder     = new BaseSqlTableModel(this);
    proxy               = new BaseSortFilterProxyModel(this);  // proxy - model pu tabelul source
    proxyPacient        = new BaseSortFilterProxyModel(this);

    modelTableSource->setTableSource("tableSource");  // pu determinarea tabelei -> nu se redacteaza -> Qt::ItemIsSelectable | Qt::ItemIsEnabled
    modelTableOrder->setTableSource("tableOrder");    // pu determinarea tabelei -> se redacteaza -> Qt::ItemIsEditable

    modelOrganizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelContracts->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelTypesPrices->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);

    modelNurses->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelDoctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    modelDoctorsExecute->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);

    ui->comboOrganization->setModel(modelOrganizations);
    ui->comboContract->setModel(modelContracts);
    ui->comboTypesPricing->setModel(modelTypesPrices);
    ui->comboNurse->setModel(modelNurses);
    ui->comboDoctor->setModel(modelDoctors);
    ui->comboDoctorExecute->setModel(modelDoctorsExecute);

    ui->comboOrganization->setStyleSheet("combobox-popup: 0;");
    ui->comboContract->setStyleSheet("combobox-popup: 0;");
    ui->comboTypesPricing->setStyleSheet("combobox-popup: 0;");
    ui->comboNurse->setStyleSheet("combobox-popup: 0;");
    ui->comboDoctor->setStyleSheet("combobox-popup: 0;");
    ui->comboDoctorExecute->setStyleSheet("combobox-popup: 0;");
    ui->comboOrganization->setStyleSheet("combobox-popup: 0;");

    if (modelOrganizations->rowCount() > 20) {
        ui->comboOrganization->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganization->setMaxVisibleItems(15);
    }

    if (modelContracts->rowCount() > 20) {
        ui->comboContract->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboContract->setMaxVisibleItems(15);
    }

    if (modelNurses->rowCount() > 20){
        ui->comboNurse->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); 
        ui->comboNurse->setMaxVisibleItems(15);
    }

    if (modelDoctors->rowCount() > 20){
        ui->comboDoctor->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboDoctor->setMaxVisibleItems(15);
    }

    if (modelDoctorsExecute->rowCount() > 20){
        ui->comboDoctorExecute->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboDoctorExecute->setMaxVisibleItems(15);
    }

    if (modelOrganizations->rowCount() > 20){
        ui->comboOrganization->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganization->setMaxVisibleItems(15);
    }

    initSetCompleter();                  // initiam completerul
    connectionsToIndexChangedCombobox(); // conectarea la modificarea indexului combobox-urilor

    updateTableSources();  // actualizarea tabelelor
    updateTableOrder();
    updateTextSumOrder();

    initFooterDoc();   // setam autorul + imaginea + Ok, Writer, Revocare
    initConnections(); // initierea conectarilor btn
}

DocOrderEcho::~DocOrderEcho()
{
    delete modelOrganizations;
    delete modelContracts;
    delete modelTypesPrices;
    delete modelPacients;
    delete modelNurses;
    delete modelDoctors;
    delete modelDoctorsExecute;
    delete completer;
    delete city_completer;
    delete modelTableSource;
    delete modelTableOrder;
    delete proxy;
    delete proxyPacient;
    delete timer;
    delete popUp;
    delete menu;
    delete setUpMenu;
    delete db;
    delete ui;
}

void DocOrderEcho::onPrintDocument(Enums::TYPE_PRINT type_print)
{
    onPrint(type_print); // pu printarea din forma de lista 'ListDocWebOrder'
}

void DocOrderEcho::m_OnOpenReport()
{
    onOpenReport(); // pu deschiderea din forma de lista 'ListDocWebOrder'
}

void DocOrderEcho::m_onWritingData()
{
    onWritingData(); // pu validarea din forma de lista 'ListDocWebOrder'
}

void DocOrderEcho::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void DocOrderEcho::dataWasModified()
{
    setWindowModified(true);
}

void DocOrderEcho::updateTimer()
{
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::dataWasModified);
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::onDateTimeChanged);
    disconnect(timer, &QTimer::timeout, this, &DocOrderEcho::updateTimer);
    ui->dateTimeDoc->setDateTime(QDateTime::currentDateTime());
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::onDateTimeChanged);
    connect(timer, &QTimer::timeout, this, &DocOrderEcho::updateTimer);
}

void DocOrderEcho::onDateTimeChanged()
{
    disconnect(timer, &QTimer::timeout, this, &DocOrderEcho::updateTimer);
}

void DocOrderEcho::changeIconForItemToolBox(const int _index)
{
    if (globals::isSystemThemeDark) {
        ui->toolBox->setItemIcon(0, (_index == 0) ? QIcon(":/img/down-arrow_tool_box_blue.png") : QIcon("://img/right-arrow_tool_box_blue.png"));
        ui->toolBox->setItemIcon(1, (_index == 1) ? QIcon(":/img/down-arrow_tool_box_blue.png") : QIcon("://img/right-arrow_tool_box_blue.png"));
        ui->toolBox->setItemIcon(2, (_index == 2) ? QIcon(":/img/down-arrow_tool_box_blue.png") : QIcon("://img/right-arrow_tool_box_blue.png"));
    } else {
        ui->toolBox->setItemIcon(0, (_index == 0) ? QIcon(":/img/down-arrow_tool_box.png") : QIcon("://img/right-arrow_tool_box.png"));
        ui->toolBox->setItemIcon(1, (_index == 1) ? QIcon(":/img/down-arrow_tool_box.png") : QIcon("://img/right-arrow_tool_box.png"));
        ui->toolBox->setItemIcon(2, (_index == 2) ? QIcon(":/img/down-arrow_tool_box.png") : QIcon("://img/right-arrow_tool_box.png"));
    }
}

void DocOrderEcho::createCatDoctor()
{
    catDoctors = new CatGeneral(this);
    catDoctors->setAttribute(Qt::WA_DeleteOnClose);
    catDoctors->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    catDoctors->setProperty("itNew", true);
    catDoctors->show();
    connect(catDoctors, &CatGeneral::createCatGeneral, this, [this]()
    {
        delete modelDoctors;
        QString strQryDoctors;
        strQryDoctors = "SELECT doctors.id, fullNameDoctors.name FROM doctors "
                        "INNER JOIN fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors "
                        "WHERE deletionMark = 0 ORDER BY fullNameDoctors.name;";
        modelDoctors = new BaseSqlQueryModel(strQryDoctors, ui->comboDoctor);
        modelDoctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
        ui->comboDoctor->setModel(modelDoctors);

        int id_doctor = catDoctors->getId();
        setIdDoctor(id_doctor);
        popUp->setPopupText(tr("Obiectul '%1' a fost salvat <br>in baza de date cu succes.").arg(catDoctors->getFullName()));
        popUp->show();
    });
}

void DocOrderEcho::openCatDoctor()
{
    int id_doctor = ui->comboDoctor->itemData(ui->comboDoctor->currentIndex(), Qt::UserRole).toInt();
    if (id_doctor == 0)
        return;

    qInfo(logInfo()) << tr("Editarea datelor doctorului '%1' cu id='%2'.")
                        .arg(ui->comboDoctor->currentText(), QString::number(m_idDoctor));

    CatGeneral* catDoctors = new CatGeneral(this);
    catDoctors->setAttribute(Qt::WA_DeleteOnClose);
    catDoctors->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    catDoctors->setProperty("itNew", false);
    catDoctors->setProperty("Id", id_doctor);
    catDoctors->show();
}

// **********************************************************************************
// --- procesarea slot-urilor

void DocOrderEcho::slot_ItNewChanged()
{
    if (m_itNew){

        setWindowTitle(tr("Comanda ecografica (crearea) %1").arg("[*]"));

        if (m_idUser == Enums::Enums::IDX_UNKNOW)
            setIdUser(globals::idUserApp);

        connect(timer, &QTimer::timeout, this, &DocOrderEcho::updateTimer); // actualizam ora reala cu secunde
        timer->start(1000);
        ui->dateTimeDoc->setDateTime(QDateTime::currentDateTime());

        int lastNumberDoc = db->getLastNumberDoc("orderEcho");                        // determinam ultimul numar a documentului
        ui->editNumberDoc->setText(QString::number(lastNumberDoc + 1));               // setam numarul documentului
        ui->dateEditBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd")); // setam data curenta
        enableDisableDataPacient(false);
        ui->toolBox->setCurrentIndex(box_organization); // setam index=0 - 'datele organizatiei'

        m_attachedImages = 0;
        changeIconForItemToolBox(0);

        QSqlQuery qry;
        qry.prepare("SELECT id_doctors, id_nurses FROM constants WHERE id_users = :id_users;");
        qry.bindValue(":id_users", globals::idUserApp);
        if (qry.exec() && qry.next()){
            setIdDoctorExecute(qry.value(0).toInt());
            setIdNurse(qry.value(1).toInt());
        }
    } else {
        ui->toolBox->setCurrentIndex(box_patient); // setam index=1 - 'datele pacientului'
    }
}

void DocOrderEcho::slot_IdChanged()
{
    if (m_id == Enums::Enums::IDX_UNKNOW)
        return;
    disconnectionsToIndexChangedCombobox(); // deconectarea de la modificarea indexului combobox-urilor
                                            // ca sa nu fie activata modificarea formei - dataWasModified()

    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::dataWasModified);
    disconnect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::onDateTimeChanged);

    QSqlQuery qry;
    qry.prepare("SELECT * FROM orderEcho WHERE id = :id AND deletionMark = 2;");
    qry.bindValue(":id", m_id);
    if (qry.exec() && qry.next()){
        QSqlRecord rec = qry.record();
        ui->editNumberDoc->setText(qry.value(rec.indexOf("numberDoc")).toString());
        ui->editNumberDoc->setDisabled(ui->editNumberDoc->text().isEmpty());

        if (globals::thisMySQL){
            static const QRegularExpression replaceT("T");
            static const QRegularExpression removeMilliseconds("\\.000");
            ui->dateTimeDoc->setDateTime(
                QDateTime::fromString(
                    qry.value(rec.indexOf("dateDoc"))
                        .toString()
                        .replace(replaceT, " ")
                        .replace(removeMilliseconds, ""),
                    "yyyy-MM-dd hh:mm:ss"
                    )
                );
        } else {
            ui->dateTimeDoc->setDateTime(QDateTime::fromString(qry.value(rec.indexOf("dateDoc")).toString(), "yyyy-MM-dd hh:mm:ss"));
        }
        m_post = qry.value(rec.indexOf("deletionMark")).toInt();
        setIdOrganization(qry.value(rec.indexOf("id_organizations")).toInt());
        setIdContract(qry.value(rec.indexOf("id_contracts")).toInt());
        setIdTypePrice(qry.value(rec.indexOf("id_typesPrices")).toInt());
        setIdDoctor(qry.value(rec.indexOf("id_doctors")).toInt());
        setIdDoctorExecute(qry.value(rec.indexOf("id_doctors_execute")).toInt());
        setIdNurse(qry.value(rec.indexOf("id_nurses")).toInt());
        setIdPacient(qry.value(rec.indexOf("id_pacients")).toInt());
        setIdUser(qry.value(rec.indexOf("id_users")).toInt());

        updateTableOrder();   // setam datele tabelei documentului
        updateTextSumOrder(); // actualizam textul total suma documentului

        ui->editComment->setPlainText(qry.value(rec.indexOf("comment")).toString());

        // setam achitarea cu card
        disconnect(ui->cardPayment, &QCheckBox::stateChanged, this, &DocOrderEcho::dataWasModified);
        if (qry.value(rec.indexOf("cardPayment")).toInt() == Enums::PAYMENT_CARD)
            ui->cardPayment->setCheckState(Qt::Checked);
        else
            ui->cardPayment->setCheckState(Qt::Unchecked);
        connect(ui->cardPayment, &QCheckBox::stateChanged, this, &DocOrderEcho::dataWasModified);

        // determinam daca sunt atasate imaginile
        if (qry.value(rec.indexOf("attachedImages")).toInt() == 0)
            m_attachedImages = 0;
        else
            m_attachedImages = 1;

        setWindowTitle(tr("Comanda ecografica (validat) %1 %2")
                           .arg(" nr." + ui->editNumberDoc->text() + " din " + ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss"),"[*]"));
    }
    connectionsToIndexChangedCombobox(); // activarea conectarii la modificarea indexului combobox-urilor
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::dataWasModified);
    connect(ui->dateTimeDoc, &QDateTimeEdit::dateTimeChanged, this, &DocOrderEcho::onDateTimeChanged);
}

void DocOrderEcho::slot_IdOrganizationChanged()
{
    if (m_idOrganization == Enums::IDX_UNKNOW)
        return;
    auto startOrganization = modelOrganizations->index(0, 0);
    auto indexOrganization = modelOrganizations->match(startOrganization, Qt::UserRole, m_idOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty())
        ui->comboOrganization->setCurrentIndex(indexOrganization.first().row());
}

void DocOrderEcho::slot_IdContractChanged()
{
    if (m_idContract == Enums::IDX_UNKNOW)
        return;
    auto indexContract = modelContracts->match(modelContracts->index(0,0), Qt::UserRole, m_idContract, 1, Qt::MatchExactly);
    if (!indexContract.isEmpty())
        ui->comboContract->setCurrentIndex(indexContract.first().row());
}

void DocOrderEcho::slot_IdTypePriceChanged()
{
    if (m_idTypePrice == Enums::IDX_UNKNOW)
        return;

    auto indexTypePrice = modelTypesPrices->match(modelTypesPrices->index(0,0), Qt::UserRole, m_idTypePrice, 1, Qt::MatchExactly);
    if (!indexTypePrice.isEmpty())
        ui->comboTypesPricing->setCurrentIndex(indexTypePrice.first().row());

    if (m_idOrganization == Enums::IDX_UNKNOW || m_idContract == Enums::IDX_UNKNOW)
        return;

    QString strQry;
    strQry = "SELECT id FROM pricings "
             "WHERE deletionMark = 2 AND "
             "id_organizations = " + QString::number(m_idOrganization) + " AND "
             "id_contracts = " + QString::number(m_idContract) + " AND "
             "id_typesPrices = " + QString::number(m_idTypePrice) + " ;";
    QMap<QString, QString> _items;
    if (db->getDataFromQueryByRecord(strQry, _items)){
        if (_items.count() != 0)
            lastIdPricings = _items.constFind("id").value().toInt();
    }
    updateTableSources();
}

void DocOrderEcho::slot_IdPacientChanged()
{
    if (m_idPacient == Enums::IDX_UNKNOW)
        return;

    auto indexPacient = modelPacients->match(modelPacients->index(0,0), Qt::UserRole, m_idPacient, 1, Qt::MatchExactly);
    if (!indexPacient.isEmpty()){
        ui->comboPacient->setCurrentIndex(indexPacient.first().row());
        completer->setCurrentRow(indexPacient.first().row());
        ui->comboPacient->setCurrentText(completer->currentCompletion());
    }

    QMap<QString, QString> _items;
    if (db->getObjectDataById("pacients", m_idPacient, _items)){
        ui->frameDataPacient->setVisible(true);
        ui->dateEditBirthday->setDate(QDate::fromString(_items.constFind("birthday").value(), "yyyy-MM-dd"));
        ui->editIDNP->setText(_items.constFind("IDNP").value());
        ui->editPolicyMedical->setText(_items.constFind("medicalPolicy").value());
        ui->editAddress->setText(_items.constFind("address").value());
        ui->editPhone->setText(_items.constFind("telephone").value());
        ui->editEmail->setText(_items.constFind("email").value());

        enableDisableDataPacient(false); // initial nu redactam datele pacientului
    }
}

void DocOrderEcho::slot_IdNurseChanged()
{
    if (m_idNurse == Enums::IDX_UNKNOW)
        return;
    auto index_nurse = modelNurses->match(modelNurses->index(0,0), Qt::UserRole, m_idNurse, 1, Qt::MatchExactly);
    if (! index_nurse.isEmpty())
        ui->comboNurse->setCurrentIndex(index_nurse.first().row());
}

void DocOrderEcho::slot_IdDoctorChanged()
{
    if (m_idDoctor == Enums::IDX_UNKNOW)
        return;
    auto indexDoctor = modelDoctors->match(modelDoctors->index(0, 0), Qt::UserRole, m_idDoctor, 1, Qt::MatchExactly);
    if (!indexDoctor.isEmpty())
        ui->comboDoctor->setCurrentIndex(indexDoctor.first().row());
}

void DocOrderEcho::slot_IdDoctorExecuteChanged()
{
    if (m_idDoctor_execute == Enums::IDX_UNKNOW)
        return;
    auto index_doctore_execute = modelDoctorsExecute->match(modelDoctorsExecute->index(0,0), Qt::UserRole, m_idDoctor_execute, 1, Qt::MatchExactly);
    if (! index_doctore_execute.isEmpty())
        ui->comboDoctorExecute->setCurrentIndex(index_doctore_execute.first().row());
}

void DocOrderEcho::slot_IdUserChanged()
{
    if (m_idUser == Enums::IDX_UNKNOW)
        return;
}

void DocOrderEcho::slot_NamePatientChanged()
{
    if (m_name_patient.isEmpty())
        return;
    ui->comboPacient->setCurrentText(m_name_patient);

    while (this->isVisible())
        filterRegExpChangedPacient();

}

void DocOrderEcho::slot_editingFinishedPhonePatient()
{
    ui->editEmail->setFocus(); // dupa ce a fost introdus telefonul setam focusul pe validarea pacientului
}

// **********************************************************************************
// --- procesarea combobox-urilor

void DocOrderEcho::activatedItemCompleter(const QModelIndex &index)
{
//    qDebug() << index.data(Qt::UserRole).toString()
//             << index.data(Qt::DisplayRole).toString(); // returneaza 'id'


    int _id = index.data(Qt::UserRole).toInt();    // determinam 'id'
    if (_id == Enums::IDX_UNKNOW || _id == Enums::IDX_WRITE)     // verificarea id
        return;

    setIdPacient(index.data(Qt::UserRole).toInt()); // setam 'id' pacientului

    if (_id > Enums::IDX_WRITE && ! ui->checkBox->isChecked()) // daca nu este nou
        ui->editFilterPattern->setFocus();              // setam focusul
}

void DocOrderEcho::stateChangedCheckBox(const int value)
{
    if (value == Qt::Checked)
        enableDisableDataPacient();
    else
        enableDisableDataPacient(false);
}

void DocOrderEcho::indexChangedComboOrganization(const int index)
{
    int id_organization = ui->comboOrganization->itemData(index, Qt::UserRole).toInt();
    setIdOrganization(id_organization);
    dataWasModified();

    QMap<QString, QString> items;
    QString strQry = "SELECT organizations.name,"
                     "organizations.id_contracts AS id_contract,"
                     "contracts.name AS nameContract,"
                     "contracts.id_typesPrices AS id_typePrice,"
                     "typesPrices.name AS nameTypePrice, "
                     "typesPrices.noncomercial AS noncomercial "
                     "FROM organizations "
                     "INNER JOIN contracts ON organizations.id_contracts = contracts.id "
                     "INNER JOIN typesPrices ON contracts.id_typesPrices = typesPrices.id "
                     "WHERE organizations.id = " + QString::number(id_organization) + ";";
    if (db->getDataFromQueryByRecord(strQry, items)){
        if (items.count() == 0)
            return;
        int id_contract    = items.constFind("id_contract").value().toInt();
        int id_typePrice   = items.constFind("id_typePrice").value().toInt();
        noncomercial_price = items.constFind("noncomercial").value().toInt();

        modelContracts->clear();                                        // actualizam solicitarea
        QString strQryContracts = "SELECT id,name FROM contracts "      // filtru dupa organizatia
                                  "WHERE deletionMark = 0 AND id_organizations = "
                                    + QString::number(id_organization)+ " AND notValid = 0;";
        modelContracts->setQuery(strQryContracts);
        if (id_contract == 0)
            ui->comboContract->setCurrentIndex(0); // setam indexul 0 - selecteaza
        else
            setIdContract(id_contract);

        if (id_typePrice == 0)
            ui->comboTypesPricing->setCurrentIndex(0); // setam indexul 0 - selecteaza
        else
            setIdTypePrice(id_typePrice);

        if (ui->comboTypesPricing->currentIndex() > Enums::IDX::IDX_WRITE)
            ui->comboDoctor->setFocus();
    }
}

void DocOrderEcho::indexChangedComboContract(const int index)
{
    int id_contract = ui->comboContract->itemData(index, Qt::UserRole).toInt();
    setIdContract(id_contract);
    dataWasModified();

    QMap<QString, QString> items;
    if (db->getObjectDataById("contracts", id_contract, items)){
        int id_typePrice = items.constFind("id_typesPrices").value().toInt();
        if (id_typePrice == 0)
            ui->comboTypesPricing->setCurrentIndex(0);
        else
            setIdTypePrice(id_typePrice);
    }
}

void DocOrderEcho::indexChangedComboTypePrice(const int index)
{
    int id_typePrice = ui->comboTypesPricing->itemData(index, Qt::UserRole).toInt();
    setIdTypePrice(id_typePrice);
    dataWasModified();
}

void DocOrderEcho::indexChangedComboDoctor(const int index)
{
    int id_doctor = ui->comboDoctor->itemData(index, Qt::UserRole).toInt();
    setIdDoctor(id_doctor);
    dataWasModified();

    ui->toolBox->setCurrentIndex(box_patient);
    ui->comboPacient->setFocus();
}

void DocOrderEcho::indexChangedComboDoctorExecute(const int index)
{
    int id_doctor_execute = ui->comboDoctorExecute->itemData(index, Qt::UserRole).toInt();
    setIdDoctorExecute(id_doctor_execute);
    dataWasModified();
}

void DocOrderEcho::indexChangedComboNurse(const int index)
{
    int id_nurse = ui->comboNurse->itemData(index, Qt::UserRole).toInt();
    setIdNurse(id_nurse);
    dataWasModified();
}

// **********************************************************************************
// --- crearea/modificarea datelor pacintului

void DocOrderEcho::onClickNewPacient()
{
    if (ui->comboPacient->currentText().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu sunt determinate datele pacientului !!!"),
                             QMessageBox::Ok);
        return;
    }

    QString _name;
    QString _fName;
    QString details_error;
    if (! splitFullNamePacient(_name, _fName))
        return;

    if (_name.isEmpty() || _fName.isEmpty())  // verificam daca variabile sunt completate
        return;

    if (ui->checkBox->isChecked()){  // crearea pacientului nou

        // 1. daca nu este IDNP - cautam pacientul dupa nume si prenume
        // 2. daca este IDNP - cautam dupa name, prenume, si IDNP
        // ... daca a fost gasit pacientul interogam
        // ... daca nu a fost gasit introducem in BD

        //*********************************************************
        // verificam daca pacientul este in baza de date
        QString _birthday;
        if (ui->editIDNP->text().isEmpty()) {
            if (existPatientByNameFName(_name, _fName, _birthday)){
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setWindowTitle(tr("Varificarea datelor"));
                msgBox.setText(tr("Pacientul(a) <b>%1, %2</b> exista in baza da date.<br>Doriți să continuați ?")
                                   .arg(_name + " " + _fName, QDate::fromString(_birthday, "yyyy-MM-dd").toString("dd.MM.yyyy")));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No){
                    return;
                }
            }
        } else {
            if (existPatientByIDNP(_name, _fName, _birthday)){
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setWindowTitle(tr("Varificarea datelor"));
                msgBox.setText(tr("Pacientul(a) %1 exista in baza da date:"
                                  " - nume: %1\n"
                                  " - prenume: %2\n"
                                  " - anul nasterii: %3\n"
                                  " - IDNP: %4")
                                   .arg(_name, _fName, QDate::fromString(_birthday, "yyyy-MM-dd").toString("dd.MM.yyyy"), ui->editIDNP->text()));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::No){
                    return;
                }
            }
        }

        //**********************************************************
        // introducem datele pacientului in baza de date
        int last_id = db->getLastIdForTable("pacients");
        if (insertDataTablePacients(last_id, _name, _fName, details_error)){
            popUp->setPopupText(tr("Datele pacientului <b>%1</b><br> au fost introduse in baza de date cu succes.").arg(ui->comboPacient->currentText()));
            popUp->show();

            qInfo(logInfo()) << tr("Crearea: pacientul '%1' cu id='%2'.")
                                    .arg(ui->comboPacient->currentText(), QString::number(last_id));

            updateModelPacients();             // actualizam 'modelPacients'
            setIdPacient(last_id + 1);         // setam 'id' pacientului
            emit mCreateNewPacient();          // emitem signal pu conectarea din alte clase

            if (ui->checkBox->isChecked())        // dupa validarea datelor pacientului
                ui->checkBox->setChecked(false);  // obiectul(pacientul) nu este nou
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Validarea datelor"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Datele pacientului %1 nu au fost salvate in baza de date."));
            msgBox.setDetailedText((details_error.isEmpty()) ? tr("eroare indisponibila") : details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }

    } else {

        if (updateDataTablePacients(_name, _fName, details_error)){
            popUp->setPopupText(tr("Datele pacientului <b>%1</b><br> au fost modificate cu succes.").arg(ui->comboPacient->currentText()));
            popUp->show();

            qInfo(logInfo()) << tr("Modificare: datele pacientului '%1' cu id='%2'.")
                                .arg(ui->comboPacient->currentText(), QString::number(m_idPacient));

            updateModelPacients();     // actualizam 'modelPacients'
            setIdPacient(m_idPacient);
            ui->editFilterPattern->setFocus(); // setam focus
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Validarea datelor"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Datele pacientului %1 nu au fost modificate."));
            msgBox.setDetailedText((details_error.isEmpty()) ? tr("eroare indisponibila") : details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }

    }
}

void DocOrderEcho::onClikEditPacient()
{
    if (ui->comboPacient->currentText().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu sunt determinate datele pacientului !!!"),
                             QMessageBox::Ok);
        return;
    }
    qInfo(logInfo()) << tr("Editarea datelor pacientului '%1' cu id='%2'.")
                        .arg(ui->comboPacient->currentText(), QString::number(m_idPacient));
    enableDisableDataPacient();
}

void DocOrderEcho::onClickClearPacient()
{
    ui->comboPacient->clear();
    setIdPacient(Enums::IDX_UNKNOW);
    ui->comboPacient->clearEditText();
    ui->frameDataPacient->setVisible(true);
    ui->dateEditBirthday->setDate(QDate::fromString("1970-01-01", "yyyy-MM-dd"));
    ui->editIDNP->clear();
    ui->editPolicyMedical->clear();
    ui->editAddress->clear();
    ui->editPhone->clear();
    ui->editFilterPattern->setFocus();

    if (ui->checkBox->isChecked())
        ui->checkBox->setChecked(false);
}

void DocOrderEcho::onClickOpenHistoryPatient()
{
    if (m_idPacient == Enums::IDX_UNKNOW || m_idPacient == 0){
        QMessageBox::warning(this,
                             tr("Verificarea datelor"),
                             tr("Nu este determinat pacientul !!!"),
                             QMessageBox::Ok);
        return;
    }

    patient_history = new PatientHistory(this);
    patient_history->setAttribute(Qt::WA_DeleteOnClose);
    patient_history->setProperty("IdPatient", m_idPacient);
    this->hide();
    patient_history->exec();
    this->show();
    ui->editFilterPattern->setFocus();
}

// **********************************************************************************
// --- procesarea actiunilor cu tabela

void DocOrderEcho::updateTextSumOrder()
{
    sumOrder = 0;
    for (int n = 0; n < modelTableOrder->rowCount(); n++) {
        if (ui->tableViewOrder->isRowHidden(n))         // verificam daca sunt randuri pu eliminare
            continue;                                   // eliminarea randurilor numai -> submitAll -> btnOk
        QString m_sum = modelTableOrder->data(modelTableOrder->index(n, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), Qt::DisplayRole).toString();
        sumOrder = sumOrder + m_sum.toDouble();
    }
    double _num = sumOrder;
    ui->labelSumOrder->setText(QString("%1").arg(_num, 0, 'f', 2) + " MDL");
}

void DocOrderEcho::onDoubleClickedTableSource(const QModelIndex &index)
{
    int _row = index.row();
//    int _id       = modelTableSource->data(modelTableSource->index(_row, column_Id), Qt::UserRole).toInt();
    QString _cod  = proxy->data(proxy->index(_row, Enums::PRICING_COLUMN::PRICING_COLUMN_COD), Qt::DisplayRole).toString();
    QString _name = proxy->data(proxy->index(_row, Enums::PRICING_COLUMN::PRICING_COLUMN_NAME), Qt::DisplayRole).toString();
    QString _price = proxy->data(proxy->index(_row, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), Qt::DisplayRole).toString();

    for (int n = 0; n < modelTableOrder->rowCount(); n++) {
        if (ui->tableViewOrder->model()->data(modelTableOrder->index(n, Enums::PRICING_COLUMN::PRICING_COLUMN_COD), Qt::DisplayRole).toString() == _cod){
            QMessageBox::warning(this, tr("Atentie"),
                                 tr("Investigatia <b>'%1 - %2'</b> exista in tabel.").arg(_cod, _name), QMessageBox::Ok);
            ui->tableViewOrder->setCurrentIndex(modelTableOrder->index(n, Enums::PRICING_COLUMN::PRICING_COLUMN_NAME));
            return;
        }
    }

    int lastId = db->getLastIdForTable("orderEchoTable");
    int lastIdOrder = db->getLastIdForTable("orderEcho");
    int _rowCount = modelTableOrder->rowCount();
    int nextId = lastId + _rowCount + 1;

    modelTableOrder->insertRow(_rowCount);
    modelTableOrder->setData(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_ID), nextId);
    modelTableOrder->setData(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_DEL_MARK), Enums::IDX_WRITE);
    modelTableOrder->setData(modelTableOrder->index(_rowCount, 2), lastIdOrder);   // id_orderEcho
    modelTableOrder->setData(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_COD), _cod);
    modelTableOrder->setData(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_NAME), _name);
    modelTableOrder->setData(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE), _price);
    ui->tableViewOrder->setCurrentIndex(modelTableOrder->index(_rowCount, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE));

    sumOrder = sumOrder + _price.toDouble();
    updateTextSumOrder();
}

void DocOrderEcho::onClickedRowTableOrder(const QModelIndex &index)
{
    int _row = index.row();
    QModelIndex indexEdit = modelTableOrder->index(_row, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE);
    ui->tableViewOrder->setCurrentIndex(indexEdit);
    ui->tableViewOrder->edit(indexEdit);
    updateTextSumOrder();
}

void DocOrderEcho::filterRegExpChanged()
{
    proxy->setFilterKeyColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_NAME);
    QRegularExpression regExp(ui->editFilterPattern->text(), QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(regExp);
}

void DocOrderEcho::filterRegExpChangedPacient()
{
    proxyPacient->setFilterKeyColumn(1);
    QRegularExpression regExp(ui->comboPacient->currentText(), QRegularExpression::CaseInsensitiveOption);
    proxyPacient->setFilterRegularExpression(regExp);
}

void DocOrderEcho::slotContextMenuRequested(QPoint pos)
{
    QAction* actionEditRow   = new QAction(QIcon(":/img/edit_x32.png"), tr("Editeaza rândul."), this);
    QAction* actionRemoveRow = new QAction(QIcon(":/img/clear_x32.png"), tr("Șterge rândul."), this);

    connect(actionEditRow, &QAction::triggered, this, &DocOrderEcho::editRowTableOrder);
    connect(actionRemoveRow, &QAction::triggered, this, &DocOrderEcho::removeRowTableOrder);

    menu->clear();
    menu->addAction(actionEditRow);
    menu->addAction(actionRemoveRow);
    menu->popup(ui->tableViewOrder->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

void DocOrderEcho::removeRowTableOrder()
{
    QModelIndexList selectedRows = ui->tableViewOrder->selectionModel()->selectedRows();

    for (int n = 0; n < selectedRows.count(); n++) {
        modelTableOrder->removeRows(selectedRows[n].row(), 1);
        ui->tableViewOrder->setRowHidden(selectedRows[n].row(), true);
    }
    updateTextSumOrder();
    dataWasModified();
}

void DocOrderEcho::editRowTableOrder()
{
    onClickedRowTableOrder(ui->tableViewOrder->currentIndex());
}

// **********************************************************************************
// --- controlul completarii obiectelor, printarea, inregistrarea datelor in BD

bool DocOrderEcho::controlRequiredObjects()
{
    if (ui->comboOrganization->currentIndex() <= Enums::IDX_WRITE){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectată <b>'Organizația'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala -> vezi: onWritingData() & onWritingDataClose()
        return false;
    }
    if (ui->comboContract->currentIndex() <= Enums::IDX_WRITE){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Contractul'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala -> vezi: onWritingData() & onWritingDataClose()
        return false;
    }
    if (ui->comboTypesPricing->currentIndex() <= Enums::IDX_WRITE){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Tipul prețului'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala -> vezi: onWritingData() & onWritingDataClose()
        return false;
    }
    if (ui->comboPacient->currentText().isEmpty() || m_idPacient == Enums::IDX_UNKNOW){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Pacientul'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala -> vezi: onWritingData() & onWritingDataClose()
        return false;
    }
    if (ui->tableViewOrder->model()->rowCount() == 0){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este aleasa nici o investigatie !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala -> vezi: onWritingData() & onWritingDataClose()
        return false;
    }
    return true;
}

void DocOrderEcho::onOpenReport()
{
    // controlam daca documentul este validat
    if (m_itNew){
        QMessageBox::warning(this, tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nRaportul ecografic nu poate fi format."),
                             QMessageBox::Ok);
        return;
    }

    // verificam daca este documentul subaltern
    int m_id_reportEcho = Enums::IDX_UNKNOW;
    if (db->existSubalternDocument("reportEcho", "id_orderEcho", QString::number(m_id), m_id_reportEcho)){
        if (m_id_reportEcho > 0){
            DocReportEcho* doc_report = new DocReportEcho(this);
            doc_report->setAttribute(Qt::WA_DeleteOnClose);
            doc_report->setProperty("ItNew", false);
            doc_report->setProperty("Id", m_id_reportEcho);
            doc_report->setProperty("IdPacient", m_idPacient);
            doc_report->setProperty("IdDocOrderEcho", m_id);
            doc_report->show();
            return;
        }
    }

    // daca nu afost gasit documentul subaltern -> deschidem selectarea investigatiilor
    CustomDialogInvestig* dialogInvestig = new CustomDialogInvestig(this);
    for (int n = 0; n < modelTableOrder->rowCount(); n++) {
        QString _cod = ui->tableViewOrder->model()->data(modelTableOrder->index(n, Enums::PRICING_COLUMN::PRICING_COLUMN_COD), Qt::DisplayRole).toString();
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
    if (dialogInvestig->exec() != QDialog::Accepted)
        return;

    this->hide(); // ascundem fereastra 'docRorderEcho'

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
    doc_report->setProperty("IdPacient", m_idPacient);
    doc_report->setProperty("IdDocOrderEcho", m_id);
    doc_report->exec();

    this->close();
}

void DocOrderEcho::onPrint(Enums::TYPE_PRINT type_print)
{
    // *************************************************************************************
    // verificam daca este validat documentul
    if (m_itNew){
        QMessageBox::warning(this, tr("Controlul validarii"),
                             tr("Documentul nu este validat !!! \nPrintare nu este posibila."),
                             QMessageBox::Ok);
        return;
    }

    // *************************************************************************************
    // alocam memoria
    m_report = new LimeReport::ReportEngine(this);
    print_model_organization = new QSqlQueryModel(this);
    print_model_patient      = new QSqlQueryModel(this);
    print_model_table        = new QSqlQueryModel(this);

    // *************************************************************************************
    // verificam daca este complectata variabila 'noncomercial_price'
    if (noncomercial_price == Enums::IDX_UNKNOW){
        QString strQry = QString("SELECT noncomercial FROM typesPrices WHERE id = '%1' AND deletionMark = '0';")
                .arg(m_idTypePrice);
        QMap<QString, QString> dataPrice;
        if (db->getDataFromQueryByRecord(strQry, dataPrice))
            noncomercial_price = dataPrice.constFind("noncomercial").value().toInt();
    }

    // *************************************************************************************
    // logotipul, semnaturile
    setImageForDocPrint();
    m_report->dataManager()->addModel("table_img", model_img, true);

    // *************************************************************************************
    // setam solicitarile in model
    print_model_organization->setQuery(db->getQryFromTableConstantById(globals::idUserApp));
    print_model_patient->setQuery(db->getQryFromTablePatientById(m_idPacient));
    if (globals::thisMySQL)
        print_model_table->setQuery(db->getQryForTableOrderById(m_id, (noncomercial_price) ? "'0-00'" : "orderEchoTable.price"));
    else
        print_model_table->setQuery(db->getQryForTableOrderById(m_id, (noncomercial_price) ? "'0-00'" : "orderEchoTable.price ||'0'"));

    double _num;
    _num = (noncomercial_price == 0) ? sumOrder : 0; // daca 'noncomercial_price' -> nu aratam suma

    // *************************************************************************************
    // transmitem variabile si modelurile generatorului de rapoarte
    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("sume_total", QString("%1").arg(_num, 0, 'f', 2));
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("v_exist_stamp", exist_stamp);
    m_report->dataManager()->setReportVariable("v_exist_signature", exist_signature);

    m_report->dataManager()->addModel("main_organization", print_model_organization, false);
    m_report->dataManager()->addModel("table_pacient", print_model_patient, false);
    m_report->dataManager()->addModel("table_table", print_model_table, false);
    m_report->setShowProgressDialog(true);

    m_report->setPreviewWindowTitle(tr("Comanda ecografică nr.") + ui->editNumberDoc->text() + tr(" din ") + ui->dateTimeDoc->dateTime().toString("dd.MM.yyyy hh:mm:ss") + tr(" (printare)"));

    // *************************************************************************************
    // verificam drumul spre forme de tipar
    QDir dir;
    if (! QFile(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Order.lrxml")).exists()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Printarea documentului"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Documentul nu poate fi printat."));
        msgBox.setDetailedText(tr("Nu a fost gasit fisierul sablon formei de tipar:\n%1").arg(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Order.lrxml")));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet(db->getStyleForButtonMessageBox());
        msgBox.exec();

        delete print_model_organization;
        delete print_model_patient;
        delete print_model_table;
        delete m_report;

        return;
    }
    m_report->loadFromFile(dir.toNativeSeparators(globals::pathTemplatesDocs + "/Order.lrxml"));

    // *************************************************************************************
    // prezentam forma de tipar
    if (type_print == Enums::TYPE_PRINT::OPEN_DESIGNER){
        qInfo(logInfo()) << tr("Printare (designer): document 'Comanda ecografica' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                .arg(QString::number(m_id), ui->editNumberDoc->text(), QString::number(m_idPacient), ui->comboPacient->currentText());
        m_report->designReport();
    } else if (type_print == Enums::TYPE_PRINT::OPEN_PREVIEW){
        qInfo(logInfo()) << tr("Printare (preview): document 'Comanda ecografica' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                .arg(QString::number(m_id), ui->editNumberDoc->text(), QString::number(m_idPacient), ui->comboPacient->currentText());
        m_report->previewReport();
    }

    // *************************************************************************************
    // elibiram memoria
    print_model_organization->deleteLater();
    print_model_patient->deleteLater();
    print_model_table->deleteLater();
    m_report->deleteLater();
}

void DocOrderEcho::openDesignerPrintDoc()
{
    onPrint(Enums::TYPE_PRINT::OPEN_DESIGNER);
}

void DocOrderEcho::openPreviewPrintDoc()
{
    onPrint(Enums::TYPE_PRINT::OPEN_PREVIEW);
}

bool DocOrderEcho::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    if (m_post == Enums::IDX_UNKNOW)  // daca a fost apasat btnOk = propritatea trebuia sa fie m_post == idx_post
        setPost(Enums::IDX_WRITE);    // setam post = idx_write

    QString details_error;

    if (m_itNew){

        if (m_attachedImages == Enums::IDX_UNKNOW)
            m_attachedImages = 0;

        m_id = db->getLastIdForTable("orderEcho") + 1; // incercare de a seta 'id' documentului

        if (! insertDataTableOrderEcho(details_error)){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Validarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Validarea documentului nu s-a efectuat."));
            msgBox.setDetailedText(details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
            m_id = Enums::IDX_UNKNOW;   // setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala
            return false;
        }

        for (int i = 0; i < modelTableOrder->rowCount(); ++i) {                                // corectam valoarea indexului 'deletionMark'
            modelTableOrder->setData(modelTableOrder->index(i, Enums::PRICING_COLUMN::PRICING_COLUMN_DEL_MARK), m_post);  // si 'id' documentului din tabela
            modelTableOrder->setData(modelTableOrder->index(i, Enums::PRICING_COLUMN::PRICING_COLUMN_ID_PRICINGS), m_id);
        }

        if (! modelTableOrder->submitAll()){           // daca nu a fost salvata tabela:
            db->removeObjectById("orderEcho", m_id);   // 1.eliminam datele salvate mai sus din sqlite
            m_id = Enums::IDX_UNKNOW;                         // 2.setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW);                       // 3.setam m_post la valoarea initiala
            qCritical(logCritical()) << modelTableOrder->lastError().text();
            return false;
        }

        if (m_post == Enums::IDX_WRITE || m_post == Enums::IDX_POST){
            popUp->setPopupText(tr("Documentul a fost %1 cu succes<br> in baza de date.").arg((m_post == Enums::IDX_WRITE) ? tr("salvat") : tr("validat")));
            popUp->show();
            setItNew(false);
            qInfo(logInfo()) << tr("Crearea: documentul 'Comanda ecografica' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                .arg(QString::number(m_id), ui->editNumberDoc->text(), QString::number(m_idPacient), ui->comboPacient->currentText());
        }
    } else {        

        if (! updateDataTableOrderEcho(details_error)){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Actualizarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msgBox.setDetailedText(details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
            m_id = Enums::IDX_UNKNOW;   // setam la valoarea initiala
            setPost(Enums::IDX_UNKNOW); // setam m_post la valoarea initiala
            return false;
        }

        if (! modelTableOrder->submitAll()){           // daca nu a fost salvata tabela:
            qCritical(logCritical()) << modelTableOrder->lastError().text();
            return false;
        }

        if (m_post == Enums::IDX_WRITE || m_post == Enums::IDX_POST){
            popUp->setPopupText(tr("Datele documentului au fost actualizate<br> cu succes."));
            popUp->show();

            setItNew(false);
            qInfo(logInfo()) << tr("Modificare: document 'Comanda ecografica' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                .arg(QString::number(m_id), ui->editNumberDoc->text(), QString::number(m_idPacient), ui->comboPacient->currentText());
        }
    }
    emit PostDocument(); // pu actualizarea listei documentelor
    return true;
}

void DocOrderEcho::onWritingDataClose()
{
    setPost(Enums::IDX_POST); // setam proprietatea 'post'

    if (onWritingData()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Printarea documentului"),
                                 tr("Dori\310\233i s\304\203 printa\310\233i documentul ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            onPrint(Enums::TYPE_PRINT::OPEN_PREVIEW);
        } else if (messange_box.clickedButton() == noButton) {

        }

        QDialog::accept();
        emit mCloseThisForm();

    }
}

void DocOrderEcho::onClose()
{
    this->close();
    emit mCloseThisForm();    
}

void DocOrderEcho::handleCompleterAddressPatients(const QString &text)
{
    if (text.startsWith("m.", Qt::CaseInsensitive) ||
        text.startsWith("s.", Qt::CaseInsensitive) ||
        text.startsWith("or.", Qt::CaseInsensitive)) {

        ui->editAddress->setCompleter(city_completer);

    } else {

        ui->editAddress->setCompleter(nullptr);

    }
}

void DocOrderEcho::setTitleDoc()
{
    setWindowTitle(tr("Comanda ecografic\304\203 %1").arg("[*]"));
}

// **********************************************************************************
// --- conexiunilie

void DocOrderEcho::initConnections()
{
    QString style_toolButton = db->getStyleForToolButton();
    ui->btnCreateCatDoctor->setStyleSheet(style_toolButton);
    ui->btnOpenCatDoctor->setStyleSheet(style_toolButton);
    ui->btnNewPacient->setStyleSheet(style_toolButton);
    ui->btnEditPacient->setStyleSheet(style_toolButton);
    ui->btnClearPacient->setStyleSheet(style_toolButton);
    ui->btnPatientHistory->setStyleSheet(style_toolButton);

    connect(this, &DocOrderEcho::ItNewChanged, this, &DocOrderEcho::slot_ItNewChanged);
    connect(this, &DocOrderEcho::IdChanged, this, &DocOrderEcho::slot_IdChanged);                          // conectarea la modificarea
    connect(this, &DocOrderEcho::IdOrganizationChanged, this, &DocOrderEcho::slot_IdOrganizationChanged);  // proprietatilor clasei
    connect(this, &DocOrderEcho::IdContractChanged, this, &DocOrderEcho::slot_IdContractChanged);          // si actiunea dupa modificare:
    connect(this, &DocOrderEcho::IdTypePriceChanged, this, &DocOrderEcho::slot_IdTypePriceChanged);        // completarea/modificarea formei
    connect(this, &DocOrderEcho::IdPacientChanged, this, &DocOrderEcho::slot_IdPacientChanged);
    connect(this, &DocOrderEcho::IdNurseChanged, this, &DocOrderEcho::slot_IdNurseChanged);
    connect(this, &DocOrderEcho::IdDoctorChanged, this, &DocOrderEcho::slot_IdDoctorChanged);
    connect(this, &DocOrderEcho::IdDoctorChangedExecute, this, &DocOrderEcho::slot_IdDoctorExecuteChanged);
    connect(this, &DocOrderEcho::IdUserChanged, this, &DocOrderEcho::slot_IdUserChanged);
    connect(this, &DocOrderEcho::NamePatientChanged, this, &DocOrderEcho::slot_NamePatientChanged);

    connect(ui->editComment, &QPlainTextEdit::textChanged, this, &DocOrderEcho::controlLengthComment);

    connect(ui->btnCreateCatDoctor, &QPushButton::clicked, this, &DocOrderEcho::createCatDoctor);
    connect(ui->btnOpenCatDoctor, &QPushButton::clicked, this, &DocOrderEcho::openCatDoctor);

    connect(ui->checkBox, QOverload<const int>::of(&QCheckBox::stateChanged), this, QOverload<const int>::of(&DocOrderEcho::stateChangedCheckBox));

    connect(ui->cardPayment, &QCheckBox::stateChanged, this, &DocOrderEcho::dataWasModified);

    connect(ui->btnNewPacient, &QAbstractButton::clicked, this, &DocOrderEcho::onClickNewPacient);
    connect(ui->btnEditPacient, &QAbstractButton::clicked, this, &DocOrderEcho::onClikEditPacient);
    connect(ui->btnClearPacient, &QAbstractButton::clicked, this, &DocOrderEcho::onClickClearPacient);
    connect(ui->btnPatientHistory, &QAbstractButton::clicked, this, &DocOrderEcho::onClickOpenHistoryPatient);

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &DocOrderEcho::onDoubleClickedTableSource);
    connect(ui->tableViewOrder->model(), &QAbstractItemModel::dataChanged, this, &DocOrderEcho::dataWasModified);  // modificarea datelor tabelului
    connect(ui->tableViewOrder->model(), &QAbstractItemModel::dataChanged, this, &DocOrderEcho::updateTextSumOrder);
    connect(ui->tableViewOrder, QOverload<const QModelIndex&>::of(&QTableView::clicked),                           // la apasare btn Key_Return sau Key_Enter
            this, &DocOrderEcho::onClickedRowTableOrder);                                                          // intram in regimul de redactare a sectiei 'Cost'
    connect(ui->tableViewOrder, &QWidget::customContextMenuRequested, this, &DocOrderEcho::slotContextMenuRequested);

    connect(ui->editFilterPattern, &QLineEdit::textChanged, this, &DocOrderEcho::filterRegExpChanged);
    connect(ui->comboPacient, &QComboBox::currentTextChanged, this, &DocOrderEcho::filterRegExpChangedPacient);

    connect(ui->btnReport, &QAbstractButton::clicked, this, &DocOrderEcho::onOpenReport);
    connect(ui->btnOk, &QAbstractButton::clicked, this, &DocOrderEcho::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &DocOrderEcho::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &DocOrderEcho::onClose);

    connect(ui->toolBox, QOverload<int>::of(&QToolBox::currentChanged), this, QOverload<int>::of(&DocOrderEcho::changeIconForItemToolBox));

    connect(ui->editPhone, &QLineEdit::editingFinished, this ,&DocOrderEcho::slot_editingFinishedPhonePatient);

    db->updateVariableFromTableSettingsUser();
    if (globals::showDesignerMenuPrint) {
        QAction *openDesignerOrder = new QAction(setUpMenu);
        openDesignerOrder->setIcon(QIcon(":/images/design.png"));
        openDesignerOrder->setText(tr("Deschide designer"));

        QAction *openPreviewOrder  = new QAction(setUpMenu);
        openPreviewOrder->setIcon(QIcon(":/images/Print.png"));
        openPreviewOrder->setText(tr("Deschide preview"));

        setUpMenu->addAction(openDesignerOrder);
        setUpMenu->addAction(openPreviewOrder);
        setUpMenu->setWindowFlags(setUpMenu->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu->setAttribute(Qt::WA_TranslucentBackground);
        ui->btnPrint->setMenu(setUpMenu);
        ui->btnPrint->setStyleSheet("padding-left: 4px; padding-right: 4px; width: 95px; height: 20px;");

        connect(openDesignerOrder, &QAction::triggered, this, &DocOrderEcho::openDesignerPrintDoc);
        connect(openPreviewOrder, &QAction::triggered, this, &DocOrderEcho::openPreviewPrintDoc);
    } else {
        connect(ui->btnPrint, &QPushButton::clicked, this, &DocOrderEcho::openPreviewPrintDoc);
    }
}

void DocOrderEcho::connectionsToIndexChangedCombobox()
{
    connect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboOrganization));

    connect(ui->comboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboContract));

    connect(ui->comboTypesPricing, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboTypePrice));

    connect(ui->comboDoctor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboDoctor));

    connect(ui->comboDoctorExecute, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboDoctorExecute));

    connect(ui->comboNurse, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboNurse));

}

void DocOrderEcho::disconnectionsToIndexChangedCombobox()
{
    disconnect(ui->comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboOrganization));

    disconnect(ui->comboContract, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboContract));

    disconnect(ui->comboTypesPricing, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&DocOrderEcho::indexChangedComboTypePrice));

    disconnect(ui->comboDoctor, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, QOverload<int>::of(&DocOrderEcho::indexChangedComboDoctor));

    disconnect(ui->comboDoctorExecute, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboDoctorExecute));

    disconnect(ui->comboNurse, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&DocOrderEcho::indexChangedComboNurse));
}

void DocOrderEcho::updateModelPacients()
{
    if (modelPacients->rowCount() > 0)
        modelPacients->clear();

    QString strQuery;
    strQuery = "SELECT pacients.id, "
               "fullNamePacients.nameBirthdayIDNP AS FullName "
               "FROM pacients "
               "INNER JOIN fullNamePacients ON pacients.id = fullNamePacients.id_pacients "
               "WHERE pacients.deletionMark = 0 ORDER BY fullNamePacients.nameBirthdayIDNP;";
    QMap<int, QString> data = db->getMapDataQuery(strQuery);

    QMapIterator<int, QString> it(data);
    while (it.hasNext()) {
        it.next();
        int     _id   = it.key();
        QString _name = it.value();
//        _name.replace("#1", "\n");

        QStandardItem *item = new QStandardItem;
        item->setData(_id,   Qt::UserRole);
        item->setData(_name, Qt::DisplayRole);

        modelPacients->appendRow(item);  // adaugam datele in model
    }
}

void DocOrderEcho::initSetCompleter()
{
    updateModelPacients();
    proxyPacient->setSourceModel(modelPacients); // setam modelul in proxyPacient

    completer->setModel(proxyPacient);                                  // setam model
    completer->setCaseSensitivity(Qt::CaseInsensitive);                 // cautare insenseibila la registru
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel); // modelul este sortat în mod sensibil.
    completer->setFilterMode(Qt::MatchContains);                        // contine elementul
    completer->setCompletionMode(QCompleter::PopupCompletion);

    ui->comboPacient->setEditable(true);
    ui->comboPacient->setCompleter(completer);

    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&DocOrderEcho::activatedItemCompleter));
}

void DocOrderEcho::enableDisableDataPacient(bool mEnabled)
{
    ui->dateEditBirthday->setEnabled(mEnabled);
    ui->editIDNP->setEnabled(mEnabled);
    ui->editPolicyMedical->setEnabled(mEnabled);
    ui->editAddress->setEnabled(mEnabled);
    ui->editPhone->setEnabled(mEnabled);
    ui->editEmail->setEnabled(mEnabled);

    if (mEnabled)
        ui->dateEditBirthday->setFocus(); // setam focusul
}

void DocOrderEcho::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/user_x32.png").pixmap(18,18);
    QLabel* labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    labelAuthor = new QLabel(this);
    labelAuthor->setText(globals::nameUserApp);
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

// **********************************************************************************
// --- solicitarile pu inserarea si actualizarea datelor in BD

bool DocOrderEcho::existPatientByNameFName(const QString &_name, const QString &_fName, QString &_birthday)
{
    QSqlQuery qry;
    qry.prepare("SELECT birthday FROM pacients "
                "WHERE name   = :name AND "
                "fName        = :fName;");
    qry.bindValue(":name",  _name);
    qry.bindValue(":fName", _fName);
    if (qry.exec() && qry.next()){
        _birthday = qry.value(0).toString();
        return true;;
    } else {
        return false;
    }
}

bool DocOrderEcho::existPatientByIDNP(const QString &_name, const QString &_fName, QString &_birthday)
{
    QSqlQuery qry;
    qry.prepare("SELECT birthday FROM pacients "
                "WHERE name   = :name AND "
                "fName        = :fName AND "
                "IDNP         = :IDNP;");
    qry.bindValue(":name",  _name);
    qry.bindValue(":fName", _fName);
    qry.bindValue(":IDNP", ui->editIDNP->text());
    if (qry.exec() && qry.next()){
        _birthday = qry.value(0).toString();
        return true;;
    } else {
        return false;
    }
}

bool DocOrderEcho::splitFullNamePacient(QString &_name, QString &_fName)
{
    //***********************************************************
    // split name & fName
    QString name_pacients_combo = ui->comboPacient->currentText();
    const int num_comma = name_pacients_combo.indexOf(",");

    QString full_name = name_pacients_combo.mid(0, num_comma);
    const int num_space = full_name.indexOf(" ");

    _name  = full_name.mid(0, num_space);
    _fName = full_name.mid(num_space + 1, full_name.length());

    //***********************************************************
    // presentation name & fName
    if (! globals::order_splitFullName)
        return true;

    QMessageBox msgBox;
    QAbstractButton* correct   = msgBox.addButton(tr("Corect"), QMessageBox::ActionRole);
    correct->setIcon(QIcon(":/img/accept_button.png"));
    QAbstractButton* incorrect = msgBox.addButton(tr("Incorect"), QMessageBox::ActionRole);
    incorrect->setIcon(QIcon(":/img/button_delete.png"));

    msgBox.setWindowTitle(tr("Verificarea datelor"));
    msgBox.setText(tr("Divizarea numelui/prenumelui pacientului:<br>"
                      " - nume: <b>%1</b><br>"
                      " - prenume: <b>%2</b>").arg(_name, _fName));
    msgBox.setCheckBox(new QCheckBox(tr("Nu arata")));
    msgBox.exec();

    if (msgBox.checkBox()->isChecked()){
        QSqlQuery qry;
        qry.prepare("UPDATE settingsUsers SET order_splitFullName = 0 WHERE order_splitFullName NOT NULL;");
        if (qry.exec())
            globals::order_splitFullName = false;
    }

    if (msgBox.clickedButton() == correct)
        return true;
    else if (msgBox.clickedButton() == incorrect)
        return false;

    return false;
}

int DocOrderEcho::getValuePaymentOrder()
{
    if (ui->cardPayment->checkState() == Qt::Checked)
        return Enums::PAYMENT_CARD;

    QSqlQuery qry;
    qry.prepare("SELECT noncomercial FROM typesPrices WHERE id = :id AND deletionMark = 0;");
    qry.bindValue(":id", m_idTypePrice);
    if (qry.exec() && qry.next()){
        if (qry.value(0).toInt() == 0)
            return Enums::PAYMENT_CASH;
        else
            return Enums::PAYMENT_TRANSFER;
    }

    qCritical(logCritical()) << tr("Nu a fost determinat statutul achitarii documentului.");

    return Enums::IDX_UNKNOW;
}

bool DocOrderEcho::insertDataTablePacients(int &last_id, const QString &_name, const QString &_fName, QString &details_error)
{
    db->getDatabase().transaction();
    QSqlQuery qry;
    qry.prepare("INSERT INTO pacients ("
                "id,"
                "deletionMark,"
                "IDNP,"
                "name,"
                "fName,"
                "mName,"
                "medicalPolicy,"
                "birthday,"
                "address,"
                "telephone,"
                "email,"
                "comment) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    qry.addBindValue(last_id + 1);
    qry.addBindValue(Enums::IDX_WRITE);
    qry.addBindValue(ui->editIDNP->text());
    qry.addBindValue(_name);
    qry.addBindValue(_fName);
    qry.addBindValue(QVariant());
    qry.addBindValue((ui->editPolicyMedical->text().isEmpty()) ? QVariant() : ui->editPolicyMedical->text());
    qry.addBindValue(ui->dateEditBirthday->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->editAddress->text());
    qry.addBindValue((ui->editPhone->text().isEmpty()) ? QVariant() : ui->editPhone->text());
    qry.addBindValue((ui->editEmail->text().isEmpty()) ? QVariant() : ui->editEmail->text());
    qry.addBindValue(QVariant());
    if (qry.exec()){
        db->getDatabase().commit();
        return true;
    } else {
        db->getDatabase().rollback();
        details_error = qry.lastError().text();
        return false;
    }
}

bool DocOrderEcho::updateDataTablePacients(const QString _name, const QString _fName, QString &details_error)
{
    db->getDatabase().transaction();
    QSqlQuery qry;
    qry.prepare("UPDATE pacients SET "
                "deletionMark  = :deletionMark,"
                "IDNP          = :IDNP,"
                "name          = :name,"
                "fName         = :fName,"
                "mName         = :mName,"
                "medicalPolicy = :medicalPolicy,"
                "birthday      = :birthday,"
                "address       = :address,"
                "telephone     = :telephone,"
                "email         = :email,"
                "comment       = :comment "
                "WHERE id = :id;");
    qry.bindValue(":id",           m_idPacient);
    qry.bindValue(":deletionMark", Enums::IDX_WRITE);
    qry.bindValue(":IDNP",         ui->editIDNP->text());
    qry.bindValue(":name",         _name);
    qry.bindValue(":fName",        _fName);
    qry.bindValue(":mName",        QVariant());
    qry.bindValue(":medicalPolicy",(ui->editPolicyMedical->text().isEmpty()) ? QVariant() : ui->editPolicyMedical->text());
    qry.bindValue(":birthday",     ui->dateEditBirthday->date().toString("yyyy-MM-dd"));
    qry.bindValue(":address",      ui->editAddress->text());
    qry.bindValue(":telephone",    (ui->editPhone->text().isEmpty()) ? QVariant() : ui->editPhone->text());
    qry.bindValue(":email",        (ui->editEmail->text().isEmpty()) ? QVariant() : ui->editEmail->text());
    qry.bindValue(":comment",      QVariant());
    if (qry.exec()){
        db->getDatabase().commit();
        return true;
    } else {
        db->getDatabase().rollback();
        details_error = qry.lastError().text();
        return false;
    }
}

bool DocOrderEcho::insertDataTableOrderEcho(QString details_error)
{
    int m_payment = getValuePaymentOrder();
    if (m_payment == Enums::IDX_UNKNOW){
        details_error = tr("Nu a fost determinat statutul achitarii documentului.");
        return false;
    }

    db->getDatabase().transaction();
    QSqlQuery qry;
    qry.prepare("INSERT INTO orderEcho ("
                "id,"
                "deletionMark, "
                "numberDoc,"
                "dateDoc,"
                "id_organizations,"
                "id_contracts,"
                "id_typesPrices,"
                "id_doctors,"
                "id_doctors_execute,"
                "id_nurses,"
                "id_pacients,"
                "id_users,"
                "sum,"
                "comment,"
                "cardPayment,"
                "attachedImages) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    qry.addBindValue(m_id);
    qry.addBindValue(m_post);
    qry.addBindValue(ui->editNumberDoc->text());
    qry.addBindValue(ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    qry.addBindValue(m_idOrganization);
    qry.addBindValue(m_idContract);
    qry.addBindValue(m_idTypePrice);
    qry.addBindValue((m_idDoctor == Enums::IDX_UNKNOW || m_idDoctor == 0) ? QVariant() : m_idDoctor);
    qry.addBindValue((m_idDoctor_execute == Enums::IDX_UNKNOW || m_idDoctor_execute == 0) ? QVariant() : m_idDoctor_execute);
    qry.addBindValue((m_idNurse == Enums::IDX_UNKNOW || m_idNurse == 0) ? QVariant() : m_idNurse);
    qry.addBindValue(m_idPacient);
    qry.addBindValue(m_idUser);
    qry.addBindValue(sumOrder);
    qry.addBindValue((ui->editComment->toPlainText().isEmpty()) ? QVariant() : ui->editComment->toPlainText());
    qry.addBindValue(m_payment);
    qry.addBindValue(m_attachedImages);
    if (qry.exec()){
        db->getDatabase().commit();
        return true;
    } else {
        details_error = qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }
}

bool DocOrderEcho::updateDataTableOrderEcho(QString details_error)
{
    int m_payment = getValuePaymentOrder();
    if (m_payment == Enums::IDX_UNKNOW){
        details_error = tr("Nu a fost determinat statutul achitarii documentului.");
        return false;
    }

    db->getDatabase().transaction();

    QSqlQuery qry;
    qry.prepare("UPDATE orderEcho SET "
                "deletionMark       = :deletionMark, "
                "numberDoc          = :numberDoc,"
                "dateDoc            = :dateDoc,"
                "id_organizations   = :id_organizations,"
                "id_contracts       = :id_contracts,"
                "id_typesPrices     = :id_typesPrices,"
                "id_doctors         = :id_doctors,"
                "id_doctors_execute = :id_doctors_execute,"
                "id_nurses          = :id_nurses,"
                "id_pacients        = :id_pacients,"
                "id_users           = :id_users,"
                "sum                = :sum,"
                "comment            = :comment,"
                "cardPayment        = :cardPayment,"
                "attachedImages     = :attachedImages "
                "WHERE id = :id;");
    qry.bindValue(":id", m_id);
    qry.bindValue(":deletionMark",       m_post);
    qry.bindValue(":numberDoc",          ui->editNumberDoc->text());
    qry.bindValue(":dateDoc",            ui->dateTimeDoc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    qry.bindValue(":id_organizations",   m_idOrganization);
    qry.bindValue(":id_contracts",       m_idContract);
    qry.bindValue(":id_typesPrices",     m_idTypePrice);
    qry.bindValue(":id_doctors",         (m_idDoctor == Enums::IDX_UNKNOW || m_idDoctor == 0) ? QVariant() : m_idDoctor);
    qry.bindValue(":id_doctors_execute", (m_idDoctor_execute == Enums::IDX_UNKNOW || m_idDoctor_execute == 0) ? QVariant() : m_idDoctor_execute);
    qry.bindValue(":id_nurses",          (m_idNurse == Enums::IDX_UNKNOW || m_idNurse == 0) ? QVariant() : m_idNurse);
    qry.bindValue(":id_pacients",        m_idPacient);
    qry.bindValue(":id_users",           m_idUser);
    qry.bindValue(":sum",                sumOrder);
    qry.bindValue(":comment",            (ui->editComment->toPlainText().isEmpty()) ? QVariant() : ui->editComment->toPlainText());
    qry.bindValue(":cardPayment",        m_payment);
    qry.bindValue(":attachedImages",     m_attachedImages);
    if (qry.exec()){
        db->getDatabase().commit();
        return true;
    } else {
        details_error = qry.lastError().text();
        db->getDatabase().rollback();
        return false;
    }
}

QString DocOrderEcho::existPacientByIDNP() const
{
    return QString("SELECT name,fName,mName,birthday "
                   "FROM pacients "
                   "WHERE IDNP = '%1' AND deletionMark = '0';").arg(ui->editIDNP->text());
}

void DocOrderEcho::updateModelDoctors()
{
    if (modelDoctors->rowCount() > 0)
        modelDoctors->clear();

    QString strQryDoctors = "SELECT doctors.id, fullNameDoctors.name FROM doctors "
                            "INNER JOIN fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors "
                            "WHERE deletionMark = 0 ORDER BY fullNameDoctors.name;";
    modelDoctors->setQuery(strQryDoctors);
}

void DocOrderEcho::updateTableSources()
{
    if (modelTableSource->rowCount() > 0)
        modelTableSource->clear();
    modelTableSource->setTable("pricingsTable");
    modelTableSource->setFilter(QString("id_pricings=%1").arg(lastIdPricings));
    modelTableSource->setSort(Enums::PRICING_COLUMN::PRICING_COLUMN_COD, Qt::AscendingOrder);
    modelTableSource->setEditStrategy(QSqlTableModel::OnManualSubmit);
    proxy->setSourceModel(modelTableSource);
    ui->tableView->setModel(proxy);
    modelTableSource->select();
    ui->tableView->selectRow(0);

    ui->tableView->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_ID);           // id
    ui->tableView->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_DEL_MARK); // deletionMark
    ui->tableView->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_ID_PRICINGS);   // id_pricings
    ui->tableView->horizontalHeader()->setStretchLastSection(true);         // extinderea ultimei sectiei
    ui->tableView->setSortingEnabled(true);                                 // setam posibilitatea sortarii
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);     // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // setam multipla selectie
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);     // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);                            // initializam meniu contextual
    ui->tableView->horizontalHeader()->setSortIndicator(3, Qt::SortOrder::AscendingOrder); // sortarea dupa 3 sectie(Cod)
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->setColumnWidth(Enums::PRICING_COLUMN::PRICING_COLUMN_COD, 70);   // codul
    ui->tableView->setColumnWidth(Enums::PRICING_COLUMN::PRICING_COLUMN_NAME, 500); // denuimirea investigatiei
    updateHeaderTableSource();
}

void DocOrderEcho::updateTableOrder()
{
    if (modelTableOrder->rowCount() > 0)
        modelTableOrder->clear();
    modelTableOrder->setTable("orderEchoTable");
    modelTableOrder->setFilter(QString("id_orderEcho=%1").arg(m_id));
    modelTableOrder->setSort(Enums::PRICING_COLUMN::PRICING_COLUMN_COD, Qt::AscendingOrder);
    modelTableOrder->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->tableViewOrder->setModel(modelTableOrder);
    modelTableOrder->select();
    ui->tableViewOrder->selectRow(0);

    ui->tableViewOrder->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_ID);           // id
    ui->tableViewOrder->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_DEL_MARK); // deletionMark
    ui->tableViewOrder->hideColumn(Enums::PRICING_COLUMN::PRICING_COLUMN_ID_PRICINGS);   // id_pricings
    ui->tableViewOrder->horizontalHeader()->setStretchLastSection(true);         // extinderea ultimei sectiei
//    ui->tableViewOrder->setSortingEnabled(true);                                 // setam posibilitatea sortarii
    ui->tableViewOrder->setSelectionBehavior(QAbstractItemView::SelectRows);     // setam alegerea randului
    ui->tableViewOrder->setSelectionMode(QAbstractItemView::ExtendedSelection);  // setam multipla selectie
    ui->tableViewOrder->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);     // permitem schimbarea size sectiilor
    ui->tableViewOrder->setContextMenuPolicy(Qt::CustomContextMenu);                            // initializam meniu contextual
    ui->tableViewOrder->horizontalHeader()->setSortIndicator(3, Qt::SortOrder::AscendingOrder); // sortarea dupa 3 sectie(Cod)
    ui->tableViewOrder->verticalHeader()->setDefaultSectionSize(16);
    ui->tableViewOrder->setColumnWidth(Enums::PRICING_COLUMN::PRICING_COLUMN_COD, 70);   // codul
    ui->tableViewOrder->setColumnWidth(Enums::PRICING_COLUMN::PRICING_COLUMN_NAME, 500); // denuimirea investigatiei
    updateHeaderTableOrder();
}

void DocOrderEcho::updateHeaderTableSource()
{
    QStringList _headers;
    _headers << tr("")    // id
             << tr("")    // deletionMark
             << tr("")    // id_pricings
             << tr("Cod")
             << tr("Denumirea investigației")
             << tr("Costul");
    for (int n = 0, m = 0; n < modelTableSource->columnCount(); n++, m++) {
        modelTableSource->setHeaderData(n, Qt::Horizontal, _headers[m]);
    }
}

void DocOrderEcho::updateHeaderTableOrder()
{
    QStringList _headers;
    _headers << tr("")    // id
             << tr("")    // deletionMark
             << tr("")    // id_pricings
             << tr("Cod")
             << tr("Denumirea investigației")
             << tr("Costul");
    for (int n = 0, m = 0; n < modelTableOrder->columnCount(); n++, m++) {
        modelTableOrder->setHeaderData(n, Qt::Horizontal, _headers[m]);
    }
}

QMap<QString, QString> DocOrderEcho::getItemsByTableOrganization()
{
    QMap<QString, QString> items;

    QString strQuery = QString("SELECT constants.id_organizations, "
                               "organizations.IDNP, "
                               "organizations.name, "
                               "organizations.address, "
                               "organizations.telephone, "
                               "fullNameDoctors.nameAbbreviated AS doctor, "
                               "organizations.email FROM constants "
                               "INNER JOIN organizations ON constants.id_organizations = organizations.id "
                               "INNER JOIN fullNameDoctors ON constants.id_doctors = fullNameDoctors.id_doctors "
                               "WHERE constants.id_users = %1;").arg(m_idUser);
    if(! db->getDataFromQueryByRecord(strQuery, items)){
        qCritical(logCritical()) << "getItemsOrganization() - "  << tr("Solicitarea nu a fost executata.");
    }
    return items;
}

QMap<QString, QString> DocOrderEcho::getItemsByTablePacient()
{
    QString strQryPacient = QString("SELECT IDNP,"
                                    "name,"
                                    "fName,"
                                    "mName,"
                                    "medicalPolicy,"
                                    "birthday,"
                                    "address "
                                    "FROM pacients WHERE id = '%1' AND deletionMark = '0';").arg(QString::number(m_idPacient));
     QMap<QString, QString> items;
     if(! db->getDataFromQueryByRecord(strQryPacient, items)){
         qCritical(logCritical()) << "getItemsByPacient() - "  << tr("Solicitarea nu a fost executata.");
     }
     return items;
}

void DocOrderEcho::setImageForDocPrint()
{
    //------------------------------------------------------------------------------------------------------
    // determinam id organizatiei principale
    int id_organization = 0;
    int id_doctor = 0;

    QMap<QString, QString> items;
    if (db->getObjectDataByMainId("constants", "id_users", globals::idUserApp, items)){
         id_organization = items.constFind("id_organizations").value().toInt();
         if (m_idDoctor_execute == Enums::IDX_UNKNOW)
            id_doctor = items.constFind("id_doctors").value().toInt();
         else
            id_doctor = m_idDoctor_execute;
    }

    //------------------------------------------------------------------------------------------------------
    // logotipul
    QPixmap pix_logo = QPixmap();
    QStandardItem* img_item_logo = new QStandardItem();
    QString name_key_logo = "logo_" + globals::nameUserApp;
    if (globals::cache_img.find(name_key_logo, &pix_logo)){
         QImage m_logo = pix_logo.toImage();
         img_item_logo->setData(m_logo.scaled(300,50), Qt::DisplayRole);
         exist_logo = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
    } else {
         QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", globals::idUserApp);
         if (! outByteArray.isEmpty() && pix_logo.loadFromData(outByteArray)){
            globals::cache_img.insert(name_key_logo, pix_logo);
            QImage m_logo = pix_logo.toImage();
            img_item_logo->setData(m_logo.scaled(300,50), Qt::DisplayRole);
            exist_logo = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
         }
    }

    //------------------------------------------------------------------------------------------------------
    // stampila organizatiei
    QPixmap pix_stamp = QPixmap();
    QStandardItem* img_item_stamp = new QStandardItem();
    QString name_key_stamp = "stamp_organization_id-" + QString::number(id_organization) + "_" + globals::nameUserApp;
    if (globals::cache_img.find(name_key_stamp, &pix_stamp)){
         QImage m_stamp = pix_stamp.toImage();
         img_item_stamp->setData(m_stamp.scaled(200,200), Qt::DisplayRole);
         exist_stamp = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
    } else {
         QByteArray outByteArray = db->getOutByteArrayImage("organizations", "stamp", "id", id_organization);
         if (! outByteArray.isEmpty() && pix_stamp.loadFromData(outByteArray)){
            globals::cache_img.insert(name_key_stamp, pix_stamp);
            QImage m_stamp = pix_stamp.toImage();
            img_item_stamp->setData(m_stamp.scaled(200,200), Qt::DisplayRole);
            exist_stamp = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
         }
    }

    //------------------------------------------------------------------------------------------------------
    // semnatura doctorului
    QPixmap pix_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    QString name_key_signature = "signature_doctor_id-" + QString::number(id_doctor) + "_" + globals::nameUserApp;
    if (globals::cache_img.find(name_key_signature, &pix_signature)){
         QImage m_signature = pix_signature.toImage();
         img_item_signature->setData(m_signature.scaled(200,200), Qt::DisplayRole);
         exist_signature = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
    } else {
         QByteArray outByteArray = db->getOutByteArrayImage("doctors", "signature", "id", id_doctor);
         if (! outByteArray.isEmpty() && pix_signature.loadFromData(outByteArray)){
            globals::cache_img.insert(name_key_signature, pix_signature);
            QImage m_signature = pix_signature.toImage();
            img_item_signature->setData(m_signature.scaled(200,200), Qt::DisplayRole);
            exist_signature = 1; // setam variabila pu prezentare in forma de tipar (0-hide, 1-show)
         }
    }

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp);
    items_img.append(img_item_signature);

    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(3);
    model_img->appendRow(items_img);
}

QStringList DocOrderEcho::loadDataFromXml(const QString &filePath, const QString &tagName)
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

void DocOrderEcho::initCompleterAddressPatients()
{
    cityList = loadDataFromXml(":/xmls/city.xml", "city");

    city_completer = new QCompleter(cityList, this);
    city_completer->setCaseSensitivity(Qt::CaseInsensitive);
    city_completer->setCompletionMode(QCompleter::PopupCompletion);

    connect(ui->editAddress, &QLineEdit::textChanged, this, &DocOrderEcho::handleCompleterAddressPatients);
}

// **********************************************************************************
// --- evenimente formei

void DocOrderEcho::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            onWritingDataClose();
            event->accept();
        } else if (messange_box.clickedButton() == noButton) {
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton) {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void DocOrderEcho::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        setTitleDoc();
        updateHeaderTableSource();
        updateHeaderTableOrder();
    }
}

void DocOrderEcho::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        if (ui->tableViewOrder->focusWidget()){
            int _row = ui->tableViewOrder->currentIndex().row();
            QModelIndex indexEdit = modelTableOrder->index(_row, Enums::PRICING_COLUMN::PRICING_COLUMN_PRICE);
            onClickedRowTableOrder(indexEdit);
            ui->tableViewOrder->clearFocus();
        } else {
            this->focusNextChild();
        }

        if (ui->comboOrganization->hasFocus()){
            ui->comboOrganization->showPopup();
        }

        if (ui->comboContract->hasFocus() && ui->comboDoctor->currentIndex() == 0){
            ui->comboDoctor->setFocus();
            ui->comboDoctor->showPopup();
        }

        if (ui->comboDoctor->hasFocus())
            ui->comboDoctor->showPopup();

    }

    if (event->key() == Qt::Key_F5){
        ui->toolBox->setCurrentIndex(box_patient);
        ui->comboPacient->setFocus();        
    }
}
