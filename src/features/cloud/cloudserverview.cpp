/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 Codreanu Alexandru <alovada.med@gmail.com>
 *
 *****************************************************************************/

#include "cloudserverview.h"
#include "ui_cloudserverview.h"

#include <app/popup.h>
#include <common/applicationpathscontext.h>
#include <common/cloudconnectioncontext.h>
#include <common/sessioncontext.h>
#include <features/cloud/cloudserverconfig.h>
#include <settings/settingsservice.h>
#include <ui/services/tablecolumnscontroller.h>
#include <ui/widgets/toolbarcustom.h>

#include <QCloseEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

CloudServerView::CloudServerView(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CloudServerView)
    , m_db(db)
    , m_settings(ApplicationPathsContext::instance().tableSettingsFilePath())
    , m_model(new QSqlQueryModel(this))
    , m_toolBar(new ToolBarCustom(this,
                                  ToolBarCustom::AddEditDelete |
                                      ToolBarCustom::UpdateColumn))
    , m_popUp(new PopUp(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Configurările serverelor cloud"));

    loadSettings();
    initTableView();
    updateTableView();
    restoreColumns();
    initToolBar();
}

CloudServerView::~CloudServerView()
{
    delete ui;
}

void CloudServerView::onAdd()
{
    CloudServerConfig dialog(this);
    dialog.exec();
    updateTableView();
}

void CloudServerView::onEdit()
{
    const QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid())
        return;

    const int userId = m_model->index(index.row(), UserId).data().toInt();
    const int organizationId =
        m_model->index(index.row(), OrganizationId).data().toInt();

    CloudServerConfig dialog(this);
    // Organizația trebuie setată după utilizator: încărcarea configurației
    // folosește ambele identificatoare.
    dialog.setProperty("ID_user", userId);
    dialog.setProperty("ID_Organization", organizationId);
    dialog.exec();

    updateTableView();
}

void CloudServerView::onDelete()
{
    const QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid())
        return;

    const int id = m_model->index(index.row(), Id).data().toInt();
    const int organizationId =
        m_model->index(index.row(), OrganizationId).data().toInt();
    const int userId = m_model->index(index.row(), UserId).data().toInt();
    const QString organization =
        m_model->index(index.row(), Organization).data().toString();
    const QString user = m_model->index(index.row(), User).data().toString();

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Eliminarea configurării cloud"),
        tr("Doriți să eliminați configurarea cloud pentru organizația „%1” "
           "și utilizatorul „%2”?")
            .arg(organization, user),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    QSqlDatabase database = m_db.getDatabase();
    if (!database.transaction()) {
        QMessageBox::critical(this,
                              tr("Eliminarea configurării cloud"),
                              tr("Tranzacția nu a putut fi pornită: %1")
                                  .arg(database.lastError().text()));
        return;
    }

    QSqlQuery query(database);
    query.prepare(QStringLiteral(R"(
        DELETE FROM
            cloudServer
        WHERE
            id = ?
    )"));
    query.addBindValue(id);

    if (!query.exec() || query.numRowsAffected() != 1) {
        const QString error = query.lastError().isValid()
            ? query.lastError().text()
            : tr("Înregistrarea selectată nu a fost găsită.");
        database.rollback();
        qCritical(logCritical())
            << "CloudServerView: eliminarea configurării a eșuat:" << error;
        QMessageBox::critical(this,
                              tr("Eliminarea configurării cloud"),
                              tr("Configurarea cloud nu a putut fi eliminată: %1")
                                  .arg(error));
        return;
    }

    if (!database.commit()) {
        const QString error = database.lastError().text();
        database.rollback();
        qCritical(logCritical())
            << "CloudServerView: confirmarea eliminării a eșuat:" << error;
        QMessageBox::critical(this,
                              tr("Eliminarea configurării cloud"),
                              tr("Eliminarea nu a putut fi confirmată: %1")
                                  .arg(error));
        return;
    }

    clearActiveCloudContextIfNeeded(organizationId, userId);
    qInfo(logInfo())
        << "Configurarea cloud a fost eliminată. id=" << id
        << "organization_id=" << organizationId << "user_id=" << userId;

    updateTableView();
    m_popUp->setPopupText(tr("Configurarea cloud a fost eliminată."));
    m_popUp->show();
}

