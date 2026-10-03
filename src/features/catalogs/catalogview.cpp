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

#include "catalogview.h"
#include "common/applicationpathscontext.h"
#include "features/catalogs/catalogdialog.h"
#include "features/patients/patientremovalrepository.h"
#include "ui_catalogview.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>

namespace {

// Coloana 2 este numele/denumirea în toate cataloagele (FullName / Name).
constexpr int defaultSortSection = 2;

// La reîncărcare, câte rânduri în plus față de poziția veche se încarcă
// pentru regăsirea înregistrării curente (mutată de sortare/redenumire).
constexpr int maxExtraRowsToRestore = 1000;

}

CatalogView::CatalogView(DataBase &db, CatalogType::Type catalogType, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CatalogView)
    , m_settings(ApplicationPathsContext::instance().tableSettingsFilePath())
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

    // Și eroarea unui lot încărcat automat de QTableView la capătul derulării.
    showLoadError();
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
        catalogCommon->setAttribute(Qt::WA_DeleteOnClose);
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
}

void CatalogView::onUpdate()
{
    updateTableView();
}

void CatalogView::onContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = ui->tableView->indexAt(pos);
    if (!index.isValid())
        return;

    ui->tableView->selectRow(index.row());
    const CatalogsCommon &item = model->itemAt(proxy->mapToSource(index).row());
    const bool marked = item.deletionMark == StatusObject::DeletionMark;

    menu->clear();
    connect(menu->addAction(tr("Editare")), &QAction::triggered,
            this, &CatalogView::onEdit);
    connect(menu->addAction(marked ? tr("Anularea marcării pentru eliminare")
                                   : tr("Marcare pentru eliminare")),
            &QAction::triggered, this, &CatalogView::onDelete);

    if (m_catalogType == CatalogType::Type::Patients) {
        menu->addSeparator();
        QAction *removeAction = menu->addAction(tr("Eliminare din baza de date"));
        removeAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Delete));
        connect(removeAction, &QAction::triggered,
                this, &CatalogView::onRemovePatient);
    }

    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}

void CatalogView::onRemovePatient()
{
    if (m_catalogType != CatalogType::Type::Patients)
        return;

    const QModelIndex idx = ui->tableView->currentIndex();
    if (!isValidIndex(idx))
        return;

    // Copiem datele: dialogurile rulează bucle de evenimente în care lista
    // se poate reîncărca.
    const CatalogsCommon item = model->itemAt(proxy->mapToSource(idx).row());
    const QString patientName = item.txtBirthday.isEmpty()
        ? item.fullName
        : QStringLiteral("%1, %2").arg(item.fullName, item.txtBirthday);

    PatientRemovalRepository repository(m_db);
    QList<PatientRemovalRepository::Reference> references;
    QString error;
    if (!repository.findReferences(item.id, &references, &error)) {
        showPatientRemovalError(patientName, error);
        return;
    }
    if (!references.isEmpty()) {
        showPatientReferences(patientName, references);
        return;
    }

    const QString styleButtons = m_db.getStyleForButtonMessageBox();
    QMessageBox messageBox(QMessageBox::Question,
                           tr("Eliminarea pacientului"),
                           tr("Eliminați definitiv pacientul <b>%1</b> din baza de date?<br><br>"
                              "Operația nu poate fi anulată.")
                               .arg(patientName.toHtmlEscaped()),
                           QMessageBox::NoButton, this);
    QPushButton *yesButton = messageBox.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messageBox.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(styleButtons);
    noButton->setStyleSheet(styleButtons);
    messageBox.setDefaultButton(noButton);
    messageBox.exec();
    if (messageBox.clickedButton() != yesButton)
        return;

    switch (repository.removePatient(item.id, &error)) {
    case PatientRemovalRepository::RemoveResult::Removed:
        popUp->setPopupText(tr("Pacientul <b>%1</b><br>a fost eliminat din baza de date.")
                                .arg(patientName.toHtmlEscaped()));
        popUp->show();
        updateTableView();
        break;

    case PatientRemovalRepository::RemoveResult::Referenced:
        // un document a fost salvat între verificare și eliminare
        if (repository.findReferences(item.id, &references, &error))
            showPatientReferences(patientName, references);
        else
            showPatientRemovalError(patientName, error);
        break;

    case PatientRemovalRepository::RemoveResult::Error:
        showPatientRemovalError(patientName, error);
        updateTableView();
        break;
    }
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

    m_filter.sortSection = defaultSortSection;
    m_filter.sortOrder = Qt::AscendingOrder;

    const QJsonObject rootObj = m_settings.getJsonObject(m_class);
    if (rootObj.isEmpty()) {
        return;
    }

    const QJsonObject catalogObj = rootObj.value(catalog).toObject();
    if (catalogObj.isEmpty()) {
        return;
    }

    // --- sortarea sectiilor
    // Secțiunea 0 (ID, mereu ascunsă) nu poate fi aleasă din antet: era doar
    // valoarea implicită salvată de versiunile anterioare.
    m_filter.sortSection = catalogObj.value("sort").toObject().value("section").toInt(defaultSortSection);
    if (m_filter.sortSection <= 0)
        m_filter.sortSection = defaultSortSection;
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
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &CatalogView::onContextMenuRequested, Qt::UniqueConnection);

    if (m_catalogType == CatalogType::Type::Patients) {
        auto *removeShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Delete),
                                             ui->tableView);
        removeShortcut->setContext(Qt::WidgetShortcut);
        connect(removeShortcut, &QShortcut::activated,
                this, &CatalogView::onRemovePatient);
    }
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &CatalogView::onDoubleClickedTableView, Qt::UniqueConnection);
}

