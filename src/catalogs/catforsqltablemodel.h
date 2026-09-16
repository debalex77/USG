#ifndef CATFORSQLTABLEMODEL_H
#define CATFORSQLTABLEMODEL_H

#include <QDialog>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QMenu>
#include <LimeReport>
#include <QStandardItemModel>

#include <delegates/checkboxdelegate.h>

#include "catalogs/groupinvestigationlist.h"
#include "delegates/combodelegate.h"
#include "delegates/doublespinboxdelegate.h"
#include "models/basesqltablemodel.h"
#include "data/database.h"
#include <common/table_sections.h>

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

    enum TypeCatalog
    {
        Investigations,
        TypesPrices,
        ConclusionTemplates,
        SystemTemplates
    };
    Q_ENUM(TypeCatalog)

    enum TypeForm
    {
        ListForm,
        SelectForm
    };
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
    void mSelectData(const QVariantMap data);
    void mCloseThisForm();

private:
    void initBtnForm();
    void initBtnToolBar();

    void updateTableView();
    void updateHeaderTableInvestigations();
    void updateHeaderTableTypesPrices();
    void updateHeaderTableConclusionTemplates();
    void updateHeaderTableFormationsBySystemTemplates();

    QString getNameTable();

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

    // void onDataChangedItemModel();
    void onDataChangedItemModel(
        const QModelIndex &topLeft,
        const QModelIndex &bottomRight,
        const QVector<int> &roles);
    void printCatalogCost();

private:
    Ui::CatForSqlTableModel *ui;

    TypeCatalog m_typeCatalog;
    TypeForm    m_typeForm;

    GroupInvestigationList *list_group;

    int ret_id       = -1;      // date ce se determinam si returnam
    QString ret_cod  = nullptr; // cand typeForm = SelectForm
    QString ret_name = nullptr;

    BaseSqlTableModel *model;   
    DataBase          *db;
    QMenu             *menu;

    LimeReport::ReportEngine *m_report;
    QStandardItemModel *print_model;

    CheckBoxDelegate      *checkbox_delegate;
    DoubleSpinBoxDelegate *db_spinbox_delegate;
    ComboDelegate         *gr_investig_delegate;

protected:
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // pu identificare butoanelor:
                                            // 'Key_Up', 'Key_Down', 'Key_Home', 'Key_End', 'Key_Prt'
                                            // se foloseste functia - keyReleaseEvent(), analogica keyPressEvent()
};

#endif // CATFORSQLTABLEMODEL_H