void CloudServerView::updateTableView()
{
    const int currentId = selectedId();

    QSqlQuery query(m_db.getDatabase());
    query.prepare(QStringLiteral(R"(
        SELECT
            cloud.id,
            cloud.id_organizations,
            cloud.id_users,
            organizations.name AS organization,
            users.name AS user_name,
            cloud.hostName,
            cloud.databaseName,
            cloud.port,
            cloud.connectionOption,
            cloud.username
        FROM
            cloudServer AS cloud
        INNER JOIN
            organizations ON organizations.id = cloud.id_organizations
        INNER JOIN
            users ON users.id = cloud.id_users
        ORDER BY
            organizations.name,
            users.name
    )"));

    if (!query.exec()) {
        qCritical(logCritical()).noquote()
            << "CloudServerView SQL error:" << query.lastError().text()
            << "\nLast query:" << query.lastQuery();
        return;
    }

    m_model->setQuery(std::move(query));
    m_model->setHeaderData(Organization, Qt::Horizontal, tr("Organizația"));
    m_model->setHeaderData(User, Qt::Horizontal, tr("Utilizatorul"));
    m_model->setHeaderData(Host, Qt::Horizontal, tr("Host"));
    m_model->setHeaderData(DatabaseName, Qt::Horizontal, tr("Baza de date"));
    m_model->setHeaderData(Port, Qt::Horizontal, tr("Port"));
    m_model->setHeaderData(ConnectionOptions, Qt::Horizontal, tr("Opțiuni conexiune"));
    m_model->setHeaderData(DatabaseUser, Qt::Horizontal, tr("Utilizator BD"));

    ui->tableView->setColumnHidden(Id, true);
    ui->tableView->setColumnHidden(OrganizationId, true);
    ui->tableView->setColumnHidden(UserId, true);

    const bool selectionRestored = currentId > 0 && selectRecord(currentId);
    if (!selectionRestored && m_model->rowCount() > 0)
        ui->tableView->selectRow(0);
}

void CloudServerView::onShowHideColumn()
{
    if (!m_columnsController)
        return;

    QToolButton *button = m_toolBar->getBtnHideShowColumn();
    m_columnsController->showMenu(
        button->mapToGlobal(QPoint(0, button->height())));
}

void CloudServerView::onColumnsChanged()
{
    if (!m_columnsController)
        return;

    m_viewSettings.hiddenSections = m_columnsController->hiddenSections();
    saveSettings();
}

void CloudServerView::onDoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
        onEdit();
}

void CloudServerView::showContextMenu(const QPoint &position)
{
    const QModelIndex index = ui->tableView->indexAt(position);
    if (index.isValid())
        ui->tableView->selectRow(index.row());

    QMenu menu(this);
    QAction *addAction = menu.addAction(QIcon(QStringLiteral(":/img/toolBar/add.png")),
                                        tr("Adaugă configurare"));
    QAction *editAction = nullptr;
    QAction *deleteAction = nullptr;
    if (index.isValid()) {
        editAction = menu.addAction(QIcon(QStringLiteral(":/img/toolBar/edit.png")),
                                    tr("Redactează configurarea"));
        deleteAction = menu.addAction(QIcon(QStringLiteral(":/img/toolBar/delete.png")),
                                      tr("Elimină configurarea"));
    }

    QAction *selected = menu.exec(ui->tableView->viewport()->mapToGlobal(position));
    if (selected == addAction)
        onAdd();
    else if (selected == editAction)
        onEdit();
    else if (selected == deleteAction)
        onDelete();
}

void CloudServerView::initToolBar()
{
    m_toolBar->setStyles(m_db.toolButtonStyleForIcon());
    ui->toolBarLayout->addWidget(m_toolBar);
    ui->toolBarLayout->addStretch();

    connect(m_toolBar, &ToolBarCustom::addDoc,
            this, &CloudServerView::onAdd, Qt::UniqueConnection);
    connect(m_toolBar, &ToolBarCustom::editDoc,
            this, &CloudServerView::onEdit, Qt::UniqueConnection);
    connect(m_toolBar, &ToolBarCustom::deleteDoc,
            this, &CloudServerView::onDelete, Qt::UniqueConnection);
    connect(m_toolBar, &ToolBarCustom::updateTable,
            this, &CloudServerView::updateTableView, Qt::UniqueConnection);
    connect(m_toolBar, &ToolBarCustom::hideShowColumn,
            this, &CloudServerView::onShowHideColumn, Qt::UniqueConnection);
}

void CloudServerView::initTableView()
{
    ui->tableView->setModel(m_model);
    ui->tableView->setWordWrap(false);
    ui->tableView->setTextElideMode(Qt::ElideRight);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);

    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);

    m_columnsController = new TableColumnsController(ui->tableView, this);
    m_columnsController->setFixedHiddenColumns({Id, OrganizationId, UserId});
    connect(m_columnsController, &TableColumnsController::columnsChanged,
            this, &CloudServerView::onColumnsChanged);

    connect(ui->tableView, &QTableView::doubleClicked,
            this, &CloudServerView::onDoubleClicked, Qt::UniqueConnection);
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &CloudServerView::showContextMenu, Qt::UniqueConnection);

}

