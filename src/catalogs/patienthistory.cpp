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

#include "patienthistory.h"
#include "ui_patienthistory.h"

#include <documents/reportdialog.h>

PatientHistory::PatientHistory(DataBase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PatientHistory)
    , m_db(db)
    , m_currentDB(m_db.getDatabase())
    , completerPatients(new QCompleter(this))
    , model_patients(new QStandardItemModel(this))
    , timerPatientSearch(new QTimer(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("Istoria pacientului"));
    ui->attachedImagesText->setText(tr(" Nu sunt atașate imagini")); // initial

    QString str_qry;
    model_table = new BaseSqlQueryModel(str_qry, ui->tableView);

    ui->btnClearPatient->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");
    ui->btnOpenCatPatient->setStyleSheet("border: 1px solid #8f8f91; "
                                          "border-radius: 4px;");

    initSetCompleter();

    connect(this, &PatientHistory::IdPatientChanged,
            this, &PatientHistory::slot_IdPatientChanged, Qt::UniqueConnection);

    connect(ui->btnClearPatient, &QAbstractButton::clicked,
            this, &PatientHistory::clearDataComboPatient, Qt::UniqueConnection);
    connect(ui->btnOpenCatPatient, &QAbstractButton::clicked,
            this, &PatientHistory::openCatPatient, Qt::UniqueConnection);

    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::clicked),
            this, &PatientHistory::onClickedTable, Qt::UniqueConnection);
    connect(ui->tableView, QOverload<const QModelIndex&>::of(&QTableView::doubleClicked),
            this, &PatientHistory::onDoubleClickedTable, Qt::UniqueConnection);

}

PatientHistory::~PatientHistory()
{
    delete ui;
}

void PatientHistory::slot_IdPatientChanged()
{
    {
        QSignalBlocker blocker(ui->comboPatient);
        const int row = ui->comboPatient->findData(m_id_patient, Qt::UserRole);
        ui->comboPatient->setCurrentIndex(row);
        if (row < 0)
            ui->comboPatient->setEditText(QString());
    }
    setWindowTitle(m_id_patient > 0
        ? tr("Istoria pacientului - %1").arg(ui->comboPatient->lineEdit()->text())
        : tr("Istoria pacientului"));
    updateTableDoc();
    loadImagesPatients();
}

void PatientHistory::slotPatientTextChanged(const QString &text)
{
    Q_UNUSED(text);
    timerPatientSearch->start();
}

void PatientHistory::updateModelPatientsByText()
{
    const QString text = ui->comboPatient->lineEdit()->text().trimmed();

    model_patients->clear();
    model_patients->setColumnCount(1);

    if (text.length() < 2)
        return;

    QSqlQuery q(m_currentDB);
    QString sql = globals().thisMySQL
                      ? m_db.getTextSQL(":/sql/queries_doc/searchPatientByText_mariadb.sql")
                      : m_db.getTextSQL(":/sql/queries_doc/searchPatientByText_sqlite.sql");

    q.prepare(sql);

    const QString prefixText   = text + "%";
    const QString containsText = "%" + text + "%";

    q.addBindValue(prefixText);
    q.addBindValue(prefixText);
    q.addBindValue(containsText);
    q.addBindValue(containsText);

    if (!q.exec()) {
        qWarning() << "Eroare exec query patients:"
                   << q.lastError().text();
        return;
    }

    while (q.next()) {
        auto *item = new QStandardItem(q.value(1).toString()); // FullName
        item->setData(q.value(0).toInt(), Qt::UserRole);       // id
        model_patients->appendRow(item);
    }

    if (model_patients->rowCount() > 0)
        completerPatients->complete();
}

void PatientHistory::activatedItemCompleter(const QModelIndex &index)
{
    timerPatientSearch->stop();

    if (! index.isValid())
        return;

    const int current_id = index.data(Qt::UserRole).toInt();
    if (current_id <= 0)
        return;

    setIdPatient(current_id); // setam id
}

void PatientHistory::onClickedTable(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    // imagine
    loadImagesPatients();
}

