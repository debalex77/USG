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

#include <customs/custommessage.h>

#include <data/popup.h>

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
    // gr_investig_delegate = new ComboDelegate("SELECT id,name FROM investigationsGroup;", this);

    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &CatForSqlTableModel::slotContextMenuRequested, Qt::UniqueConnection);
    connect(this, &CatForSqlTableModel::typeCatalogChanged,
            this, &CatForSqlTableModel::slot_typeCatalogChanged, Qt::UniqueConnection);
    connect(this, &CatForSqlTableModel::typeFormChanged,
            this, &CatForSqlTableModel::slot_typeFormChanged, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::dataChanged,
            this, &CatForSqlTableModel::onDataChangedItemModel, Qt::QueuedConnection);

    if (globals().isSystemThemeDark)
        ui->frameBtn->setObjectName("customFrame");
}

CatForSqlTableModel::~CatForSqlTableModel()
{
    delete ui;
}

void CatForSqlTableModel::initBtnForm()
{
    connect(ui->btnAdd, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::onAddRowTable, Qt::UniqueConnection);
    connect(ui->btnMarkDeletion, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::onMarkDeletion, Qt::UniqueConnection);
    connect(ui->btnClose, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::onClose, Qt::UniqueConnection);
    connect(ui->btnGroupInvestigation, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::onOpenGroupInvestigations, Qt::UniqueConnection);
    connect(ui->btnUpdate, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::updateTableView, Qt::UniqueConnection);
    connect(ui->btnCatalogCost, &QAbstractButton::clicked,
            this, &CatForSqlTableModel::printCatalogCost, Qt::UniqueConnection);
}

void CatForSqlTableModel::initBtnToolBar()
{   
    connect(ui->btnBarAdd, &QToolButton::clicked,
            this, &CatForSqlTableModel::onAddRowTable, Qt::UniqueConnection);
    connect(ui->btnBarEdit, &QToolButton::clicked,
            this, &CatForSqlTableModel::onEditRowTable, Qt::UniqueConnection);
    connect(ui->btnBarDeletion, &QToolButton::clicked,
            this, &CatForSqlTableModel::onMarkDeletion, Qt::UniqueConnection);
    connect(ui->btnBarUpdateTable, &QToolButton::clicked,
            this, &CatForSqlTableModel::updateTableView, Qt::UniqueConnection);
}

void CatForSqlTableModel::updateTableView()
{
    switch (m_typeCatalog) {
    case Investigations:
        setWindowTitle(tr("Clasificatorul investiga\310\233iilor"));
        model->setTable("investigations");
        // model->setSort(InvestigationsSections::Owner, Qt::AscendingOrder);
        updateHeaderTableInvestigations();
        model->setEditStrategy(QSqlTableModel::OnRowChange);            // Înregistrarea modificărilordupă editarea celule
        model->setSort(InvestigationsSections::Id, Qt::AscendingOrder); // sortarea datelor de la sectia 0
        break;
    case TypesPrices:
        setWindowTitle(tr("Tipul pre\310\233urilor"));
        model->setTable("typesPrices");
        updateHeaderTableTypesPrices();
        model->setEditStrategy(QSqlTableModel::OnRowChange);        // Înregistrarea modificărilordupă editarea celule
        model->setSort(TypePricesSections::Id, Qt::AscendingOrder); // sortarea datelor de la sectia 0

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
        model->sort(ConclusionTemplatesSections::System, Qt::AscendingOrder);
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
        model->setSort(SystemFormationsTemplatesSections::Name, Qt::AscendingOrder);
        updateHeaderTableFormationsBySystemTemplates();
        break;
    default:
        break;
    }

    ui->tableView->setModel(model); // setam modelul
    model->select();

    switch (m_typeCatalog) {
    case Investigations:
        ui->tableView->setColumnWidth(InvestigationsSections::DeletionMark, 25); // deletionMark
        ui->tableView->setColumnHidden(InvestigationsSections::Id, true);        // ascundem id
        ui->tableView->setColumnWidth(InvestigationsSections::Cod, 70);          // cod
        ui->tableView->setColumnWidth(InvestigationsSections::Name, 900);        // name
        ui->tableView->setColumnWidth(InvestigationsSections::Use, 80);          // use
        ui->tableView->setColumnHidden(InvestigationsSections::Uuid, true);      // ascundem uuid
        // setam delegat
        ui->tableView->setItemDelegateForColumn(InvestigationsSections::Use, checkbox_delegate);
        ui->tableView->setItemDelegateForColumn(InvestigationsSections::Owner, gr_investig_delegate);
        break;
    case TypesPrices:
        ui->tableView->setColumnWidth(TypePricesSections::DeletionMark, 25); // deletionMark
        ui->tableView->setColumnHidden(TypePricesSections::Id, true);        // ascundem id
        ui->tableView->setColumnWidth(TypePricesSections::Name, 1100);       // name
        ui->tableView->setColumnHidden(TypePricesSections::Uuid, true);      // ascundem uuid
        // setam delegat
        ui->tableView->setItemDelegateForColumn(TypePricesSections::Noncomercial, checkbox_delegate);
        ui->tableView->setItemDelegateForColumn(TypePricesSections::Discount, db_spinbox_delegate);
        break;
    case ConclusionTemplates:
        ui->tableView->setColumnWidth(ConclusionTemplatesSections::DeletionMark, 25); // deletionMark
        ui->tableView->setColumnHidden(ConclusionTemplatesSections::Id, true);        // ascundem id
        ui->tableView->setColumnWidth(ConclusionTemplatesSections::Code, 50);         // cod
        ui->tableView->setColumnWidth(ConclusionTemplatesSections::Name, 550);        // name
        ui->tableView->setColumnHidden(ConclusionTemplatesSections::Uuid, true);      // ascundem uuid
        break;
    case SystemTemplates:
        ui->tableView->setColumnWidth(SystemFormationsTemplatesSections::DeletionMark, 25); // deletionMark
        ui->tableView->setColumnHidden(SystemFormationsTemplatesSections::Id, true);        // ascundem id
        ui->tableView->setColumnWidth(SystemFormationsTemplatesSections::Name, 550);        // name
        ui->tableView->setColumnHidden(SystemFormationsTemplatesSections::Uuid, true);      // ascundem uuid
        break;
    default:
        break;
    }

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
             << "" // deletionMark
             << tr("Cod MS")
             << tr("Denumirea investigatiei")
             << tr("Utilizare")
             << tr("Grupa")
             << ""; // Uuid
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){ // setam headerul
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }
}

