#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QDialog>
#include <QStringListModel>

#include <models/basesqlquerymodel.h>
#include <models/basesqltablemodel.h>
#include <delegates/checkboxdelegate.h>
#include <delegates/combodelegate.h>
#include <data/database.h>

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
    void initConnections();
    void setListStep();
    void updateTextListStep();
    void updateTableInvestigations();
    void updateTableTypePrices();
    void updateTableUsers();

private slots:
    void clickedBackPage();
    void clickedNextPage();
    void loadInvestigations();
    void loadTypePrices();

    void handleUpdateProgress(int num_records, int value);

private:
    Ui::FirstRunWizard *ui;

    enum IndexPage {
        page_first,
        page_classifiers,
        page_typePrices,
        page_doctors,
        page_nurses,
        page_users,
        page_organizations
    };

    int currentStepIndex = -1;

    DataBase *db;
    QStringListModel *model;
    BaseSqlTableModel *model_investigations;
    BaseSqlTableModel *model_typePrices;
    BaseSqlQueryModel *model_users;

    CheckBoxDelegate *checkbox_delegate;
    ComboDelegate *gr_investig_delegate;
};

#endif // FIRSTRUNWIZARD_H
