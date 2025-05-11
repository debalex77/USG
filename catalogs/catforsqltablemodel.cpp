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

#include "catforsqltablemodel.h"
#include "ui_catforsqltablemodel.h"

#include <QDomDocument>
#include <QScreen>
#include <QToolButton>

CatForSqlTableModel::CatForSqlTableModel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatForSqlTableModel)
{
    ui->setupUi(this);

    db      = new DataBase(this);
    menu    = new QMenu(this);
    model   = new BaseSqlTableModel(this);
    checkbox_delegate    = new CheckBoxDelegate(this);
    db_spinbox_delegate  = new DoubleSpinBoxDelegate(0.00, 999999.99, 0.05, 2, this);
    gr_investig_delegate = new ComboDelegate("SELECT id,name FROM investigationsGroup;", this);

    connect(ui->tableView, &QWidget::customContextMenuRequested, this, &CatForSqlTableModel::slotContextMenuRequested);
    connect(this, &CatForSqlTableModel::typeCatalogChanged, this, &CatForSqlTableModel::slot_typeCatalogChanged);
    connect(this, &CatForSqlTableModel::typeFormChanged, this, &CatForSqlTableModel::slot_typeFormChanged);
    connect(model, &QAbstractItemModel::dataChanged, this, &CatForSqlTableModel::onDataChangedItemModel, Qt::QueuedConnection);

    if (globals().isSystemThemeDark)
        ui->frameBtn->setObjectName("customFrame");
}

CatForSqlTableModel::~CatForSqlTableModel()
{
    delete db;
    delete menu;
    delete checkbox_delegate;
    delete db_spinbox_delegate;
    delete gr_investig_delegate;
    delete model;
    delete ui;
}

void CatForSqlTableModel::initBtnForm()
{
    ui->btnAdd->setToolTip(tr("Insereaza (ins)"));
    ui->btnMarkDeletion->setToolTip(tr("Elimina (del)"));
    ui->btnClose->setToolTip(tr("Inchide (esc)"));
    ui->btnUpdate->setToolTip(tr("Actualizeaza (F5)"));

    ui->btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));
    ui->btnMarkDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
    ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
    ui->btnUpdate->setShortcut(QKeySequence(Qt::Key_F5));

    connect(ui->btnAdd, &QAbstractButton::clicked, this, &CatForSqlTableModel::onAddRowTable);
    connect(ui->btnMarkDeletion, &QAbstractButton::clicked, this, &CatForSqlTableModel::onMarkDeletion);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &CatForSqlTableModel::onClose);
    connect(ui->btnGroupInvestigation, &QAbstractButton::clicked, this, &CatForSqlTableModel::onOpenGroupInvestigations);
    connect(ui->btnUpdate, &QAbstractButton::clicked, this, &CatForSqlTableModel::updateTableView);
    connect(ui->btnCatalogCost, &QAbstractButton::clicked, this, &CatForSqlTableModel::printCatalogCost);
}

void CatForSqlTableModel::initBtnToolBar()
{   
    connect(ui->btnBarAdd, &QToolButton::clicked, this, &CatForSqlTableModel::onAddRowTable);
    connect(ui->btnBarEdit, &QToolButton::clicked, this, &CatForSqlTableModel::onEditRowTable);
    connect(ui->btnBarDeletion, &QToolButton::clicked, this, &CatForSqlTableModel::onMarkDeletion);
    connect(ui->btnBarUpdateTable, &QToolButton::clicked, this, &CatForSqlTableModel::updateTableView);
}

