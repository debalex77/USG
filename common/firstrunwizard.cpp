#include "firstrunwizard.h"
#include "ui_firstrunwizard.h"

FirstRunWizard::FirstRunWizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FirstRunWizard)
{
    ui->setupUi(this);

    // setam titlul ferestrei
    setWindowTitle(tr("Asistent de configurare inițială"));

    // alocam memoria pentru modelul listei
    db = new DataBase(this);
    model = new QStringListModel(this);
    model_investigations = new BaseSqlTableModel(this);
    model_typePrices     = new BaseSqlTableModel(this);

    QString str_users = R"(SELECT id, deletionMark, name FROM users ORDER BY name)";
    model_users = new BaseSqlQueryModel(str_users, this);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_doctors = R"(
        SELECT
            doctors.id,
            doctors.deletionMark,
            fullNameDoctors.name AS FullName,
            doctors.telephone,
            doctors.email,
            doctors.comment
        FROM
            doctors
        INNER JOIN
            fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors
        ORDER BY
            fullNameDoctors.name;
    )";
    model_doctors = new BaseSqlQueryModel(str_doctors, this);
    model_doctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_nurses = R"(
        SELECT
            nurses.id,
            nurses.deletionMark,
            fullNameNurses.name AS FullName,
            nurses.telephone,
            nurses.email,
            nurses.comment
        FROM
            nurses
        INNER JOIN
            fullNameNurses ON nurses.id = fullNameNurses.id_nurses
        ORDER BY
            fullNameNurses.name;
    )";
    model_nurses = new BaseSqlQueryModel(str_nurses, this);
    model_nurses->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    QString str_organizations = R"(
        SELECT
            id,
            deletionMark,
            name,
            IDNP,
            address,
            telephone,
            email,
            comment
        FROM
            organizations
        ORDER BY
            name;
    )";
    model_organizations = new BaseSqlQueryModel(str_organizations, this);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    // setam lista de pași
    setListStep();

    // setam pagina inițială
    currentStepIndex = 0;
    ui->stackedWidget->setCurrentIndex(currentStepIndex);

    // conectăm semnalele la sloturi
    initBtnToolBar();
    initConnections();

    // actualizăm aspectul listei (pasul activ)
    updateTextListStep();

    ui->splitter->setSizes({240, 9999});

    ui->frame->setStyleSheet(R"(
        QFrame {
            background-color: #2b2b2b;
            border: 1px solid #444444;
            border-radius: 4px;
        }
        QFrame > * {
            background-color: transparent;
            color: white;
        }
    )");
    // ui->frame_2->setStyleSheet(R"(
    //     QFrame {
    //         background-color: #2b2b2b;
    //         border: 1px solid #444444;
    //         border-radius: 4px;
    //     }
    //     QFrame > * {
    //         background-color: transparent;
    //         color: white;
    //     }
    // )");

}

FirstRunWizard::~FirstRunWizard()
{
    delete ui;
}

void FirstRunWizard::initBtnToolBar()
{
    QString style_btnToolBar = db->getStyleForButtonToolBar();
    ui->btnAdd_users->setStyleSheet(style_btnToolBar);
    ui->btnEdit_users->setStyleSheet(style_btnToolBar);
    ui->btnRemove_users->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_users, &QToolButton::clicked, this, &FirstRunWizard::addCatUser);
    connect(ui->btnEdit_users, &QToolButton::clicked, this, &FirstRunWizard::editCatUser);
    connect(ui->btnRemove_users, &QToolButton::clicked, this, &FirstRunWizard::removeCatUser);

    ui->btnAdd_doctors->setStyleSheet(style_btnToolBar);
    ui->btnEdit_doctors->setStyleSheet(style_btnToolBar);
    ui->btnRemove_doctors->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_doctors, &QToolButton::clicked, this, &FirstRunWizard::addCatDoctor);
    connect(ui->btnEdit_doctors, &QToolButton::clicked, this, &FirstRunWizard::editCatDoctor);
    connect(ui->btnRemove_doctors, &QToolButton::clicked, this, &FirstRunWizard::removeCatDoctor);

    ui->btnAdd_nurses->setStyleSheet(style_btnToolBar);
    ui->btnEdit_nurses->setStyleSheet(style_btnToolBar);
    ui->btnRemove_nurses->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_nurses, &QToolButton::clicked, this, &FirstRunWizard::addCatNurse);
    connect(ui->btnEdit_nurses, &QToolButton::clicked, this, &FirstRunWizard::editCatNurse);
    connect(ui->btnRemove_nurses, &QToolButton::clicked, this, &FirstRunWizard::removeCatNurse);

    ui->btnAdd_organizations->setStyleSheet(style_btnToolBar);
    ui->btnEdit_organizations->setStyleSheet(style_btnToolBar);
    ui->btnRemove_organizations->setStyleSheet(style_btnToolBar);

    connect(ui->btnAdd_organizations, &QToolButton::clicked, this, &FirstRunWizard::addCatOrganization);
    connect(ui->btnEdit_organizations, &QToolButton::clicked, this, &FirstRunWizard::editCatOrganization);
    connect(ui->btnRemove_organizations, &QToolButton::clicked, this, &FirstRunWizard::removeCatOrganization);

}

