#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QDialog>
#include <QStringListModel>

#include <catalogs/catalogdialog.h>
#include <catalogs/organizationdialog.h>
#include <catalogs/userdialog.h>
#include <customs/custommessage.h>

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
    explicit FirstRunWizard(DataBase &db, QWidget *parent = nullptr);
    ~FirstRunWizard();

signals:
    void finishLoadClassifier(const QString &msg);

private:
    void initBtnToolBar();
    void initConnections();
    void updateTablesFromListStep();
    void setListStep();
    void updateTextListStep();
    void updateTableInvestigations();
    void updateTableTypePrices();
    void updateTableUsers();
    void updateTableDoctors();
    void updateTableNurses();
    void updateTableOrganizations();

private slots:
    void onStepClicked(const QModelIndex &index);
    void clickedBackPage();
    void clickedNextPage();
    void finishWizard();
    void loadInvestigations();
    void loadTypePrices();

    void handleUpdateProgress(int num_records, int value);
    void handleFinishedProgress(const QString txt);

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

    QStringList err;

    PopUp             *popUp;
    DataBase          &m_db;
    QStringListModel  *model;
    BaseSqlTableModel *model_investigations;
    BaseSqlTableModel *model_typePrices;
    BaseSqlQueryModel *model_users;
    BaseSqlQueryModel *model_doctors;
    BaseSqlQueryModel *model_nurses;
    BaseSqlQueryModel *model_organizations;

};

#endif // FIRSTRUNWIZARD_H
