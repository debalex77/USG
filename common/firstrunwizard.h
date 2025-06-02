#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QDialog>

namespace Ui {
class FirstRunWizard;
}

class FirstRunWizard : public QDialog
{
    Q_OBJECT

public:
    explicit FirstRunWizard(QWidget *parent = nullptr);
    ~FirstRunWizard();

private:
    Ui::FirstRunWizard *ui;
};

#endif // FIRSTRUNWIZARD_H
