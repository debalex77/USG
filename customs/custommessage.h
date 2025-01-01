#ifndef CUSTOMMESSAGE_H
#define CUSTOMMESSAGE_H

#include <QDialog>

namespace Ui {
class CustomMessage;
}

class CustomMessage : public QDialog
{
    Q_OBJECT

public:
    explicit CustomMessage(QWidget *parent = nullptr);
    ~CustomMessage();

    void setTextTitle(QString text);
    void setDetailedText(QString text);

private slots:
    void onShowDetailedText();

private:
    Ui::CustomMessage *ui;
};

#endif // CUSTOMMESSAGE_H
