#include "listform.h"
#include "ui_listform.h"

ListForm::ListForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListForm)
{
    ui->setupUi(this);

    db    = new DataBase(this);  // alocam memoria p-u lucru cu BD
    popUp = new PopUp(this);     // alocam memoria p-u mesaje
    menu  = new QMenu(this);     // meniu contextual

    QString strQuery;
    model = new BaseSqlQueryModel(strQuery, ui->tabView);
    model->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);
    proxy = new BaseSortFilterProxyModel(this);
    proxy->setListFormType(BaseSortFilterProxyModel::ListFormType::ListCatalog);

    ui->btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    ui->btnAdd->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnAdd->setLayout(new QGridLayout);
    ui->btnAdd->setShortcut(QKeySequence(Qt::Key_Insert));

    ui->btnEdit->setIcon(QIcon(":/img/edit_x32.png"));
    ui->btnEdit->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnEdit->setLayout(new QGridLayout);
    ui->btnEdit->setShortcut(QKeySequence(Qt::Key_F2));

    ui->btnDelMark->setIcon(QIcon(":/img/clear_x32.png"));
    ui->btnDelMark->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnDelMark->setLayout(new QGridLayout);
    ui->btnDelMark->setShortcut(QKeySequence(Qt::Key_Delete));

    ui->btnCancel->setIcon(QIcon(":/img/close_x32.png"));
    ui->btnCancel->setStyleSheet("padding-left: 3px; text-align: left;");
    ui->btnCancel->setLayout(new QGridLayout);
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(this, &ListForm::typeListFormChanged, this, &ListForm::changedTypeListForm);

    connect(ui->btnAdd, &QAbstractButton::clicked, this, &ListForm::onAddObject);
    connect(ui->btnEdit, &QAbstractButton::clicked, this, &ListForm::onEditObject);
    connect(ui->btnDelMark, &QAbstractButton::clicked, this, &ListForm::onDeletionMarkObject);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &ListForm::onClose);

    connect(ui->tabView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked), this, &ListForm::onDoubleClickedTable);
    connect(ui->tabView, &QWidget::customContextMenuRequested, this, &ListForm::slotContextMenuRequested);
}

ListForm::~ListForm()
{    
    delete db;
    delete popUp;
    delete menu;
    delete model;
    delete proxy;
    delete ui;
}

// **********************************************************************************
// --- actualizarea solicitarilor tabelelor

void ListForm::updateTableView()
{    
    delete model;
    delete proxy;

    QString strQuery;
    model = new BaseSqlQueryModel(strQuery, ui->tabView);
    model->setProperty("modelParent", BaseSqlQueryModel::ModelParent::GeneralListForm);
    proxy = new BaseSortFilterProxyModel(this);

    switch (m_typeListForm) {
    case Doctors:        
        model->setTypeCatalogs(BaseSqlQueryModel::TypeCatalogs::Doctors);
        model->setQuery(qryTableDoctors());
        break;
    case Nurses:        
        model->setTypeCatalogs(BaseSqlQueryModel::TypeCatalogs::Nurses);
        model->setQuery(qryTableNurses());
        break;
    case Pacients:        
        model->setTypeCatalogs(BaseSqlQueryModel::TypeCatalogs::Pacients);
        model->setQuery(qryTablePacients());
        break;
    case Organizations:        
        model->setTypeCatalogs(BaseSqlQueryModel::TypeCatalogs::Organizations);
        model->setQuery(qryTableOrganizations());
        break;
    case Users:      
        model->setTypeCatalogs(BaseSqlQueryModel::TypeCatalogs::Users);
        model->setQuery(qryTableUsers());
        break;
    default:
        qWarning(logWarning()) << tr("Nu corect a fost transmisa proprietatea 'typeListForm' clasei '%1' !!!").arg(metaObject()->className());
        break;
    }

    while (model->canFetchMore())
        model->fetchMore();

    proxy->setSourceModel(model);
    ui->tabView->setModel(proxy);

    ui->tabView->hideColumn(0);                                                      // id-ascundem
    ui->tabView->setSortingEnabled(true);                                            // setam posibilitatea sortarii
    ui->tabView->setSelectionBehavior(QAbstractItemView::SelectRows);                // setam alegerea randului
    ui->tabView->setSelectionMode(QAbstractItemView::SingleSelection);               // setam singura alegerea(nu multipla)
    ui->tabView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tabView->setContextMenuPolicy(Qt::CustomContextMenu);                        // initializam meniu contextual
    ui->tabView->verticalHeader()->setDefaultSectionSize(14);
    ui->tabView->horizontalHeader()->setStretchLastSection(true);                    // extinderea ultimei sectiei
    ui->tabView->setFocus();                                                         // focusam la tabela
    ui->tabView->selectRow(0);                                                       // selectam primul rand
}

