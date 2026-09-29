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

#include <features/catalogs/catalogview.h>
#include <features/catalogs/catalogtableeditor.h>
#include <features/catalogs/onlineaccountview.h>

#include <features/assistant/asistanttipapp.h>
#include <features/catalogs/groupinvestigationlist.h>
#include <features/catalogs/normograms.h>

#include <app/authorizationuser.h>
#include <app/about.h>
#include <settings/appsettings.h>
#include <app/mdiareacontainer.h>
#include <database/database.h>
#include <app/popup.h>
#include <infrastructure/reporting/reports.h>
#include <database/updatereleasesapp.h>
#include <app/downloaderversion.h>
#include <app/downloader.h>

#include <features/orders/orderview.h>
#include <features/reports/reportview.h>

#include <features/appointments/appointmentdialog.h>
#include <features/patients/patienthistory.h>
#include <features/pricing/pricingview.h>

#include <features/cloud/cloudserverconfig.h>
#include <features/cloud/cloudserverview.h>
#include <infrastructure/backup/archivecreationhandler.h>
#include <core/version.h>

//=============================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class InfoWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(DataBase &db, QWidget *parent = nullptr);
    ~MainWindow();

    // Rulează verificarea/migrarea schemei înainte ca utilizatorul să poată
    // lucra în fereastra principală. La eșec aplicația nu trebuie continuată.
    bool completeStartup();

signals:
    void restartApproved();

private:
    void initButton();
    void initActions();
    void updateTextBtn(); // pu traducerea dinamica
    QString databaseSchemaVersion();
    bool setDatabaseSchemaVersion(const QString &version);
    void updateWindowTitle();
    void appendMigrationMessage(const QString &message);
    QString confirmedDatabaseVersion;
    void closeDatabases();
    bool closeAndSaveSettingsSubwindows();
    bool createAutomaticSqliteArchive();
    void applyTrayPreference();

public slots:
    void mDockWidgetShowTex(const QString txtMsg);

private slots:
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
    void restartAfterLanguageChange();
    void openUserSettings();
    void openPricing();
    void removeSubWindow();
    void onOpenLMDesigner();
    void onReadyVersion();
    void onShowAsistantTip();
    void onBlockApp();
    void openOnlineAccountView();
    void openCloudServerView();
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
    QProgressBar *progress;

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
    QToolButton *btnUserManual;
    QToolButton *btnAbout;
    QToolButton *btnBlock;

    QPointer<AppSettings>     appSett;
    bool m_restartRequested = false;
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

    QSystemTrayIcon *trayIcon = nullptr;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
};
#endif // MAINWINDOW_H