void CatForSqlTableModel::updateTableView()
{
    switch (m_typeCatalog) {
    case Investigations:
        setWindowTitle(tr("Clasificatorul investiga\310\233iilor"));
        model->setTable("investigations");
        // model->setSort(investig_owner, Qt::AscendingOrder);
        updateHeaderTableInvestigations();
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(investig_id, Qt::AscendingOrder);     // sortarea datelor de la sectia 0
        break;
    case TypesPrices:
        setWindowTitle(tr("Tipul pre\310\233urilor"));
        model->setTable("typesPrices");
        updateHeaderTableTypesPrices();
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(typePrice_id, Qt::AscendingOrder);                  // sortarea datelor de la sectia 0

        ui->btnGroupInvestigation->setVisible(false);
        break;
    case ConclusionTemplates:
        setWindowTitle(tr("\310\230abloane concluziilor"));
        model->setTable("conclusionTemplates");
        if (m_filter_templates != nullptr){
            if (globals().thisMySQL)
                model->setFilter(QString("`system` = '%1'").arg(m_filter_templates));
            else
                model->setFilter(QString("system = '%1'").arg(m_filter_templates));
        }
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(template_name, Qt::AscendingOrder);
        updateHeaderTableConclusionTemplates();
        break;
    case SystemTemplates:
        setWindowTitle(tr("\310\230abloane descrierilor forma\310\233iunilor"));
        model->setTable("formationsSystemTemplates");
        if (m_filter_templates != nullptr){
            if (globals().thisMySQL)
                model->setFilter(QString("typeSystem = '%1'").arg(m_filter_templates));
            else
                model->setFilter(QString("typeSystem = '%1'").arg(m_filter_templates));
        }
        model->setEditStrategy(QSqlTableModel::OnRowChange); // Înregistrarea modificărilordupă editarea celule
        model->setSort(template_system_name, Qt::AscendingOrder);
        updateHeaderTableFormationsBySystemTemplates();
        break;
    default:
        break;
    }

    ui->tableView->setModel(model);                        // setam modelul
    model->select();

    switch (m_typeCatalog) {
    case Investigations:
        ui->tableView->setColumnWidth(investig_deletionMark, 25);
        ui->tableView->setColumnHidden(investig_id, true); // ascundem id
        ui->tableView->setColumnWidth(investig_cod, 70);
        ui->tableView->setColumnWidth(investig_name, 900); // denuimirea investigatiei
        ui->tableView->setColumnWidth(investig_use, 80); // denuimirea investigatiei
        ui->tableView->setItemDelegateForColumn(investig_use, checkbox_delegate);
        ui->tableView->setItemDelegateForColumn(investig_owner, gr_investig_delegate);
        break;
    case TypesPrices:
        ui->tableView->setColumnWidth(typePrice_deletionMark, 25);
        ui->tableView->setColumnHidden(typePrice_id, true); // ascundem id
        ui->tableView->setColumnWidth(typePrice_name, 1100);
        ui->tableView->setItemDelegateForColumn(typePrice_noncomercial, checkbox_delegate);
        ui->tableView->setItemDelegateForColumn(typePrice_discount, db_spinbox_delegate);
        break;
    case ConclusionTemplates:
        ui->tableView->setColumnWidth(template_deletionMark, 25);
        ui->tableView->setColumnHidden(template_id, true); // ascundem id
        ui->tableView->setColumnWidth(template_cod, 50);
        ui->tableView->setColumnWidth(template_name, 550);
        break;
    case SystemTemplates:
         ui->tableView->setColumnWidth(template_system_deletionMark, 25);
        ui->tableView->setColumnHidden(template_system_id, true); // ascundem id
        ui->tableView->setColumnWidth(template_system_name, 550);
        break;
    default:
        break;
    }

    // ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);     // initializam meniu contextual
    ui->tableView->horizontalHeader()->setStretchLastSection(true); // largirea ultimei colonite
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
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
             << tr("Utilizare")
             << tr("Grupa");
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

void CatForSqlTableModel::updateHeaderTableFormationsBySystemTemplates()
{
    QStringList _headers;
    _headers << tr("id")
             << tr("") // deletionMark
             << tr("Descrierea")
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
    case SystemTemplates:
        nameTable = "formationsSystemTemplates";
        break;
    }
    return nameTable;
}

void CatForSqlTableModel::onSelectRowTable(const QModelIndex &index)
{
    int _row     = index.row();
    int _id      = model->index(_row, investig_id).data(Qt::DisplayRole).toInt();
    QString _cod;
    QString _name;
    if (m_typeCatalog == SystemTemplates) {
        _cod = nullptr;
        _name = model->index(_row, template_system_name).data(Qt::DisplayRole).toString();
    } else {
        _cod = model->index(_row, investig_cod).data(Qt::DisplayRole).toString();
        _name = model->index(_row, investig_name).data(Qt::DisplayRole).toString();
    }

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
        model->setData(model->index(row, template_id), db->getLastIdForTable("conclusionTemplates") + 1);
        model->setData(model->index(row, template_deletionMark), 0);
        model->setData(model->index(row, template_cod), db->getLastIdForTable("conclusionTemplates") + 1);
        ui->tableView->setCurrentIndex(model->index(row, template_name));
        ui->tableView->edit(model->index(row, template_name));
        break;
    case SystemTemplates:
        model->setData(model->index(row, template_system_id), db->getLastIdForTable("formationsSystemTemplates") + 1);
        model->setData(model->index(row, template_system_deletionMark), 0);
        if (m_filter_templates != nullptr && m_typeCatalog == SystemTemplates)
            model->setData(model->index(row, template_system_system), m_filter_templates);
        ui->tableView->setCurrentIndex(model->index(row, template_system_name));
        ui->tableView->edit(model->index(row, template_system_name));
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
        ui->tableView->edit(model->index(_index.row(), template_name));
        break;
    case SystemTemplates:
        ui->tableView->edit(model->index(_index.row(), template_system_name));
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

void CatForSqlTableModel::onOpenGroupInvestigations()
{
    list_group = new GroupInvestigationList(this);
    list_group->setAttribute(Qt::WA_DeleteOnClose);
    list_group->show();
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
    QAction* actionEditObject = new QAction(QIcon(":/img/edit_x32.png"), tr("Editeaza obiect"), this);
    QAction* actionMarkObject = new QAction(QIcon(":/img/clear_x32.png"), strActionDelMark, this);

    connect(actionAddObject, &QAction::triggered, this, &CatForSqlTableModel::onAddRowTable);
    connect(actionEditObject, &QAction::triggered, this, &CatForSqlTableModel::onEditRowTable);
    connect(actionMarkObject, &QAction::triggered, this, &CatForSqlTableModel::onMarkDeletion);

    menu->clear();
    menu->addAction(actionAddObject);
    menu->addAction(actionEditObject);
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
        ui->layoutToolBar->hide(); // ascunde toate butoane
        initBtnForm();
    } else if (m_typeForm == TypeForm::SelectForm){
        ui->frameBtn->setVisible(false);
        ui->layoutToolBar->show(); // prezentam butoane
        initBtnToolBar();
    }
    updateTableView();
}

