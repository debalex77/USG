#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlQuery"
#include "QSqlError"
#include "userpreferences.h"
#include <QDirIterator>
#include <QDockWidget>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings(ORGANIZATION_NAME, USG_VERSION_FULL); // denumirea organizatiei si aplicatiei

    if (globals::minimizeAppToTray)
        initMinimizeAppToTray();

    mdiArea = new QMdiArea(this);                      // alocam memoria p/u MdiArea
    mdiArea->setViewMode(QMdiArea::TabbedView);        // proprietatea de prezentarea sub-windows
    mdiArea->setTabsClosable(true);                    // proprietatea de inchidere a sub-windows
    setCentralWidget(mdiArea);                         // instalam ca widgetul central
    mdiAreaCont = new MdiAreaContainer(mdiArea, this); // initierea container-lui MdiArea
    downloader_version = new DownloaderVersion(this);  // verificarea versiunei

    db = new DataBase(this);

    update_app = new UpdateReleasesApp(this);

    dock_widget         = new QDockWidget(this);
    textEdit_dockWidget = new QTextEdit(this);
    dock_widget->setWidget(textEdit_dockWidget);
    dock_widget->minimumSizeHint();
    addDockWidget(Qt::BottomDockWidgetArea, dock_widget);
    dock_widget->hide();

    popUp = new PopUp(); // alocam memoria p/u vizualizarea notelor
    menu  = new QMenu(this);

    txt_title_bar = new QLabel(ui->statusbar);
    statusBar()->addWidget(txt_title_bar);

    initButton();
    initActions();
    updateTextBtn();

    timer = new QTimer(this);
    timer_doc_widget = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);  // pu aprecierea daca e deschisa forma MainWindow
    connect(timer_doc_widget, &QTimer::timeout, this, &MainWindow::updateTimerDocWidget);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete downloader_version;
    delete update_app;
    delete textEdit_dockWidget;
    delete dock_widget;
    delete mdiAreaCont;
    delete mdiArea;
    delete popUp;
    delete menu;
    delete toolBar;
    delete ui;
}

// **********************************************************************************
// --- initierea connectarilor si prezentarea textului informativ

void MainWindow::mDockWidgetShowTex(const QString txtMsg)
{
    textEdit_dockWidget->setHtml(txtMsg);
    textEdit_dockWidget->setFixedHeight(70);
    dock_widget->show();
}

