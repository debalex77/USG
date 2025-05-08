#ifndef REPORTS_H
#define REPORTS_H

#include <QDialog>
#include <QKeyEvent>
#include <LimeReport>
#include <QComboBox>
#include <QSpinBox>
#include <QStyleFactory>
#include <QProgressDialog>
#include <infowindow.h>

#include <data/globals.h>
#include <models/basesqlquerymodel.h>
#include <catalogs/catgeneral.h>
#include <catalogs/catorganizations.h>
#include <catalogs/customperiod.h>
#include <common/reportsettingsmanager.h>

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
    void initConnections();
    void initPercentCombobox();
    QString getMainQry();
    QString getQuerySystem(const QString str_sytem);
    void setImageForReports();
    void setReportVariabiles();

    void saveSettingsReport();

    void setReportVolumInvestigationsPerCod();

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
    void sendReportToEmail();

    void slotGetCallbackDataDoctor(LimeReport::CallbackInfo info, QVariant &data);
    void slotGetCallbackDataInvestigation(LimeReport::CallbackInfo info, QVariant &data);
    void prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data);
    void slotChangePosDoctors(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);
    void slotChangePosInvestigation(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);

private:
    Ui::Reports *ui;
    ReportSettingsManager settings;

    DataBase *db;

    BaseSqlQueryModel *modelOrganizations;
    BaseSqlQueryModel *modelContracts;
    BaseSqlQueryModel *modelDoctors;
    CustomPeriod      *customPeriod;

    CatOrganizations *cat_organization;
    CatContracts     *cat_contract;
    CatGeneral       *cat_doctor;

    QComboBox *m_scalePercent;
    QSpinBox  *m_pageNavigator;
    LimeReport::ReportEngine        *m_report;
    LimeReport::PreviewReportWidget *m_preview;
    QStandardItemModel *model_img;

    QProgressDialog *m_progressDialog;

    InfoWindow *info_window;

    QStyle *style_fusion = QStyleFactory::create("Fusion");

    LimeReport::ICallbackDatasource *callbackDatasource;
    QSqlQuery *list_investigation;
    QSqlQuery *list_doctor;

    int m_id          = -1;
    int m_id_onLaunch = -1;
    int exist_logo    = 0;
    int exist_stamp_organization = 0;
    int exist_signature_doctore = 0;
    QString m_emailTo = nullptr;
    bool send_email =  false;
    int m_currentPage;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // REPORTS_H