void FirstRunWizard::initConnections()
{
    connect(ui->btnLoadInvestigations, &QPushButton::clicked, this, &FirstRunWizard::loadInvestigations);
    connect(ui->btnLoadTypePrices, &QPushButton::clicked, this, &FirstRunWizard::loadTypePrices);

    connect(ui->btnBack, &QPushButton::clicked, this, &FirstRunWizard::clickedBackPage);
    connect(ui->btnNext, &QPushButton::clicked, this, &FirstRunWizard::clickedNextPage);
    connect(ui->btnClose, &QPushButton::clicked, this, &FirstRunWizard::close);
}

void FirstRunWizard::setListStep()
{
    QStringList listStep {
        tr("1. Inițierea"),
        tr("2. Clasificator \"Investigații\"."),
        tr("3. Tipul prețurilor"),
        tr("4. Utilizatori"),
        tr("5. Doctori"),
        tr("6. As.medicale"),
        tr("7. Organizații/centre medicale")
    };

    model->setStringList(listStep);
    ui->listView->setModel(model);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setFocusPolicy(Qt::NoFocus);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setStyleSheet(R"(
        QListView {
            background-color: transparent;
            border: none;
        }
        QListView::item {
            color: white;
            padding: 6px;
            font-size: 14px;
        }
        QListView::item:selected {
            font-weight: bold;
            text-decoration: underline;
            background-color: transparent;
            color: #00BFFF;
        }
    )");
}

void FirstRunWizard::updateTextListStep()
{
    QModelIndex index = model->index(currentStepIndex);
    ui->listView->setCurrentIndex(index);
    ui->listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
}

