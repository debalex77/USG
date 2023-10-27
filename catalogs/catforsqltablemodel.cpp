#include "catforsqltablemodel.h"
#include "ui_catforsqltablemodel.h"

#include <QDomDocument>

CatForSqlTableModel::CatForSqlTableModel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatForSqlTableModel)
{
    ui->setupUi(this);

    db      = new DataBase(this);
    menu    = new QMenu(this);
    model   = new BaseSqlTableModel(this);
    toolBar = new QToolBar(this);

    connect(ui->tableView, &QWidget::customContextMenuRequested, this, &CatForSqlTableModel::slotContextMenuRequested);
    connect(this, &CatForSqlTableModel::typeCatalogChanged, this, &CatForSqlTableModel::slot_typeCatalogChanged);
    connect(this, &CatForSqlTableModel::typeFormChanged, this, &CatForSqlTableModel::slot_typeFormChanged);
    connect(model, &QAbstractItemModel::dataChanged, this, &CatForSqlTableModel::onDataChangedItemModel);
}

CatForSqlTableModel::~CatForSqlTableModel()
{
    delete db;
    delete menu;
    delete model;
    if (m_typeForm == TypeForm::SelectForm)
        delete toolBar;
    delete ui;
}

void CatForSqlTableModel::initBtnForm()
{
    ui->btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    ui->btnAdd->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));
    ui->btnAdd->setLayout(new QGridLayout);

    ui->btnMarkDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    ui->btnMarkDeletion->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnMarkDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
    ui->btnMarkDeletion->setLayout(new QGridLayout);

    ui->btnClose->setIcon(QIcon(":/img/close_x32.png"));
    ui->btnClose->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
    ui->btnClose->setLayout(new QGridLayout);

    connect(ui->btnAdd, &QAbstractButton::clicked, this, &CatForSqlTableModel::onAddRowTable);
    connect(ui->btnMarkDeletion, &QAbstractButton::clicked, this, &CatForSqlTableModel::onMarkDeletion);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &CatForSqlTableModel::onClose);
}

void CatForSqlTableModel::initBtnToolBar()
{   
    toolBar->clear();
    btnBarAdd          = new QToolButton(this);
    btnBarEdit         = new QToolButton(this);
    btnBarDeletion     = new QToolButton(this);
    btnBarUpdateTable  = new QToolButton(this);

    btnBarAdd->setIcon(QIcon(":/img/add_x32.png"));
    btnBarAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnBarAdd->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnBarAdd->setShortcut(QKeySequence(Qt::Key_Insert));

    btnBarEdit->setIcon(QIcon(":/img/edit_x32.png"));
    btnBarEdit->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnBarEdit->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnBarEdit->setShortcut(QKeySequence(Qt::Key_F2));

    btnBarDeletion->setIcon(QIcon(":/img/clear_x32.png"));
    btnBarDeletion->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnBarDeletion->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnBarDeletion->setShortcut(QKeySequence(Qt::Key_Delete));

    btnBarUpdateTable->setIcon(QIcon(":/img/update_x32.png"));
    btnBarUpdateTable->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnBarUpdateTable->setStyleSheet("padding-left: 2px; padding-right: 2px; height: 18px; width: 12px;");
    btnBarUpdateTable->setShortcut(QKeySequence(Qt::Key_F5));

    toolBar->addSeparator();
    toolBar->addWidget(btnBarAdd);
    toolBar->addWidget(btnBarEdit);
    toolBar->addWidget(btnBarDeletion);
    toolBar->addWidget(btnBarUpdateTable);
    toolBar->addSeparator();

    QSpacerItem *itemSpacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addItem(itemSpacer);

    connect(btnBarAdd, &QToolButton::clicked, this, &CatForSqlTableModel::onAddRowTable);
    connect(btnBarEdit, &QToolButton::clicked, this, &CatForSqlTableModel::onEditRowTable);
    connect(btnBarDeletion, &QToolButton::clicked, this, &CatForSqlTableModel::onMarkDeletion);
    connect(btnBarUpdateTable, &QToolButton::clicked, this, &CatForSqlTableModel::updateTableView);
}

