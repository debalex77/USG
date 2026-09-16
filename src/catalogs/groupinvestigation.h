#ifndef GROUPINVESTIGATION_H
#define GROUPINVESTIGATION_H

#include <QDialog>

namespace Ui {
class GroupInvestigation;
}

class GroupInvestigation : public QDialog
{
    Q_OBJECT

public:
    explicit GroupInvestigation(QWidget *parent = nullptr);
    ~GroupInvestigation();

private slots:
    void onAddInvestigation();
    void onRemoveInvestigation();
    void onClearTable();

    void getDataSelectedInvestigation();

private:
    Ui::GroupInvestigation *ui;
};

#endif // GROUPINVESTIGATION_H
