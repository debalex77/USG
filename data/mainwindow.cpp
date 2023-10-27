#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlQuery"
#include "QSqlError"
#include <QDirIterator>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME); // denumirea organizatiei si aplicatiei

    mdiArea = new QMdiArea(this);               // alocam memoria p/u MdiArea
    mdiArea->setViewMode(QMdiArea::TabbedView); // proprietatea de prezentarea sub-windows
    mdiArea->setTabsClosable(true);             // proprietatea de inchidere a sub-windows
    setCentralWidget(mdiArea);                  // instalam ca widgetul central
    mdiAreaCont = new MdiAreaContainer(mdiArea, this); //initierea container-lui MdiArea

    update_app = new UpdateReleasesApp(this);

    dock_widget         = new QDockWidget(this);
    textEdit_dockWidget = new QTextEdit(this);
    dock_widget->setWidget(textEdit_dockWidget);
    dock_widget->minimumSizeHint();
    addDockWidget(Qt::BottomDockWidgetArea, dock_widget);
    dock_widget->hide();

    popUp = new PopUp(); // alocam memoria p/u vizualizarea notelor
    menu  = new QMenu(this);

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

    btnDoctors->setIcon(QIcon(":/img/doctor_x32.png"));
    btnDoctors->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnDoctors->setCursor(Qt::ArrowCursor);

    btnNurses->setIcon(QIcon(":/img/nurse_x32.png"));
    btnNurses->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnNurses->setCursor(Qt::ArrowCursor);

    btnPacients->setIcon(QIcon(":/img/pacient_x32.png"));
    btnPacients->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPacients->setCursor(Qt::ArrowCursor);

    btnHistoryPatient->setIcon(QIcon(":/img/medical-history.png"));
    btnHistoryPatient->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnHistoryPatient->setCursor(Qt::ArrowCursor);

    btnUsers->setIcon(QIcon(":/img/user_x32.png"));
    btnUsers->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnUsers->setCursor(Qt::ArrowCursor);

    btnPatientAppointments->setIcon(QIcon(":/img/registration_patients.png"));
    btnPatientAppointments->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPatientAppointments->setCursor(Qt::ArrowCursor);

    btnOrderEcho->setIcon(QIcon(":/img/orderEcho_x32.png"));
    btnOrderEcho->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnOrderEcho->setCursor(Qt::ArrowCursor);

    btnDocExamen->setIcon(QIcon(":/img/examenEcho.png"));
    btnDocExamen->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnDocExamen->setCursor(Qt::ArrowCursor);

    btnOrganizations->setIcon(QIcon(":/img/company_x32.png"));
    btnOrganizations->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnOrganizations->setCursor(Qt::ArrowCursor);

    btnInvestigations->setIcon(QIcon(":/img/investigations_x32.png"));
    btnInvestigations->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnInvestigations->setCursor(Qt::ArrowCursor);

    btnReports->setIcon(QIcon(":/img/reports.png"));
    btnReports->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnReports->setCursor(Qt::ArrowCursor);

    btnPricing->setIcon(QIcon(":/img/price_x32.png"));
    btnPricing->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPricing->setCursor(Qt::ArrowCursor);

    btnSettings->setIcon(QIcon(":/img/settings_x32.png"));
    btnSettings->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnSettings->setCursor(Qt::ArrowCursor);

    btnAbout->setIcon(QIcon(":/img/info_x32.png"));
    btnAbout->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnAbout->setCursor(Qt::ArrowCursor);

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

    connect(btnHistoryPatient, &QAbstractButton::clicked, this, &::MainWindow::openHistoryPatient);
    connect(btnReports, &QAbstractButton::clicked, this, &MainWindow::openReports);
    connect(btnInvestigations, &QToolButton::clicked, this, &MainWindow::openCatInvestigations);
    connect(btnPricing, &QAbstractButton::clicked, this, &MainWindow::openPricing);
    connect(btnPatientAppointments, &QToolButton::clicked, this, &MainWindow::openPatientAppointments);
    connect(btnOrderEcho, &QToolButton::clicked, this, &MainWindow::openOrderEcho);
    connect(btnDocExamen, &QAbstractButton::clicked, this, &MainWindow::openDocExamen);
    connect(btnSettings, &QAbstractButton::clicked, this, &MainWindow::openAppSettings);
    connect(btnAbout, &QAbstractButton::clicked, this, &MainWindow::openAbout);
}

