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

#include "reportsemailselectiondialog.h"

#include <core/loggingcategories.h>
#include <models/queryrolesmodel.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

namespace {
enum Column { ColNumber, ColDate, ColPatient, ColIdnp, ColOrder, ColImages, ColCount };

constexpr int IdRole = Qt::UserRole + 1;
constexpr int SortRole = Qt::UserRole + 2;
}

ReportsEmailSelectionDialog::ReportsEmailSelectionDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_model(new QStandardItemModel(this))
    , m_proxy(new QSortFilterProxyModel(this))
{
    setWindowTitle(tr("Transmiterea rapoartelor ecografice prin e-mail"));
    setWindowIcon(QIcon(":/img/documents/reportEcho.png"));
    resize(860, 560);

    buildUi();
}

void ReportsEmailSelectionDialog::buildUi()
{
    m_comboOrganization = new QComboBox(this);
    m_organizations = new QueryRolesModel(
        m_db.getTextSQL(QStringLiteral(":/sql/queries/organizations_combo_view.sql")),
        m_comboOrganization);
    m_organizations->setEmptyRowEnabled(true);
    m_comboOrganization->setModel(m_organizations);
    m_comboOrganization->setModelColumn(m_organizations->columnIndex(QStringLiteral("name")));

    m_lblRecipient = new QLabel(this);
    m_lblRecipient->setTextFormat(Qt::PlainText);

    m_dateStart = new QDateEdit(this);
    m_dateEnd = new QDateEdit(this);
    for (QDateEdit *edit : {m_dateStart, m_dateEnd}) {
        edit->setCalendarPopup(true);
        edit->setDisplayFormat(QStringLiteral("dd.MM.yyyy"));
    }
    m_dateStart->setDate(QDate::currentDate().addMonths(-1));
    m_dateEnd->setDate(QDate::currentDate());
    auto *btnReload = new QPushButton(tr("Afișează"), this);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Căutare după pacient, IDNP sau număr ..."));
    m_search->setClearButtonEnabled(true);

    m_model->setColumnCount(ColCount);
    m_model->setHorizontalHeaderLabels({tr("Nr. raport"), tr("Data"), tr("Pacient"),
                                        tr("IDNP"), tr("Comanda"), tr("Imagini")});
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setSortRole(SortRole);

    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(ColDate, Qt::DescendingOrder);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(ColPatient, QHeaderView::Stretch);

    auto *btnCheckAll = new QPushButton(tr("Bifează tot"), this);
    auto *btnUncheckAll = new QPushButton(tr("Debifează tot"), this);
    m_chkImages = new QCheckBox(tr("Atașează și imaginile rapoartelor"), this);
    m_lblSelection = new QLabel(this);

    m_buttons = new QDialogButtonBox(this);
    m_btnAccept = m_buttons->addButton(tr("Exportă și pregătește e-mailul"),
                                       QDialogButtonBox::AcceptRole);
    m_buttons->addButton(QDialogButtonBox::Cancel);

    auto *rowOrganization = new QHBoxLayout;
    rowOrganization->addWidget(new QLabel(tr("Organizația:"), this));
    rowOrganization->addWidget(m_comboOrganization, 1);
    rowOrganization->addSpacing(12);
    rowOrganization->addWidget(new QLabel(tr("Perioada:"), this));
    rowOrganization->addWidget(m_dateStart);
    rowOrganization->addWidget(new QLabel(QStringLiteral("-"), this));
    rowOrganization->addWidget(m_dateEnd);
    rowOrganization->addWidget(btnReload);

    auto *rowChecks = new QHBoxLayout;
    rowChecks->addWidget(btnCheckAll);
    rowChecks->addWidget(btnUncheckAll);
    rowChecks->addSpacing(12);
    rowChecks->addWidget(m_chkImages);
    rowChecks->addStretch(1);
    rowChecks->addWidget(m_lblSelection);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(rowOrganization);
    layout->addWidget(m_lblRecipient);
    layout->addWidget(m_search);
    layout->addWidget(m_table, 1);
    layout->addLayout(rowChecks);
    layout->addWidget(new QLabel(tr("Se afișează doar rapoartele validate."), this));
    layout->addWidget(m_buttons);

    connect(m_comboOrganization, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportsEmailSelectionDialog::onOrganizationChanged);
    connect(btnReload, &QPushButton::clicked,
            this, &ReportsEmailSelectionDialog::reloadReports);
    connect(m_search, &QLineEdit::textChanged,
            m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_model, &QStandardItemModel::itemChanged,
            this, &ReportsEmailSelectionDialog::onItemChanged);
    connect(btnCheckAll, &QPushButton::clicked,
            this, [this]() { setVisibleRowsChecked(true); });
    connect(btnUncheckAll, &QPushButton::clicked,
            this, [this]() { setVisibleRowsChecked(false); });
    connect(m_buttons, &QDialogButtonBox::accepted,
            this, &ReportsEmailSelectionDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected,
            this, &ReportsEmailSelectionDialog::reject);

    updateRecipientInfo();
    updateSelectionInfo();
}