void CatForSqlTableModel::onDataChangedItemModel()
{
    int row = ui->tableView->currentIndex().row();
    int column = ui->tableView->currentIndex().column();

    QModelIndex index;
    switch (m_typeCatalog) {
    case Investigations:
        if (column == investig_cod)
            index = ui->tableView->model()->index(row, investig_name, QModelIndex());
        else if (column == investig_name)
            index = ui->tableView->model()->index(row, investig_use, QModelIndex());
        break;
    case TypesPrices:
        index = ui->tableView->model()->index(row, typePrice_discount, QModelIndex());
        break;
    case ConclusionTemplates:
        index = ui->tableView->model()->index(row, template_name, QModelIndex());
        break;
    case SystemTemplates:
        index = ui->tableView->model()->index(row, template_system_name, QModelIndex());
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

void CatForSqlTableModel::printCatalogCost()
{
    QDir dir;

    // Deschidem fișierul XML
    QFile file(":/xmls/investig_2024.xml");
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << "Nu se poate deschide fișierul XML:" << file.errorString();
        return;
    }

    // Citim și parsează XML-ul
    QDomDocument doc;
    if (! doc.setContent(&file)) {
        qWarning(logWarning()) << "Eroare la parsarea fișierului XML.";
        file.close();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "list_investigation") {
        qWarning(logWarning()) << "Tagul rădăcină al fișierului XML este incorect.";
        return;
    }

    print_model = new QStandardItemModel(this);

    // Parcurgem intrările din XML
    QDomNodeList entries = root.elementsByTagName("entry");
    for (int i = 0; i < entries.count(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.isNull())
            continue;

        QString cod  = entry.attribute("cod");
        QString name = entry.attribute("name");
        QString cost = entry.attribute("cost");

        QStandardItem* item_cod = new QStandardItem();
        item_cod->setData(cod, Qt::DisplayRole);

        QStandardItem* item_name = new QStandardItem();
        item_name->setData(name, Qt::DisplayRole);

        QStandardItem* item_cost = new QStandardItem();
        item_cost->setData(QString::number(cost.toInt(), 'f', 2) , Qt::DisplayRole);

        QList<QStandardItem *> items;
        items.append(item_cod);
        items.append(item_name);
        items.append(item_cost);
        print_model->appendRow(items);
    }

    m_report = new LimeReport::ReportEngine(this);

    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->addModel("print_model", print_model, false);
    m_report->setShowProgressDialog(true);
    m_report->setPreviewWindowTitle(tr("Catalogul tarifelor unice (modificat 2024)"));
    if (! m_report->loadFromFile(dir.toNativeSeparators(globals().pathTemplatesDocs + "/CatalogCost.lrxml"))) {
        QMessageBox::warning(this,
                             tr("Verificarea \310\231ablonului"),
                             tr("Nu a fost g\304\203sit formular de tipar !!!<br>"
                                "Probabil \303\256n set\304\203rile aplica\310\233iei nu este setat corect drumul spre formularele de tipar."),
                             QMessageBox::Ok);
    }
    m_report->previewReport();

    print_model->deleteLater();
    m_report->deleteLater();
}

void CatForSqlTableModel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        switch (m_typeCatalog) {
        case Investigations:
            setWindowTitle(tr("Clasificatorul investiga\310\233iilor"));
            updateHeaderTableInvestigations();
            break;
        case TypesPrices:
            setWindowTitle(tr("Tipul pre\310\233urilor"));
            updateHeaderTableTypesPrices();
            break;
        case ConclusionTemplates:
            setWindowTitle(tr("\310\230abloane concluziilor"));
            updateHeaderTableConclusionTemplates();
            break;
        case SystemTemplates:
            setWindowTitle(tr("\310\230abloane concluziilor"));
            updateHeaderTableFormationsBySystemTemplates();
            break;
        default:
            break;
        }
    }
}
