#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QDialog>
#include <QStringListModel>

#include <catalogs/catgeneral.h>
#include <catalogs/catorganizations.h>
#include <catalogs/catusers.h>

#include <models/basesqlquerymodel.h>
#include <models/basesqltablemodel.h>
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
    void initBtnToolBar();
    void initConnections();
    void setListStep();
    void updateTextListStep();
    void updateTableInvestigations();
    void updateTableTypePrices();
    void updateTableUsers();
    void updateTableDoctors();
    void updateTableNurses();
    void updateTableOrganizations();

private slots:
    void clickedBackPage();
    void clickedNextPage();
    void loadInvestigations();
    void loadTypePrices();

    void handleUpdateProgress(int num_records, int value);

    void addCatUser();
    void editCatUser();
    void removeCatUser();

    void addCatDoctor();
    void editCatDoctor();
    void removeCatDoctor();

    void addCatNurse();
    void editCatNurse();
    void removeCatNurse();

    void addCatOrganization();
    void editCatOrganization();
    void removeCatOrganization();

private:
    Ui::FirstRunWizard *ui;

    enum IndexPage {
        page_first,
        page_classifiers,
        page_typePrices,
        page_users,
        page_doctors,
        page_nurses,
        page_organizations
    };

    int currentStepIndex = -1;

    DataBase *db;
    QStringListModel *model;
    BaseSqlTableModel *model_investigations;
    BaseSqlTableModel *model_typePrices;
    BaseSqlQueryModel *model_users;
    BaseSqlQueryModel *model_doctors;
    BaseSqlQueryModel *model_nurses;
    BaseSqlQueryModel *model_organizations;

};

#endif // FIRSTRUNWIZARD_H
