#ifndef CATALOGVIEW_H
#define CATALOGVIEW_H

#include <QDialog>
#include <QMenu>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMdiSubWindow>

#include <catalogs/organizationdialog.h>
#include <catalogs/contractdialog.h>
#include <catalogs/userdialog.h>

#include <common/table_sections.h>
#include <common/reportsettingsmanager.h>
#include <common/property_macros.h>
#include <common/tablecolumnscontroller.h>
#include <common/globals.h>

#include <customs/toolbarcustom.h>

#include <delegates/centericondelegate.h>

#include <data/database.h>
#include <data/popup.h>

#include <models/catalogsmodel.h>
#include <models/sortmodel.h>

namespace Ui {
class CatalogView;
}

class CatalogView : public QDialog
{
    Q_OBJECT

public:
    explicit CatalogView(DataBase &db,
                         CatalogType::Type catalogType,
                         QWidget *parent = nullptr);
    ~CatalogView();

private slots:
    void onScroll(int value);

    void onAdd();
    void onEdit();
    void onDelete();
    void onUpdate();
    void onShowHideColumn();

    void onClickedTableView(const QModelIndex &index);
    void onDoubleClickedTableView(const QModelIndex &index);
    void onColumnsChanged();

private:
    void loadFilterBySettings();
    void loadSizeSection();
    void saveSizeSection();

    void initTableView();
    void updateTableView();
    void initToolBar();

    bool isValidIndex(const QModelIndex &index);
    int lastVisibleSection() const;

    void reject(); // pu inchiderea subferestrei MDI la apasarea ESC

private:
    Ui::CatalogView *ui;

    ReportSettingsManager m_settings;
    CatalogViewFilter m_filter;
    CatalogType::Type m_catalogType;

    const QString m_class = "CatalogView";

    DataBase &m_db;
    PopUp    *popUp;
    QMenu    *menu;

    ToolBarCustom *toolBar;

    int m_currentRow = -1;

    CatalogsModel *model;
    SortModel     *proxy;

    TableColumnsController *m_columnsController = nullptr;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // 'Key_Up', 'Key_Down' etc
};

#endif // CATALOGVIEW_H