void CatalogView::updateTableView()
{
    qInfo(logInfo()) << "CatalogView: vizualizarea/actualizarea jurnalului" << CatalogType::enumToStringRo(m_catalogType);
    if (!model || !proxy)
        return;

    // Înregistrarea curentă se reține după id: reîncărcarea aduce doar primul
    // lot, iar după editare/sortare același număr de rând poate fi alt rând.
    qint64 currentId = 0;
    int currentRow = -1;
    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        currentRow = currentIndex.row();
        currentId = proxy->index(currentRow, 0).data(Qt::UserRole).toLongLong();
    }

    {
        QSignalBlocker blocker(ui->tableView->horizontalHeader());
        ui->tableView->horizontalHeader()->setSortIndicator(m_filter.sortSection, m_filter.sortOrder);
    }
    model->setSort(m_filter.sortSection, m_filter.sortOrder);

    int rowToSelect = currentId > 0 ? rowById(currentId, 0) : -1;
    if (rowToSelect < 0 && currentRow >= 0) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const int rowLimit = currentRow + 1 + maxExtraRowsToRestore;
        while (model->canFetchMore() && proxy->rowCount() < rowLimit
               && (currentId > 0 ? rowToSelect < 0 : proxy->rowCount() <= currentRow)) {
            const int loadedRows = proxy->rowCount();
            model->fetchMore();
            if (proxy->rowCount() == loadedRows)
                break;
            if (currentId > 0)
                rowToSelect = rowById(currentId, loadedRows);
        }
        QApplication::restoreOverrideCursor();
    }

    showLoadError();

    if (proxy->rowCount() <= 0) {
        ui->tableView->clearSelection();
        return;
    }

    // înregistrarea nu mai există sau nu a fost găsită: aceeași poziție
    if (rowToSelect < 0)
        rowToSelect = qBound(0, currentRow, proxy->rowCount() - 1);

    ui->tableView->selectRow(rowToSelect);
    ui->tableView->scrollTo(proxy->index(rowToSelect, defaultSortSection));
}

int CatalogView::rowById(qint64 id, int fromRow) const
{
    for (int row = qMax(fromRow, 0); row < proxy->rowCount(); ++row) {
        if (proxy->index(row, 0).data(Qt::UserRole).toLongLong() == id)
            return row;
    }
    return -1;
}