void CatForSqlTableModel::updateHeaderTableTypesPrices()
{
    QStringList _headers;
    _headers << tr("id")
             << "" // deletionMark
             << tr("Denumirea pretului")
             << tr("Reducere (%)")
             << tr("Noncomercial")
             << ""; // uuid
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){ // setam headerul
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }
}

void CatForSqlTableModel::updateHeaderTableConclusionTemplates()
{
    QStringList _headers;
    _headers << tr("id")
             << "" // deletionMark
             << tr("Cod")
             << tr("Concluzia")
             << tr("Sistema")
             << ""; // uuid
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
             << tr("Sistema")
             << ""; // uuid
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
    int row = index.row();
    int id  = model->index(row, InvestigationsSections::Id).data(Qt::DisplayRole).toInt();
    QString cod;
    QString name;
    if (m_typeCatalog == SystemTemplates) {
        cod  = nullptr;
        name = model->index(row, SystemFormationsTemplatesSections::Name).data(Qt::DisplayRole).toString();
    } else {
        cod  = model->index(row, InvestigationsSections::Cod).data(Qt::DisplayRole).toString();
        name = model->index(row, InvestigationsSections::Name).data(Qt::DisplayRole).toString();
    }

    QVariantMap data;
    data["id"] = id;
    data["cod"] = cod;
    data["name"] = name;

    emit mSelectData(data);
}