void MainWindow::initActions()
{
    QAction* actionDoctors       = new QAction(QIcon(":img/doctor_x32.png"), tr("Doctori"), this);
    QAction* actionNurses        = new QAction(QIcon(":img/nurse_x32.png"), tr("As.medicale"), this);
    QAction* actionPacients      = new QAction(QIcon(":img/pacient_x32.png"), tr("Pacienți"), this);
    QAction* actionOrganizations = new QAction(QIcon(":img/company_x32.png"), tr("Persoane juridice"), this);
    QAction* actionUsers         = new QAction(QIcon(":img/user_x32.png"), tr("Utilizatori"), this);
    QAction* actionAppSettings   = new QAction(QIcon(":img/settings_x32.png"), tr("Setările aplicației"), this);
    QAction* actionUserSettings  = new QAction(QIcon(":img/settings_x32.png"), tr("Setările utilizatorilor"), this);
    QAction* actionOpenPricing  = new QAction(QIcon(":/img/price_x32.png"), tr("Formarea prețurilor"), this);
    QAction* actionOpenAppointments  = new QAction(QIcon(":/img/registration_patients.png"), tr("Programarea pacienților"), this);
    QAction* actionOpenOrderEcho  = new QAction(QIcon(":/img/orderEcho_x32.png"), tr("Comanda ecografică"), this);
    QAction* actionOpenReportEcho  = new QAction(QIcon(":/img/examenEcho.png"), tr("Raport ecografic"), this);
    QAction* actionOpenDesigner   = new QAction(QIcon("://images/logo1.png"), tr("LimeReport (designer)"), this);

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

    QAction* actionTypesPrice = new QAction(QIcon(":/img/typesPrices_x32.png"), tr("Tipul prețurilor"), this);
    QAction* actionInvestigations = new QAction(QIcon(":/img/investigations_x32.png"), tr("Investigații"), this);
    clasifiers->addAction(actionInvestigations);
    clasifiers->addAction(actionTypesPrice);
    connect(actionInvestigations, &QAction::triggered, this, &MainWindow::openCatInvestigations);
    connect(actionTypesPrice, &QAction::triggered, this, &MainWindow::openCatTypesPrices);

    ui->menuDocumente->addAction(actionOpenPricing);
    ui->menuDocumente->addSeparator();
    ui->menuDocumente->addAction(actionOpenAppointments);
    ui->menuDocumente->addAction(actionOpenOrderEcho);
    ui->menuDocumente->addAction(actionOpenReportEcho);

    connect(actionOpenDesigner, &QAction::triggered, this, &MainWindow::onOpenLMDesigner);
    ui->menuRapoarte->addAction(actionOpenDesigner);

    ui->menuService->addAction(actionAppSettings);
    ui->menuService->addAction(actionUserSettings);

    QAction* actionSourceCode = new QAction(QIcon(":/img/github.png"), tr("Cod sursă"), this);
    ui->menuAssistance->addAction(actionSourceCode);
    connect(actionSourceCode, &QAction::triggered, this, &MainWindow::openSourceCode);

    QAction* action_user_manual = new QAction(QIcon(":/img/help_question.png"), tr("Manual Online"), this);
    ui->menuAssistance->addAction(action_user_manual);
    connect(action_user_manual, &QAction::triggered, this, &MainWindow::openUserManual);

    QAction* action_open_history = new QAction(QIcon(":/img/history.png"), tr("Istoria versiunilor"), this);
    ui->menuAssistance->addAction(action_open_history);
    connect(action_open_history, &QAction::triggered, this, &MainWindow::openHistoryVersion);

    QAction* actionCheckUpdate = new QAction(QIcon(":/img/update_app.png"), tr("Verifică versiunea nouă"), this);
    ui->menuAssistance->addAction(actionCheckUpdate);
    connect(actionCheckUpdate, &QAction::triggered, this, &MainWindow::checkUpdateApp);

    QAction* action_about = new QAction(QIcon(":/img/info_x32.png"), tr("Despre aplicația"), this);
    ui->menuAssistance->addSeparator();
    ui->menuAssistance->addAction(action_about);
    connect(action_about, &QAction::triggered, this, &MainWindow::openAbout);
}