void CatForSqlTableModel::updateTableView()
{
    switch (m_typeCatalog) {
    case Investigations:
        setWindowTitle(tr("Clasificatorul investigatiilor"));
        model->setTable("investigations");
        updateHeaderTableInvestigations();
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(investig_id, Qt::AscendingOrder);                  // sortarea datelor de la sectia 0
        break;
    case TypesPrices:
        setWindowTitle(tr("Tipul preturilor"));
        model->setTable("typesPrices");
        updateHeaderTableTypesPrices();
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(typePrice_id, Qt::AscendingOrder);                  // sortarea datelor de la sectia 0
        break;
    case ConclusionTemplates:
        setWindowTitle(tr("Șabloane concluziilor"));
        model->setTable("conclusionTemplates");
        if (m_filter_templates != nullptr){
            if (globals::thisMySQL)
                model->setFilter(QString("`system` = '%1'").arg(m_filter_templates));
            else
                model->setFilter(QString("system = '%1'").arg(m_filter_templates));
        }
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(template_name, Qt::AscendingOrder);
        updateHeaderTableConclusionTemplates();
        break;
    default:
        break;
    }

    ui->tableView->setModel(model);                        // setam modelul
    model->select();

    switch (m_typeCatalog) {
    case Investigations:
        ui->tableView->setColumnWidth(investig_deletionMark, 5);
        ui->tableView->setColumnHidden(investig_id, true); // ascundem id
        ui->tableView->setColumnWidth(investig_cod, 70);
        ui->tableView->hideColumn(investig_use);
        ui->tableView->setColumnWidth(investig_cod, 70);   // codul
        ui->tableView->setColumnWidth(investig_name, 500); // denuimirea investigatiei
        break;
    case TypesPrices:
        ui->tableView->setColumnWidth(typePrice_deletionMark, 5);
        ui->tableView->setColumnHidden(typePrice_id, true); // ascundem id
        ui->tableView->setColumnWidth(typePrice_name, 1200);
        break;
    case ConclusionTemplates:
        ui->tableView->setColumnWidth(template_deletionMark, 5);
        ui->tableView->setColumnHidden(template_id, true); // ascundem id
        ui->tableView->setColumnWidth(template_cod, 50);
        ui->tableView->setColumnWidth(template_name, 550);
        break;
    default:
        break;
    }

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);     // initializam meniu contextual
    ui->tableView->horizontalHeader()->setStretchLastSection(true); // largirea ultimei colonite
    ui->tableView->verticalHeader()->setDefaultSectionSize(14);
    if (m_typeForm == TypeForm::SelectForm){
        model->setMainFlag(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers | QAbstractItemView::SelectedClicked);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
                this, &CatForSqlTableModel::onSelectRowTable);
    }
    if (ui->tableView->model()->rowCount() > 0)
        ui->tableView->setCurrentIndex(model->index(0, 2));
}

void CatForSqlTableModel::updateHeaderTableInvestigations()
{
    QStringList _headers;
    _headers << tr("id")
             << tr("") // deletionMark
             << tr("Cod MS")
             << tr("Denumirea investigatiei")
             << tr("Use");
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){ // setam headerul
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }
}

void CatForSqlTableModel::updateHeaderTableTypesPrices()
{
    QStringList _headers;
    _headers << tr("id")
             << tr("") // deletionMark
             << tr("Denumirea pretului")
             << tr("Reducere (%)")
             << tr("Noncomercial");
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){ // setam headerul
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }
}

void CatForSqlTableModel::updateHeaderTableConclusionTemplates()
{
    QStringList _headers;
    _headers << tr("id")
             << tr("") // deletionMark
             << tr("Cod")
             << tr("Concluzia")
             << tr("Sistema");
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){ // setam headerul
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }
}

QString CatForSqlTableModel::getNameTable()
{
    QString nameTable;
    switch (m_typeCatalog) {
    case Investigations:
        nameTable = "investigations";
        break;
    case TypesPrices:
        nameTable = "typesPrices";
        break;
    case ConclusionTemplates:
        nameTable = "conclusionTemplates";
        break;
    }
    return nameTable;
}

void CatForSqlTableModel::onSelectRowTable(const QModelIndex &index)
{
    int _row     = index.row();
    int _id      = model->index(_row, investig_id).data(Qt::DisplayRole).toInt();
    QString _cod = model->index(_row, investig_cod).data(Qt::DisplayRole).toString();
    QString _name = model->index(_row, investig_name).data(Qt::DisplayRole).toString();

    ret_id = _id;
    ret_cod = _cod;
    ret_name = _name;
    emit mSelectData();
}

void CatForSqlTableModel::onAddRowTable()
{
    if (m_typeForm == TypeForm::SelectForm)      // daca forma de selectie
        model->setMainFlag(Qt::ItemIsEditable | Qt::ItemIsEnabled);  // setam flagul pu editare

    int row = model->rowCount();
    model->insertRow(row);
    switch (m_typeCatalog) {
    case Investigations:
        model->setData(model->index(row, investig_id), row + 1);
        model->setData(model->index(row, investig_deletionMark), 0);
        model->setData(model->index(row, investig_use), 1);
        ui->tableView->setCurrentIndex(model->index(row, investig_cod)); // setam indexul curent
        ui->tableView->edit(model->index(row, investig_cod));            // intram in regimul de editare
        break;
    case TypesPrices:
        model->setData(model->index(row, typePrice_id), row + 1);
        model->setData(model->index(row, typePrice_deletionMark), 0);
        ui->tableView->setCurrentIndex(model->index(row, typePrice_name));
        ui->tableView->edit(model->index(row, typePrice_name));
        break;
    case ConclusionTemplates:
        model->setData(model->index(row, template_id), row + 1);
        model->setData(model->index(row, template_deletionMark), 0);
        ui->tableView->setCurrentIndex(model->index(row, template_cod));
        ui->tableView->edit(model->index(row, template_cod));
        break;
    default:
        break;
    }
}