void CatalogView::fetchAllRows()
{
    if (!model || !model->canFetchMore())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int previousCount = -1;
    while (model->canFetchMore() && model->rowCount() != previousCount) {
        previousCount = model->rowCount();
        model->fetchMore();
    }
    QApplication::restoreOverrideCursor();

    showLoadError();
}

void CatalogView::showPatientReferences(const QString &patientName,
                                        const QList<PatientRemovalRepository::Reference> &references)
{
    using Kind = PatientRemovalRepository::Reference::Kind;

    // Lista detaliată este limitată; totalurile acoperă toate documentele.
    constexpr int maxListedReferences = 50;

    int orders = 0;
    int reports = 0;
    int appointments = 0;
    QStringList lines;
    for (const PatientRemovalRepository::Reference &reference : references) {
        const QString dateTime = reference.dateDoc.toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
        QString line;
        switch (reference.kind) {
        case Kind::Order:
            ++orders;
            line = tr("Comanda ecografică nr.%1 din %2").arg(reference.numberDoc, dateTime);
            break;
        case Kind::Report:
            ++reports;
            line = tr("Raport ecografic nr.%1 din %2").arg(reference.numberDoc, dateTime);
            break;
        case Kind::Appointment:
            ++appointments;
            line = tr("Programare din %1")
                       .arg(reference.dateDoc.toString(QStringLiteral("dd.MM.yyyy")));
            break;
        }
        if (lines.size() < maxListedReferences)
            lines.append(line);
    }
    if (references.size() > maxListedReferences)
        lines.append(tr("... și încă %1").arg(references.size() - maxListedReferences));

    const QString details =
        tr("Comenzi ecografice: %1\nRapoarte ecografice: %2\nProgramări: %3")
            .arg(orders).arg(reports).arg(appointments)
        + QStringLiteral("\n\n") + lines.join(QLatin1Char('\n'))
        + QStringLiteral("\n\n")
        + tr("Pacientul poate fi marcat pentru eliminare (tasta Delete sau meniul contextual).");

    CustomMessage message(this);
    message.setWindowTitle(tr("Eliminarea pacientului"));
    message.setTextTitle(tr("Pacientul <b>%1</b> figurează în documente și nu poate fi "
                            "eliminat din baza de date.")
                             .arg(patientName.toHtmlEscaped()));
    message.setDetailedText(details);
    message.exec();
}

void CatalogView::showPatientRemovalError(const QString &patientName, const QString &error)
{
    CustomMessage message(this);
    message.setWindowTitle(tr("Eliminarea pacientului"));
    message.setTextTitle(tr("Pacientul <b>%1</b> nu a fost eliminat din baza de date.")
                             .arg(patientName.toHtmlEscaped()));
    message.setDetailedText(error);
    message.exec();
}

void CatalogView::showLoadError()
{
    if (!model)
        return;

    const QString error = model->takeLastError();
    if (error.isEmpty())
        return;

    CustomMessage message(this);
    message.setWindowTitle(windowTitle());
    message.setTextTitle(tr("Lista nu a putut fi încărcată complet."));
    message.setDetailedText(error);
    message.exec();
}