void ListForm::updateHeaderTable()
{
    model->setHeaderData(1, Qt::Horizontal, tr(""));

    switch (m_typeListForm) {
    case Doctors:
        setWindowTitle("Catalogul: Doctori");
        model->setHeaderData(2, Qt::Horizontal, tr("NPP doctorului"));
        model->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
        model->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
        model->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));
        break;
    case Nurses:
        setWindowTitle(tr("Catalogul: Asistente medicale"));
        model->setHeaderData(2, Qt::Horizontal, tr("NPP as.medicale"));
        model->setHeaderData(3, Qt::Horizontal, tr("Telefoane"));
        model->setHeaderData(4, Qt::Horizontal, tr("E-mail"));
        model->setHeaderData(5, Qt::Horizontal, tr("Comentariu"));
        break;
    case Pacients:
        setWindowTitle(tr("Catalogul: Pacienți"));
        model->setHeaderData(2, Qt::Horizontal, tr("NPP pacientului"));
        model->setHeaderData(3, Qt::Horizontal, tr("Data nașterii"));
        model->setHeaderData(4, Qt::Horizontal, tr("Adresa"));
        model->setHeaderData(5, Qt::Horizontal, tr("Telefoane"));
        model->setHeaderData(6, Qt::Horizontal, tr("IDNP"));
        model->setHeaderData(7, Qt::Horizontal, tr("Comentariu"));
        break;
    case Organizations:
        setWindowTitle(tr("Catalogul: Persoane juridice"));
        model->setHeaderData(2, Qt::Horizontal, tr("Denumirea organizației"));
        model->setHeaderData(3, Qt::Horizontal, tr("IDNP"));
        model->setHeaderData(4, Qt::Horizontal, tr("Adresa"));
        model->setHeaderData(5, Qt::Horizontal, tr("Telefoane"));
        model->setHeaderData(6, Qt::Horizontal, tr("E-mail"));
        model->setHeaderData(7, Qt::Horizontal, tr("Comentariu"));
        break;
    case Users:
        setWindowTitle(tr("Catalogul: Utilizatori"));
        model->setHeaderData(2, Qt::Horizontal, tr("Nume utilizatorului"));
        break;
    default:
        qWarning(logWarning()) << tr("Nu corect a fost transmisa proprietatea 'typeListForm' clasei '%1' !!!").arg(metaObject()->className());
        break;
    }
}

const QString ListForm::getNameTable()
{
    switch (m_typeListForm) {
    case Doctors:
        return "doctors";
    case Nurses:
        return "nurses";
    case Pacients:
        return "pacients";
    case Organizations:
        return "organizations";
    case Users:
        return "users";
    default:
        qWarning(logWarning()) << tr("Eroare la determinarea tipului tabelei. Nu este determinata proprietatea 'typeListForm' !!!");
        return nullptr;
    }
}

// **********************************************************************************
// --- salvarea si setarea dimensiunilor(sortarea) sectiilor + alte