void CatForSqlTableModel::onEditRowTable()
{
    if (m_typeForm == TypeForm::SelectForm)      // daca forma de selectie
        model->setMainFlag(Qt::ItemIsEditable | Qt::ItemIsEnabled);  // setam flagul pu editare

    QModelIndex _index = ui->tableView->currentIndex();
    if (! _index.isValid())
        return;
    switch (m_typeCatalog) {
    case Investigations:
        ui->tableView->edit(model->index(_index.row(), investig_name));
        break;
    case TypesPrices:
        ui->tableView->edit(model->index(_index.row(), typePrice_name));
        break;
    case ConclusionTemplates:
        ui->tableView->edit(model->index(_index.row(), template_cod));
        break;
    default:
        break;
    }
}

void CatForSqlTableModel::onMarkDeletion()
{
    if (ui->tableView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atentie"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    int _row     = ui->tableView->currentIndex().row();
    int _id      = model->index(_row, 0).data(Qt::DisplayRole).toInt();
    int _delMark = model->index(_row, 1).data(Qt::DisplayRole).toInt();
    QString _cod = model->index(_row, 2).data(Qt::DisplayRole).toString();

    if (!db->deletionMarkObject(getNameTable(), _id)){
        QMessageBox::warning(this, tr("Marcarea/demarcarea obiectului"),
                             tr("Marcarea/demarcarea obiectului cu codul \"<b>%1</b>\" nu este reusita.").arg(_cod), QMessageBox::Ok);
        return;
    }
    if (_delMark == 0)
        model->setData(model->index(_row, 4), 1); // setam ca este actual
    else
        model->setData(model->index(_row, 4), 0);

    model->select();
}

void CatForSqlTableModel::onClose()
{
    this->close();
    emit mCloseThisForm();
}

void CatForSqlTableModel::slotContextMenuRequested(QPoint pos)
{
    int _row = ui->tableView->currentIndex().row();
    int _id = model->index(_row, 0).data(Qt::DisplayRole).toInt();
    QString _cod = model->index(_row, 2).data(Qt::DisplayRole).toString();

    QString strActionDelMark;
    int statusDeletionMark = db->statusDeletionMarkObject(getNameTable(), _id);
    if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_MARK){
        strActionDelMark = tr("Demarcheaza obiectul cu codul '%1'").arg(_cod);
    } else if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_UNMARK){
        strActionDelMark = tr("Marcheaza pentru eliminare obiectul co codul '%1'").arg(_cod);
    } else {
        qWarning(logWarning()) << tr("%1 - slotContextMenuRequested():").arg(metaObject()->className())
                               << tr("Status 'deletionMark' a obiectului cu ID=%1 nu este determinat !!!").arg(QString::number(_id));
    }

    QAction* actionAddObject  = new QAction(QIcon(":/img/add_x32.png"), tr("Adauga obiect nou"), this);
    QAction* actionMarkObject = new QAction(QIcon(":/img/clear_x32.png"), strActionDelMark, this);

    connect(actionAddObject, &QAction::triggered, this, &CatForSqlTableModel::onAddRowTable);
    connect(actionMarkObject, &QAction::triggered, this, &CatForSqlTableModel::onMarkDeletion);

    menu->clear();
    menu->addAction(actionAddObject);
    menu->addAction(actionMarkObject);
    menu->popup(ui->tableView->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

void CatForSqlTableModel::slot_typeCatalogChanged()
{
    if (m_typeForm == TypeForm::ListForm)
        updateTableView();
}

void CatForSqlTableModel::slot_typeFormChanged()
{
    if (m_typeForm == TypeForm::ListForm){
        ui->frameBtn->setVisible(true);
        initBtnForm();
    } else if (m_typeForm == TypeForm::SelectForm){
        ui->frameBtn->setVisible(false);
        initBtnToolBar();
    }
    updateTableView();
}

void CatForSqlTableModel::onDataChangedItemModel()
{
    int row = ui->tableView->currentIndex().row();
    QModelIndex index;
    switch (m_typeCatalog) {
    case Investigations:
        index = ui->tableView->model()->index(row, investig_name, QModelIndex());
        break;
    case TypesPrices:
        index = ui->tableView->model()->index(row, typePrice_discount, QModelIndex());
        break;
    case ConclusionTemplates:
        index = ui->tableView->model()->index(row, template_name, QModelIndex());
        break;
    default:
        break;
    }
    if (!index.isValid())
        return;
    ui->tableView->setCurrentIndex(index); // setam indexul curent
    ui->tableView->edit(index);            // intram in regimul de editare

    model->submitAll();                    // tranzactia de transfer de datele in BD
}

void CatForSqlTableModel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        switch (m_typeCatalog) {
        case Investigations:
            setWindowTitle(tr("Clasificatorul investigatiilor"));
            updateHeaderTableInvestigations();
            break;
        case TypesPrices:
            setWindowTitle(tr("Tipul preturilor"));
            updateHeaderTableTypesPrices();
            break;
        case ConclusionTemplates:
            setWindowTitle(tr("Șabloane concluziilor"));
            updateHeaderTableConclusionTemplates();
            break;
        default:
            break;
        }
    }
}
