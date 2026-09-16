#ifndef CONTRACTDIALOG_H
#define CONTRACTDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolBar>
#include <QUuid>

#include <data/database.h>

#include <customs/custommessage.h>

#include <common/property_macros.h>
#include <common/table_sections.h>
#include <common/balloontip.h>

#include <models/queryrolesmodel.h>

namespace Ui {
class ContractDialog;
}

class ContractDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContractDialog(DataBase &db, QWidget *parent = nullptr);
    ~ContractDialog();

    // declaram proprietatile si functiile setter/getter + functia de update
    // DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc)
    // vezi common/property_macros.h
    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(int, idOrganization, IdOrganization, idOrganizationChanged, slot_IdOrganizationChanged)
    DECLARE_PROPERTY_UPDATE(int, idTypePrice, IdTypePrice, idTypePriceChanged, slot_IdTypePriceChanged)
    DECLARE_PROPERTY_UPDATE(StatusObject::Column, statusCatalog, StatusCatalog, statusCatalogChanged, slot_StatusCatalogChanged)

    QString getNameParentContract();

    // procedura de marcare pu eliminare din view
    bool setDeleteMarkOrganization(QString &err);

signals:
    void isNewChanged();
    void idChanged();
    void idOrganizationChanged();
    void idTypePriceChanged();
    void statusCatalogChanged();

    void contractCreated();
    void contractChanged();
    void contractDeletedMark();

private:
    void initConnections();

    void updateModelOrganizations();
    void updateModelTypesPrices();

    bool controlRequiredObjects();
    bool insertIntoTableContracts(QStringList &err);
    bool updateDataTableContracts(QStringList &err);
    bool handleDeletionMark(QString &err);

private slots:
    void dataWasModified();

    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_IdTypePriceChanged();
    void slot_StatusCatalogChanged();

    void changedIndexComboOrganization(const int index);
    void currentIndexTypesPricesChanged(const int index);

    bool onWritingData();
    void onWritingDataClose();
    void onClose();

private:
    Ui::ContractDialog *ui;
    DataBase        &m_db;
    QString styleButtonMessageBox;
    QueryRolesModel *model_organizations = nullptr;
    QueryRolesModel *model_typesPrices   = nullptr;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
};

#endif // CONTRACTDIALOG_H
