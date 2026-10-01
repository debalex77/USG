#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QUuid>

#include <common/property_macros.h>
#include <common/table_sections.h>
#include <common/globals.h>
#include <ui/widgets/balloontip.h>

#include <settings/appsettings.h>
#include <database/database.h>
#include <app/popup.h>

#include <ui/dialogs/custommessage.h>

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

    // Crearea administratorului inițial (baza fără utilizatori): parola este
    // opțională, iar utilizatorul devine cel memorat în profil.
    // Se apelează înainte de setIsNew(true).
    void setInitialAdministrator(bool initialAdministrator);

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
    bool reencryptCloudPasswords(QSqlDatabase &database,
                                 const QByteArray &oldHash,
                                 const QByteArray &newHash,
                                 QString *error);
    bool onSave();
    bool onSaveAndClose();

private:
    Ui::UserDialog *ui;

    DataBase &m_db;
    PopUp    *popUp;

    QString styleForButtonMessageBox;

    bool m_initialAdministrator = false;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // USERDIALOG_H
