#ifndef REPORTDASHBOARD_H
#define REPORTDASHBOARD_H

#include <QDialog>

namespace Ui {
class ReportDashboard;
}

class ReportDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDashboard(QWidget *parent = nullptr);
    ~ReportDashboard();

private:
    Ui::ReportDashboard *ui;
};

#endif // REPORTDASHBOARD_H
