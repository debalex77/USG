#include "chooseformprint.h"
#include "ui_chooseformprint.h"
#include <QGuiApplication>
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QScreen>
#endif

ChooseFormPrint::ChooseFormPrint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseFormPrint)
{
    ui->setupUi(this);

    setWindowTitle(tr("Alege forma de tipar"));

    QButtonGroup* groupBtn = new QButtonGroup(this);
    groupBtn->addButton(ui->btnComplex);
    groupBtn->addButton(ui->btnOrgasInternal);
    groupBtn->addButton(ui->btnUrinarySystem);
    groupBtn->addButton(ui->btnProstate);
    groupBtn->addButton(ui->btnGynecology);
    groupBtn->addButton(ui->btnBreast);
    groupBtn->addButton(ui->btnThyroid);
    groupBtn->addButton(ui->btnOB_0);
    groupBtn->addButton(ui->btnOB_1);
    groupBtn->addButton(ui->btnOB_2);
    ui->btnComplex->setChecked(true);

    connect(ui->btnOK, &QPushButton::clicked, this, &ChooseFormPrint::onAccept);
    connect(ui->btnClose, &QPushButton::clicked, this, &ChooseFormPrint::close);

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;

    move(x, y);
}

ChooseFormPrint::~ChooseFormPrint()
{
    delete ui;
}

ChooseFormPrint::TypeFormPrint ChooseFormPrint::getTypeFormPrint()
{
    if (ui->btnComplex->isChecked())
        return complex;
    if (ui->btnOrgasInternal->isChecked())
        return organs_internal;
    if (ui->btnUrinarySystem->isChecked())
        return urinary_system;
    if (ui->btnProstate->isChecked())
        return prostate;
    if (ui->btnGynecology->isChecked())
        return gynecology;
    if (ui->btnBreast->isChecked())
        return breast;
    if (ui->btnThyroid->isChecked())
        return thyroid;
    if (ui->btnOB_0->isChecked())
        return gestation0;
    if (ui->btnOB_1->isChecked())
        return gestation1;
    if (ui->btnOB_2->isChecked())
        return gestation2;

    return complex;
}

void ChooseFormPrint::setTypeFormPrint(const TypeFormPrint typePrint)
{
    if (typePrint == complex)
        ui->btnComplex->setChecked(true);
    if (typePrint == organs_internal)
        ui->btnOrgasInternal->setChecked(true);
    if (typePrint == urinary_system)
        ui->btnUrinarySystem->setChecked(true);
    if (typePrint == prostate)
        ui->btnProstate->setChecked(true);
    if (typePrint == gynecology)
        ui->btnGynecology->setChecked(true);
    if (typePrint == breast)
        ui->btnBreast->setChecked(true);
    if (typePrint == thyroid)
        ui->btnThyroid->setChecked(true);
    if (typePrint == gestation0)
        ui->btnOB_0->setChecked(true);
    if (typePrint == gestation1)
        ui->btnOB_1->setChecked(true);
    if (typePrint == gestation2)
        ui->btnOB_2->setChecked(true);
}

void ChooseFormPrint::onAccept()
{
    emit mChooseFormPrint();
    QDialog::accept();
}
