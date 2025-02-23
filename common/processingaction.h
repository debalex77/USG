#ifndef PROCESSINGACTION_H
#define PROCESSINGACTION_H

#include <QDialog>
#include <QMovie>

namespace Ui {
class ProcessingAction;
}

class ProcessingAction : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString txtInfo READ getTxtInfo WRITE setTxtInfo NOTIFY txtInfoChanged FINAL)
public:
    explicit ProcessingAction(QWidget *parent = nullptr);
    ~ProcessingAction();

    QString getTxtInfo();
    void setTxtInfo(QString txtInfo);

signals:
    void txtInfoChanged();

private slots:
    void slot_txtInfoChanged();

private:
    Ui::ProcessingAction *ui;

    QString m_txtInfo;
};

#endif // PROCESSINGACTION_H