void MainWindow::initButton()
{
    // alocam memoria
    toolBar                 = new QToolBar(tr("Bara cu instrumente"));
#if defined (Q_OS_LINUX)

#elif defined(Q_OS_WIN)
    toolBar->setStyleSheet("font-family: 'Segoe UI';");
#endif
    btnDoctors              = new QToolButton(toolBar);
    btnNurses               = new QToolButton(toolBar);
    btnPacients             = new QToolButton(toolBar);
    btnHistoryPatient       = new QToolButton(toolBar);
    btnUsers                = new QToolButton(toolBar);
    btnPatientAppointments  = new QToolButton(toolBar);
    btnOrderEcho            = new QToolButton(toolBar);
    btnDocExamen            = new QToolButton(toolBar);
    btnOrganizations        = new QToolButton(toolBar);
    btnInvestigations       = new QToolButton(toolBar);
    btnReports              = new QToolButton(toolBar);
    btnPricing              = new QToolButton(toolBar);
    btnSettings             = new QToolButton(toolBar);
    btnAbout                = new QToolButton(toolBar);
    btnBlock                = new QToolButton(toolBar);

    btnDoctors->setIcon(QIcon(":/img/doctor_x32.png"));
    btnDoctors->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnDoctors->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnDoctors->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnDoctors->setStyleSheet("font-size: 13pt;");
#endif

    btnNurses->setIcon(QIcon(":/img/nurse_x32.png"));
    btnNurses->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnNurses->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnNurses->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnNurses->setStyleSheet("font-size: 13pt;");
#endif

    btnPacients->setIcon(QIcon(":/img/pacient_x32.png"));
    btnPacients->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPacients->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnPacients->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnPacients->setStyleSheet("font-size: 13pt;");
#endif

    btnHistoryPatient->setIcon(QIcon(":/img/medical-history.png"));
    btnHistoryPatient->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnHistoryPatient->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnHistoryPatient->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnHistoryPatient->setStyleSheet("font-size: 13pt;");
#endif

    btnUsers->setIcon(QIcon(":/img/user_x32.png"));
    btnUsers->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnUsers->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnUsers->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnUsers->setStyleSheet("font-size: 13pt;");
#endif

    btnPatientAppointments->setIcon(QIcon(":/img/registration_patients.png"));
    btnPatientAppointments->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPatientAppointments->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnPatientAppointments->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnPatientAppointments->setStyleSheet("font-size: 13pt;");
#endif

    btnOrderEcho->setIcon(QIcon(":/img/orderEcho_x32.png"));
    btnOrderEcho->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnOrderEcho->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnOrderEcho->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnOrderEcho->setStyleSheet("font-size: 13pt;");
#endif

    btnDocExamen->setIcon(QIcon(":/img/examenEcho.png"));
    btnDocExamen->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnDocExamen->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnDocExamen->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnDocExamen->setStyleSheet("font-size: 13pt;");
#endif

    btnOrganizations->setIcon(QIcon(":/img/company_x32.png"));
    btnOrganizations->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnOrganizations->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnOrganizations->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnOrganizations->setStyleSheet("font-size: 13pt;");
#endif

    btnInvestigations->setIcon(QIcon(":/img/investigations_x32.png"));
    btnInvestigations->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnInvestigations->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnInvestigations->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnInvestigations->setStyleSheet("font-size: 13pt;");
#endif

    btnReports->setIcon(QIcon(":/img/reports.png"));
    btnReports->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnReports->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnReports->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnReports->setStyleSheet("font-size: 13pt;");
#endif

    btnPricing->setIcon(QIcon(":/img/price_x32.png"));
    btnPricing->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPricing->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnPricing->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnPricing->setStyleSheet("font-size: 13pt;");
#endif

    btnSettings->setIcon(QIcon(":/img/settings_x32.png"));
    btnSettings->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnSettings->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnSettings->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnSettings->setStyleSheet("font-size: 13pt;");
#endif

    btnAbout->setIcon(QIcon(":/img/info_x32.png"));
    btnAbout->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnAbout->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnAbout->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnAbout->setStyleSheet("font-size: 13pt;");
#endif

    btnBlock->setIcon(QIcon(":/img/lock.png"));
    btnBlock->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnBlock->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
    btnBlock->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
    btnBlock->setStyleSheet("font-size: 13pt;");
#endif

    //adaugam widget-uri in toolBar
    toolBar->addWidget(btnDoctors);
    toolBar->addWidget(btnNurses);
    toolBar->addSeparator();
    toolBar->addWidget(btnPacients);
    toolBar->addWidget(btnHistoryPatient);
    toolBar->addSeparator();
    toolBar->addWidget(btnOrganizations);
    toolBar->addSeparator();
    toolBar->addWidget(btnInvestigations);
    toolBar->addWidget(btnPricing);
    toolBar->addSeparator();
    toolBar->addWidget(btnPatientAppointments);
    toolBar->addWidget(btnOrderEcho);
    toolBar->addWidget(btnDocExamen);
    toolBar->addSeparator();
    toolBar->addWidget(btnReports);
    toolBar->addSeparator();
    toolBar->addWidget(btnUsers);
    toolBar->addSeparator();
    toolBar->addWidget(btnSettings);
    toolBar->addSeparator();
    toolBar->addWidget(btnAbout);
    toolBar->addSeparator();
    toolBar->addWidget(btnBlock);
    addToolBar(toolBar);

    btnDoctors->setMouseTracking(true);
    btnDoctors->installEventFilter(this);
    btnNurses->setMouseTracking(true);
    btnNurses->installEventFilter(this);
    btnPacients->setMouseTracking(true);
    btnPacients->installEventFilter(this);
    btnHistoryPatient->setMouseTracking(true);
    btnHistoryPatient->installEventFilter(this);
    btnUsers->setMouseTracking(true);
    btnUsers->installEventFilter(this);
    btnPatientAppointments->setMouseTracking(true);
    btnPatientAppointments->installEventFilter(this);
    btnOrderEcho->setMouseTracking(true);
    btnOrderEcho->installEventFilter(this);
    btnDocExamen->setMouseTracking(true);
    btnDocExamen->installEventFilter(this);
    btnOrganizations->setMouseTracking(true);
    btnOrganizations->installEventFilter(this);
    btnInvestigations->setMouseTracking(true);
    btnInvestigations->installEventFilter(this);
    btnPricing->setMouseTracking(true);
    btnPricing->installEventFilter(this);
    btnReports->setMouseTracking(true);
    btnReports->installEventFilter(this);
    btnSettings->setMouseTracking(true);
    btnSettings->installEventFilter(this);
    btnAbout->setMouseTracking(true);
    btnAbout->installEventFilter(this);

    connect(btnDoctors, &QAbstractButton::clicked, this, [this](){
       openListForm(static_cast<int>(ListForm::TypeListForm::Doctors));
    });
    connect(btnNurses, &QAbstractButton::clicked, this, [this](){
       openListForm(static_cast<int>(ListForm::TypeListForm::Nurses));
    });
    connect(btnPacients, &QAbstractButton::clicked, this, [this](){
       openListForm(static_cast<int>(ListForm::TypeListForm::Pacients));
    });
    connect(btnUsers, &QAbstractButton::clicked, this, [this](){
       openListForm(static_cast<int>(ListForm::TypeListForm::Users));
    });
    connect(btnOrganizations, &QAbstractButton::clicked, this, [this](){
       openListForm(static_cast<int>(ListForm::TypeListForm::Organizations));
    });

    connect(btnHistoryPatient, &QAbstractButton::clicked, this, &MainWindow::openHistoryPatient);
    connect(btnReports, &QAbstractButton::clicked, this, &MainWindow::openReports);
    connect(btnInvestigations, &QToolButton::clicked, this, &MainWindow::openCatInvestigations);
    connect(btnPricing, &QAbstractButton::clicked, this, &MainWindow::openPricing);
    connect(btnPatientAppointments, &QToolButton::clicked, this, &MainWindow::openPatientAppointments);
    connect(btnOrderEcho, &QToolButton::clicked, this, &MainWindow::openOrderEcho);
    connect(btnDocExamen, &QAbstractButton::clicked, this, &MainWindow::openDocExamen);
    connect(btnSettings, &QAbstractButton::clicked, this, &MainWindow::openAppSettings);
    connect(btnAbout, &QAbstractButton::clicked, this, &MainWindow::openAbout);
    connect(btnBlock, &QAbstractButton::clicked, this, &MainWindow::onBlockApp);
}