void FirstRunWizard::updateTableInvestigations()
{
    if (ui->stackedWidget->currentIndex() != page_classifiers)
        return;

    model_investigations->setTable("investigations");
    model_investigations->setSort(0, Qt::AscendingOrder);

    model_investigations->setHeaderData(0, Qt::Horizontal, "ID");
    model_investigations->setHeaderData(1, Qt::Horizontal, "");
    model_investigations->setHeaderData(2, Qt::Horizontal, "Cod MS");
    model_investigations->setHeaderData(3, Qt::Horizontal, "Denumirea investigatiei");

    ui->tableView->setModel(model_investigations);

    ui->tableView->setColumnWidth(1, 25);
    ui->tableView->setColumnHidden(0, true); // ascundem id
    ui->tableView->setColumnWidth(2, 70);
    ui->tableView->setColumnWidth(3, 400); // denuimirea investigatiei

    model_investigations->select();

    ui->tableView->setColumnHidden(4, true);
    ui->tableView->setColumnHidden(5, true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    if (globals().firstLaunch)
        ui->progressBar->setValue(0);
    else
        ui->progressBar->setValue(100);

    if (model_investigations->rowCount() > 0) {
        ui->tableView->setFocus();   // focusam la tabela
        ui->tableView->selectRow(0); // selectam 1 rand
    }
}

void FirstRunWizard::updateTableTypePrices()
{
    if (ui->stackedWidget->currentIndex() != page_typePrices)
        return;

    model_typePrices->setTable("typesPrices");
    model_typePrices->setSort(0, Qt::AscendingOrder);

    model_typePrices->setHeaderData(0, Qt::Horizontal, "ID");
    model_typePrices->setHeaderData(1, Qt::Horizontal, "");
    model_typePrices->setHeaderData(2, Qt::Horizontal, "Denumirea pretului");
    model_typePrices->setHeaderData(3, Qt::Horizontal, "Reducere (%)");
    model_typePrices->setHeaderData(4, Qt::Horizontal, "Noncomercial");

    ui->tableView_typePrices->setModel(model_typePrices);

    ui->tableView_typePrices->setColumnWidth(1, 25);
    ui->tableView_typePrices->setColumnHidden(0, true); // ascundem id
    ui->tableView_typePrices->setColumnWidth(2, 450);
    ui->tableView_typePrices->setColumnHidden(3, true);
    ui->tableView_typePrices->setColumnHidden(4, true);
    ui->tableView_typePrices->horizontalHeader()->setStretchLastSection(true);

    model_typePrices->select();

    if (model_typePrices->rowCount() > 0) {
        ui->tableView_typePrices->setFocus();   // focusam la tabela
        ui->tableView_typePrices->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableUsers()
{
    if (ui->stackedWidget->currentIndex() != page_users)
        return;

    model_users->setHeaderData(1, Qt::Horizontal, "");
    model_users->setHeaderData(2, Qt::Horizontal, "Nume utilizatorilor");

    ui->tableView_users->setModel(model_users);
    ui->tableView_users->hideColumn(0);                                                      // id-ascundem
    ui->tableView_users->setColumnWidth(1, 25);
    ui->tableView_users->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_users->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_users->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_users->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_users->rowCount() > 0) {
        ui->tableView_users->setFocus();   // focusam la tabela
        ui->tableView_users->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableDoctors()
{
    if (ui->stackedWidget->currentIndex() != page_doctors)
        return;

    model_doctors->setHeaderData(1, Qt::Horizontal, "");
    model_doctors->setHeaderData(2, Qt::Horizontal, tr("NPP doctorului"));
    model_doctors->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
    model_doctors->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
    model_doctors->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_doctors->setModel(model_doctors);
    ui->tableView_doctors->hideColumn(0);                                                      // id-ascundem
    ui->tableView_doctors->setColumnWidth(1, 25);
    ui->tableView_doctors->setColumnWidth(2, 160);
    ui->tableView_doctors->setColumnWidth(3, 90);
    ui->tableView_doctors->setColumnWidth(4, 160);
    ui->tableView_doctors->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_doctors->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_doctors->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_doctors->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_doctors->rowCount() > 0) {
        ui->tableView_doctors->setFocus();   // focusam la tabela
        ui->tableView_doctors->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableNurses()
{
    if (ui->stackedWidget->currentIndex() != page_nurses)
        return;

    model_nurses->setHeaderData(1, Qt::Horizontal, "");
    model_nurses->setHeaderData(2, Qt::Horizontal, tr("NPP as.medicale"));
    model_nurses->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
    model_nurses->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
    model_nurses->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_nurses->setModel(model_nurses);
    ui->tableView_nurses->hideColumn(0);                                                      // id-ascundem
    ui->tableView_nurses->setColumnWidth(1, 25);
    ui->tableView_nurses->setColumnWidth(2, 160);
    ui->tableView_nurses->setColumnWidth(3, 90);
    ui->tableView_nurses->setColumnWidth(4, 160);
    ui->tableView_nurses->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_nurses->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_nurses->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_nurses->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_nurses->rowCount() > 0) {
        ui->tableView_nurses->setFocus();   // focusam la tabela
        ui->tableView_nurses->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::updateTableOrganizations()
{
    if (ui->stackedWidget->currentIndex() != page_organizations)
        return;

    model_organizations->setHeaderData(1, Qt::Horizontal, "");
    model_organizations->setHeaderData(2, Qt::Horizontal, tr("Denumirea organizației"));
    model_organizations->setHeaderData(3, Qt::Horizontal, tr("IDNP"));
    model_organizations->setHeaderData(4, Qt::Horizontal, tr("Adresa"));
    model_organizations->setHeaderData(5, Qt::Horizontal, tr("Telefoane"));
    model_organizations->setHeaderData(6, Qt::Horizontal, tr("E-mail"));
    model_organizations->setHeaderData(7, Qt::Horizontal, tr("Comentariu"));

    ui->tableView_organizations->setModel(model_organizations);
    ui->tableView_organizations->hideColumn(0);                                                      // id-ascundem
    ui->tableView_organizations->setColumnWidth(1, 25);
    ui->tableView_organizations->setColumnWidth(2, 200);
    ui->tableView_organizations->setColumnWidth(3, 120);
    ui->tableView_organizations->setColumnWidth(4, 240);
    ui->tableView_organizations->setColumnWidth(5, 230);
    ui->tableView_organizations->setColumnWidth(6, 216);
    ui->tableView_organizations->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tableView_organizations->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tableView_organizations->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView_organizations->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei

    if (model_organizations->rowCount() > 0) {
        ui->tableView_organizations->setFocus();   // focusam la tabela
        ui->tableView_organizations->selectRow(0); // selectam primul rand
    }
}

void FirstRunWizard::clickedBackPage()
{
    if (currentStepIndex > 0) {
        currentStepIndex--;
        ui->stackedWidget->setCurrentIndex(currentStepIndex);
        updateTextListStep();
        updateTableInvestigations();
        updateTableTypePrices();
        updateTableUsers();
        updateTableDoctors();
        updateTableNurses();
        updateTableOrganizations();
    }
}

void FirstRunWizard::clickedNextPage()
{
    if (currentStepIndex < ui->stackedWidget->count() - 1) {
        currentStepIndex++;
        ui->stackedWidget->setCurrentIndex(currentStepIndex);
        updateTextListStep();
        updateTableInvestigations();
        updateTableTypePrices();
        updateTableUsers();
        updateTableDoctors();
        updateTableNurses();
        updateTableOrganizations();
    }
}

void FirstRunWizard::loadInvestigations()
{
    if (! globals().firstLaunch)
        return;

    db->updateInvestigationFromXML_2024();
    connect(db, &DataBase::updateProgress, this, &FirstRunWizard::handleUpdateProgress);
}

void FirstRunWizard::loadTypePrices()
{
    if (! globals().firstLaunch)
        return;

    db->insertDataForTabletypesPrices();
}

void FirstRunWizard::handleUpdateProgress(int num_records, int value)
{
    if (num_records <= 0)
        return;

    // Calculează progresul
    int value_progress = (value * 100) / num_records;

    ui->progressBar->setValue(value_progress);
}

void FirstRunWizard::addCatUser()
{
    CatUsers* cat_user = new CatUsers(this);
    cat_user->setProperty("itNew", true);
    cat_user->exec();
}

void FirstRunWizard::editCatUser()
{
    if (ui->tableView_users->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_users->currentIndex().row();
    int id_user = model_users->data(model_users->index(row, 0), Qt::DisplayRole).toInt();

    CatUsers* cat_user = new CatUsers(this);
    cat_user->setProperty("itNew", false);
    cat_user->setProperty("Id", id_user);
    cat_user->exec();
}

void FirstRunWizard::removeCatUser()
{

}

void FirstRunWizard::addCatDoctor()
{
    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", true);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    cat_General->exec();
}

void FirstRunWizard::editCatDoctor()
{
    if (ui->tableView_doctors->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_doctors->currentIndex().row();
    int id_doctor = model_doctors->data(model_doctors->index(row, 0), Qt::DisplayRole).toInt();

    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", false);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    cat_General->setProperty("Id", id_doctor);
    cat_General->exec();
}

void FirstRunWizard::removeCatDoctor()
{

}

void FirstRunWizard::addCatNurse()
{
    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", true);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
    cat_General->exec();
}

void FirstRunWizard::editCatNurse()
{
    if (ui->tableView_nurses->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_nurses->currentIndex().row();
    int id_nurse = model_nurses->data(model_nurses->index(row, 0), Qt::DisplayRole).toInt();

    CatGeneral *cat_General = new CatGeneral(this);
    cat_General->setProperty("itNew", false);
    cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
    cat_General->setProperty("Id", id_nurse);
    cat_General->exec();
}

void FirstRunWizard::removeCatNurse()
{

}

void FirstRunWizard::addCatOrganization()
{
    CatOrganizations *catOrganization = new CatOrganizations(this);
    catOrganization->setProperty("itNew", true);
    catOrganization->exec();
}

void FirstRunWizard::editCatOrganization()
{
    if (ui->tableView_organizations->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    auto row = ui->tableView_organizations->currentIndex().row();
    int id_organization = model_organizations->data(model_organizations->index(row, 0), Qt::DisplayRole).toInt();

    CatOrganizations *catOrganization = new CatOrganizations(this);
    catOrganization->setProperty("itNew", false);
    catOrganization->setProperty("Id", id_organization);
    catOrganization->exec();
}

void FirstRunWizard::removeCatOrganization()
{

}
