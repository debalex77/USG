#include "onlineaccountview.h"
#include "ui_onlineaccountview.h"
#include <data/database_common.h>

OnlineAccountView::OnlineAccountView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OnlineAccountView)
    , m_settings(globals().pathSettingsCommon)
    , m_db(db)
    , popUp(new PopUp(this))
    , toolBar(new ToolBarCustom(this,
                                ToolBarCustom::AddEditDelete |
                                    ToolBarCustom::UpdateColumn))
    , model(new OnlineAccountModel(this))
    , toolButtonStyleForIcon(m_db.toolButtonStyleForIcon())
{
    ui->setupUi(this);

    const QString schema = globals().thisMySQL
        ? QStringLiteral(":/sql/mariadb/tables/online_account.sql")
        : QStringLiteral(":/sql/sqlite/tables/online_account.sql");
    DataBaseCommon::execFileBatch(m_db.getDatabase(), schema, "onlineAccount");

    setWindowTitle(tr("Online account"));

    loadFilterJournalBySettings();

    initTableView();
    updateTableView();

    initToolBar();
}

OnlineAccountView::~OnlineAccountView()
{
    delete ui;
}

void OnlineAccountView::onAdd()
{
    auto *dlg = new OnlineAccountDialog(m_db, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setProperty("isNew", true);

    connect(dlg, &OnlineAccountDialog::onlineAccountCreated,
            this, &OnlineAccountView::updateTableView, Qt::UniqueConnection);

    dlg->show();
}

void OnlineAccountView::onEdit()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!idx.isValid())
        return;

    const QVariantMap data = model->rowDataByRow(idx.row());

    auto *dlg = new OnlineAccountDialog(m_db, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setProperty("isNew", false);
    dlg->setProperty("id", data["id"].toInt());
    dlg->setProperty("idOrganization", data["id_organizations"].toInt());

    connect(dlg, &OnlineAccountDialog::onlineAccountChanged,
            this, &OnlineAccountView::updateTableView, Qt::UniqueConnection);

    dlg->show();
}

void OnlineAccountView::onDelete()
{
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!idx.isValid())
        return;

    QVariantMap data = model->rowDataByRow(idx.row());

    QSqlQuery q(m_db.getDatabase());
    q.prepare("DELETE FROM onlineAccount WHERE id = :id");
    q.bindValue(":id", data["id"].toInt());
    if (!q.exec()) {
        qWarning(logWarning()).noquote()
            << "SQL error:" << q.lastError().text()
            << "\nLast query:" << q.lastQuery();
        return;
    }
    qInfo(logInfo()) << QStringLiteral("A fost eliminat online account '%1' din baza de date.")
        .arg(data["email"].toString());
    updateTableView();
}

void OnlineAccountView::onUpdate()
{
    updateTableView();
}

void OnlineAccountView::onShowHideColumn()
{
    if (!m_columnsController)
        return;

    auto btn = toolBar->getBtnHideShowColumn();
    QPoint p = QPoint(0, btn->height());
    const QPoint globalPos = btn->mapToGlobal(p);
    m_columnsController->showMenu(globalPos);
}

void OnlineAccountView::onColumnsChanged()
{
    if (!m_columnsController)
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

    saveSettingsJournal();
}

void OnlineAccountView::onDoubleClickedTableView(const QModelIndex &index)
{
    Q_UNUSED(index);
    onEdit();
}

void OnlineAccountView::slotContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = ui->tableView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);

    QAction *actionNew = menu.addAction(QIcon(":/img/toolBar/add.png"),
                                           tr("Creează account now."));
    QAction *actionEdit = menu.addAction(QIcon(":/img/toolBar/edit.png"),
                                            tr("Editează account."));
    QAction *actionDelete = menu.addAction(QIcon(":/img/toolBar/delete.png"),
                                              tr("Elimină account"));

    QAction *selectedAction = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == actionNew)
        onAdd();
    else if (selectedAction == actionEdit)
        onEdit();
    else if (selectedAction == actionDelete)
        onDelete();
}

void OnlineAccountView::loadFilterJournalBySettings()
{
    const QJsonObject obj = m_settings.getJsonObject(m_className);

    // --- size section
    const QJsonObject sectionsObj = obj.value("sections").toObject();
    for (auto it = sectionsObj.begin(); it != sectionsObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.sectionSizes[index] = it.value().toInt();
    }

    // --- hide/show section
    const QJsonObject hiddenObj = obj.value("hide_show_sections").toObject();
    for (auto it = hiddenObj.begin(); it != hiddenObj.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok)
            m_filter.hiddenSections[index] = (it.value().toInt() != 0);
    }
}