void CatalogView::initToolBar()
{
    toolBar->setStyles(m_db.toolButtonStyleForIcon());

    ui->layoutToolBar->addWidget(toolBar);
    ui->layoutToolBar->addStretch();

    if (m_catalogType == CatalogType::Type::Patients)
        initSearchEdit();

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

    // La pacienți butonul Delete deschide submeniul: marcare și eliminare
    // din baza de date; tasta Delete marchează direct.
    if (m_catalogType == CatalogType::Type::Patients) {
        QToolButton *deleteButton = toolBar->getBtnDeletDoc();

        // părinte = view-ul (ca menuSetFilter din OrderView), nu butonul:
        // altfel meniul moștenește QSS-ul butoanelor din toolbar
        auto *deleteMenu = new QMenu(this);
        QAction *markAction   = deleteMenu->addAction(tr("Marcare pentru eliminare"));
        QAction *removeAction = deleteMenu->addAction(tr("Eliminare din baza de date"));
        removeAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Delete));

        connect(markAction, &QAction::triggered,
                this, &CatalogView::onDelete);
        connect(removeAction, &QAction::triggered,
                this, &CatalogView::onRemovePatient);

        // textul marcării după starea pacientului curent
        connect(deleteMenu, &QMenu::aboutToShow, this, [this, markAction]() {
            const QModelIndex idx = ui->tableView->currentIndex();
            const bool marked = idx.isValid()
                && model->itemAt(proxy->mapToSource(idx).row()).deletionMark
                       == StatusObject::DeletionMark;
            markAction->setText(marked ? tr("Anularea marcării pentru eliminare")
                                       : tr("Marcare pentru eliminare"));
        });

        // ca „Filtru rapid” din OrderView: click deschide meniul, fără săgeată
        deleteButton->setMenu(deleteMenu);
        deleteButton->setPopupMode(QToolButton::InstantPopup);
        deleteButton->setStyleSheet(deleteButton->styleSheet()
            + QStringLiteral("QToolButton::menu-indicator { image: none; width: 0px; }"));

        // tasta Delete marchează direct (shortcut-ul butonului ar deschide meniul)
        deleteButton->setShortcut(QKeySequence());
        auto *markShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
        connect(markShortcut, &QShortcut::activated,
                this, &CatalogView::onDelete);
    }
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
    // Esc în câmpul de căutare golește textul, nu închide catalogul.
    if (obj == m_searchEdit && event->type() == QEvent::KeyPress
        && static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape
        && !m_searchEdit->text().isEmpty()) {
        m_searchEdit->clear();
        applySearch();
        return true;
    }
    return QDialog::eventFilter(obj, event);
}

void CatalogView::initSearchEdit()
{
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Căutare: nume, prenume, IDNP, dd.MM.yyyy"));
    m_searchEdit->setToolTip(tr("Căutarea pacientului după nume, prenume, IDNP "
                                "sau data nașterii (dd.MM.yyyy) – (Ctrl+F)"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaxLength(100);
    m_searchEdit->setMinimumWidth(280);
    m_searchEdit->installEventFilter(this);

    auto *searchLabel = new QLabel(tr("Căutare pacient:"), this);
    searchLabel->setBuddy(m_searchEdit);
    ui->layoutToolBar->addWidget(searchLabel);
    ui->layoutToolBar->addWidget(m_searchEdit);

    // Căutarea pornește după o scurtă pauză în tastare, nu la fiecare literă.
    m_searchTimer.setSingleShot(true);
    m_searchTimer.setInterval(400);

    connect(&m_searchTimer, &QTimer::timeout,
            this, &CatalogView::applySearch);

    connect(m_searchEdit, &QLineEdit::textChanged,
            this, [this]() { m_searchTimer.start(); });

    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        applySearch();
        ui->tableView->setFocus();
    });

    auto *searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    searchShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(searchShortcut, &QShortcut::activated, this, [this]() {
        m_searchEdit->setFocus();
        m_searchEdit->selectAll();
    });
}

void CatalogView::applySearch()
{
    m_searchTimer.stop();
    if (!m_searchEdit || !model)
        return;

    const QString text = m_searchEdit->text().simplified();
    if (text == m_appliedSearch)
        return;
    m_appliedSearch = text;

    // Rezultatul căutării începe de la primul rând (fără regăsirea celui curent).
    model->setSearchText(text);
    model->setSort(m_filter.sortSection, m_filter.sortOrder);
    showLoadError();

    ui->tableView->scrollToTop();
    if (proxy->rowCount() > 0)
        ui->tableView->selectRow(0);
    else
        ui->tableView->clearSelection();
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
    if (event->key() == Qt::Key_End) {
        // ultimul rând al catalogului, nu doar al loturilor încărcate
        fetchAllRows();
        if (proxy->rowCount() > 0) {
            ui->tableView->selectRow(proxy->rowCount() - 1);
            ui->tableView->scrollToBottom();
        }
    }
    if (event->key() == Qt::Key_Home)
        ui->tableView->selectRow(0);
}
