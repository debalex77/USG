#ifndef CATINVESTIGATIONS_H
#define CATINVESTIGATIONS_H

#include <QDialog>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QMenu>

#include "basesqltablemodel.h"
#include "database.h"

namespace Ui {
class CatInvestigations;
}

class CatInvestigations : public QDialog
{
    Q_OBJECT

public:
    explicit CatInvestigations(QWidget *parent = nullptr);
    ~CatInvestigations();

signals:
    void mCloseThisForm();

private slots:
    void onAddRowTable();
    void onMarkDeletion();
    void onClose();

    void slotContextMenuRequested(QPoint pos);

private:
    Ui::CatInvestigations *ui;
    BaseSqlTableModel* model;
    DataBase* db;
    QMenu* menu;

protected:
    void changeEvent(QEvent *event);
};

#endif // CATINVESTIGATIONS_H
