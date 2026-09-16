#include "catalogview.h"
#include "catalogs/catalogdialog.h"
#include "ui_catalogview.h"

CatalogView::CatalogView(DataBase &db, CatalogType::Type catalogType, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CatalogView)
    , m_settings(globals().pathSettingsCommon)
    , m_catalogType(catalogType)
    , m_db(db)
    , popUp(new PopUp(this))
    , menu(new QMenu(this))
    , toolBar(new ToolBarCustom(this,
                                 ToolBarCustom::AddEditDelete |
                                 ToolBarCustom::UpdateColumn))
    , model(new CatalogsModel(m_db, m_catalogType, this))
    , proxy(new SortModel(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("Catalog: %1")
                       .arg(CatalogType::enumToStringRo(m_catalogType)));

    loadFilterBySettings();

    initTableView();
    updateTableView();

    initToolBar();
}

CatalogView::~CatalogView()
{
    delete ui;
}

void CatalogView::onScroll(int value)
{
    auto *sb = ui->tableView->verticalScrollBar();
    if (!sb)
        return;

    // când ajunge aproape de final
    if (value < sb->maximum() - 8)
        return;

    if (!proxy || !model)
        return;

    if (model->canFetchMore())
        model->fetchMore();
}

void CatalogView::onAdd()
{
    if (m_catalogType == CatalogType::Type::Doctors ||
        m_catalogType == CatalogType::Type::Nurses ||
        m_catalogType == CatalogType::Type::Patients) {

        CatalogDialog *catalogCommon = new CatalogDialog(m_db, m_catalogType, this);
        catalogCommon->setAttribute(Qt::WA_DeleteOnClose);
        catalogCommon->setProperty("isNew", true);
        connect(catalogCommon, &CatalogDialog::catalogDialogCreated,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        catalogCommon->show();

    } else if (m_catalogType == CatalogType::Type::Users) {

        UserDialog *user = new UserDialog(m_db, this);
        user->setAttribute(Qt::WA_DeleteOnClose);
        user->setProperty("isNew", true);
        connect(user, &UserDialog::userCreated,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        user->show();

    } else if (m_catalogType == CatalogType::Type::Organizations) {

        OrganizationDialog *org = new OrganizationDialog(m_db, this);
        org->setAttribute(Qt::WA_DeleteOnClose);
        org->setProperty("isNew", true);
        connect(org, &OrganizationDialog::organizationCreated,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        org->show();
    }
}

void CatalogView::onEdit()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxy->mapToSource(idx);
    const CatalogsCommon &item = model->itemAt(sourceIndex.row());

    if (m_catalogType == CatalogType::Type::Doctors ||
        m_catalogType == CatalogType::Type::Nurses ||
        m_catalogType == CatalogType::Type::Patients) {

        CatalogDialog *catalogCommon = new CatalogDialog(m_db, m_catalogType, this);
        catalogCommon->setProperty("isNew", false);
        catalogCommon->setProperty("id", item.id);
        connect(catalogCommon, &CatalogDialog::catalogDialogChanged,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        catalogCommon->show();

    } else if (m_catalogType == CatalogType::Type::Users) {

        UserDialog *user = new UserDialog(m_db, this);
        user->setAttribute(Qt::WA_DeleteOnClose);
        user->setProperty("isNew", false);
        user->setProperty("id", item.id);
        connect(user, &UserDialog::userChanged,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        user->show();

    } else if (m_catalogType == CatalogType::Type::Organizations) {
        OrganizationDialog *org = new OrganizationDialog(m_db, this);
        org->setAttribute(Qt::WA_DeleteOnClose);
        org->setProperty("isNew", false);
        org->setProperty("id", item.id);
        connect(org, &OrganizationDialog::organizationChanged,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        org->show();
    }
}

void CatalogView::onDelete()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    const QModelIndex sourceIndex = proxy->mapToSource(idx);
    const CatalogsCommon &item = model->itemAt(sourceIndex.row());

    bool isMark = item.deletionMark == StatusObject::DeletionMark;
    QString err;

    if (m_catalogType == CatalogType::Type::Doctors ||
        m_catalogType == CatalogType::Type::Nurses ||
        m_catalogType == CatalogType::Type::Patients) {

        CatalogDialog catalogCommon(m_db, m_catalogType, this);
        catalogCommon.setProperty("isNew", false);
        catalogCommon.setProperty("id", item.id);
        catalogCommon.setProperty("statusCatalog", isMark
                                                        ? StatusObject::ZeroWrite
                                                        : StatusObject::DeletionMark);
        connect(&catalogCommon, &CatalogDialog::catalogDialogDeletedMark,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        if (catalogCommon.setDeleteMarkCatalog(err)) {
            popUp->setPopupText(
                isMark
                    ? tr("%1 <b>%2</b><br>"
                         "nu mai este marcată pentru eliminare.")
                          .arg(CatalogType::enumToString(m_catalogType),
                               item.fullName)
                    : tr("%1 <b>%2</b><br>"
                         "a fost marcată pentru eliminare.")
                          .arg(CatalogType::enumToString(m_catalogType),
                               item.fullName)
                );
            popUp->show();
        }  else {
            if (err.isEmpty())
                return;
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("%1 '%2' nu poate fi marcat pentru eliminare !!!")
                                  .arg(CatalogType::enumToString(m_catalogType),
                                       item.fullName));
            msg.setDetailedText(err);
            msg.exec();
        }

    } else if (m_catalogType == CatalogType::Type::Users) {

        UserDialog user(m_db, this);
        user.setProperty("isNew", false);
        user.setProperty("id", item.id);
        user.setProperty("statusCatalog", isMark
                                               ? StatusObject::ZeroWrite
                                               : StatusObject::DeletionMark);
        connect(&user, &UserDialog::userDeletedMark,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        if (user.setDeleteMarkUser(err)) {
            popUp->setPopupText(
                isMark
                    ? tr("Utilizatorul <b>%1</b><br>"
                         "nu mai este marcată pentru eliminare.")
                          .arg(item.fullName)
                    : tr("Utilizatorul <b>%1</b><br>"
                         "a fost marcată pentru eliminare.")
                          .arg(item.fullName)
                );
            popUp->show();
        } else {
            if (err.isEmpty())
                return;
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Marcarea utilizatorului '%1' nu s-a efectuat !!!")
                                 .arg(item.fullName));
            msg.setDetailedText(err);
            msg.exec();
        }

    } else if (m_catalogType == CatalogType::Type::Organizations) {

        OrganizationDialog org(m_db, this);
        org.setProperty("isNew", false);
        org.setProperty("id", item.id);
        org.setProperty("statusCatalog", isMark
                                            ? StatusObject::ZeroWrite
                                            : StatusObject::DeletionMark);

        connect(&org, &OrganizationDialog::organizationDeletedMark,
                this, &CatalogView::updateTableView, Qt::UniqueConnection);
        if (org.setDeleteMarkOrganization(err)) {
            popUp->setPopupText(
                isMark
                    ? tr("Organizația <b>%1</b><br>"
                         "nu mai este marcată pentru eliminare.")
                          .arg(item.fullName)
                    : tr("Organizația <b>%1</b><br>"
                         "a fost marcată pentru eliminare.")
                          .arg(item.fullName)
                );
            popUp->show();

        } else {
            if (err.isEmpty())
                return;
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Marcarea organizatiei '%1' nu s-a efectuat !!!")
                                  .arg(item.fullName));
            msg.setDetailedText(err);
            msg.exec();
        }
    }

    ui->tableView->selectRow(m_currentRow);
}

void CatalogView::onUpdate()
{
    updateTableView();
}

void CatalogView::onShowHideColumn()
{
    if (!m_columnsController)
        return;

    auto btn = toolBar->getBtnHideShowColumn();
    QPoint p = QPoint(0, btn->height());
    const QPoint globalPos = btn->mapToGlobal(p);
    m_columnsController->showMenu(globalPos);
}

void CatalogView::onClickedTableView(const QModelIndex &index)
{
    if (! index.isValid())
        return;
    m_currentRow = index.row();
}

void CatalogView::onDoubleClickedTableView(const QModelIndex &index)
{
    Q_UNUSED(index);
    onEdit();
}

void CatalogView::onColumnsChanged()
{
    if (!m_columnsController || !ui || !ui->tableView)
        return;

    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    m_filter.hiddenSections = m_columnsController->hiddenSections();

    const int count = header->count();
    for (int col = 0; col < count; ++col) {

        if (!ui->tableView->isColumnHidden(col) && header->sectionSize(col) <= 0) {
            int width = m_filter.sectionSizes.value(col, 0);
            if (width <= 0) {
                width = header->sectionSizeHint(col);
                if (width <= 0)
                    width = header->defaultSectionSize();
            }
            header->resizeSection(col, width);
        }

    }

    ui->tableView->update();

    saveSizeSection();
}

void CatalogView::loadFilterBySettings()
{
    const QString catalog = CatalogType::enumToString(m_catalogType);

    const QJsonObject rootObj = m_settings.getJsonObject(m_class);
    if (rootObj.isEmpty()) {
        return;
    }

    const QJsonObject catalogObj = rootObj.value(catalog).toObject();
    if (catalogObj.isEmpty()) {
        return;
    }

    // --- sortarea sectiilor
    m_filter.sortSection = catalogObj.value("sort").toObject().value("section").toInt(0);
    m_filter.sortOrder = catalogObj.value("sort").toObject().value("direction").toInt(0) == 0
                             ? Qt::AscendingOrder
                             : Qt::DescendingOrder;

    // --- size section
    const QJsonObject sectionsObj = catalogObj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = catalogObj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void CatalogView::loadSizeSection()
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    ui->tableView->setUpdatesEnabled(false);

    const bool oldStretch = header->stretchLastSection();
    header->setStretchLastSection(false);

    const int colCount = header->count();

    // 1. restauram latimile
    for (int col = 0; col < colCount; ++col) {

        int width = m_filter.sectionSizes.value(col, 0);

        if (width <= 0) {
            width = header->sectionSizeHint(col);
            if (width <= 0)
                width = header->defaultSectionSize();
        }

        header->resizeSection(col, width);
    }

    // 2. restauram hide/show
    if (m_columnsController)
        m_columnsController->setHiddenSections(m_filter.hiddenSections);

    header->setStretchLastSection(oldStretch);

    if (ui->tableView->model() && ui->tableView->model()->rowCount() > 0)
        ui->tableView->selectRow(0);

    ui->tableView->setUpdatesEnabled(true);
}

void CatalogView::saveSizeSection()
{
    const QString prefix = CatalogType::enumToString(m_catalogType) + "/";

    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    // sortare
    m_settings.setValue(m_class, prefix + "sort/section",
                        header->sortIndicatorSection());
    m_settings.setValue(m_class, prefix + "sort/direction",
                        static_cast<int>(header->sortIndicatorOrder()));

    const int lastVisible = lastVisibleSection();

    // sections + hidden
    const int count = header->count();
    for (int section = 0; section < count; ++section) {

        m_settings.setValue(m_class,
                            QString(prefix + "hide_show_sections/%1").arg(section),
                            header->isSectionHidden(section) ? 1 : 0);

        // NU salvam latimea pentru ultima sectie vizibila, fiindca e stretch-uită
        if (section == lastVisible)
            continue;

        const int width = header->sectionSize(section);
        if (width > 0) {
            m_settings.setValue(m_class,
                                QString(prefix + "sections/%1").arg(section),
                                width);
        }

    }

    m_settings.save();
}

void CatalogView::initTableView()
{
    model->setBatchSize(100);

    proxy->setSourceModel(model);
    proxy->setSortRole(CatalogsModel::SortRole);
    proxy->setDynamicSortFilter(false);

    ui->tableView->setModel(proxy);

    ui->tableView->hideColumn(0); //Id

    if (!m_columnsController)
        m_columnsController = new TableColumnsController(ui->tableView, this);

    switch (m_catalogType) {
    case CatalogType::Type::Doctors:
        ui->tableView->hideColumn(DoctorsSections::Uuid);
        m_columnsController->setFixedHiddenColumns({DoctorsSections::Id,
                                                    DoctorsSections::Uuid});
        m_columnsController->setExcludedFromMenuColumns({DoctorsSections::DeletionMark});
        break;
    case CatalogType::Type::Nurses:
        ui->tableView->hideColumn(NursesSections::Uuid);
        m_columnsController->setFixedHiddenColumns({NursesSections::Id,
                                                    NursesSections::Uuid});
        m_columnsController->setExcludedFromMenuColumns({NursesSections::DeletionMark});
        break;
    case CatalogType::Type::Patients:
        ui->tableView->hideColumn(PatientsColumns::Uuid);
        ui->tableView->hideColumn(NursesSections::Uuid);
        m_columnsController->setFixedHiddenColumns({PatientsColumns::Id,
                                                    PatientsColumns::Uuid});
        m_columnsController->setExcludedFromMenuColumns({PatientsColumns::DeletionMark});
        break;
    case CatalogType::Type::Users:
        ui->tableView->hideColumn(UsersSections::Password);
        ui->tableView->hideColumn(UsersSections::Hash);
        ui->tableView->hideColumn(UsersSections::Uuid);
        m_columnsController->setFixedHiddenColumns({UsersSections::Id,
                                                    UsersSections::Uuid,
                                                    UsersSections::Password,
                                                    UsersSections::Hash});
        m_columnsController->setExcludedFromMenuColumns({UsersSections::DeletionMark});
        break;
    case CatalogType::Type::Organizations:
        ui->tableView->hideColumn(OrganizationsSections::Id_contracts);
        ui->tableView->hideColumn(OrganizationsSections::Stamp);
        ui->tableView->hideColumn(OrganizationsSections::Uuid);
        m_columnsController->setFixedHiddenColumns({OrganizationsSections::Id,
                                                    OrganizationsSections::Id_contracts,
                                                    OrganizationsSections::Stamp,
                                                    OrganizationsSections::Uuid});
        m_columnsController->setExcludedFromMenuColumns({OrganizationsSections::DeletionMark});
        break;
    default:
        break;
    }

    ui->tableView->setItemDelegateForColumn(1, new CenterIconDelegate(ui->tableView));
    ui->tableView->setWordWrap(false);
    ui->tableView->setTextElideMode(Qt::ElideRight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    // Sorting must happen in SQL before LIMIT, not only on fetched rows.
    ui->tableView->setSortingEnabled(false);
    auto *sortHeader = ui->tableView->horizontalHeader();
    sortHeader->setSectionsClickable(true);
    sortHeader->setSortIndicatorShown(true);
    connect(sortHeader, &QHeaderView::sortIndicatorChanged, this,
            [this](int column, Qt::SortOrder order) {
        m_filter.sortSection = column;
        m_filter.sortOrder = order;
        model->setSort(column, order);
        ui->tableView->scrollToTop();
    });
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_columnsController, &TableColumnsController::columnsChanged,
            this, &CatalogView::onColumnsChanged);

    loadSizeSection();

    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &CatalogView::onScroll, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),
            this, &CatalogView::onClickedTableView, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &CatalogView::onDoubleClickedTableView, Qt::UniqueConnection);
}

void CatalogView::updateTableView()
{
    qInfo(logInfo()) << "CatalogView: vizualizarea/actualizarea jurnalului" << CatalogType::enumToStringRo(m_catalogType);
    if (!model || !proxy)
        return;

    if (!model)
        return;

    {
        QSignalBlocker blocker(ui->tableView->horizontalHeader());
        ui->tableView->horizontalHeader()->setSortIndicator(m_filter.sortSection, m_filter.sortOrder);
    }
    model->setSort(m_filter.sortSection, m_filter.sortOrder);

    if (proxy->rowCount() <= 0)
        return;

    int rowToSelect = 0;
    if (m_currentRow >= 0 && m_currentRow < proxy->rowCount())
        rowToSelect = m_currentRow;

    ui->tableView->selectRow(rowToSelect);
}

void CatalogView::initToolBar()
{
    toolBar->setStyles(m_db.toolButtonStyleForIcon());

    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addStretch();

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &CatalogView::onAdd, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &CatalogView::onEdit, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &CatalogView::onDelete, Qt::UniqueConnection);

    connect(toolBar, &ToolBarCustom::updateTable,
            this, &CatalogView::onUpdate, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::hideShowColumn,
            this, &CatalogView::onShowHideColumn, Qt::UniqueConnection);
}

bool CatalogView::isValidIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        QMessageBox::warning(this,
                             tr("Informație"),
                             tr("Nu este marcat rândul."),
                             QMessageBox::Ok);
        return false;
    }
    return true;
}

int CatalogView::lastVisibleSection() const
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return -1;

    for (int section = header->count() - 1; section >= 0; --section) {
        if (!header->isSectionHidden(section))
            return section;
    }

    return -1;
}

void CatalogView::reject()
{
    if (auto *sub = qobject_cast<QMdiSubWindow *>(parentWidget())) {
        sub->close();      // inchide subfereastra MDI si dialogul intern
        return;
    }

    QDialog::reject();     // fallback normal
}

void CatalogView::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSizeSection();
    }
}

bool CatalogView::eventFilter(QObject *obj, QEvent *event)
{
    return QDialog::eventFilter(obj, event);
}

void CatalogView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Catalog: %1")
                           .arg(CatalogType::enumToStringRo(m_catalogType)));
    }
}

void CatalogView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End)
        ui->tableView->selectRow(model->rowCount() - 1);
    if (event->key() == Qt::Key_Home)
        ui->tableView->selectRow(0);
}
