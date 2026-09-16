#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QUuid>

#include <common/property_macros.h>
#include <common/table_sections.h>
#include <common/globals.h>
#include <common/balloontip.h>

#include <data/appsettings.h>
#include <data/database.h>
#include <data/popup.h>

#include <customs/custommessage.h>

namespace Ui {
class UserDialog;
}

class UserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserDialog(DataBase &db, QWidget *parent = nullptr);
    ~UserDialog();

    // declaram proprietatile si functiile setter/getter + functia de update
    // DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc)
    // vezi common/property_macros.h
    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(StatusObject::Column, statusCatalog, StatusCatalog, statusCatalogChanged, slot_StatusCatalogChanged)

    // procedura de marcare pu eliminare din view
    bool setDeleteMarkUser(QString &err);

signals:
    void isNewChanged();
    void idChanged();
    void statusCatalogChanged();

    void userCreated();
    void userChanged();
    void userCreatedReturnID(const int id);

    void userDeletedMark();

private slots:
    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_StatusCatalogChanged();

    void dataWasModified();

    bool controlRequiredObjects();
    bool userExistsByName();
    bool handleInsert();
    bool handleUpdate();
    bool onSave();
    bool onSaveAndClose();

private:
    Ui::UserDialog *ui;

    DataBase &m_db;
    PopUp    *popUp;

    QString styleForButtonMessageBox;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // USERDIALOG_H
