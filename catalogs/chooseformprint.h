#ifndef CHOOSEFORMPRINT_H
#define CHOOSEFORMPRINT_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class ChooseFormPrint;
}

class ChooseFormPrint : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseFormPrint(QWidget *parent = nullptr);
    ~ChooseFormPrint();

    enum TypeFormPrint
    {
        complex, organs_internal, urinary_system, prostate, gynecology, breast, thyroid, gestation0, gestation1, gestation2
    };

    TypeFormPrint getTypeFormPrint();
    void setTypeFormPrint(const TypeFormPrint typePrint);

signals:
    void mChooseFormPrint();

private slots:
    void onAccept();

private:
    Ui::ChooseFormPrint *ui;
};

#endif // CHOOSEFORMPRINT_H
