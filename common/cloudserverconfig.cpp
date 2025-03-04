#include "cloudserverconfig.h"
#include "ui_cloudserverconfig.h"

CloudServerConfig::CloudServerConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CloudServerConfig)
{
    ui->setupUi(this);
}

CloudServerConfig::~CloudServerConfig()
{
    delete ui;
}
