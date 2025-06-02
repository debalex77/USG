#include "firstrunwizard.h"
#include "ui_firstrunwizard.h"

FirstRunWizard::FirstRunWizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FirstRunWizard)
{
    ui->setupUi(this);
}

FirstRunWizard::~FirstRunWizard()
{
    delete ui;
}
