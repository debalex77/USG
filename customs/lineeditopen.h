#ifndef LINEEDITOPEN_H
#define LINEEDITOPEN_H

#include <QLineEdit>

class LineEditOpen : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditOpen(QWidget *parent = nullptr);
    ~LineEditOpen();

signals:
    void onClickedButton();

private slots:
    void onTextChanged(const QString &text);

private:
    QAction *action_open;
};

#endif // LINEEDITOPEN_H