void CloudServerView::loadSettings()
{
    const QJsonObject object = m_settings.getJsonObject(m_settingsKey);
    const QJsonObject sections = object.value(QStringLiteral("sections")).toObject();
    for (auto it = sections.begin(); it != sections.end(); ++it) {
        bool ok = false;
        const int column = it.key().toInt(&ok);
        if (ok)
            m_viewSettings.sectionSizes[column] = it.value().toInt();
    }

    const QJsonObject hidden =
        object.value(QStringLiteral("hide_show_sections")).toObject();
    for (auto it = hidden.begin(); it != hidden.end(); ++it) {
        bool ok = false;
        const int column = it.key().toInt(&ok);
        if (ok)
            m_viewSettings.hiddenSections[column] = it.value().toInt() != 0;
    }
}

void CloudServerView::restoreColumns()
{
    QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    ui->tableView->setUpdatesEnabled(false);
    const bool stretch = header->stretchLastSection();
    header->setStretchLastSection(false);

    for (int column = 0; column < header->count(); ++column) {
        int width = m_viewSettings.sectionSizes.value(column, 0);
        if (width <= 0)
            width = qMax(header->sectionSizeHint(column), header->defaultSectionSize());
        header->resizeSection(column, width);
    }

    if (m_columnsController)
        m_columnsController->setHiddenSections(m_viewSettings.hiddenSections);

    header->setStretchLastSection(stretch);
    ui->tableView->setUpdatesEnabled(true);
}

void CloudServerView::saveSettings()
{
    QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return;

    const int lastVisible = lastVisibleSection();
    for (int column = 0; column < header->count(); ++column) {
        m_settings.setValue(m_settingsKey,
                            QStringLiteral("hide_show_sections/%1").arg(column),
                            header->isSectionHidden(column) ? 1 : 0);
        if (column == lastVisible)
            continue;

        const int width = header->sectionSize(column);
        if (width > 0) {
            m_settings.setValue(m_settingsKey,
                                QStringLiteral("sections/%1").arg(column),
                                width);
        }
    }
    m_settings.save();
}

int CloudServerView::lastVisibleSection() const
{
    const QHeaderView *header = ui->tableView->horizontalHeader();
    if (!header)
        return -1;

    for (int column = header->count() - 1; column >= 0; --column) {
        if (!header->isSectionHidden(column))
            return column;
    }
    return -1;
}

int CloudServerView::selectedId() const
{
    const QModelIndex index = ui->tableView->currentIndex();
    return index.isValid() ? m_model->index(index.row(), Id).data().toInt() : -1;
}

bool CloudServerView::selectRecord(int id)
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (m_model->index(row, Id).data().toInt() != id)
            continue;
        ui->tableView->selectRow(row);
        ui->tableView->scrollTo(m_model->index(row, Organization));
        return true;
    }
    return false;
}

void CloudServerView::clearActiveCloudContextIfNeeded(int organizationId, int userId)
{
    if (organizationId != SettingsService::instance().organization().organizationId
        || userId != SessionContext::instance().userId()) {
        return;
    }

    CloudConnectionContext::instance().setData(CloudConnectionData{});

    Settings::SynchronizationSettings synchronization =
        SettingsService::instance().synchronization();
    synchronization.configured = false;
    synchronization.enabled = false;
    SettingsService::instance().setSynchronization(synchronization);
}

void CloudServerView::reject()
{
    if (auto *subWindow = qobject_cast<QMdiSubWindow *>(parentWidget())) {
        subWindow->close();
        return;
    }
    QDialog::reject();
}

void CloudServerView::closeEvent(QCloseEvent *event)
{
    saveSettings();

    // Nu apelăm QDialog::closeEvent(): acesta invocă reject(), iar reject()
    // solicită închiderea QMdiSubWindow-ului părinte. La închiderea inițiată
    // chiar de QMdiSubWindow s-ar forma astfel o buclă de evenimente Close.
    event->accept();
}

void CloudServerView::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setWindowTitle(tr("Configurările serverelor cloud"));
        updateTableView();
    }
}

void CloudServerView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_End && m_model->rowCount() > 0) {
        ui->tableView->selectRow(m_model->rowCount() - 1);
        return;
    }
    if (event->key() == Qt::Key_Home && m_model->rowCount() > 0) {
        ui->tableView->selectRow(0);
        return;
    }
    QDialog::keyReleaseEvent(event);
}
