#ifndef LINEEDITCUSTOM_H
#define LINEEDITCUSTOM_H

#include <QLineEdit>

class LineEditCustom : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditCustom(QWidget *parent = nullptr);
    ~LineEditCustom();

signals:
    void onClickSelect();
    void onClickAddItem();

};

#endif // LINEEDITCUSTOM_H
