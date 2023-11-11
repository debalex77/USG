#ifndef REPORTS_H
#define REPORTS_H

#include <QDialog>
#include <QKeyEvent>
#include <LimeReport>
#include <QComboBox>
#include <QSpinBox>
#include <QStyleFactory>
#include <QProgressDialog>

#include <data/globals.h>
#include <models/basesqlquerymodel.h>
#include <catalogs/catgeneral.h>
#include <catalogs/catorganizations.h>
#include <catalogs/customperiod.h>

namespace Ui {
class Reports;
}

class Reports : public QDialog
{
    Q_OBJECT

public:
    explicit Reports(QWidget *parent = nullptr);
    ~Reports();

    void loadSettingsReport();

private:
    void getNameReportsFromDorectory();
    void initConnections();
    void initPercentCombobox();
    void setStyleForButtom();
    QString getMainQry();
    QString getQuerySystem(const QString str_sytem);
    void setImageForReports();
    void setReportVariabiles();

    void saveSettingsReport();
    void insertUpdateDataReportInTableSettingsReports(const bool insertData);
    int getIdReportShowOnLaunch() const;

private slots:
    void renderStarted();
    void renderPageFinished(int renderedPageCount);
    void renderFinished();

    void slotSetTextScalePercentChanged(int percent);
    void slotScalePercentChanged(const int percent);
    void slotPageNavigatorChanged(const int page);
    void slotPagesSet(const int pagesCount);
    void slotPageChanged(const int page);

    void generateReport();
    void openDesignerReport();
    void openSettingsReport();
    void openCustomPeriod();
    void typeReportsCurrentIndexChanged(const int index);
    void organizationCurrentIndexChanged(const int index);
    void openCatOrganization();
    void openCatContract();
    void openCatDoctor();

private:
    Ui::Reports *ui;

    DataBase* db;

    QStandardItemModel* model_img;

    BaseSqlQueryModel* modelOrganizations;
    BaseSqlQueryModel* modelContracts;
    BaseSqlQueryModel* modelDoctors;
    CustomPeriod*      customPeriod;

    CatOrganizations* cat_organization;
    CatContracts*     cat_contract;
    CatGeneral*       cat_doctor;

    QComboBox* m_scalePercent;
    QSpinBox*  m_pageNavigator;
    LimeReport::ReportEngine* m_report;
    LimeReport::PreviewReportWidget* m_preview;

    QProgressDialog* m_progressDialog;

    QStyle* style_fusion = QStyleFactory::create("Fusion");

    int m_id = -1;
    int m_id_onLaunch = -1;
    int exist_logo = 0;
    int m_currentPage;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // REPORTS_H
