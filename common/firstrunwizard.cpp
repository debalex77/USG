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
    checkbox_delegate    = new CheckBoxDelegate(this);
    gr_investig_delegate = new ComboDelegate("SELECT id,name FROM investigationsGroup;", this);

    QString str_users = R"(
        SELECT id, deletionMark, name
        FROM users ORDER BY name;
    )";
    model_users = new BaseSqlQueryModel(str_users, this);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);

    // setam lista de pași
    setListStep();

    // setam pagina inițială
    currentStepIndex = 0;
    ui->stackedWidget->setCurrentIndex(currentStepIndex);

    // conectăm semnalele la sloturi
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
    model_investigations->setHeaderData(4, Qt::Horizontal, "Utilizare");
    model_investigations->setHeaderData(5, Qt::Horizontal, "Grupa");

    ui->tableView->setModel(model_investigations);

    ui->tableView->setColumnWidth(1, 25);
    ui->tableView->setColumnHidden(0, true); // ascundem id
    ui->tableView->setColumnWidth(2, 70);
    ui->tableView->setColumnWidth(3, 400); // denuimirea investigatiei
    ui->tableView->setColumnWidth(4, 80); // denuimirea investigatiei

    model_investigations->select();

    ui->tableView->setItemDelegateForColumn(4, checkbox_delegate);
    ui->tableView->setItemDelegateForColumn(5, gr_investig_delegate);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    if (globals().firstLaunch)
        ui->progressBar->setValue(0);
    else
        ui->progressBar->setValue(100);
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
    ui->tableView_typePrices->setItemDelegateForColumn(4, checkbox_delegate);
    ui->tableView_typePrices->horizontalHeader()->setStretchLastSection(true);

    model_typePrices->select();
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
    ui->tableView_users->setFocus();                                                         // focusam la tabela
    ui->tableView_users->selectRow(0);                                                       // selectam primul rand
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