void ReportsEmailSelectionDialog::setInitialState(int organizationId,
                                                  const QDate &startDate,
                                                  const QDate &endDate,
                                                  const QSet<qint64> &preselectedIds)
{
    m_preselectedIds = preselectedIds;
    if (startDate.isValid())
        m_dateStart->setDate(startDate);
    if (endDate.isValid())
        m_dateEnd->setDate(endDate);

    const int row = organizationId > 0
        ? m_organizations->rowById(QStringLiteral("id"), organizationId)
        : -1;
    if (row >= 0 && row != m_comboOrganization->currentIndex())
        m_comboOrganization->setCurrentIndex(row); // -> onOrganizationChanged()
    else
        onOrganizationChanged();
}

int ReportsEmailSelectionDialog::organizationId() const
{
    return m_comboOrganization->currentData(
        m_organizations->roleForColumn(QStringLiteral("id"))).toInt();
}

bool ReportsEmailSelectionDialog::includeImages() const
{
    return m_chkImages->isChecked();
}

QVector<qint64> ReportsEmailSelectionDialog::selectedReportIds() const
{
    // Ordinea din tabel (inclusiv sortarea aleasă) devine ordinea din scrisoare.
    QVector<qint64> ids;
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex index = m_proxy->index(row, ColNumber);
        if (index.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            ids << index.data(IdRole).toLongLong();
    }
    // Rândurile bifate, dar ascunse de căutare, se adaugă la final.
    for (int row = 0; row < m_model->rowCount(); ++row) {
        const QStandardItem *item = m_model->item(row, ColNumber);
        const qint64 id = item->data(IdRole).toLongLong();
        if (item->checkState() == Qt::Checked && !ids.contains(id))
            ids << id;
    }
    return ids;
}

int ReportsEmailSelectionDialog::checkedCount() const
{
    int count = 0;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (m_model->item(row, ColNumber)->checkState() == Qt::Checked)
            ++count;
    }
    return count;
}

void ReportsEmailSelectionDialog::onOrganizationChanged()
{
    updateRecipientInfo();
    reloadReports();
}

void ReportsEmailSelectionDialog::updateRecipientInfo()
{
    m_recipientEmail.clear();
    const int orgId = organizationId();
    if (orgId <= 0) {
        m_lblRecipient->setText(tr("Selectați organizația căreia i se transmit rapoartele."));
        return;
    }

    QSqlQuery query(m_db.getDatabase());
    query.prepare(m_db.getTextSQL(QStringLiteral(":/sql/queries/organizations_byID_select.sql")));
    query.addBindValue(orgId);
    if (!query.exec()) {
        qWarning(logWarning()) << "ReportsEmailSelectionDialog: datele organizației nu pot fi citite:"
                               << query.lastError().text();
    } else if (query.next()) {
        m_recipientEmail = query.value(QStringLiteral("email")).toString().trimmed();
    }

    m_lblRecipient->setText(m_recipientEmail.isEmpty()
        ? tr("Destinatar: organizația nu are adresa de e-mail indicată în catalog.")
        : tr("Destinatar: %1").arg(m_recipientEmail));
}

