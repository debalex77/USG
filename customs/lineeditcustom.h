#ifndef LINEEDITCUSTOM_H
#define LINEEDITCUSTOM_H

#include <QAction>
#include <QLineEdit>

class LineEditCustom : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditCustom(QWidget *parent = nullptr);
    ~LineEditCustom();

    QAction* actionAddItem()  const;
    QAction* actionOpenList() const;

signals:
    void onClickSelect();
    void onClickAddItem();

private:
    QAction* m_actionAddItem  = nullptr;
    QAction* m_actionOpenList = nullptr;
};

#endif // LINEEDITCUSTOM_H