void ListForm::loadSizeSectionTable()
{
    for (int numSection = 0; numSection < ui->tabView->horizontalHeader()->count(); ++numSection) {

        const QString type_form = enumToString(m_typeListForm);
        const int find_id = db->findIdFromTableSettingsForm(type_form, numSection);
        if (find_id > 0){

            //---------- setarea dimensiunilor sectiilor
            const int section_size = db->getSizeSectionFromTableSettingsForm(numSection, type_form, find_id);
            if (section_size >= 0)
                ui->tabView->setColumnWidth(numSection, section_size);
            else
                ui->tabView->setColumnWidth(numSection, sizeSectionDefault(numSection));

            //---------- setarea directiei sortarii
            const int direction_sorting = db->getDirectionSortingFromTableSettingsForm(numSection, type_form);
            if (direction_sorting >= 0)
                ui->tabView->horizontalHeader()->setSortIndicator(numSection, (direction_sorting == 0) ? Qt::AscendingOrder : Qt::DescendingOrder);

        } else {
            ui->tabView->setColumnWidth(numSection, sizeSectionDefault(numSection));
        }
    }

    // pozitionam cursorul
    ui->tabView->selectRow(0);
}

void ListForm::saveSizeSectionTable()
{
    for (int numSection = 0; numSection < ui->tabView->horizontalHeader()->count(); ++numSection) {

        // determinarea si setarea variabilei directiei de sortare
        int directionSorting = -1;
        if (numSection == ui->tabView->horizontalHeader()->sortIndicatorSection())
            directionSorting = ui->tabView->horizontalHeader()->sortIndicatorOrder();

        // variabile pu inserarea sau actualizarea datelor in tabela
        const QString type_form = enumToString(m_typeListForm);
        const int find_id = db->findIdFromTableSettingsForm(type_form, numSection);
        const int size_section = ui->tabView->horizontalHeader()->sectionSize(numSection);

        // inserarea dimensiunilor sectiilor si directia sortarii
        if (find_id > 0)
            db->insertUpdateDataTableSettingsForm(false, type_form, numSection, size_section, find_id, directionSorting);
        else
            db->insertUpdateDataTableSettingsForm(true, type_form, numSection, size_section, -1, directionSorting);
    }
}

int ListForm::sizeSectionDefault(const int numberSection)
{  
    switch (numberSection) {
    case section_deletionMark:
        return sz_deletionMark;
    case section_name:
        return sz_name;
    case section_telephone:
        return sz_telephone;
    case section_address:
        return sz_address;
    default:
        return 0;
    }
}

// **********************************************************************************
// --- solicitari

const QString ListForm::qryTableDoctors()
{
    return QString("SELECT doctors.id,"
                   "  doctors.deletionMark,"
                   "  fullNameDoctors.name AS FullName,"
                   "  doctors.telephone,"
                   "  doctors.email,"
                   "  doctors.comment "
                   "FROM "
                   "  doctors "
                   "INNER JOIN "
                   "  fullNameDoctors ON doctors.id = fullNameDoctors.id_doctors "
                   "GROUP BY fullNameDoctors.name;");
}

const QString ListForm::qryTableNurses()
{
    return QString("SELECT nurses.id,"
                   "  nurses.deletionMark,"
                   "  fullNameNurses.name AS FullName,"
                   "  nurses.telephone,"
                   "  nurses.email,"
                   "  nurses.comment "
                   "FROM nurses "
                   "INNER JOIN"
                   "  fullNameNurses ON nurses.id = fullNameNurses.id_nurses "
                   "GROUP BY fullNameNurses.name;");
}

const QString ListForm::qryTablePacients()
{
    return QString("SELECT pacients.id,"
                   "  pacients.deletionMark,"
                   "  fullNamePacients.name AS FullName,"
                   "  pacients.birthday,"
                   "  pacients.address,"
                   "  pacients.telephone,"
                   "  pacients.IDNP,"
                   "  pacients.comment "
                   "FROM "
                   "  pacients "
                   "INNER JOIN fullNamePacients ON pacients.id = fullNamePacients.id_pacients "
                   "GROUP BY fullNamePacients.name;");
}

const QString ListForm::qryTableUsers()
{
    return QString("SELECT id,"
                   "deletionMark,"
                   "name FROM users "
                   "ORDER BY name;");
}