void PatientHistory::onDoubleClickedTable(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const int row = index.row();
    QModelIndex idx_id = model_table->index(row, ReportForHistoryPatientSections::Id);
    const int id_doc = model_table->data(idx_id, Qt::DisplayRole).toInt();

    ReportDialog::ReportDialogParameters parameters;
    parameters.isNew = false;
    parameters.id = id_doc;

    auto *report = new ReportDialog(m_db, parameters, this);
    report->setAttribute(Qt::WA_DeleteOnClose);
    report->show();
}

void PatientHistory::clearDataComboPatient()
{
    setIdPatient(-1);
    ui->attachedImagesText->setText(tr("Nu sunt atașate imagini"));
}

void PatientHistory::openCatPatient()
{
    if (m_id_patient == -1)
        return;

    CatalogDialog *cat_patients = new CatalogDialog(m_db, CatalogType::Type::Patients, this);
    cat_patients->setAttribute(Qt::WA_DeleteOnClose);
    cat_patients->setProperty("isNew", false);
    cat_patients->setProperty("id", m_id_patient);
    cat_patients->show();
}

void PatientHistory::initSetCompleter()
{
    model_patients->clear();
    model_patients->setColumnCount(1);

    completerPatients->setModel(model_patients);
    completerPatients->setCompletionColumn(0);
    completerPatients->setCaseSensitivity(Qt::CaseInsensitive);
    completerPatients->setCompletionMode(QCompleter::PopupCompletion);
    completerPatients->setFilterMode(Qt::MatchContains);
    completerPatients->setModelSorting(QCompleter::UnsortedModel);

    ui->comboPatient->setEditable(true);
    ui->comboPatient->lineEdit()->setCompleter(completerPatients);

    timerPatientSearch->setSingleShot(true);
    timerPatientSearch->setInterval(250);

    connect(ui->comboPatient->lineEdit(), &QLineEdit::textEdited,
            this, &PatientHistory::slotPatientTextChanged,
            Qt::UniqueConnection);

    connect(timerPatientSearch, &QTimer::timeout,
            this, &PatientHistory::updateModelPatientsByText,
            Qt::UniqueConnection);

    connect(completerPatients, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex &>::of(&PatientHistory::activatedItemCompleter));
}

void PatientHistory::updateModelPatients()
{
    if (model_patients->rowCount() > 0)
        model_patients->clear();

    const QString strQuery =
        m_db.getTextSQL(":/sql/queries/patients_completer_view.sql");
    QSqlQuery qry(m_db.getDatabase());
    if (! qry.exec(strQuery)) {
        qWarning() << "Eroare exec query:" << qry.lastError().text();
        return;
    }

    // pu performanta cream container
    QList<QStandardItem*> items;

    // prelucrarea solicitarii si completarea containerului 'items'
    while (qry.next()) {
        int     _id   = qry.value(0).toInt();
        QString _name = qry.value(1).toString();

        auto *item = new QStandardItem;
        item->setData(_id, Qt::UserRole);       // pentru identificator logic
        item->setData(_name, Qt::DisplayRole);  // ce se afișează în combobox

        items.append(item); // introducem in container
    }

    // adaugam toate randurile printr-o tranzactie/simultan (eficient si rapid)
    model_patients->invisibleRootItem()->appendRows(items);
}

void PatientHistory::updateTableDoc()
{
    QString str_qry = m_db.getTextSQL(":/sql/queries/patientHistory_doc_select.sql");
    QSqlQuery qry(m_db.getDatabase());
    if (!qry.prepare(str_qry)) {
        qWarning(logWarning()) << "PatientHistory prepare error:" << qry.lastError().text();
        model_table->clear();
        return;
    }
    qry.addBindValue(m_id_patient);
    if (!qry.exec()) {
        qWarning(logWarning()) << "PatientHistory query error:" << qry.lastError().text();
        model_table->clear();
        return;
    }

    model_table->setQuery(std::move(qry));
    ui->tableView->setModel(model_table);
    ui->tableView->hideColumn(ReportForHistoryPatientSections::Id);
    ui->tableView->hideColumn(ReportForHistoryPatientSections::Concluzion);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // setam alegerea randului
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // setam singura alegerea(nu multipla)
    ui->tableView->setSortingEnabled(false);                              // setam posibilitatea sortarii
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // permitem schimbarea size sectiilor
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);  // extinderea ultimei sectiei
    ui->tableView->selectRow(0);
    model_table->setHeaderData(1, Qt::Horizontal, tr("Documente"));
}

