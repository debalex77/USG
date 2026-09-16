#ifndef ONLINEACCOUNTDIALOG_H
#define ONLINEACCOUNTDIALOG_H

#include <QDialog>
#include <QKeyEvent>

#include <data/database.h>

#include <common/table_sections.h>
#include <common/property_macros.h>
#include <common/balloontip.h>
#include <common/cryptomanager.h>

#include <models/queryrolesmodel.h>

namespace Ui {
class OnlineAccountDialog;
}

class OnlineAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineAccountDialog(DataBase &db, QWidget *parent = nullptr);
    ~OnlineAccountDialog();

    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(int, idOrganization, IdOrganization, idOrganizationChanged, slot_IdOrganizationChanged)
    DECLARE_PROPERTY_UPDATE(StatusObject::Column, statusCatalog, StatusCatalog, statusCatalogChanged, slot_StatusCatalogChanged)

signals:
    void isNewChanged();
    void idChanged();
    void idOrganizationChanged();
    void statusCatalogChanged();

    void onlineAccountCreated();
    void onlineAccountChanged();

private slots:
    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_StatusCatalogChanged();

    void dataWasModified();

    void changedIndexComboOrganization(const int index);

    bool controlRequiredObjects();
    bool handleInsert();
    bool handleUpdate();
    bool onSave();
    bool onSaveAndClose();

private:
    void updateModelOrganization();

    void initConnections();
    void initFooter();

    bool confirmSaveIfModified();

private:
    Ui::OnlineAccountDialog *ui;

    DataBase &m_db;

    QueryRolesModel *modelOrganization = nullptr;

    bool m_loadingData = false;

    QString styleBtnMessageBox;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
    // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // ONLINEACCOUNTDIALOG_H
