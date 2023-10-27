#include "classifierinvestigations.h"
#include "ui_classifierinvestigations.h"

ClassifierInvestigations::ClassifierInvestigations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClassifierInvestigations)
{
    ui->setupUi(this);

    QStringList _headers;
    _headers << tr("id")
             << tr("")
             << tr("Cod MS")
             << tr("Denumirea investigației")
             << tr("Actual");

    model = new BaseSqlTableModel(this);
    model->setTable("investigations");
    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){
        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
    }

    model->setEditStrategy(QSqlTableModel::OnFieldChange); // Înregistrarea modificărilor va fi executată
                                                           // automat după editarea celule

    model->setSort(0,Qt::AscendingOrder);        // sortarea datelor de la sectia 0
    ui->tableView->setModel(model);

    model->select();

//    model = new QSqlTableModel(this);
//    model->setTable("classifierInvestigations");

//    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){
//        model->setHeaderData(i, Qt::Horizontal, _headers[j]);
//    }

//    model->setEditStrategy(QSqlTableModel::OnFieldChange); // Înregistrarea modificărilor va fi executată
//                                                           // automat după editarea celule

//    model->setSort(0,Qt::AscendingOrder);        // sortarea datelor de la sectia 0
//    ui->tableView->setModel(model);

//    model->select();

    ui->tableView->setColumnHidden(0, true);     // ascundem id
    ui->tableView->resizeColumnsToContents();    // dimensiunea colonitelor dupa conținut
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnAdd, &QAbstractButton::clicked, this, &ClassifierInvestigations::onAddRowTable);
    connect(ui->btnMarkDeletion, &QAbstractButton::clicked, this, &ClassifierInvestigations::onMarkDeletion);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &ClassifierInvestigations::onClose);
}

ClassifierInvestigations::~ClassifierInvestigations()
{
    delete model;
    delete ui;
}

void ClassifierInvestigations::onAddRowTable()
{
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), row + 1); // index(row, 0) - id
    model->setData(model->index(row, 1), 0);       // index(row, 1) - deletionMark
    model->submitAll();                            // acceptam transferarea in SQL

    ui->tableView->edit(model->index(row, 2));
    ui->tableView->selectRow(row);
}

void ClassifierInvestigations::onMarkDeletion()
{

}

void ClassifierInvestigations::onClose()
{
    this->close();
}