void MainWindow::initActions()
{
    QAction* actionDoctors          = new QAction(QIcon(":img/doctor_x32.png"), tr("Doctori"), this);
    QAction* actionNurses           = new QAction(QIcon(":img/nurse_x32.png"), tr("As.medicale"), this);
    QAction* actionPacients         = new QAction(QIcon(":img/pacient_x32.png"), tr("Pacienți"), this);
    QAction* actionOrganizations    = new QAction(QIcon(":img/company_x32.png"), tr("Persoane juridice"), this);
    QAction* actionUsers            = new QAction(QIcon(":img/user_x32.png"), tr("Utilizatori"), this);
    QAction* actionAppSettings      = new QAction(QIcon(":img/settings_x32.png"), tr("Setările aplicației"), this);
    QAction* actionUserSettings     = new QAction(QIcon(":/img/user_preferences.png"), tr("Preferințele utilizatorului"), this);
    QAction* actionOpenPricing      = new QAction(QIcon(":/img/price_x32.png"), tr("Formarea prețurilor"), this);
    QAction* actionOpenAppointments = new QAction(QIcon(":/img/registration_patients.png"), tr("Programarea pacienților"), this);
    QAction* actionOpenOrderEcho    = new QAction(QIcon(":/img/orderEcho_x32.png"), tr("Comanda ecografică"), this);
    QAction* actionOpenReportEcho   = new QAction(QIcon(":/img/examenEcho.png"), tr("Raport ecografic"), this);
    QAction* actionOpenDesigner     = new QAction(QIcon("://images/logo1.png"), tr("LimeReport (designer)"), this);
    QAction* actionOpenNormograms   = new QAction(QIcon("://img/normograma.png"), tr("Normograme"), this);

    connect(actionDoctors, &QAction::triggered, this, [this](){
        openListForm(static_cast<int>(ListForm::TypeListForm::Doctors));
     });

    connect(actionNurses, &QAction::triggered, this, [this](){
        openListForm(static_cast<int>(ListForm::TypeListForm::Nurses));
     });

    connect(actionPacients, &QAction::triggered, this, [this](){
        openListForm(static_cast<int>(ListForm::TypeListForm::Pacients));
     });

    connect(actionOrganizations, &QAction::triggered, this, [this](){
        openListForm(static_cast<int>(ListForm::TypeListForm::Organizations));
     });

    connect(actionUsers, &QAction::triggered, this, [this](){
        openListForm(static_cast<int>(ListForm::TypeListForm::Users));
     });

    connect(actionAppSettings, &QAction::triggered, this, &MainWindow::openAppSettings);
    connect(actionUserSettings, &QAction::triggered, this, &MainWindow::openUserSettings);
    connect(actionOpenPricing, &QAction::triggered, this, &MainWindow::openPricing);
    connect(actionOpenAppointments, &QAction::triggered, this, &MainWindow::openPatientAppointments);
    connect(actionOpenOrderEcho, &QAction::triggered, this, &MainWindow::openOrderEcho);
    connect(actionOpenReportEcho, &QAction::triggered, this, &MainWindow::openDocExamen);

    ui->menuCataloage->addAction(actionDoctors);
    ui->menuCataloage->addAction(actionNurses);
    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionPacients);
    ui->menuCataloage->addAction(actionOrganizations);
    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionUsers);

    QMenu* clasifiers = new QMenu(ui->menuCataloage);
    ui->menuCataloage->addMenu(clasifiers);
    clasifiers->setTitle(tr("Clasificatori"));

    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionOpenNormograms);
    connect(actionOpenNormograms, &QAction::triggered, this, &MainWindow::openNormograms);

    QAction* actionTypesPrice = new QAction(QIcon(":/img/typesPrices_x32.png"), tr("Tipul prețurilor"), this);
    QAction* actionInvestigations = new QAction(QIcon(":/img/investigations_x32.png"), tr("Investigații"), this);
    QAction* actionGroupInvestigations = new QAction(QIcon(":/img/tree_yellow.png"), tr("Arbore investigațiilor"), this);
    QAction* actionConcluzionTemplets = new QAction(QIcon(":/img/templates.png"), tr("Șabloane concluziilor"), this);

    clasifiers->addAction(actionInvestigations);
    clasifiers->addAction(actionGroupInvestigations);
    clasifiers->addSeparator();
    clasifiers->addAction(actionTypesPrice);
    clasifiers->addSeparator();
    clasifiers->addAction(actionConcluzionTemplets);

    connect(actionInvestigations, &QAction::triggered, this, &MainWindow::openCatInvestigations);
    connect(actionGroupInvestigations, &QAction::triggered, this, &MainWindow::openGroupInvestigation);
    connect(actionTypesPrice, &QAction::triggered, this, &MainWindow::openCatTypesPrices);
    connect(actionConcluzionTemplets, &QAction::triggered, this, &MainWindow::openConcluzionTemplets);

    ui->menuDocumente->addAction(actionOpenPricing);
    ui->menuDocumente->addSeparator();
    ui->menuDocumente->addAction(actionOpenAppointments);
    ui->menuDocumente->addAction(actionOpenOrderEcho);
    ui->menuDocumente->addAction(actionOpenReportEcho);

    connect(actionOpenDesigner, &QAction::triggered, this, &MainWindow::onOpenLMDesigner);
    ui->menuRapoarte->addAction(actionOpenDesigner);

    ui->menuService->addAction(actionAppSettings);
    ui->menuService->addAction(actionUserSettings);

    QAction *actionSourceCode = new QAction(QIcon(":/img/github.png"), tr("Cod sursă"), this);
    ui->menuAssistance->addAction(actionSourceCode);
    connect(actionSourceCode, &QAction::triggered, this, &MainWindow::openSourceCode);

    QAction *actionReportBug = new QAction(QIcon(":/img/bug.png"), tr("Raportează eroare"), this);
    ui->menuAssistance->addAction(actionReportBug);
    ui->menuAssistance->addSeparator();
    connect(actionReportBug, &QAction::triggered, this, &MainWindow::openReportBug);

    QAction *action_user_manual = new QAction(QIcon(":/img/help_question.png"), tr("Manual Online"), this);
    ui->menuAssistance->addAction(action_user_manual);
    connect(action_user_manual, &QAction::triggered, this, &MainWindow::openUserManual);

    QAction *actionOpenReleases = new QAction(QIcon(":/img/history.png"), tr("Istoria versiunilor"), this);
    ui->menuAssistance->addAction(actionOpenReleases);
    connect(actionOpenReleases, &QAction::triggered, this, &MainWindow::openDescriptionRealease);

    QAction *actionCheckUpdate = new QAction(QIcon(":/img/update_app.png"), tr("Verifică versiunea nouă"), this);
    ui->menuAssistance->addAction(actionCheckUpdate);
    connect(actionCheckUpdate, &QAction::triggered, this, &MainWindow::checkUpdateApp);

    QAction* actionShowAsistantTip = new QAction(QIcon(":/img/oxygen/ktip.png"), tr("Prezentarea asistentului de sfaturi"), this);
    ui->menuAssistance->addAction(actionShowAsistantTip);
    connect(actionShowAsistantTip, &QAction::triggered, this, &MainWindow::onShowAsistantTip);

    QAction* action_about = new QAction(QIcon(":/img/info_x32.png"), tr("Despre aplicația"), this);
    ui->menuAssistance->addSeparator();
    ui->menuAssistance->addAction(action_about);
    connect(action_about, &QAction::triggered, this, &MainWindow::openAbout);
}

