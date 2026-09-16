#include "reportdashboard.h"
#include "ui_reportdashboard.h"

ReportDashboard::ReportDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDashboard)
{
    ui->setupUi(this);
}

ReportDashboard::~ReportDashboard()
{
    delete ui;
}
