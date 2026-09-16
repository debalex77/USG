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
#include <QPointer>
#include <QSystemTrayIcon>

#include <views/catalogview.h>
#include <views/catalogtableeditor.h>
#include <views/onlineaccountview.h>

#include <catalogs/asistanttipapp.h>
#include <catalogs/groupinvestigationlist.h>
#include <catalogs/normograms.h>

#include <data/authorizationuser.h>
#include <data/about.h>
#include <data/appsettings.h>
#include <data/mdiareacontainer.h>
#include <data/database.h>
#include <data/popup.h>
#include <data/reports.h>
#include <data/updatereleasesapp.h>
#include <data/downloaderversion.h>
#include <data/downloader.h>

#include <views/orderview.h>
#include <views/reportview.h>

#include <documents/appointmentdialog.h>
#include <views/pricingview.h>

#include <common/cloudserverconfig.h>
#include <common/archivecreationhandler.h>
#include <common/version.h>

//=============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(DataBase &db, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initButton();
    void initActions();
    void updateTextBtn(); // pu traducerea dinamica
    QString getVersionAppInTableSettingsUsers();
    bool setVersionAppInTableSettingsUsers();
    void closeDatabases();
    void closeAndSaveSettingsSubwindows();

public slots:
    void mDockWidgetShowTex(const QString txtMsg);

private slots:
    void updateTimer();
    void launchFirstRunWizard();
    void checkUpdateApp();
    void openDescriptionRealease();
    void openSourceCode();
    void openReportBug();
    void openUserManual();
    void openAbout();

    void openCatalogDoctors();
    void openCatalogNurses();
    void openCatalogPacients();
    void openCatalogUsers();
    void openCatalogOrganizations();

    void openNormograms();
    void openInvestigations();
    void openGroupInvestigation();
    void openConcluzionTemplets();
    void openTypesPrices();
    void openPatientAppointments();
    void openOrderView();
    void openHistoryPatient();
    void openDocExamen();
    void openReports();
    void openAppSettings();
    void openUserSettings();
    void openPricing();
    void removeSubWindow();
    void onOpenLMDesigner();
    void onReadyVersion();
    void onShowAsistantTip();
    void onBlockApp();
    void openOnlineAccountView();
    void openCloudServerConfig();
    void openFirstRunWizard();
    void openArchiveHandler();

    void downloadNewVersionApp(const QString str_new_version);
    void onUpdateProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onNewAppFinishedDownload();

    void initMinimizeAppToTray();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void handleUpdateProgress(int num_records, int value);
    void handleFinishedProgress(const QString textTitle);

private:
    Ui::MainWindow *ui;

    QMdiArea         *mdiArea;
    MdiAreaContainer *mdiAreaCont;

    DataBase     &m_db;
    PopUp        *popUp;
    QTextEdit    *textEdit;
    QMenu        *menu;
    QTimer       *timer;
    QProgressBar *progress;

    int pause_timer = 0; //pu pauza in 5 secunde

    QDockWidget *dock_widget;
    QTextEdit   *textEdit_dockWidget;

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
    QToolButton *btnBlock;

    QPointer<AppSettings>     appSett;
    Reports                  *reports;
    ReportView               *list_report;
    Normograms               *normograms;
    GroupInvestigationList   *group_investigation;
    AppointmentDialog        *registration_patients;
    PatientHistory           *patient_history;
    UpdateReleasesApp        *update_app;
    LimeReport::ReportEngine *m_report;
    DownloaderVersion        *downloader_version;
    Downloader               downloader;
    AsistantTipApp           *asistant_tip;
    InfoWindow               *info_window;
    AuthorizationUser        *autorization;
    ArchiveCreationHandler   *archive_handler;

    QLabel *txt_title_bar = nullptr;

    QSystemTrayIcon *trayIcon;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
};
#endif // MAINWINDOW_H