// **********************************************************************************
// --- procesarea actiunilor

void MainWindow::checkUpdateApp()
{
    // verificam daca este versiunea noua
    downloader_version->getData();
    connect(downloader_version, &DownloaderVersion::onReady, this, &MainWindow::onReadyVersion);
}

void MainWindow::openDescriptionRealease()
{
    QFile file(":/releases.md");
    if (! file.open(QIODevice::ReadOnly))
        return;

    info_window = new InfoWindow(this);
    info_window->setAttribute(Qt::WA_DeleteOnClose);
    info_window->setTypeInfo(InfoWindow::TypeInfo::INFO_REALEASE);
    info_window->setTex(file.readAll());
    info_window->show();
}

void MainWindow::openSourceCode()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG"));
}

void MainWindow::openReportBug()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG/issues/new?assignees=&labels=&projects=&template=bug_report.md&title="));
}

void MainWindow::openUserManual()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG/wiki/User-manual-(ro)"));
}

void MainWindow::updateTextBtn()
{
    btnDoctors->setText(tr("Doctori"));
    btnNurses->setText(tr("As.medicale"));
    btnPacients->setText(tr("Pacienți"));
    btnHistoryPatient->setText(tr("Istoria"));
    btnUsers->setText(tr("Utilizatori"));
    btnPatientAppointments->setText(tr("Programarea"));
    btnOrderEcho->setText(tr("Comanda ecograf."));
    btnDocExamen->setText(tr("Raport ecograf."));
    btnOrganizations->setText(tr("Centre medicale"));
    btnInvestigations->setText(tr("Investigații"));
    btnReports->setText(tr("Rapoarte"));
    btnPricing->setText(tr("Prețuri"));
    btnSettings->setText(tr("Setări"));
    btnAbout->setText(tr("Despre aplicația"));
    btnBlock->setText(tr("Blocare"));
}

// **********************************************************************************
// --- procesarea determinarii/setarii versiunilor aplicatiei

QString MainWindow::getVersionAppInTableSettingsUsers()
{
    QString version_app;
    QSqlQuery qry;
    if (! globals::mySQLhost.isEmpty())
        qry.prepare("SELECT versionApp FROM userPreferences WHERE id_users = :id_users AND versionApp IS NOT Null;");
    else
        qry.prepare("SELECT versionApp FROM userPreferences WHERE id_users = :id_users AND versionApp NOT NULL;");
    qry.bindValue(":id_users", globals::idUserApp);
    if (qry.exec() && qry.next())
        version_app = qry.value(0).toString();
    else
        qCritical(logCritical()) << tr("%1 - getVersionAppInTableSettingsUsers()").arg(metaObject()->className())
                                 << tr("Eroare solicitarii - determinarea versiunei aplicatiei - %1").arg(qry.lastError().text());

    return version_app;
}

void MainWindow::setVersionAppInTableSettingsUsers()
{
    QSqlQuery qry;
    if (! globals::mySQLhost.isEmpty())
        qry.prepare("UPDATE userPreferences SET versionApp = :versionApp WHERE id_users = :id_users AND versionApp IS NOT Null;");
    else
        qry.prepare("UPDATE userPreferences SET versionApp = :versionApp WHERE id_users = :id_users AND versionApp NOT NULL;");
    qry.bindValue(":versionApp", USG_VERSION_FULL);
    qry.bindValue(":id_users", globals::idUserApp);
    if (! qry.exec() && qry.next())
        qCritical(logCritical()) << tr("%1 - setVersionAppInTableSettingsUsers()").arg(metaObject()->className())
                                 << tr("Eroare de actualizare a veriunei aplicatiei din tabela 'settingsUsers'.");
}

