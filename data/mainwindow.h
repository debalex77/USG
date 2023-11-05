#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QEvent>
#include <QSettings>
#include <QMdiArea>
#include <QTextEdit>
#include <QTranslator>
#include <QToolBar>
#include <QToolButton>
#include <QMetaEnum>
#include <QMenu>
#include <QTimer>
#include <QProgressBar>

#include "catalogs/catforsqltablemodel.h"
#include "docs/listdoc.h"
#include "docs/listdocreportorder.h"
#include <catalogs/listform.h>
#include <data/about.h>
#include "data/appsettings.h"
#include "data/usersettings.h"
#include "docs/docorderecho.h"
#include "docs/docappointmentspatients.h"
#include "data/mdiareacontainer.h"
#include "data/database.h"
#include "data/popup.h"
#include <data/reports.h>
#include <data/updatereleasesapp.h>

//=============================================================
#define APPLICATION_NAME_SHORT  QCoreApplication::tr("USG")
#define ORGANIZATION_NAME       "SC 'Alovada-Med' SRL"
#define APPLICATION_NAME        QCoreApplication::tr("USG - Evidența examinărilor ecografice")
#define APPLICATION_VERSION     "2.0.1"

//=============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initButton();
    void initActions();
    void updateTextBtn(); // pu traducerea dinamica
    QString getVersionAppInTableSettingsUsers();
    void setVersionAppInTableSettingsUsers();
    void closeDatabases();

public slots:
    void mDockWidgetShowTex(const QString txtMsg);

private slots:
    void updateTimer();
    void updateTimerDocWidget();
    void checkUpdateApp();
    void openSourceCode();
    void openUserManual();
    void openAbout();
    void openListForm(const int indexEnum);
    void openCatInvestigations();
    void openCatTypesPrices();
    void openPatientAppointments();
    void openOrderEcho();
    void openHistoryPatient();
    void openDocExamen();
    void openReports();
    void openAppSettings();
    void openUserSettings();
    void openPricing();
    void removeSubWindow();
    void onOpenLMDesigner();

private:
    Ui::MainWindow *ui;

    QMdiArea*         mdiArea;
    MdiAreaContainer* mdiAreaCont;

    DataBase*  db;
    PopUp*     popUp;
    QTextEdit* textEdit;
    QMenu*     menu;
    QTimer*    timer;
    QTimer*    timer_doc_widget;
    int        pause_timer = 0; //pu pauza in 5 secunde
    QProgressBar* progress;

    QDockWidget* dock_widget;
    QTextEdit* textEdit_dockWidget;

    QToolBar    *toolBar;
    QToolButton *btnDoctors;
    QToolButton *btnNurses;
    QToolButton *btnPacients;
    QToolButton *btnHistoryPatient;
    QToolButton *btnUsers;
    QToolButton *btnPatientAppointments;
    QToolButton *btnOrderEcho;
    QToolButton *btnDocExamen;
    QToolButton *btnReports;
    QToolButton *btnOrganizations;
    QToolButton *btnInvestigations;
    QToolButton *btnPricing;
    QToolButton *btnSettings;
    QToolButton *btnAbout;

    AppSettings*     appSett;
    UserSettings*    userSettings;
    ListDoc*         listDoc;
    Reports*         reports;
    DocOrderEcho*    docOrderEcho;
    ListDocReportOrder* list_order;
    ListDocReportOrder* list_report;
    CatForSqlTableModel* cat_investigations;
    DocAppointmentsPatients* registration_patients;
    PatientHistory* patient_history;
    UpdateReleasesApp* update_app;
    LimeReport::ReportEngine* m_report;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
};
#endif // MAINWINDOW_H