const QString ListForm::qryTableOrganizations()
{
    return QString("SELECT id,"
                   "deletionMark,"
                   "name,"
                   "IDNP,"
                   "address,"
                   "telephone,"
                   "email,"
                   "comment FROM organizations "
                   "ORDER BY name;");
}

QString ListForm::enumToString(TypeListForm typeCatalog)
{
    switch (typeCatalog) {
    case TypeListForm::Doctors:       return "doctors";
    case TypeListForm::Nurses:        return "nurses";
    case TypeListForm::Organizations: return "organizations";
    case TypeListForm::Pacients:      return "pacients";
    case TypeListForm::Users:         return "users";
    default:                          return "unknow type";
    }
}

// **********************************************************************************
// --- procesarea actiunilor butoanelor

void ListForm::changedTypeListForm()
{
    updateTableView();
    updateHeaderTable();
    loadSizeSectionTable();
}

bool ListForm::onAddObject()
{
    switch (m_typeListForm) {
    case Doctors:
        cat_General = new CatGeneral(this);
        cat_General->setProperty("itNew", true);
        cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
        cat_General->setAttribute(Qt::WA_DeleteOnClose);
        connect(cat_General, &CatGeneral::createCatGeneral, this, [this]()
        {
            updateTableView();
            popUp->setPopupText(tr("Obiectul <b>'%1'</b> a fost salvat <br>in baza de date cu succes.").arg(cat_General->getFullName()));
            popUp->show();
        });
        cat_General->exec();
        break;
    case Nurses:
        cat_General = new CatGeneral(this);
        cat_General->setProperty("itNew", true);
        cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
        cat_General->setAttribute(Qt::WA_DeleteOnClose);
        connect(cat_General, &CatGeneral::createCatGeneral, this, [this]()
        {
            updateTableView();
            updateHeaderTable();
            popUp->setPopupText(tr("Obiectul <b>'%1'</b> a fost salvat <br>in baza de date cu succes.").arg(cat_General->getFullName()));
            popUp->show();
        });
        cat_General->exec();
        break;
    case Pacients:
        cat_General = new CatGeneral(this);
        cat_General->setProperty("itNew", true);
        cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Pacients);
        cat_General->setAttribute(Qt::WA_DeleteOnClose);
        connect(cat_General, &CatGeneral::createCatGeneral, this, [this]()
        {
            updateTableView();
            updateHeaderTable();
            popUp->setPopupText(tr("Obiectul <b>'%1'</b> a fost salvat <br>in baza de date cu succes.").arg(cat_General->getFullName()));
            popUp->show();
        });
        cat_General->exec();
        break;
    case Organizations:
        cat_Organizations = new CatOrganizations(this);
        cat_Organizations->setProperty("itNew", true);
        cat_Organizations->setAttribute(Qt::WA_DeleteOnClose);
        connect(cat_Organizations, &CatOrganizations::mWriteNewOrganization, this, [this]()
        {
            updateTableView();
            updateHeaderTable();
            popUp->setPopupText(tr("Obiectul <b>'%1'</b> a fost salvat <br>in baza de date cu succes.")
                                .arg(cat_Organizations->getNameOrganization()));
            popUp->show();

        });
        cat_Organizations->exec();
        break;
    case Users:
        cat_Users = new CatUsers(this);
        cat_Users->setProperty("itNew", true);
        cat_Users->setProperty("PwdHash", false);
        cat_Users->setAttribute(Qt::WA_DeleteOnClose);
        connect(cat_Users, &CatUsers::mCreateNewUser, this, [this]()
        {
            updateTableView();
            updateHeaderTable();
            popUp->setPopupText(tr("Obiectul <b>'%1'</b> a fost salvat <br>in baza de date cu succes.").arg(cat_Users->getNameUser()));
            popUp->show();
        });
        cat_Users->exec();
        break;
    default:
        qWarning(logWarning()) << tr("Nu corect a fost transmisa proprietatea 'typeListForm' clasei '%1' !!!").arg(metaObject()->className());
        break;
    }
    return false;
}