void MainWindow::closeDatabases()
{
    if (db->getDatabase().open())
        db->getDatabase().close();
    if (db->getDatabaseImage().open())
        db->getDatabaseImage().close();
}

// **********************************************************************************
// --- actualizarea la lansare aplicatiei

void MainWindow::updateTimer()
{
    if (this->isVisible()){
        if (pause_timer < 1){  // pauza in 2-3 secunde
            pause_timer += 1;
            return;
        }
        timer->stop(); // oprim timerul

        if (globals::thisMySQL)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (.sqlite3): utilizator (") + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(": utilizator (") + globals::nameUserApp + ")");

        if (globals::firstLaunch){
            textEdit_dockWidget->setHtml(tr("%1   %2: Se completează catalogul <b><u>'Investigații'</u></b>.")
                                         .arg(db->getHTMLImageInfo(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")));
            textEdit_dockWidget->setFixedHeight(70);
            dock_widget->show();
            timer_doc_widget->start(1000);
        }

        // Verificam daca este schimbata versiunea aplicatiei
        QString version_app = getVersionAppInTableSettingsUsers();

        if (version_app != USG_VERSION_FULL){
            if (update_app->execUpdateCurrentRelease(version_app)){
                setVersionAppInTableSettingsUsers();
                openDescriptionRealease();
                // prezentam informatia de actualizarea in panoul informativ
                textEdit_dockWidget->clear();
                textEdit_dockWidget->setHtml(tr("%1   Aplicația a fost actualizată până la versiunea: USG v" USG_VERSION_FULL).arg(db->getHTMLImageInfo()));
                textEdit_dockWidget->setFixedHeight(100);
                dock_widget->show();
            }
        }

        // verificam versiunea noua
        if (globals::checkNewVersionApp)
            checkUpdateApp();

        // prezentarea istoriei versiunilor
        if (globals::showHistoryVersion && version_app == USG_VERSION_FULL)
            openDescriptionRealease();

        // prezentarea manualului online
        if (globals::showUserManual)
            openUserManual();

        // prezentarea sfaturilor
        if (globals::showAsistantHelper)
            onShowAsistantTip();
    }
}

void MainWindow::updateTimerDocWidget()
{
    if (dock_widget->isVisible()){
        timer_doc_widget->stop();

        txt_title_bar->setText(tr("Se incarca clasificatorul \"Investigații\" ... "));

        progress = new QProgressBar(ui->statusbar);
        progress->setAlignment(Qt::AlignRight);
        progress->setMaximumSize(120, 15);
        progress->hide(); // Ascunde inițial bara de progres
        txt_title_bar->hide();
        ui->statusbar->addWidget(txt_title_bar);
        ui->statusbar->addWidget(progress);

        db->updateInvestigationFromXML_2024();
        connect(db, &DataBase::updateProgress, this, &MainWindow::handleUpdateProgress);
        connect(db, &DataBase::finishedProgress, this, &MainWindow::handleFinishedProgress);

        db->insertDataForTabletypesPrices(); //completarea preturilor

        textEdit_dockWidget->setHtml(tr("%1<br>%2 %3 Completat clasificatorul <b><u>'Investigații'</u></b> sursa - <b>Catalogul tarifelor unice (anexa nr.3)</b>.<br>"
                                        "     <b>Vezi:</b> <em>Meniu principal al aplicației -> Cataloage -> Clasificatori -> Investigații<em>")
                                     .arg(textEdit_dockWidget->toHtml(), db->getHTMLImageInfo(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")));

        // Lansarea - User Manual
        globals::firstLaunch = false;
    }
}

// **********************************************************************************
// --- procesarea slot-urilor

void MainWindow::openAbout()
{
    About* win_about = new About(this);
    win_about->exec();
    delete win_about;
}

void MainWindow::openListForm(const int indexEnum)
{
    ListForm* listForm = new ListForm(this);

    switch (indexEnum) {
    case 0:
        listForm->setProperty("typeListForm", ListForm::TypeListForm::Doctors);
        listForm->setWindowIcon(QIcon(":/img/doctor_x32.png"));
        break;
    case 1:
        listForm->setProperty("typeListForm", ListForm::TypeListForm::Nurses);
        listForm->setWindowIcon(QIcon(":/img/nurse_x32.png"));
        break;
    case 2:
        listForm->setProperty("typeListForm", ListForm::TypeListForm::Pacients);
        listForm->setWindowIcon(QIcon(":/img/pacient_x32.png"));
        break;
    case 3:
        listForm->setProperty("typeListForm", ListForm::TypeListForm::Users);
        listForm->setWindowIcon(QIcon(":/img/user_x32.png"));
        break;
    case 4:
        listForm->setProperty("typeListForm", ListForm::TypeListForm::Organizations);
        listForm->setWindowIcon(QIcon(":/img/company_x32.png"));
        break;
    default:
        qWarning(logWarning()) << tr("Nu a fost determinat indexul 'enum=TypeListForm' !!!");
        break;
    }
    mdiAreaCont->addWidget(listForm);

    connect(listForm, &ListForm::mCloseThisForm, this, &MainWindow::removeSubWindow);
}

void MainWindow::openNormograms()
{
    normograms = new Normograms(this);
    normograms->setAttribute(Qt::WA_DeleteOnClose);
    normograms->show();
}

void MainWindow::openCatInvestigations()
{
    cat_investigations = new CatForSqlTableModel(this);
    cat_investigations->setAttribute(Qt::WA_DeleteOnClose);
    cat_investigations->setWindowIcon(QIcon(":/img/investigations_x32.png"));
    cat_investigations->setProperty("typeCatalog", cat_investigations->Investigations);
    cat_investigations->setProperty("typeForm", cat_investigations->ListForm);
    mdiAreaCont->addWidget(cat_investigations);
    cat_investigations->show();

    connect(cat_investigations, &CatForSqlTableModel::mCloseThisForm, this, &MainWindow::removeSubWindow);
}

void MainWindow::openGroupInvestigation()
{
    group_investigation = new GroupInvestigationList(this);
    group_investigation->setAttribute(Qt::WA_DeleteOnClose);
    mdiAreaCont->addWidget(group_investigation);
    group_investigation->show();
}

void MainWindow::openConcluzionTemplets()
{
    cat_concluzionTemplets = new CatForSqlTableModel(this);
    cat_concluzionTemplets->setAttribute(Qt::WA_DeleteOnClose);
    cat_concluzionTemplets->setWindowIcon(QIcon(":/img/templates.png"));
    cat_concluzionTemplets->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::ConclusionTemplates);
    cat_concluzionTemplets->setProperty("typeForm",    CatForSqlTableModel::TypeForm::ListForm);
    mdiAreaCont->addWidget(cat_concluzionTemplets);
    cat_concluzionTemplets->show();
}

void MainWindow::openCatTypesPrices()
{
    cat_investigations = new CatForSqlTableModel(this);
    cat_investigations->setAttribute(Qt::WA_DeleteOnClose);
    cat_investigations->setWindowIcon(QIcon(":/img/typesPrices_x32.png"));
    cat_investigations->setProperty("typeCatalog", cat_investigations->TypesPrices);
    cat_investigations->setProperty("typeForm",    cat_investigations->ListForm);
    mdiAreaCont->addWidget(cat_investigations);
    cat_investigations->show();

    connect(cat_investigations, &CatForSqlTableModel::mCloseThisForm, this, &MainWindow::removeSubWindow);
}

void MainWindow::openPatientAppointments()
{
    registration_patients = new DocAppointmentsPatients(this);
    registration_patients->setAttribute(Qt::WA_DeleteOnClose);
    mdiAreaCont->addWidget(registration_patients);
    registration_patients->show();
    connect(registration_patients, &DocAppointmentsPatients::mCloseThisForm, this, &MainWindow::removeSubWindow);

//    patientAppointments = new DocPatientAppointments(this);
//    patientAppointments->setAttribute(Qt::WA_DeleteOnClose);
//    mdiAreaCont->addWidget(patientAppointments);
//    patientAppointments->show();

//    connect(patientAppointments, &DocPatientAppointments::mCloseThisForm, this, &MainWindow::removeSubWindow);
}

void MainWindow::openOrderEcho()
{
    list_order = new ListDocReportOrder(this);
    list_order->setAttribute(Qt::WA_DeleteOnClose);
    list_order->setWindowIcon(QIcon(":/img/orderEcho_x32.png"));
    list_order->setProperty("typeDoc", list_order->orderEcho);
    mdiAreaCont->addWidget(list_order);
    list_order->show();

    txt_title_bar->setText(tr("Se incarca documente ... "));

    progress = new QProgressBar(ui->statusbar);
    progress->setAlignment(Qt::AlignRight);
    progress->setMaximumSize(120, 15);
    progress->hide(); // Ascunde inițial bara de progres
    txt_title_bar->hide();
    ui->statusbar->addWidget(txt_title_bar);
    ui->statusbar->addWidget(progress);

    connect(list_order, &ListDocReportOrder::updateProgress, this, &MainWindow::handleUpdateProgress);
    connect(list_order, &ListDocReportOrder::finishedProgress, this, &MainWindow::handleFinishedProgress);
}

void MainWindow::openHistoryPatient()
{
    patient_history = new PatientHistory(this);
    patient_history->setAttribute(Qt::WA_DeleteOnClose);
    patient_history->show();
}

void MainWindow::openDocExamen()
{
    list_report = new ListDocReportOrder(this);
    list_report->setAttribute(Qt::WA_DeleteOnClose);
    list_report->setWindowIcon(QIcon(":/img/orderEcho_x32.png"));
    list_report->setProperty("typeDoc", list_report->reportEcho);
    mdiAreaCont->addWidget(list_report);
    list_report->show();

    txt_title_bar->setText(tr("Se incarca documente ... "));

    progress = new QProgressBar(ui->statusbar);
    progress->setAlignment(Qt::AlignRight);
    progress->setMaximumSize(120, 15);
    progress->hide(); // Ascunde inițial bara de progres
    txt_title_bar->hide();
    ui->statusbar->addWidget(txt_title_bar);
    ui->statusbar->addWidget(progress);

    connect(list_report, &ListDocReportOrder::updateProgress, this, &MainWindow::handleUpdateProgress);
    connect(list_report, &ListDocReportOrder::finishedProgress, this, &MainWindow::handleFinishedProgress);
}

void MainWindow::openReports()
{
    reports = new Reports(this);
    reports->setAttribute(Qt::WA_DeleteOnClose);
    reports->loadSettingsReport();
    mdiAreaCont->addWidget(reports);
    reports->show();
}

void MainWindow::openAppSettings()
{
    appSett = new AppSettings(this);
    appSett->setAttribute(Qt::WA_DeleteOnClose);
    appSett->setWindowIcon(QIcon(":/img/settings_x32.png"));
    appSett->readSettings();
    appSett->show();
}

void MainWindow::openUserSettings()
{
    auto preference = new UserPreferences(this);
    preference->setProperty("Id", globals::idUserApp);
    if (preference->exec() == QDialog::Accepted)
        db->updateVariableFromTableSettingsUser();
}

void MainWindow::openPricing()
{
    listDoc = new ListDoc(this);
    listDoc->setAttribute(Qt::WA_DeleteOnClose);
    listDoc->setProperty("ModeSelection", false);
    listDoc->setWindowIcon(QIcon(":/img/price_x32.png"));
    mdiAreaCont->addWidget(listDoc);
    listDoc->show();
}

void MainWindow::removeSubWindow()
{
    // mdiAreaCont->remove(mdiAreaCont->currentIndex());
    auto activeSubWindow = mdiAreaCont->currentSubWindow();
    if (activeSubWindow) {
        activeSubWindow->close(); // Închide subfereastra activă
    }
}

void MainWindow::onOpenLMDesigner()
{
    m_report = new LimeReport::ReportEngine(this);
    m_report->setShowDesignerModal(false);
    m_report->designReport();
}

void MainWindow::onReadyVersion()
{
    QDir dir;
    QString path_file_version = dir.toNativeSeparators(QDir::tempPath() + "/usg_version.txt");
    QFile file(path_file_version);
    if (! file.open(QIODevice::ReadOnly))
        return;

    QString version_online = file.readAll().trimmed();
    QString str_version_online = version_online;
    str_version_online.replace(1,1,"");
    str_version_online.replace(2,1,"");
    const int num_version_online = str_version_online.toInt();

    QString version_app = QString(USG_VERSION_FULL);
    version_app.replace(1,1,"");
    version_app.replace(2,1,"");
    const int num_version_app = version_app.toInt();

    if (USG_VERSION_FULL != version_online && num_version_app < num_version_online){
        textEdit_dockWidget->setHtml(tr("%1   %2: Exist\304\203 o versiune nou\304\203 a aplica\310\233iei <b><u>%3</u></b>.")
                                         .arg(db->getHTMLImageInfo(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"), version_online));
        textEdit_dockWidget->setFixedHeight(70);
        dock_widget->show();

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea actualiz\304\203rii"),
                                 tr("Dori\310\233i s\304\203 desc\304\203rca\310\233i versiunea nou\304\203 ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton){
            downloadNewVersionApp(version_online);
        } else if (messange_box.clickedButton() == noButton) {

        }

    } else {
        txt_title_bar->setText(tr("Folosiți cea mai recentă versiune \"%1\".").arg(USG_VERSION_FULL));
    }

#if defined(Q_OS_LINUX)
    QString str_cmd = "rm " + path_file_version;
    system(str_cmd.toStdString().c_str());
#elif defined (Q_OS_WIN)
    QString str_cmd = "del " + path_file_version;
    system(str_cmd.toStdString().c_str());
#endif
}

void MainWindow::onShowAsistantTip()
{
    asistant_tip =  new AsistantTipApp(this);
    asistant_tip->setAttribute(Qt::WA_DeleteOnClose);
    asistant_tip->setStep();
    asistant_tip->show();
}

void MainWindow::onBlockApp()
{
    this->hide();
    autorization = new AuthorizationUser(this);
    if (autorization->exec() == QDialog::Accepted)
        this->show();
}

// **********************************************************************************
// --- descarcarea fisierului cu aplicatia noua si prezentarea progress bar

void MainWindow::downloadNewVersionApp(const QString str_new_version)
{
    txt_title_bar->setText(tr("Se descarc\304\203 fi\310\231ierul ... "));

    progress = new QProgressBar(ui->statusbar);
    progress->setAlignment(Qt::AlignRight);
    progress->setMaximumSize(120, 15);
    ui->statusbar->addWidget(txt_title_bar);
    ui->statusbar->addWidget(progress);

    connect(&downloader, &Downloader::updateDownloadProgress, this, &MainWindow::onUpdateProgress);
    connect(&downloader, &Downloader::finishedDownload, this, &MainWindow::onNewAppFinishedDownload);

    QString str_url = GITHUB_URL_DOWNLOAD  "/v" + str_new_version + "/USG_v" + str_new_version + "_Linux_amd64.deb";
    downloader.get(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QUrl(str_url));
}

void MainWindow::onUpdateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    progress->setMaximum(bytesTotal);
    progress->setValue(bytesReceived);
}

void MainWindow::onNewAppFinishedDownload()
{
    delete progress;
    txt_title_bar->setText(tr("Fi\310\231ierul este desc\304\203rcat cu succes."));
}

// **********************************************************************************
// --- minimizarea in tray

void MainWindow::initMinimizeAppToTray()
{
    /* initierea iconitei in tray */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/img/eco_systemTray.png"));
    trayIcon->setToolTip(APPLICATION_NAME + " v" + USG_VERSION_FULL);

    /* meniu */
    QMenu   *menu = new QMenu(this);
    QAction *viewWindow = new QAction(tr("Maximiza\310\233i fereastra"), this);
    QAction *quitAction = new QAction(tr("Ie\310\231ire"), this);

    connect(viewWindow, &QAction::triggered, this, &MainWindow::show);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    menu->addAction(viewWindow);
    menu->addAction(quitAction);

    /* setam meniu contextual */
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    // connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    //         this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        if(globals::minimizeAppToTray){
            if(!this->isVisible()){
                this->show();
            } else {
                this->hide();
            }
        }
        break;
    default:
        break;
    }
}

void MainWindow::handleUpdateProgress(int num_records, int value)
{
    if (num_records <= 0)
        return;

    if (! txt_title_bar->isVisible())
        txt_title_bar->show();

    // Calculează progresul
    int value_progress = (value * 100) / num_records;

    if (! progress->isVisible())
        progress->show();

    progress->setValue(value_progress);
}

void MainWindow::handleFinishedProgress(const QString textTitle)
{
    delete progress;
    txt_title_bar->setText(textTitle);
}

// **********************************************************************************
// --- evenimentele formei

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (globals::minimizeAppToTray && this->isVisible()) {

        event->ignore();
        this->hide();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);

        trayIcon->showMessage(APPLICATION_NAME,
                              tr("Aplica\310\233ia este minimizat\304\203 în tray. "
                                 "Pentru a maximiza fereastra aplica\310\233iei, "
                                 "face\310\233i clic pe pictograma aplica\310\233iei din tray."),
                              icon,
                              2000);

    } else {

        if (! globals::showQuestionCloseApp){
            closeDatabases();
            qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicatia.")
                                    .arg(globals::nameUserApp);
            event->accept();
            return;
        }

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Finisarea lucrului"),
                                 tr("Dori\310\233i s\304\203 \303\256nchide\310\233i aplica\310\233ia ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton){
            closeDatabases();
            qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicația.").arg(globals::nameUserApp);
            event->accept();
        } else if (messange_box.clickedButton() == noButton) {
            event->ignore();
        }

    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == btnDoctors){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnDoctors->pos().x() - 20, btnDoctors->pos().y() + 82)); // determinam parametrii globali
            popUp->setPopupText(tr("Lista cu doctori."));         // setam textul
            popUp->showFromGeometryTimer(p);                      // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                     // ascundem nota
            return true;
        }
    } else if (obj == btnNurses){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnNurses->pos().x() - 40, btnNurses->pos().y() + 82)); // determinam parametrii globali
            popUp->setPopupText(tr("Lista as.medicale."));    // setam textul
            popUp->showFromGeometryTimer(p);                     // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                    // ascundem nota
            return true;
        }
    } else if (obj == btnPacients){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnPacients->pos().x() - 80, btnPacients->pos().y() + 82));    // determinam parametrii globali
            popUp->setPopupText(tr("Lista pacienților \n"
                                   "înregistrați în baza de date.")); // setam textul
            popUp->showFromGeometryTimer(p);                          // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                         // ascundem nota
            return true;
        }
    } else if (obj == btnHistoryPatient){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnHistoryPatient->pos().x() - 50, btnHistoryPatient->pos().y() + 82));    // determinam parametrii globali
            popUp->setPopupText(tr("Vizualizarea istorie \n"
                                   "pacienților.")); // setam textul
            popUp->showFromGeometryTimer(p);                          // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                         // ascundem nota
            return true;
        }
    } else if (obj == btnOrganizations){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnOrganizations->pos().x() - 18, btnOrganizations->pos().y() + 82)); // determinam parametrii globali
            popUp->setPopupText(tr("Lista persoanelor \n"
                                   "juridice."));  // setam textul
            popUp->showFromGeometryTimer(p);                        // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                       // ascundem nota
            return true;
        }
    } else if (obj == btnUsers){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnUsers->pos().x() - 44, btnUsers->pos().y() + 82));
            popUp->setPopupText(tr("Lista utilizatorilor")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnInvestigations){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnInvestigations->pos().x() - 64, btnInvestigations->pos().y() + 82));
            popUp->setPopupText(tr("Clasificatorul investigațiilor")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnPricing){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnPricing->pos().x() - 62, btnPricing->pos().y() + 82));
            popUp->setPopupText(tr("Documente cu\n formarea  preturilor")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnPatientAppointments){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnPatientAppointments->pos().x() - 44, btnPatientAppointments->pos().y() + 82));
            popUp->setPopupText(tr("Programarea pacienților \n la investigații ecografice")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnOrderEcho){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnOrderEcho->pos().x() - 34, btnOrderEcho->pos().y() + 82));
            popUp->setPopupText(tr("Comanda pentru\n investigatiile ecografice")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnDocExamen){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnDocExamen->pos().x() - 30, btnDocExamen->pos().y() + 82));
            popUp->setPopupText(tr("Examinarea ecografica")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnReports){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnReports->pos().x() - 55, btnReports->pos().y() + 82));
            popUp->setPopupText(tr("Rapoarte investigațiilor \n ecografice")); // setam textul
            popUp->showFromGeometryTimer(p);                                  // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                                                 // ascundem nota
            return true;
        }
    } else if (obj == btnSettings){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnSettings->pos().x() - 50, btnSettings->pos().y() + 82));
            popUp->setPopupText(tr("Setările principale\n"
                                   "ale aplicației."));  // setam textul
            popUp->showFromGeometryTimer(p);             // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                            // ascundem nota
            return true;
        }
    } else if (obj == btnAbout){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(btnAbout->pos().x() - 20, btnAbout->pos().y() + 82));
            popUp->setPopupText(tr("Informația generală\n"
                                   "despre aplicația."));  // setam textul
            popUp->showFromGeometryTimer(p);               // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                              // ascundem nota
            return true;
        }
    }
    return false;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);    // traducem
        if (globals::thisMySQL)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (.sqlite3): base - '%1', utilizator (").arg(globals::sqliteNameBase) + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(": utilizator (") + globals::nameUserApp + ")");
        updateTextBtn();
    } else if (event->type() == QEvent::WindowTitleChange){
        if (globals::thisMySQL)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(" (.sqlite3): base - '%1', utilizator (").arg(globals::sqliteNameBase) + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + USG_VERSION_FULL + tr(": utilizator (") + globals::nameUserApp + ")");
    }
}

