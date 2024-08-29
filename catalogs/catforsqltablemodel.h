#ifndef CATFORSQLTABLEMODEL_H
#define CATFORSQLTABLEMODEL_H

#include <QDialog>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>

#include <delegates/checkboxdelegate.h>

#include "catalogs/groupinvestigationlist.h"
#include "delegates/combodelegate.h"
#include "delegates/doublespinboxdelegate.h"
#include "models/basesqltablemodel.h"
#include "data/database.h"

namespace Ui {
class CatForSqlTableModel;
}

class CatForSqlTableModel : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(TypeCatalog typeCatalog READ getTypeCatalog WRITE setTypeCatalog NOTIFY typeCatalogChanged)
    Q_PROPERTY(TypeForm typeForm READ getTypeForm WRITE setTypeForm NOTIFY typeFormChanged)

public:
    explicit CatForSqlTableModel(QWidget *parent = nullptr);
    ~CatForSqlTableModel();

    enum TypeCatalog{Investigations, TypesPrices, ConclusionTemplates};
    Q_ENUM(TypeCatalog)
    enum TypeForm{ListForm, SelectForm};
    Q_ENUM(TypeForm)

    void setTypeCatalog(TypeCatalog typeCatalog)
    {m_typeCatalog = typeCatalog; emit typeCatalogChanged();}
    TypeCatalog getTypeCatalog()
    {return m_typeCatalog;}

    void setTypeForm(TypeForm typeForm)
    {m_typeForm = typeForm; emit typeFormChanged();}
    TypeForm getTypeForm()
    {return m_typeForm;}

    int getSelectId()
    {return ret_id;}
    QString getSelectCod()
    {return ret_cod;}
    QString getSelectName()
    {return ret_name;}

    QString m_filter_templates = nullptr;

signals:
    void typeCatalogChanged();
    void typeFormChanged();
    void mSelectData();
    void mCloseThisForm();

private:
    void initBtnForm();
    void initBtnToolBar();

    void updateTableView();
    void updateHeaderTableInvestigations();
    void updateHeaderTableTypesPrices();
    void updateHeaderTableConclusionTemplates();

    QString getNameTable();

    enum sectionsInvestigations
    {
        investig_id           = 0,
        investig_deletionMark = 1,
        investig_cod          = 2,
        investig_name         = 3,
        investig_use          = 4,
        investig_owner        = 5
    };

    enum sectionsTypesPrices
    {
        typePrice_id           = 0,
        typePrice_deletionMark = 1,
        typePrice_name         = 2,
        typePrice_discount     = 3,
        typePrice_noncomercial = 4
    };

    enum sectionsConclusionTemplates
    {
        template_id           = 0,
        template_deletionMark = 1,
        template_cod          = 2,
        template_name         = 3,
        template_system       = 4
    };

private slots:
    void onSelectRowTable(const QModelIndex &index);

    void onAddRowTable();
    void onEditRowTable();
    void onMarkDeletion();
    void onClose();

    void onOpenGroupInvestigations();

    void slotContextMenuRequested(QPoint pos);
    void slot_typeCatalogChanged();
    void slot_typeFormChanged();

    void onDataChangedItemModel();

private:
    Ui::CatForSqlTableModel *ui;

    TypeCatalog m_typeCatalog;
    TypeForm    m_typeForm;

    QToolBar    *toolBar;
    QToolButton *btnBarAdd;
    QToolButton *btnBarEdit;
    QToolButton *btnBarDeletion;
    QToolButton *btnBarUpdateTable;

    GroupInvestigationList *list_group;

    int ret_id       = -1;      // date ce se determinam si returnam
    QString ret_cod  = nullptr; // cand typeForm = SelectForm
    QString ret_name = nullptr;

    BaseSqlTableModel *model;   
    DataBase          *db;
    QMenu             *menu;

    CheckBoxDelegate      *checkbox_delegate;
    DoubleSpinBoxDelegate *db_spinbox_delegate;
    ComboDelegate         *gr_investig_delegate;

protected:
    void changeEvent(QEvent *event);
};

#endif // CATFORSQLTABLEMODEL_H