void ListForm::onEditObject()
{
    if (ui->tabView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    int _id = proxy->data(proxy->index(ui->tabView->currentIndex().row(), 0), Qt::DisplayRole).toInt();

    QMap<QString, QString> items;

    switch (m_typeListForm) {
    case Doctors:
        if (db->getObjectDataById("doctors", _id, items)){
            cat_General = new CatGeneral(this);
            cat_General->setProperty("itNew", false);
            cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
            cat_General->setProperty("Id", _id);
            cat_General->setAttribute(Qt::WA_DeleteOnClose);
            connect(cat_General, &CatGeneral::changedCatGeneral, this, [this]()
            {
                updateTableView();
                updateHeaderTable();
                popUp->setPopupText(tr("Datele obiectului <b>'%1'</b> <br>au fost modificate cu succes.").arg(cat_General->getFullName()));
                popUp->show();
            });
            cat_General->exec();
        }
        break;
    case Nurses:
        if (db->getObjectDataById("nurses", _id, items)){
            cat_General = new CatGeneral(this);
            cat_General->setProperty("itNew", false);
            cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
            cat_General->setProperty("Id", _id);
            cat_General->setAttribute(Qt::WA_DeleteOnClose);
            connect(cat_General, &CatGeneral::changedCatGeneral, this, [this]()
            {
                updateTableView();
                updateHeaderTable();
                popUp->setPopupText(tr("Datele obiectului <b>'%1'</b> <br>au fost modificate cu succes.").arg(cat_General->getFullName()));
                popUp->show();
            });
            cat_General->exec();
        }
        break;
    case Pacients:
        if (db->getObjectDataById("pacients", _id, items)){
            cat_General = new CatGeneral(this);
            cat_General->setProperty("itNew", false);
            cat_General->setProperty("typeCatalog", CatGeneral::TypeCatalog::Pacients);
            cat_General->setProperty("Id", _id);
            cat_General->setAttribute(Qt::WA_DeleteOnClose);
            connect(cat_General, &CatGeneral::changedCatGeneral, this, [this]()
            {
                updateTableView();
                updateHeaderTable();
                popUp->setPopupText(tr("Datele obiectului <b>'%1'</b> <br>au fost modificate cu succes.").arg(cat_General->getFullName()));
                popUp->show();
            });
            cat_General->exec();
        }
        break;
    case Organizations:
        if (db->getObjectDataById("organizations", _id, items)){
            cat_Organizations = new CatOrganizations(this);
            cat_Organizations->setProperty("itNew", false);
            cat_Organizations->setProperty("Id", _id);
            cat_Organizations->setAttribute(Qt::WA_DeleteOnClose);
            connect(cat_Organizations, &CatOrganizations::mChangedDateOrganization, this, [this]()
            {
                updateTableView();
                updateHeaderTable();
                popUp->setPopupText(tr("Datele obiectului <b>'%1'</b><br>au fost modificate cu succes.").arg(cat_Organizations->getNameOrganization()));
                popUp->show();
            });
            cat_Organizations->exec();
        }
        break;
    case Users:
        if (db->getObjectDataById("users", _id, items)){
            cat_Users = new CatUsers(this);
            cat_Users->setProperty("itNew", false);
            cat_Users->setProperty("Id", _id);
            cat_Users->setAttribute(Qt::WA_DeleteOnClose);
            connect(cat_Users, &CatUsers::mChangedDataUser, this, [this]()
            {
                updateTableView();
                updateHeaderTable();
                popUp->setPopupText(tr("Datele obiectului <b>'%1'</b><br>au fost modificate cu succes.").arg(cat_Users->getNameUser()));
                popUp->show();
            });
            cat_Users->exec();
        }
        break;
    default:
        qWarning(logWarning()) << tr("Eroare la editare a obiectului: nu este determinata proprietatea 'typeListForm' !!!");
        break;
    }
}

void ListForm::onDeletionMarkObject()
{
    if (ui->tabView->currentIndex().row() == -1){
        QMessageBox::warning(this, tr("Atenție"), tr("Nu este marcat randul !!!."), QMessageBox::Ok);
        return;
    }

    int _row            = ui->tabView->currentIndex().row();
    int _id             = proxy->index(_row, section_id).data(Qt::DisplayRole).toInt();
    QString _nameObject = proxy->index(_row, section_name).data(Qt::DisplayRole).toString();

    QString _nameTable = getNameTable();
    if (_nameTable == nullptr)
        return;

    if (db->deletionMarkObject(_nameTable, _id)){        
        popUp->setPopupText(tr("Obiectul <b>%1<b><br> a fost marcat/demarcat cu succes in baza de date.").arg(_nameObject));
        popUp->show();
        qInfo(logInfo()) << tr("Obiectul '%1' cu id='%2' a fost marcat/demarcat cu succes.").arg(_nameObject, QString::number(_id));
    } else {
        QMessageBox::warning(this, tr("Marcarea/demarcarea obiectului"),
                             tr("Marcarea/demarcarea obiectului \"<b>%1</b>\" nu este reusita.").arg(_nameObject), QMessageBox::Ok);
        qInfo(logInfo()) << tr("Eraoare la marcare pentru eliminarea din baza de date a obiectului '%1' cu id='%2'").arg(_nameObject, QString::number(_id));
        return;
    }
    updateTableView();
    updateHeaderTable();
}

void ListForm::onClose()
{
    this->close();
    emit mCloseThisForm();
}

void ListForm::onDoubleClickedTable(const QModelIndex &index)
{
    Q_UNUSED(index);
    onEditObject();
}

// **********************************************************************************
// --- meniu contextual

void ListForm::slotContextMenuRequested(QPoint pos)
{
    int _row = ui->tabView->currentIndex().row();
    int _id  = proxy->index(_row, section_id).data(Qt::DisplayRole).toInt();
    QString _nameObject = proxy->index(_row, section_name).data(Qt::DisplayRole).toString();

    QString _nameTable = getNameTable();
    if (_nameTable == nullptr)
        return;

    QString strQueryDeletionMark;
    int statusDeletionMark = db->statusDeletionMarkObject(_nameTable, _id);
    if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_UNMARK)
        strQueryDeletionMark = tr("Marchează obiectul \"%1\" pentru eliminarea din baza de date.").arg(_nameObject);
    else if (statusDeletionMark == DataBase::REQUIRED_NUMBER::DELETION_MARK)
        strQueryDeletionMark = tr("Demarchează obiectul \"%1\" pentru eliminarea din baza de date.").arg(_nameObject);
    else
        qWarning(logWarning()) << tr("%1 - slotContextMenuRequested():").arg(metaObject()->className())
                               << tr("Status 'deletionMark' a obiectului cu ID=%1 nu este determinat !!!").arg(QString::number(_id));

    QAction* actionAddObject  = new QAction(QIcon(":/img/add_x32.png"), tr("Adaugă obiect nou."), this);
    QAction* actionEditObject = new QAction(QIcon(":/img/edit_x32.png"), tr("Editează obiect \"%1\".").arg(_nameObject), this);
    QAction* actionMarkObject = new QAction(QIcon(":/img/clear_x32.png"), strQueryDeletionMark, this);

    connect(actionAddObject, &QAction::triggered, this, &ListForm::onAddObject);
    connect(actionEditObject, &QAction::triggered, this, &ListForm::onEditObject);
    connect(actionMarkObject, &QAction::triggered, this, &ListForm::onDeletionMarkObject);

    menu->clear();
    menu->addAction(actionAddObject);
    menu->addAction(actionEditObject);
    menu->addAction(actionMarkObject);
    menu->popup(ui->tabView->viewport()->mapToGlobal(pos)); // prezentarea meniului
}

// **********************************************************************************
// --- evenimente

void ListForm::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSizeSectionTable();
    }
}

void ListForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        updateHeaderTable();
    }
}

void ListForm::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End){
        ui->tabView->selectRow(model->rowCount() - 1);
    } else if (event->key() == Qt::Key_Home){
        ui->tabView->selectRow(0);
    }
}


