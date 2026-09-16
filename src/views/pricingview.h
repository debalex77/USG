#ifndef PRICINGVIEW_H
#define PRICINGVIEW_H

#include <QDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>

#include <data/database.h>
#include <common/globals.h>
#include <data/popup.h>

#include <documents/pricingdialog.h>

#include <catalogs/customperiod.h>

#include <delegates/centericondelegate.h>

#include <models/queryrolesmodel.h>
#include <models/pricingmodel.h>
#include <models/pricingsortmodel.h>

#include <common/appmetatypes.h>
#include <common/table_sections.h>
#include <common/reportsettingsmanager.h>

#include <customs/custommessage.h>

namespace Ui {
class PricingView;
}

class PricingView : public QDialog
{
    Q_OBJECT

public:
    explicit PricingView(DataBase &db, QWidget *parent = nullptr);
    ~PricingView();

    /** completam structura setarilor jurnalului
     *  (vezi common/appmetatypes.h) */
    void loadDocJournalSettings();

    void initPeriod();
    void initBtnToolBar();
    void initBtnFilter();
    void updateTextPeriod();

    void updateModelOrganizations();
    void updateModelContracts();
    void updateModelUsers();

    void loadDataFilter();
    void loadSizeSection();
    void saveSettingsJournal();

    void initTableView();
    void updateTableView();

    void initConnections();
    void loadPeriodFilterSizeSection();

private slots:
    void openCustomPeriod();
    void applyFilter();
    void clearFilter();
    void openFilter();
    void setFilterByIDOrganization();

    void indexChangedCombo(int index);

    void createNewDoc();
    void editDoc();
    void deleteDoc();
    void printDoc();

    void onDoubleClickedTable(const QModelIndex &index);

    void validatePeriodAndUpdate();

    void slotContextMenuRequested(const QPoint &pos);

    void updatePostDocs();

private:
    Ui::PricingView *ui;

    // setarile si variabile pu filtru si data
    const QString type_doc = "DocPricing";
    ReportSettingsManager m_settings;
    PricingJournalSettings journalSettings;

    DataBase &m_db;
    PopUp    *popUp;
    QMenu    *menu;

    int m_currentRow = -1;

    PricingModel     *model; // model pu tableView
    PricingSortModel *proxy; // model pu sortare

    QueryRolesModel *modelOrganizations = nullptr;
    QueryRolesModel *modelContracts     = nullptr;
    QueryRolesModel *modelUsers         = nullptr;

    QString m_numberDoc;
    QMap<QString, QVariant> itemsFilter;
    QMap<QString, QDateTime> itemsFilterPeriod;

    QString toolButtonStyleForText;
    QString toolButtonStyleForIcon;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // pu identificare butoanelor:
                                            // 'Key_Up', 'Key_Down', 'Key_Home'
};

#endif // PRICINGVIEW_H
