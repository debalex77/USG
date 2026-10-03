#ifndef CATALOGVIEW_H
#define CATALOGVIEW_H

#include <QDialog>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMdiSubWindow>

#include <features/catalogs/organizationdialog.h>
#include <features/catalogs/contractdialog.h>
#include <features/catalogs/userdialog.h>

#include <common/table_sections.h>
#include <settings/layoutsettingsmanager.h>
#include <common/property_macros.h>
#include <ui/services/tablecolumnscontroller.h>
#include <common/globals.h>

#include <ui/widgets/toolbarcustom.h>

#include <ui/delegates/centericondelegate.h>

#include <database/database.h>
#include <app/popup.h>

#include <features/catalogs/catalogsmodel.h>
#include <features/patients/patientremovalrepository.h>
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
    void onContextMenuRequested(const QPoint &pos);
    void onRemovePatient();
    void onShowHideColumn();

    void onDoubleClickedTableView(const QModelIndex &index);
    void onColumnsChanged();

private:
    void loadFilterBySettings();
    void loadSizeSection();
    void saveSizeSection();

    void initTableView();
    void updateTableView();
    void initToolBar();
    void initSearchEdit();
    void applySearch();

    int rowById(qint64 id, int fromRow) const;
    void fetchAllRows();
    void showLoadError();
    void showPatientReferences(const QString &patientName,
                               const QList<PatientRemovalRepository::Reference> &references);
    void showPatientRemovalError(const QString &patientName, const QString &error);

    bool isValidIndex(const QModelIndex &index);
    int lastVisibleSection() const;

    void reject(); // pu inchiderea subferestrei MDI la apasarea ESC

private:
    Ui::CatalogView *ui;

    LayoutSettingsManager m_settings;
    CatalogViewFilter m_filter;
    CatalogType::Type m_catalogType;

    const QString m_class = "CatalogView";

    DataBase &m_db;
    PopUp    *popUp;
    QMenu    *menu;

    ToolBarCustom *toolBar;

    CatalogsModel *model;
    SortModel     *proxy;

    TableColumnsController *m_columnsController = nullptr;

    // Căutarea pacienților (doar în catalogul Pacienți).
    QLineEdit *m_searchEdit = nullptr;
    QTimer     m_searchTimer;
    QString    m_appliedSearch;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // 'Key_Up', 'Key_Down' etc
};

#endif // CATALOGVIEW_H
