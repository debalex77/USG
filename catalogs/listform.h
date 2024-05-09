#ifndef LISTFORM_H
#define LISTFORM_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolButton>
#include <QMenu>
#include <QSqlQueryModel>

#include "catalogs/catgeneral.h"
#include "catalogs/catusers.h"
#include "catalogs/catorganizations.h"

#include "data/database.h"
#include <data/appsettings.h>
#include "data/popup.h"
#include <data/globals.h>
#include "models/basesqlquerymodel.h"
#include "models/basesortfilterproxymodel.h"

namespace Ui {
class ListForm;
}

class ListForm : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(TypeListForm typeListForm READ getTypeListForm WRITE setTypeListForm NOTIFY typeListFormChanged)

public:
    explicit ListForm(QWidget *parent = nullptr);
    ~ListForm();

    enum TypeListForm{Doctors, Nurses, Pacients, Users, Organizations};
    Q_ENUM(TypeListForm)

    void setTypeListForm(TypeListForm typeListForm)
    {m_typeListForm = typeListForm; emit typeListFormChanged();}

    ListForm::TypeListForm getTypeListForm()
    {return m_typeListForm;}

private:
    void updateTableView();
    void updateHeaderTable();

    const QString getNameTable();

    void loadSizeSectionTable();
    void saveSizeSectionTable();
    int sizeSectionDefault(const int numberSection);

    const QString qryTableDoctors();
    const QString qryTableNurses();
    const QString qryTablePacients();
    const QString qryTableUsers();
    const QString qryTableOrganizations();

    QString enumToString(TypeListForm typeCatalog);

    enum sections
    {
        section_id           = 0,
        section_deletionMark = 1,
        section_name         = 2,
        section_telephone    = 3,
        section_address      = 4,
        section_comment      = 5
    };

signals:
    void mCloseThisForm();
    void typeListFormChanged();

private slots:
    void changedTypeListForm();

    bool onAddObject();
    void onEditObject();
    void onDeletionMarkObject();
    void onClose();

    void onDoubleClickedTable(const QModelIndex &index);
    void slotContextMenuRequested(QPoint pos);

private:
    Ui::ListForm *ui;

    TypeListForm m_typeListForm;

    static const int sz_deletionMark = 5;   // size section default
    static const int sz_name         = 250;
    static const int sz_telephone    = 120;
    static const int sz_address      = 350;

    QToolButton *btnAdd;
    QToolButton *btnEdit;
    QToolButton *btnClear;

    DataBase *db;
    PopUp    *popUp;
    QMenu    *menu;

    BaseSqlQueryModel        *model;
    BaseSortFilterProxyModel *proxy;

    CatGeneral       *cat_General;
    CatUsers         *cat_Users;
    CatOrganizations *cat_Organizations;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // pu identificare butoanelor:
                                            // 'Key_Up', 'Key_Down', 'Key_Home', 'Key_End', 'Key_Prt'
                                            // se foloseste functia - keyReleaseEvent(), analogica keyPressEvent()
};

#endif // LISTFORM_H