void CatForSqlTableModel::onAddRowTable()
{
    if (m_typeForm == TypeForm::SelectForm)
        model->setMainFlag(Qt::ItemIsEditable | Qt::ItemIsEnabled);

    const int row = model->rowCount();

    // Helper: focus + edit
    auto focusEdit = [&](int col){
        const QModelIndex idx = model->index(row, col);
        if (!idx.isValid()) return;
        ui->tableView->setCurrentIndex(idx);
        ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtBottom);
        ui->tableView->edit(idx);
    };

    // Inițializăm rândul fără să declanșăm dataChanged care-ți mută cursorul
    // QSignalBlocker blocker(model);
    model->insertRow(row);

    switch (m_typeCatalog) {
    case Investigations: {
        // Recomand: ID din DB (nu row+1) — dacă ai AUTOINCREMENT, nici nu seta manual
        model->setData(model->index(row, InvestigationsSections::Id), db->getLastIdForTable("investigations") + 1);
        model->setData(model->index(row, InvestigationsSections::DeletionMark), 0);
        model->setData(model->index(row, InvestigationsSections::Use), 1);
        model->setData(model->index(row, InvestigationsSections::Uuid), QUuid::createUuid().toRfc4122());
        break;
    }
    case TypesPrices: {
        model->setData(model->index(row, TypePricesSections::Id), db->getLastIdForTable("typesPrices") + 1);
        model->setData(model->index(row, TypePricesSections::DeletionMark), 0);
        model->setData(model->index(row, TypePricesSections::Uuid), QUuid::createUuid().toRfc4122());
        break;
    }
    case ConclusionTemplates: {
        const int newId = db->getLastIdForTable("conclusionTemplates") + 1;
        model->setData(model->index(row, ConclusionTemplatesSections::Id), newId);
        model->setData(model->index(row, ConclusionTemplatesSections::DeletionMark), 0);
        model->setData(model->index(row, ConclusionTemplatesSections::Uuid), QUuid::createUuid().toRfc4122());
        model->setData(model->index(row, ConclusionTemplatesSections::Code), newId);
        break;
    }
    case SystemTemplates: {
        model->setData(model->index(row, SystemFormationsTemplatesSections::Id),
                       db->getLastIdForTable("formationsSystemTemplates") + 1);
        model->setData(model->index(row, SystemFormationsTemplatesSections::DeletionMark), 0);
        model->setData(model->index(row, SystemFormationsTemplatesSections::Uuid), QUuid::createUuid().toRfc4122());
        if (m_filter_templates != nullptr && m_typeCatalog == SystemTemplates)
            model->setData(model->index(row, SystemFormationsTemplatesSections::System), m_filter_templates);
        break;
    }
    default:
        break;
    }

    // blocker.unblock(); // semnalele revin

    // Setăm coloana de start pentru editare (în afara blocării)
    switch (m_typeCatalog) {
    case Investigations:      focusEdit(InvestigationsSections::Cod);  break;
    case TypesPrices:         focusEdit(TypePricesSections::Name);     break;
    case ConclusionTemplates: focusEdit(ConclusionTemplatesSections::Name); break;
    case SystemTemplates:     focusEdit(SystemFormationsTemplatesSections::Name); break;
    default: break;
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
        ui->tableView->edit(model->index(_index.row(), InvestigationsSections::Name));
        break;
    case TypesPrices:
        ui->tableView->edit(model->index(_index.row(), TypePricesSections::Name));
        break;
    case ConclusionTemplates:
        ui->tableView->edit(model->index(_index.row(), ConclusionTemplatesSections::Name));
        break;
    case SystemTemplates:
        ui->tableView->edit(model->index(_index.row(), SystemFormationsTemplatesSections::Name));
        break;
    default:
        break;
    }
}

void CatForSqlTableModel::onMarkDeletion()
{
    // verificam daca este ales randul
    if (ui->tableView->currentIndex().row() == -1){
        QMessageBox::warning(
            this,
            tr("Atentie"),
            tr("Nu este marcat randul !!!."), QMessageBox::Ok
            );
        return;
    }

    // determinam ID
    int _row = ui->tableView->currentIndex().row();
    int _id  = model->index(_row, 0).data(Qt::DisplayRole).toInt();

    // anuntam variabile si pregatim conditia
    QStringList err;
    QMap<QString, QVariant> where;
    where["id"] = _id;

    // eliminam din baza de date
    bool remove = db->deleteFromTable(this->metaObject()->className(), getNameTable(), where, err);

    // prezentam mesaj de eliminare sau de eroare
    if (remove) {
        PopUp *popUp = new PopUp(this);
        popUp->setPopupText(tr("Obiectul eliminat cu succes."));
        popUp->show();
    } else {
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QApplication::applicationName());
        msg->setTextTitle(tr(""));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }

    model->select();
}

void CatForSqlTableModel::onClose()
{
    this->close();
    emit mCloseThisForm();
}

void CatForSqlTableModel::onOpenGroupInvestigations()
{
    list_group = new GroupInvestigationList(*db, this);
    list_group->setAttribute(Qt::WA_DeleteOnClose);
    list_group->show();
}

