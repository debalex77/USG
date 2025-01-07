#ifndef CUSTOMPERIOD_H
#define CUSTOMPERIOD_H

#include <QDialog>
#include <QDateEdit>
#include <QButtonGroup>
#include <data/database.h>
#include <data/globals.h>

namespace Ui {
class CustomPeriod;
}

class CustomPeriod : public QDialog
{
    Q_OBJECT

public:
    explicit CustomPeriod(QWidget *parent = nullptr);
    ~CustomPeriod();

    QDateTime getDateStart();
    QDateTime getDateEnd();

    void setDateStart(QDate _dateStart);
    void setDateEnd(QDate _dateEnd);

signals:
    void mChangePeriod();

private slots:
    void stateEnableObjects();
    void setDateByRadioBtn();
    void onAccept();

private:
    void initConnections();

private:
    Ui::CustomPeriod *ui;
    DataBase *db;
};

#endif // CUSTOMPERIOD_H
