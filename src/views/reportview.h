#ifndef REPORTVIEW_H
#define REPORTVIEW_H

#include <QDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QToolButton>
#include <QBuffer>
#include <QToolTip>

#include <catalogs/customperiod.h>
#include <common/globals.h>
#include <common/tablecolumnscontroller.h>
#include <customs/toolbarcustom.h>
#include <delegates/centericondelegate.h>
#include <documents/orderdialog.h>
#include <documents/reportdialog.h>
#include <data/popup.h>
#include <models/reportjournalmodel.h>
#include <models/queryrolesmodel.h>
#include <models/sortmodel.h>

#include <common/appmetatypes.h>
#include <common/reportsettingsmanager.h>
#include <common/table_sections.h>
#include <data/database.h>

class QDateTimeEdit;
class ToolBarCustom;
class ReportJournalModel;
class SortModel;
class TableColumnsController;
class QueryRolesModel;
class QStandardItemModel;
class PopUp;

namespace Ui { class Form; }

class ReportView final : public QDialog
{
    Q_OBJECT

public:
    explicit ReportView(DataBase &db, QWidget *parent = nullptr);
    ~ReportView() override;

private slots:
    void reload();
    void addReport();
    void removeReport();
    void editReport();
    void printReport();
    void openOrder();
    void applyFilter();
    void clearFilter();
    void toggleFilter();
    void choosePeriod();
    void showColumnsMenu();
    void showContextMenu(const QPoint &pos);
    void fetchNextBatch(int value);
    void updateReportPreview();
    void toggleReportPreview();
    void organizationChanged(int index);
    void onColumnsChanged();

private:
    bool isValidIndex(const QModelIndex &index);
    const ReportJournal::Item *currentItem(bool showWarning = true) const;
    void buildUi();
    void initTable();
    void initConnections();
    void initFilterModels();
    void updateContractsModel();
    void restoreFilterControls();
    void updatePeriodText();
    void configurePrintButton();
    void printReportDocument(PrintType::Column typePrint);
    void loadTableSettings();
    void restoreTableSections();
    void saveTableSettings();
    bool removeCloudDocuments(const QByteArray &reportUuid,
                              bool removeAssociatedOrder,
                              QString *error);
    int lastVisibleSection() const;

protected:
    bool previewImagesDocs(QEvent *event);
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    Ui::Form *ui = nullptr;
    ReportSettingsManager m_settings;
    const QString m_settingsGroup = "ReportView";
    DataBase &m_db;
    JournalFilter m_filter;

    ToolBarCustom *m_toolBar = nullptr;
    PopUp *m_popup = nullptr;
    ReportJournalModel *m_model = nullptr;
    SortModel *m_proxy = nullptr;
    TableColumnsController *m_columns = nullptr;
    QueryRolesModel *m_organizations = nullptr;
    QueryRolesModel *m_contracts = nullptr;
    QueryRolesModel *m_users = nullptr;
    QStandardItemModel *m_previewModel = nullptr;
};

#endif // REPORTVIEW_H
