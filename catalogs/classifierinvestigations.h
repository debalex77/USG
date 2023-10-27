#ifndef CLASSIFIERINVESTIGATIONS_H
#define CLASSIFIERINVESTIGATIONS_H

#include <QDialog>
#include <QSqlTableModel>

#include "basesqltablemodel.h"

namespace Ui {
class ClassifierInvestigations;
}

class ClassifierInvestigations : public QDialog
{
    Q_OBJECT

public:
    explicit ClassifierInvestigations(QWidget *parent = nullptr);
    ~ClassifierInvestigations();

private slots:
    void onAddRowTable();
    void onMarkDeletion();
    void onClose();

private:
    Ui::ClassifierInvestigations *ui;
//    QSqlTableModel* model;
    BaseSqlTableModel* model;
};

#endif // CLASSIFIERINVESTIGATIONS_H