void CatForSqlTableModel::slotContextMenuRequested(QPoint pos)
{
    QAction *actionAddObject  = new QAction(QIcon(":/img/toolBar/add.png"), tr("Adauga obiect nou"), this);
    QAction *actionEditObject = new QAction(QIcon(":/img/toolBar/edit.png"), tr("Editeaza obiect"), this);
    QAction *actionMarkObject = new QAction(QIcon(":/img/toolBar/delete.png"), tr("Eliminare obiectului"), this);

    connect(actionAddObject, &QAction::triggered,
            this, &CatForSqlTableModel::onAddRowTable, Qt::UniqueConnection);
    connect(actionEditObject, &QAction::triggered,
            this, &CatForSqlTableModel::onEditRowTable, Qt::UniqueConnection);
    connect(actionMarkObject, &QAction::triggered,
            this, &CatForSqlTableModel::onMarkDeletion, Qt::UniqueConnection);

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

        ui->btnAdd->setToolTip(tr("Insereaza (ins)"));
        ui->btnMarkDeletion->setToolTip(tr("Elimina (del)"));
        ui->btnClose->setToolTip(tr("Inchide (esc)"));
        ui->btnUpdate->setToolTip(tr("Actualizeaza (F5)"));

        ui->btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));
        ui->btnMarkDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
        ui->btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
        ui->btnUpdate->setShortcut(QKeySequence(Qt::Key_F5));

    } else if (m_typeForm == TypeForm::SelectForm){
        ui->frameBtn->setVisible(false);
        ui->layoutToolBar->show(); // prezentam butoane
        initBtnToolBar();

        ui->btnBarAdd->setToolTip(tr("Insereaza (ins)"));
        ui->btnBarEdit->setToolTip(tr("Editeaza (F2)"));
        ui->btnBarDeletion->setToolTip(tr("Elimina (del)"));
        ui->btnBarUpdateTable->setToolTip(tr("Actualizeaza (F5)"));

        ui->btnBarAdd->setShortcut(QKeySequence(Qt::Key_Insert));
        ui->btnBarEdit->setShortcut(QKeySequence(Qt::Key_F2));
        ui->btnBarDeletion->setShortcut(QKeySequence(Qt::Key_Delete));
        ui->btnBarUpdateTable->setShortcut(QKeySequence(Qt::Key_F5));
    }
    updateTableView();
}

void CatForSqlTableModel::onDataChangedItemModel(
    const QModelIndex &topLeft,
    const QModelIndex &bottomRight,
    const QVector<int> & /*roles*/)
{
    Q_UNUSED(bottomRight);

    if (!topLeft.isValid())
        return;

    // dacă s-au schimbat mai multe celule, luăm prima
    const int row = topLeft.row();
    const int col = topLeft.column();

    auto focusEditAt = [&](int targetCol){
        const QModelIndex idx = model->index(row, targetCol);
        if (!idx.isValid())
            return;
        ui->tableView->setCurrentIndex(idx);
        ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtBottom);
        ui->tableView->edit(idx);
    };

    switch (m_typeCatalog) {
    case Investigations:
        if (col == InvestigationsSections::Cod)
            focusEditAt(InvestigationsSections::Name);
        else if (col == InvestigationsSections::Name)
            focusEditAt(InvestigationsSections::Owner);
        break;

    case TypesPrices:
        focusEditAt(TypePricesSections::Discount);
        break;

    case ConclusionTemplates:
        focusEditAt(ConclusionTemplatesSections::Name);
        break;

    case SystemTemplates:
        focusEditAt(SystemFormationsTemplatesSections::Name);
        if (col == SystemFormationsTemplatesSections::Name)
            focusEditAt(SystemFormationsTemplatesSections::System);
        break;

    default:
        break;
    }

    model->submitAll();
}

void CatForSqlTableModel::printCatalogCost()
{
    QDir dir;

    // Deschidem fișierul XML
    QFile file(":/xmls/investig_2024.xml");
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << "Nu se poate deschide fișierul XML:"
                               << file.errorString();
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
                                "Probabil \303\256n set\304\203rile aplica\310\233iei "
                                "nu este setat corect drumul spre formularele de tipar."),
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

void CatForSqlTableModel::keyReleaseEvent(QKeyEvent *event)
{
    int rowCount = model->rowCount();

    if (event->key() == Qt::Key_End) {
        ui->tableView->selectRow(rowCount - 1);

    } else if (event->key() == Qt::Key_Home) {
        ui->tableView->selectRow(0);

    } else if (event->key() == Qt::Key_PageDown) {
        int currentRow  = ui->tableView->currentIndex().row();
        int visibleRows = ui->tableView->viewport()->height() / ui->tableView->rowHeight(0);

        int newRow = qMin(currentRow + visibleRows, rowCount - 1);
        ui->tableView->selectRow(newRow);
        ui->tableView->scrollTo(model->index(newRow, 0));

    } else if (event->key() == Qt::Key_PageUp) {
        int currentRow = ui->tableView->currentIndex().row();
        int visibleRows = ui->tableView->viewport()->height() / ui->tableView->rowHeight(0);

        int newRow = qMax(currentRow - visibleRows, 0);
        ui->tableView->selectRow(newRow);
        ui->tableView->scrollTo(model->index(newRow, 0));
    }
}
