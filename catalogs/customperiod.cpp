#include "customperiod.h"
#include "ui_customperiod.h"

CustomPeriod::CustomPeriod(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomPeriod)
{
    ui->setupUi(this);

    setWindowTitle(tr("Alege perioada"));

    db = new DataBase(this);

    QButtonGroup* groupBtn = new QButtonGroup(this);
    groupBtn->addButton(ui->radioBtnYear);
    groupBtn->addButton(ui->radioBtnMounth);
    groupBtn->addButton(ui->radioBtnDay);
    groupBtn->addButton(ui->radioBtnInterval);
    ui->radioBtnYear->setChecked(true);

    QDate cDate = QDate::currentDate();

    ui->date_mounth->setLocale(QLocale(globals::langApp));

    ui->date_year->setDate(cDate);
    ui->date_year->setMinimumDate(QDate(1910, 01, 01));
    ui->date_year->setMaximumDate(QDate(9999, 12, 31));
    ui->date_mounth->setDate(cDate);
    ui->date_mounth->setMinimumDate(QDate(1910, 01, 01));
    ui->date_mounth->setMaximumDate(QDate(9999, 12, 31));
    ui->date_day->setDate(cDate);
    ui->date_day->setMinimumDate(QDate(1910, 01, 01));
    ui->date_day->setMaximumDate(QDate(9999, 12, 31));
    ui->date_start->setDate(cDate);
    ui->date_end->setDate(cDate);

    ui->date_start->setEnabled(false);
    ui->date_end->setEnabled(false);

    stateEnableObjects();
    initConnections();
}

CustomPeriod::~CustomPeriod()
{
    delete db;
    delete ui;
}

QDateTime CustomPeriod::getDateStart()
{
    return QDateTime(ui->date_start->date(), QTime(00, 00, 00));
}

QDateTime CustomPeriod::getDateEnd()
{
    return QDateTime(ui->date_end->date(), QTime(23, 59, 59));
}

void CustomPeriod::setDateStart(QDate _dateStart)
{
    ui->radioBtnInterval->setChecked(true);
    ui->date_start->setDate(_dateStart);
    stateEnableObjects();
}

void CustomPeriod::setDateEnd(QDate _dateEnd)
{
        ui->date_end->setDate(_dateEnd);
}

void CustomPeriod::stateEnableObjects()
{
    ui->btnBackYear->setEnabled(ui->radioBtnYear->isChecked());
    ui->btnNextYear->setEnabled(ui->radioBtnYear->isChecked());
    ui->date_year->setEnabled(ui->radioBtnYear->isChecked());
    ui->btnBackMounth->setEnabled(ui->radioBtnMounth->isChecked());
    ui->btnNextMounth->setEnabled(ui->radioBtnMounth->isChecked());
    ui->date_mounth->setEnabled(ui->radioBtnMounth->isChecked());
    ui->date_day->setEnabled(ui->radioBtnDay->isChecked());
    ui->date_start->setEnabled(ui->radioBtnInterval->isChecked());
    ui->date_end->setEnabled(ui->radioBtnInterval->isChecked());

    setDateByRadioBtn();
}

void CustomPeriod::setDateByRadioBtn()
{
    if (ui->radioBtnYear->isChecked()){
        QDate _start(ui->date_year->date().year(), 1, 1);
        QDate _end(ui->date_year->date().year(), 12, 31);
        ui->date_start->setDate(_start);
        ui->date_end->setDate(_end);
    }

    if (ui->radioBtnMounth->isChecked()){
        QDate _start(ui->date_mounth->date().year(), ui->date_mounth->date().month(), 1);
        QDate _end(_start.addMonths(1).addDays(-1));
        ui->date_start->setDate(_start);
        ui->date_end->setDate(_end);
    }

    if (ui->radioBtnDay->isChecked()){
        ui->date_start->setDate(ui->date_day->date());
        ui->date_end->setDate(ui->date_day->date());
    }

    ui->labelInterval->setText(tr("Perioada instalata: %1 - %2")
                               .arg(ui->date_start->date().toString("dd.MM.yyyy"),
                                    ui->date_end->date().toString("dd.MM.yyyy")));
}

void CustomPeriod::onAccept()
{
    emit mChangePeriod();
    QDialog::accept();
}

void CustomPeriod::initConnections()
{
    QString style_toolButton = db->getStyleForToolButton();
    ui->btnBackYear->setStyleSheet(style_toolButton);
    ui->btnNextYear->setStyleSheet(style_toolButton);
    ui->btnBackMounth->setStyleSheet(style_toolButton);
    ui->btnNextMounth->setStyleSheet(style_toolButton);

    connect(ui->btnClose, &QPushButton::clicked, this, &CustomPeriod::close);
    connect(ui->radioBtnYear, &QRadioButton::clicked, this, &CustomPeriod::stateEnableObjects);
    connect(ui->radioBtnMounth, &QRadioButton::clicked, this, &CustomPeriod::stateEnableObjects);
    connect(ui->radioBtnDay, &QRadioButton::clicked, this, &CustomPeriod::stateEnableObjects);
    connect(ui->radioBtnInterval, &QRadioButton::clicked, this, &CustomPeriod::stateEnableObjects);

    connect(ui->btnBackYear, &QPushButton::clicked, this, [this]()
    {
        ui->date_year->setDate(ui->date_year->date().addYears(-1));
    });

    connect(ui->btnNextYear, &QPushButton::clicked, this, [this]()
    {
        ui->date_year->setDate(ui->date_year->date().addYears(1));
    });

    connect(ui->btnBackMounth, &QPushButton::clicked, this, [this]()
    {
        ui->date_mounth->setDate(ui->date_mounth->date().addMonths(-1));
    });

    connect(ui->btnNextMounth, &QPushButton::clicked, this, [this]()
    {
        ui->date_mounth->setDate(ui->date_mounth->date().addMonths(1));
    });

    connect(ui->btnWrite, &QPushButton::clicked, this, &CustomPeriod::mChangePeriod);
    connect(ui->btnOK, &QPushButton::clicked, this, &CustomPeriod::onAccept);

    connect(ui->date_year, &QDateEdit::dateChanged, this, &CustomPeriod::setDateByRadioBtn);
    connect(ui->date_mounth, &QDateEdit::dateChanged, this, &CustomPeriod::setDateByRadioBtn);
    connect(ui->date_day, &QDateEdit::dateChanged, this, &CustomPeriod::setDateByRadioBtn);
}