void OnlineAccountView::initToolBar()
{
    toolBar->setStyles(toolButtonStyleForIcon);
    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addStretch();

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &OnlineAccountView::onAdd, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &OnlineAccountView::onEdit, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &OnlineAccountView::onDelete, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::updateTable,
            this, &OnlineAccountView::onUpdate, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::hideShowColumn,
            this, &OnlineAccountView::onShowHideColumn, Qt::UniqueConnection);
}

void OnlineAccountView::initTableView()
{
    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true); // ID
    ui->tableView->setColumnHidden(2, true); // id_organizations
    ui->tableView->setColumnHidden(3, true); // id_users

    ui->tableView->setWordWrap(false);
    ui->tableView->setTextElideMode(Qt::ElideRight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    if (!m_columnsController)
        m_columnsController = new TableColumnsController(ui->tableView, this);
    m_columnsController->setFixedHiddenColumns({0,2,3});
    m_columnsController->setExcludedFromMenuColumns({1});

    connect(m_columnsController, &TableColumnsController::columnsChanged,
            this, &OnlineAccountView::onColumnsChanged);

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &OnlineAccountView::onDoubleClickedTableView, Qt::UniqueConnection);
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &OnlineAccountView::slotContextMenuRequested, Qt::UniqueConnection);

    loadSizeSection();
}

void OnlineAccountView::updateTableView()
{
    int currentId = -1;

    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid())
        currentId = model->idByRow(currentIndex.row());

    QSqlQuery q(m_db.getDatabase());
    q.prepare(R"(
        SELECT
            id,
            0 AS deletionMark,
            id_organizations,
            id_users,
            email,
            smtp_server,
            port,
            username
        FROM
            onlineAccount
    )");

    if (!q.exec()) {
        qCritical(logCritical()).noquote()
            << "SQL error:" << q.lastError().text()
            << "\nLast query:" << q.lastQuery();
        return;
    }

    QList<QVariantMap> rows;

    while (q.next()) {
        QVariantMap row;
        row["id"]               = q.value("id");
        row["deletionMark"]     = q.value("deletionMark");
        row["id_organizations"] = q.value("id_organizations");
        row["id_users"]         = q.value("id_users");
        row["email"]            = q.value("email");
        row["smtp_server"]      = q.value("smtp_server");
        row["port"]             = q.value("port");
        row["username"]         = q.value("username");
        rows.append(row);
    }

    model->setRows(rows);

    if (currentId >= 0) {
        const int row = model->rowById(currentId);
        if (row >= 0) {
            ui->tableView->selectRow(row);
            ui->tableView->scrollTo(model->index(row, 0));
        }
    }
}

void OnlineAccountView::loadSizeSection()
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

int OnlineAccountView::lastVisibleSection() const
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

void OnlineAccountView::saveSettingsJournal()
{
    auto *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    const int lastVisible = lastVisibleSection();

    // sections + hidden
    const int count = header->count();
    for (int section = 0; section < count; ++section) {

        m_settings.setValue(m_className,
                            QString("hide_show_sections/%1").arg(section),
                            header->isSectionHidden(section) ? 1 : 0);

        // NU salvam latimea pentru ultima sectie vizibila, fiindca e stretch-uită
        if (section == lastVisible)
            continue;

        const int width = header->sectionSize(section);
        if (width > 0) {
            m_settings.setValue(m_className,
                                QString("sections/%1").arg(section),
                                width);
        }

    }

    m_settings.save();
}

void OnlineAccountView::reject()
{
    if (auto *sub = qobject_cast<QMdiSubWindow *>(parentWidget())) {
        sub->close();      // inchide subfereastra MDI si dialogul intern
        return;
    }

    QDialog::reject();     // fallback normal
}

void OnlineAccountView::closeEvent(QCloseEvent *event)
{
    if (event->type() == QEvent::Close){
        saveSettingsJournal();
    }
}

void OnlineAccountView::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void OnlineAccountView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End){
        ui->tableView->selectRow(model->rowCount() - 1);
        return;
    } else if (event->key() == Qt::Key_Home){
        ui->tableView->selectRow(0);
        return;
    }
    QDialog::keyPressEvent(event);
}
