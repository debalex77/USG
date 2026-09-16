#ifndef ORDERVIEW_H
#define ORDERVIEW_H

#include <QDialog>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMessageBox>
#include <QBuffer>
#include <QToolTip>
#include <QMdiSubWindow>
#include <QSqlQueryModel>
#include <functional>

#include <catalogs/customperiod.h>
#include <catalogs/catalogdialog.h> //doctors, nurses, patients
#include <catalogs/organizationdialog.h>
#include <catalogs/agentsendemail.h>

#include <common/appmetatypes.h>
#include <common/reportsettingsmanager.h>
#include <common/tablecolumnscontroller.h>
#include <common/globals.h>
#include <common/processingaction.h>

#include <customs/toolbarcustom.h>
#include <customs/customdialoginvestig.h>

#include <delegates/centericondelegate.h>
#include <data/database.h>
#include <data/popup.h>

#include <documents/orderdialog.h>
#include <documents/reportdialog.h>

#include <threads/databaseprovider.h>
#include <threads/docemailexporterworker.h>

#include <models/queryrolesmodel.h>
#include <models/orderjournalmodel.h>
#include <models/sortmodel.h>

namespace Ui {
class OrderView;
}

class OrderView : public QDialog
{
    Q_OBJECT

public:
    explicit OrderView(DataBase &db, QWidget *parent = nullptr);
    ~OrderView();

public slots:
    void updatePrintButtons(bool showDesignerMenuPrint);

private slots:
    void onScroll(int value);

    void onAddDoc();
    void onEditDoc();
    void onDeleteDoc();
    void onAddFilter();
    void onSetFilter();
    void onDeleteFilter();
    void onUpdateTableView();
    void onHideShowColumn();
    void onPrintDoc();
    void onSendEmail();
    void onCreateReport();
    void onViewTabOrder();
    void onOpenPeriod();
    void onSearchPacients();

    void openPreviewOrder();
    void printPreviewOrder();

    void openPreviewReport();
    void printPreviewReport();

    void updateEmailExportProgress(const QString &text);
    void launchEmailAgent(const QVector<DatesForAgentEmail> &exportedData);

    void onClickedTableView(const QModelIndex &index);
    void onDoubleClickedTableView(const QModelIndex &index);
    void onColumnsChanged();

    void indexChangedCombo(int index);
    void onApplyFilter();

    void slotContextMenuRequested(const QPoint &pos);

private:
    /** completam structura setarilor jurnalului
     *  (vezi common/appmetatypes.h) */
    void loadFilterJournalBySettings();
    void initBoxFilterAndTableFooter();

    void updateTextPeriod();
    void loadFilterData();
    void loadSizeSection();
    void saveSettingsJournal();

    void updateModelOrganizations();
    void updateModelContracts();
    void updateModelUsers();

    int lastVisibleSection() const;

    void initTableView();
    void updateTableView();
    void updateDocumentPreview();
    void configurePrintButton(QPushButton *button,
                              const std::function<void(PrintType::Column)> &printAction);
    void printOrder(PrintType::Column typePrint);
    void printReport(PrintType::Column typePrint);

    void initToolBar();
    void initBtnFilter();

    bool isValidIndex(const QModelIndex &index);

    void clearFilter();
    void validatePeriodAndUpdate();

    void reject(); // pu inchiderea subferestrei MDI la apasarea ESC

private:
    Ui::OrderView *ui;
    ReportSettingsManager m_settings;
    JournalFilter m_filter;
    const QString m_typeJournal = "OrderView";

    int m_currentRow = -1;

    DataBase &m_db;
    DatabaseProvider m_dbProvider;
    PopUp *popUp;
    QMenu *menuSetFilter = nullptr;

    QueryRolesModel *modelOrganizations = nullptr;
    QueryRolesModel *modelContracts     = nullptr;
    QueryRolesModel *modelUsers         = nullptr;

    TableColumnsController *m_columnsController = nullptr;
    ToolBarCustom     *toolBar;
    OrderJournalModel *modelTable;
    SortModel         *proxyTable;

    QSqlQueryModel *modelViewOrder  = nullptr;
    QSqlQueryModel *modelViewReport = nullptr;
    bool m_viewTabVisible = false;

    QString styleBtnMessageBox;

    ProcessingAction *loader = nullptr;

protected:
    bool previewImagesDocs(QEvent *event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // 'Key_Up', 'Key_Down' etc
};

#endif // ORDERVIEW_H