void PatientHistory::loadImagesPatients()
{
    ui->diagnoza->clear();
    for (QLabel *label : {ui->image1, ui->image_2, ui->image_3, ui->image_4, ui->image_5})
        label->setText(tr("Imagine lipsește"));
    if (m_id_patient == -1)
        return;

    if (!ui->tableView->currentIndex().isValid())
        return;

    const int row              = ui->tableView->currentIndex().row();
    QModelIndex idx_id_doc     = model_table->index(row, ReportForHistoryPatientSections::Id);
    QModelIndex idx_concluzion = model_table->index(row, ReportForHistoryPatientSections::Concluzion);
    const int id_doc           = model_table->data(idx_id_doc, Qt::DisplayRole).toInt();
    const QString concluzion   = model_table->data(idx_concluzion, Qt::DisplayRole).toString();
    if (!concluzion.isEmpty())
        ui->diagnoza->setPlainText(concluzion);

    int image_count = 0;

    // imagine
    QSqlQuery qry(m_db.getDatabaseImage());
    qry.prepare(m_db.getTextSQL(":/sql/queries/imagesReports_byID_report_select.sql"));
    qry.addBindValue(id_doc);
    if (! qry.exec()){
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii de extragere a imaginei: %1")
                                      .arg(qry.lastError().text());
    } else {
        if (qry.next()){
            QSqlRecord rec = qry.record();
            QByteArray outByteArray1 = QByteArray::fromBase64(qry.value(rec.indexOf("image_1")).toByteArray());
            QByteArray outByteArray2 = QByteArray::fromBase64(qry.value(rec.indexOf("image_2")).toByteArray());
            QByteArray outByteArray3 = QByteArray::fromBase64(qry.value(rec.indexOf("image_3")).toByteArray());
            QByteArray outByteArray4 = QByteArray::fromBase64(qry.value(rec.indexOf("image_4")).toByteArray());
            QByteArray outByteArray5 = QByteArray::fromBase64(qry.value(rec.indexOf("image_5")).toByteArray());

            QPixmap outPixmap1 = QPixmap();
            if (! outByteArray1.isEmpty() && outPixmap1.loadFromData(outByteArray1)) {
                ui->image1->setPixmap(outPixmap1.scaled(600,344, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                image_count ++;
            } else {
                ui->image1->setText(tr("Imagine lipsește"));
            }

            QPixmap outPixmap2 = QPixmap();
            if (! outByteArray2.isEmpty() && outPixmap2.loadFromData(outByteArray2)) {
                ui->image_2->setPixmap(outPixmap2.scaled(600,344, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                image_count ++;
            } else {
                ui->image_2->setText(tr("Imagine lipsește"));
            }

            QPixmap outPixmap3 = QPixmap();
            if (! outByteArray3.isEmpty() && outPixmap3.loadFromData(outByteArray3)) {
                ui->image_3->setPixmap(outPixmap3.scaled(600,344, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                image_count ++;
            } else {
                ui->image_3->setText(tr("Imagine lipsește"));
            }

            QPixmap outPixmap4 = QPixmap();
            if (! outByteArray4.isEmpty() && outPixmap4.loadFromData(outByteArray4)) {
                ui->image_4->setPixmap(outPixmap4.scaled(600,344, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                image_count ++;
            } else {
                ui->image_4->setText(tr("Imagine lipsește"));
            }

            QPixmap outPixmap5 = QPixmap();
            if (! outByteArray5.isEmpty() && outPixmap5.loadFromData(outByteArray5)) {
                ui->image_5->setPixmap(outPixmap5.scaled(600,344, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                image_count ++;
            } else {
                ui->image_5->setText(tr("Imagine lipsește"));
            }
        } else {
            ui->image1->setText(tr("Imagine lipsește"));
            ui->image_2->setText(tr("Imagine lipsește"));
            ui->image_3->setText(tr("Imagine lipsește"));
            ui->image_4->setText(tr("Imagine lipsește"));
            ui->image_5->setText(tr("Imagine lipsește"));
        }

        if (image_count > 0)
            ui->attachedImagesText->setText(tr("Atașate %1 imagini").arg(image_count));
        else
            ui->attachedImagesText->setText(tr("Nu sunt atașate imagini"));
    }
}