void MainWindow::checkUpdateApp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG/releases"));
}

void MainWindow::openSourceCode()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG"));
}

void MainWindow::openUserManual()
{
    QDesktopServices::openUrl(QUrl("https://github.com/debalex77/USG/wiki/User-Manual-(ro)"));
}

void MainWindow::openHistoryVersion()
{
    history_version = new HistoryVersion(this);
    history_version->setAttribute(Qt::WA_DeleteOnClose);
    history_version->show();
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
}

QString MainWindow::getVersionAppInTableSettingsUsers()
{
    QString version_app;
    QSqlQuery qry;
    if (! globals::mySQLhost.isEmpty())
        qry.prepare("SELECT versionApp FROM settingsUsers WHERE id_users = :id_users AND versionApp IS NOT Null;");
    else
        qry.prepare("SELECT versionApp FROM settingsUsers WHERE id_users = :id_users AND versionApp NOT NULL;");
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
        qry.prepare("UPDATE settingsUsers SET versionApp = :versionApp WHERE id_users = :id_users AND versionApp IS NOT Null;");
    else
        qry.prepare("UPDATE settingsUsers SET versionApp = :versionApp WHERE id_users = :id_users AND versionApp NOT NULL;");
    qry.bindValue(":versionApp", APPLICATION_VERSION);
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

void MainWindow::updateTimer()
{
    if (this->isVisible()){
        if (pause_timer < 1){  // pauza in 2-3 secunde
            pause_timer += 1;
            return;
        }
        timer->stop(); // oprim timerul

        if (globals::thisMySQL)
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (.sqlite3): utilizator (") + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(": utilizator (") + globals::nameUserApp + ")");

        if (globals::firstLaunch){
            textEdit_dockWidget->setHtml(tr("%1   %2: Se completează catalogul <b><u>'Investigații'</u></b>.")
                                         .arg(db->getHTMLImageInfo(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")));
            textEdit_dockWidget->setFixedHeight(70);
            dock_widget->show();
            timer_doc_widget->start(1000);
        }

        // Verificam daca este schimbata versiunea aplicatiei
        QString version_app = getVersionAppInTableSettingsUsers();

        if (version_app != APPLICATION_VERSION){
            // prezentam informatia de actualizarea in panoul informativ
            textEdit_dockWidget->clear();
            textEdit_dockWidget->setHtml(tr("%1   Aplicația a fost actualizată până la versiunea: USG v" APPLICATION_VERSION).arg(db->getHTMLImageInfo()));
            textEdit_dockWidget->setFixedHeight(70);
            dock_widget->show();
            if (update_app->execUpdateCurrentRelease(APPLICATION_VERSION)){
                setVersionAppInTableSettingsUsers();
                openHistoryVersion();
            }
        }
        if (globals::showHistoryVersion && version_app == APPLICATION_VERSION)
            openHistoryVersion();
        if (globals::showUserManual){
            QDir dir;
            QString str = dir.toNativeSeparators(dir.currentPath() + "/UserManual.pdf");
            QDesktopServices::openUrl(QUrl::fromLocalFile(str));
        }
    }
}

void MainWindow::updateTimerDocWidget()
{
    if (dock_widget->isVisible()){
        timer_doc_widget->stop();

        globals::thisMySQL = (globals::sqlitePathBase.isEmpty());
        globals::thisSqlite = (globals::mySQLhost.isEmpty());
        if (globals::thisMySQL & globals::thisSqlite || !globals::thisMySQL & !globals::thisSqlite)
            QMessageBox::critical(this, "Atentie", "Nu sunt determinate variabile globale !!!", QMessageBox::Ok);

        db->loadInvestigationFromXml();
        db->insertDataForTabletypesPrices(); //completarea preturilor

        textEdit_dockWidget->setHtml(tr("%1<br>%2 %3 Finisarea completării catalogului <b><u>'Investigații'</u></b> sursa - <b>Catalogul tarifelor unice (anexa nr.3)</b>.<br>"
                                        "     <b>Vezi:</b> <em>Meniu principal al aplicației -> Cataloage -> Clasificatori -> Investigații<em>")
                                     .arg(textEdit_dockWidget->toHtml(), db->getHTMLImageInfo(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")));
        QDir dir;
        QString str = dir.toNativeSeparators(dir.currentPath() + "/UserManual.pdf");
        QDesktopServices::openUrl(QUrl::fromLocalFile(str));

        // Lansarea - User Manual
        globals::firstLaunch = false;
    }
}

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

void MainWindow::openCatTypesPrices()
{
    cat_investigations = new CatForSqlTableModel(this);
    cat_investigations->setAttribute(Qt::WA_DeleteOnClose);
    cat_investigations->setWindowIcon(QIcon(":/img/typesPrices_x32.png"));
    cat_investigations->setProperty("typeCatalog", cat_investigations->TypesPrices);
    cat_investigations->setProperty("typeForm", cat_investigations->ListForm);
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
    userSettings = new UserSettings(this);
    userSettings->setAttribute(Qt::WA_DeleteOnClose);
    userSettings->setWindowIcon(QIcon(":img/settings_x32.png"));
    userSettings->setProperty("Id", globals::idUserApp);
    userSettings->show();
}

void MainWindow::openPricing()
{
    listDoc = new ListDoc(this);
    listDoc->setAttribute(Qt::WA_DeleteOnClose);
    listDoc->setWindowIcon(QIcon(":/img/price_x32.png"));
    mdiAreaCont->addWidget(listDoc);
    listDoc->show();
}

void MainWindow::removeSubWindow()
{
    mdiAreaCont->remove(mdiAreaCont->currentIndex());
}

void MainWindow::onOpenLMDesigner()
{
    m_report = new LimeReport::ReportEngine(this);
    m_report->setShowDesignerModal(false);
    m_report->designReport();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (! globals::showQuestionCloseApp){
        closeDatabases();
        qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicația.").arg(globals::nameUserApp);
        event->accept();
        return;
    }

    QMessageBox messange_box(QMessageBox::Question, tr("Finisarea lucrului"), tr("Doriți să inchideți aplicația ?"),
                                                                    QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
    messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//    messange_box.addButton(tr("Da"), QMessageBox::Yes);
//    messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
    if (messange_box.exec() == QMessageBox::Yes){
        closeDatabases();
         qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicația.").arg(globals::nameUserApp);
         event->accept();
     } else {
         event->ignore();
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
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (.sqlite3): base - '%1', utilizator (").arg(globals::sqliteNameBase) + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(": utilizator (") + globals::nameUserApp + ")");
        updateTextBtn();
    } else if (event->type() == QEvent::WindowTitleChange){
        if (globals::thisMySQL)
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (MySQL: %1@%2): utilizator (").arg(globals::mySQLnameBase, globals::mySQLhost) +
                           globals::nameUserApp + ")");
        else if (globals::thisSqlite)
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(" (.sqlite3): base - '%1', utilizator (").arg(globals::sqliteNameBase) + globals::nameUserApp + ")");
        else
            setWindowTitle(APPLICATION_NAME + " v." + APPLICATION_VERSION + tr(": utilizator (") + globals::nameUserApp + ")");
    }
}