void ReportsEmailSelectionDialog::reloadReports()
{
    // Bifele se păstrează la schimbarea perioadei pentru rapoartele rămase.
    if (m_model->rowCount() > 0) {
        m_preselectedIds.clear();
        for (int row = 0; row < m_model->rowCount(); ++row) {
            const QStandardItem *item = m_model->item(row, ColNumber);
            if (item->checkState() == Qt::Checked)
                m_preselectedIds.insert(item->data(IdRole).toLongLong());
        }
    }

    m_bulkUpdate = true;
    m_model->removeRows(0, m_model->rowCount());

    const int orgId = organizationId();
    if (orgId > 0) {
        if (m_dateStart->date() > m_dateEnd->date()) {
            m_bulkUpdate = false;
            QMessageBox::warning(this, tr("Verificarea perioadei"),
                                 tr("Data de sfârșit nu poate fi mai mică decât data de început."),
                                 QMessageBox::Ok);
            updateSelectionInfo();
            return;
        }

        QSqlQuery query(m_db.getDatabase());
        query.setForwardOnly(true);
        query.prepare(m_db.getTextSQL(QStringLiteral(":/sql/queries_doc/reports_email_select.sql")));
        query.bindValue(QStringLiteral(":idOrganization"), orgId);
        query.bindValue(QStringLiteral(":startDate"), QDateTime(m_dateStart->date(), QTime(0, 0)));
        query.bindValue(QStringLiteral(":endDate"), QDateTime(m_dateEnd->date(), QTime(23, 59, 59)));
        if (!query.exec()) {
            qWarning(logWarning()) << "ReportsEmailSelectionDialog: rapoartele nu pot fi citite:"
                                   << query.lastError().text();
            QMessageBox::warning(this, windowTitle(),
                                 tr("Lista rapoartelor nu a putut fi încărcată:\n%1")
                                     .arg(query.lastError().text()),
                                 QMessageBox::Ok);
        } else {
            int checked = 0;
            while (query.next()) {
                const qint64 id = query.value(QStringLiteral("id")).toLongLong();
                const QDateTime dateDoc = query.value(QStringLiteral("dateDoc")).toDateTime();
                const int images = query.value(QStringLiteral("attachedImages")).toInt();

                auto *number = new QStandardItem(query.value(QStringLiteral("numberDoc")).toString());
                number->setCheckable(true);
                const bool check = m_preselectedIds.contains(id) && checked < MaxReports;
                number->setCheckState(check ? Qt::Checked : Qt::Unchecked);
                checked += check ? 1 : 0;
                number->setData(id, IdRole);

                auto *date = new QStandardItem(dateDoc.toString(QStringLiteral("dd.MM.yyyy hh:mm")));
                auto *patient = new QStandardItem(
                    QStringLiteral("%1 %2")
                        .arg(query.value(QStringLiteral("last_name")).toString(),
                             query.value(QStringLiteral("first_name")).toString())
                        .simplified());
                auto *idnp = new QStandardItem(query.value(QStringLiteral("idnp")).toString());
                auto *order = new QStandardItem(query.value(QStringLiteral("orderNumber")).toString());
                auto *imageItem = new QStandardItem(images > 0 ? tr("da") : QString());
                imageItem->setTextAlignment(Qt::AlignCenter);

                const QList<QStandardItem *> items = {number, date, patient, idnp, order, imageItem};
                for (QStandardItem *cell : items)
                    cell->setData(cell->text(), SortRole);
                date->setData(dateDoc, SortRole); // sortare cronologică
                m_model->appendRow(items);
            }
        }
    }

    m_bulkUpdate = false;
    m_table->resizeColumnToContents(ColNumber);
    m_table->resizeColumnToContents(ColDate);
    updateSelectionInfo();
}

void ReportsEmailSelectionDialog::onItemChanged(QStandardItem *item)
{
    if (m_bulkUpdate || !item || item->column() != ColNumber)
        return;

    if (item->checkState() == Qt::Checked && checkedCount() > MaxReports) {
        item->setCheckState(Qt::Unchecked); // -> onItemChanged(), fără mesaj
        QMessageBox::information(this, windowTitle(),
                                 tr("Într-o scrisoare pot fi transmise cel mult %1 rapoarte.")
                                     .arg(MaxReports),
                                 QMessageBox::Ok);
        return;
    }
    updateSelectionInfo();
}

void ReportsEmailSelectionDialog::setVisibleRowsChecked(bool checked)
{
    m_bulkUpdate = true;
    int count = checked ? checkedCount() : 0;
    bool limitReached = false;
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex source = m_proxy->mapToSource(m_proxy->index(row, ColNumber));
        QStandardItem *item = m_model->itemFromIndex(source);
        if (!item)
            continue;
        if (!checked) {
            item->setCheckState(Qt::Unchecked);
            continue;
        }
        if (item->checkState() == Qt::Checked)
            continue;
        if (count >= MaxReports) {
            limitReached = true;
            break;
        }
        item->setCheckState(Qt::Checked);
        ++count;
    }
    m_bulkUpdate = false;
    updateSelectionInfo();

    if (limitReached) {
        QMessageBox::information(this, windowTitle(),
                                 tr("Au fost bifate primele %1 rapoarte: într-o scrisoare "
                                    "pot fi transmise cel mult %1 rapoarte.")
                                     .arg(MaxReports),
                                 QMessageBox::Ok);
    }
}

void ReportsEmailSelectionDialog::updateSelectionInfo()
{
    const int count = checkedCount();
    m_lblSelection->setText(tr("Bifate: %1 din %2").arg(count).arg(m_model->rowCount()));
    m_btnAccept->setEnabled(count > 0);
}

void ReportsEmailSelectionDialog::accept()
{
    if (organizationId() <= 0 || checkedCount() == 0) {
        QMessageBox::information(this, windowTitle(),
                                 tr("Selectați organizația și bifați cel puțin un raport."),
                                 QMessageBox::Ok);
        return;
    }

    if (m_recipientEmail.isEmpty()) {
        const auto answer = QMessageBox::question(
            this, windowTitle(),
            tr("Organizația selectată nu are adresa de e-mail indicată.\n"
               "Adresa destinatarului va trebui introdusă manual în agentul e-mail.\n\n"
               "Continuați?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;
    }

    QDialog::accept();
}
