#include "groupinvestigation.h"
#include "ui_groupinvestigation.h"

GroupInvestigation::GroupInvestigation(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GroupInvestigation)
{
    ui->setupUi(this);

    // cat_investigations = new CatForSqlTableModel(this);
    // cat_investigations->setProperty("typeCatalog", cat_investigations->Investigations);
    // cat_investigations->setProperty("typeForm",    cat_investigations->SelectForm);
    // cat_investigations->setGeometry(1000, 400, 800, 400);

    connect(ui->btnAdd, &QPushButton::clicked, this, &GroupInvestigation::onAddInvestigation);
    connect(ui->btnRemove, &QPushButton::clicked, this, &GroupInvestigation::onRemoveInvestigation);
    connect(ui->btnClear, &QPushButton::clicked, this, &GroupInvestigation::onClearTable);
}

GroupInvestigation::~GroupInvestigation()
{
    // delete cat_investigations;
    delete ui;
}

void GroupInvestigation::onAddInvestigation()
{
    // connect(cat_investigations, &CatForSqlTableModel::mSelectData, this, &GroupInvestigation::getDataSelectedInvestigation);
    // cat_investigations->show();
}

void GroupInvestigation::onRemoveInvestigation()
{

}

void GroupInvestigation::onClearTable()
{

}

void GroupInvestigation::getDataSelectedInvestigation()
{
    // const QString cod  = cat_investigations->getSelectCod();
    // const QString name = cat_investigations->getSelectName();
}
