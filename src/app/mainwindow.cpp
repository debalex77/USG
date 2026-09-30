/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlQuery"
#include "QSqlError"
#include <QDirIterator>
#include <QDockWidget>
#include <QFontDatabase>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVersionNumber>
#include <app/firstrunwizard.h>
#include <common/cloudconnectioncontext.h>
#include <common/maindatabaseconnectioncontext.h>
#include <ui/dialogs/infowindow.h>
#include <common/sessioncontext.h>
#include <settings/settingsservice.h>
#include <settings/settingsdialog.h>

MainWindow::MainWindow(DataBase &db, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mdiArea(new QMdiArea(this))
    , mdiAreaCont(new MdiAreaContainer(mdiArea, this))
    , m_db(db)
    , popUp(new PopUp(this))
    , menu(new QMenu(this))
    , dock_widget(new QDockWidget(this))
    , textEdit_dockWidget(new QTextEdit(this))
    , toolBar(new QToolBar(tr("Bara cu instrumente"), this))
    , update_app(new UpdateReleasesApp(this))
    , downloader_version(new DownloaderVersion(this))
{
    ui->setupUi(this);
    QSettings settings(ORGANIZATION_NAME, USG_VERSION_FULL); // denumirea organizatiei si aplicatiei

    qInfo(logInfo()) << "Se lanseaza fereastra principala a aplicatiei";

    if (SettingsService::instance().user().minimizeToTray)
        initMinimizeAppToTray();

    connect(&SettingsService::instance(), &SettingsService::userChanged,
            this, [this](const Settings::UserPreferencesData &) {
                applyTrayPreference();
            });

    mdiArea->setViewMode(QMdiArea::TabbedView);        // proprietatea de prezentarea sub-windows
    mdiArea->setTabsClosable(true);                    // proprietatea de inchidere a sub-windows
    setCentralWidget(mdiArea);                         // instalam ca widgetul central

    dock_widget->setWidget(textEdit_dockWidget);
    dock_widget->minimumSizeHint();
    addDockWidget(Qt::BottomDockWidgetArea, dock_widget);
    dock_widget->hide();

    txt_title_bar = new QLabel(ui->statusbar);
    progress = new QProgressBar(ui->statusbar);
    progress->setObjectName(QStringLiteral("statusMigrationProgress"));
    progress->setAlignment(Qt::AlignCenter);
    progress->setTextVisible(true);
    progress->setFormat(QStringLiteral("%p%"));
    progress->setMinimumSize(110, 16);
    progress->setMaximumSize(110, 16);
    progress->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    progress->setRange(0, 100);
    progress->hide();
    txt_title_bar->hide();
    // Ordinea vizuală: bara fixă în stânga, apoi mesajul care folosește
    // spațiul rămas până la marginea dreaptă.
    statusBar()->addWidget(progress);
    statusBar()->addWidget(txt_title_bar, 1);

    connect(update_app, &UpdateReleasesApp::migrationProgress,
            this, [this](int value, int maximum, const QString &message) {
                progress->setRange(0, maximum);
                progress->setValue(value);
                progress->setVisible(true);
                progress->raise();
                appendMigrationMessage(message);
                txt_title_bar->setText(message);
                txt_title_bar->show();
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });

    initButton();
    initActions();
    updateTextBtn();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// **********************************************************************************
// --- initierea connectarilor si prezentarea textului informativ

void MainWindow::mDockWidgetShowTex(const QString txtMsg)
{
    textEdit_dockWidget->setHtml(txtMsg);
    textEdit_dockWidget->setFixedHeight(100);
    dock_widget->show();
}

void MainWindow::initButton()
{
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

    btnDoctors->setIcon(QIcon(":/img/catalogs/doctor.png"));
    btnNurses->setIcon(QIcon(":/img/catalogs/nurse.png"));
    btnPacients->setIcon(QIcon(":/img/catalogs/pacient.png"));
    btnHistoryPatient->setIcon(QIcon(":/img/documents/medical_history.png"));
    btnHistoryPatient->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnUsers->setIcon(QIcon(":/img/catalogs/user.png"));
    btnPatientAppointments->setIcon(QIcon(":/img/documents/appointment_pacients.png"));
    btnOrderEcho->setIcon(QIcon(":/img/documents/orderEcho.png"));
    btnDocExamen->setIcon(QIcon(":/img/documents/reportEcho.png"));
    btnOrganizations->setIcon(QIcon(":/img/catalogs/company.png"));
    btnInvestigations->setIcon(QIcon(":/img/catalogs/investigations.png"));
    btnReports->setIcon(QIcon(":/img/documents/reports.png"));
    btnPricing->setIcon(QIcon(":/img/documents/pricing.png"));
    btnSettings->setIcon(QIcon(":/img/catalogs/settings.png"));
    btnAbout->setIcon(QIcon(":/img/common/info.png"));
    btnBlock->setIcon(QIcon(":/img/common/lock.png"));

    QList<QToolButton*> buttons = toolBar->findChildren<QToolButton*>();
    for (QToolButton *btn : std::as_const(buttons)) {
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setCursor(Qt::ArrowCursor);
#if defined(Q_OS_LINUX)
        btn->setStyleSheet("font-size: 11pt;");
#elif defined(Q_OS_MACOS)
        btn->setStyleSheet("font-size: 13pt;");
#endif
        btn->installEventFilter(this);
        btn->setMouseTracking(true);
    }

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

    connect(btnDoctors, &QAbstractButton::clicked,
            this, &MainWindow::openCatalogDoctors, Qt::UniqueConnection);
    connect(btnNurses, &QAbstractButton::clicked,
            this, &MainWindow::openCatalogNurses, Qt::UniqueConnection);
    connect(btnPacients, &QAbstractButton::clicked,
            this, &MainWindow::openCatalogPacients, Qt::UniqueConnection);
    connect(btnUsers, &QAbstractButton::clicked,
            this, &MainWindow::openCatalogUsers, Qt::UniqueConnection);
    connect(btnOrganizations, &QAbstractButton::clicked,
            this, &MainWindow::openCatalogOrganizations, Qt::UniqueConnection);

    connect(btnHistoryPatient, &QAbstractButton::clicked,
            this, &MainWindow::openHistoryPatient, Qt::UniqueConnection);
    connect(btnReports, &QAbstractButton::clicked,
            this, &MainWindow::openReports, Qt::UniqueConnection);
    connect(btnInvestigations, &QToolButton::clicked,
            this, &MainWindow::openInvestigations, Qt::UniqueConnection);
    connect(btnPricing, &QAbstractButton::clicked,
            this, &MainWindow::openPricing, Qt::UniqueConnection);
    connect(btnPatientAppointments, &QToolButton::clicked,
            this, &MainWindow::openPatientAppointments, Qt::UniqueConnection);
    connect(btnOrderEcho, &QToolButton::clicked,
            this, &MainWindow::openOrderView, Qt::UniqueConnection);
    connect(btnDocExamen, &QAbstractButton::clicked,
            this, &MainWindow::openDocExamen, Qt::UniqueConnection);
    connect(btnSettings, &QAbstractButton::clicked,
            this, &MainWindow::openAppSettings, Qt::UniqueConnection);
    connect(btnAbout, &QAbstractButton::clicked,
            this, &MainWindow::openAbout, Qt::UniqueConnection);
    connect(btnBlock, &QAbstractButton::clicked,
            this, &MainWindow::onBlockApp, Qt::UniqueConnection);
}

void MainWindow::initActions()
{
    //---------------------------------------------------------
    // 1. Cataloage
    //---------------------------------------------------------
    QAction *actionDoctors       = new QAction(QIcon(":/img/catalogs/doctor.png"), tr("Doctori"), this);
    QAction *actionNurses        = new QAction(QIcon(":/img/catalogs/nurse.png"), tr("As.medicale"), this);
    QAction *actionPacients      = new QAction(QIcon(":/img/catalogs/pacient.png"), tr("Pacienți"), this);
    QAction *actionOrganizations = new QAction(QIcon(":/img/catalogs/company.png"), tr("Persoane juridice"), this);
    QAction *actionUsers         = new QAction(QIcon(":/img/catalogs/user.png"), tr("Utilizatori"), this);
    // -- submeniu 'Clasificatori'
    QAction *actionInvestigations      = new QAction(QIcon(":/img/catalogs/investigations.png"), tr("Investigații"), this);
    QAction *actionGroupInvestigations = new QAction(QIcon(":/img/catalogs/tree_yellow.png"), tr("Arbore investigațiilor"), this);
    QAction *actionTypesPrice          = new QAction(QIcon(":/img/catalogs/types_prices.png"), tr("Tipul prețurilor"), this);
    QAction *actionConcluzionTemplets  = new QAction(QIcon(":/img/catalogs/templates.png"), tr("Șabloane concluziilor"), this);
    // --
    QAction *actionOpenNormograms   = new QAction(QIcon("://img/normograma.png"), tr("Normograme"), this);

    // ----- conectarea
    connect(actionDoctors, &QAction::triggered,
            this, &MainWindow::openCatalogDoctors, Qt::UniqueConnection);
    connect(actionNurses, &QAction::triggered,
            this, &MainWindow::openCatalogNurses, Qt::UniqueConnection);
    connect(actionPacients, &QAction::triggered,
            this, &MainWindow::openCatalogPacients, Qt::UniqueConnection);
    connect(actionOrganizations, &QAction::triggered,
            this, &MainWindow::openCatalogOrganizations, Qt::UniqueConnection);
    connect(actionUsers, &QAction::triggered,
            this, &MainWindow::openCatalogUsers, Qt::UniqueConnection);

    connect(actionInvestigations, &QAction::triggered,
            this, &MainWindow::openInvestigations, Qt::UniqueConnection);
    connect(actionGroupInvestigations, &QAction::triggered,
            this, &MainWindow::openGroupInvestigation, Qt::UniqueConnection);
    connect(actionTypesPrice, &QAction::triggered,
            this, &MainWindow::openTypesPrices, Qt::UniqueConnection);
    connect(actionConcluzionTemplets, &QAction::triggered,
            this, &MainWindow::openConcluzionTemplets, Qt::UniqueConnection);

    connect(actionOpenNormograms, &QAction::triggered,
            this, &MainWindow::openNormograms, Qt::UniqueConnection);

    // ----- crearea meniului
    ui->menuCataloage->addAction(actionDoctors);
    ui->menuCataloage->addAction(actionNurses);
    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionPacients);
    ui->menuCataloage->addAction(actionOrganizations);
    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionUsers);

    QMenu *clasifiers = new QMenu(ui->menuCataloage);
    clasifiers->setTitle(tr("Clasificatori"));
    clasifiers->addAction(actionInvestigations);
    clasifiers->addAction(actionGroupInvestigations);
    clasifiers->addSeparator();
    clasifiers->addAction(actionTypesPrice);
    clasifiers->addSeparator();
    clasifiers->addAction(actionConcluzionTemplets);

    ui->menuCataloage->addMenu(clasifiers);
    ui->menuCataloage->addSeparator();
    ui->menuCataloage->addAction(actionOpenNormograms);

    //---------------------------------------------------------
    // 2. Documente
    //---------------------------------------------------------
    QAction *actionOpenPricing      = new QAction(QIcon(":/img/documents/pricing.png"), tr("Formarea prețurilor"), this);
    QAction *actionOpenAppointments = new QAction(QIcon(":/img/documents/appointment_pacients.png"), tr("Programarea pacienților"), this);
    QAction *actionOpenOrderEcho    = new QAction(QIcon(":/img/documents/orderEcho.png"), tr("Comanda ecografică"), this);
    QAction *actionOpenReportEcho   = new QAction(QIcon(":/img/documents/reportEcho.png"), tr("Raport ecografic"), this);

    // ----- conectarea
    connect(actionOpenPricing, &QAction::triggered,
            this, &MainWindow::openPricing, Qt::UniqueConnection);
    connect(actionOpenAppointments, &QAction::triggered,
            this, &MainWindow::openPatientAppointments, Qt::UniqueConnection);
    connect(actionOpenOrderEcho, &QAction::triggered,
            this, &MainWindow::openOrderView, Qt::UniqueConnection);
    connect(actionOpenReportEcho, &QAction::triggered,
            this, &MainWindow::openDocExamen, Qt::UniqueConnection);

    // ----- crearea meniului
    ui->menuDocumente->addAction(actionOpenPricing);
    ui->menuDocumente->addSeparator();
    ui->menuDocumente->addAction(actionOpenAppointments);
    ui->menuDocumente->addAction(actionOpenOrderEcho);
    ui->menuDocumente->addAction(actionOpenReportEcho);

    //---------------------------------------------------------
    // 3. Rapoarte
    //---------------------------------------------------------
    QAction *actionOpenDesigner = new QAction(QIcon("://images/logo1.png"), tr("LimeReport (designer)"), this);

    // ----- conectarea
    connect(actionOpenDesigner, &QAction::triggered,
            this, &MainWindow::onOpenLMDesigner, Qt::UniqueConnection);

    // ----- crearea meniului
    ui->menuRapoarte->addAction(actionOpenDesigner);

    //---------------------------------------------------------
    // 4. Setari
    //---------------------------------------------------------
    QAction *actionAppSettings      = new QAction(QIcon(":img/settings_x32.png"), tr("Setările aplicației"), this);
    QAction *actionUserSettings     = new QAction(QIcon(":/img/catalogs/user_preferences.png"), tr("Preferințele utilizatorului"), this);
    QAction *actionOnlineAccount    = new QAction(QIcon(":/img/toolbar/email.png"), tr("Cont online"), this);
    QAction *actionCloudServer      = new QAction(QIcon(":/img/database/cloud-server.png"), tr("Configurarea cloud server"), this);
    QAction *actionFirstWizard      = new QAction(QIcon(":/img/catalogs/firstWizard.png"), tr("Asistent primei lansari"), this);
    QAction *actionCreationArchive  = new QAction(QIcon(":/img/common/archive.png"), tr("Creeaza arhiva"), this);

    // ----- conectarea
    connect(actionAppSettings, &QAction::triggered,
            this, &MainWindow::openAppSettings, Qt::UniqueConnection);
    connect(actionUserSettings, &QAction::triggered,
            this, &MainWindow::openUserSettings, Qt::UniqueConnection);
    connect(actionOnlineAccount, &QAction::triggered,
            this, &MainWindow::openOnlineAccountView, Qt::UniqueConnection);
    connect(actionCloudServer, &QAction::triggered,
            this, &MainWindow::openCloudServerView, Qt::UniqueConnection);
    connect(actionFirstWizard, &QAction::triggered,
            this, &MainWindow::openFirstRunWizard, Qt::UniqueConnection);
    connect(actionCreationArchive, &QAction::triggered,
            this, &MainWindow::openArchiveHandler, Qt::UniqueConnection);

    // ----- crearea meniului
    ui->menuService->addAction(actionAppSettings);
    ui->menuService->addAction(actionUserSettings);
    ui->menuService->addSeparator();
    ui->menuService->addAction(actionOnlineAccount);
    ui->menuService->addSeparator();
    ui->menuService->addAction(actionCloudServer);
    ui->menuService->addSeparator();
    ui->menuService->addAction(actionFirstWizard);
    ui->menuService->addSeparator();
    ui->menuService->addAction(actionCreationArchive);

    //---------------------------------------------------------
    // 5. Asistenta
    //---------------------------------------------------------
    QAction *actionSourceCode      = new QAction(QIcon(":/img/common/github.png"), tr("Cod sursă"), this);
    QAction *actionReportBug       = new QAction(QIcon(":/img/common/bug.png"), tr("Raportează eroare"), this);
    QAction *action_user_manual    = new QAction(QIcon(":/img/common/help.png"), tr("Manual Online"), this);
    QAction *actionOpenReleases    = new QAction(QIcon(":/img/documents/history.png"), tr("Istoria versiunilor"), this);
    QAction *actionCheckUpdate     = new QAction(QIcon(":/img/actions/update_app.png"), tr("Verifică versiunea nouă"), this);
    QAction *actionShowAsistantTip = new QAction(QIcon(":/img/oxygen/ktip.png"), tr("Prezentarea asistentului de sfaturi"), this);
    QAction *action_about          = new QAction(QIcon(":/img/common/info.png"), tr("Despre aplicația"), this);

    // ----- conectarea
    connect(actionSourceCode, &QAction::triggered,
            this, &MainWindow::openSourceCode, Qt::UniqueConnection);
    connect(actionReportBug, &QAction::triggered,
            this, &MainWindow::openReportBug, Qt::UniqueConnection);
    connect(action_user_manual, &QAction::triggered,
            this, &MainWindow::openUserManual, Qt::UniqueConnection);
    connect(actionOpenReleases, &QAction::triggered,
            this, &MainWindow::openDescriptionRealease, Qt::UniqueConnection);
    connect(actionCheckUpdate, &QAction::triggered,
            this, &MainWindow::checkUpdateApp, Qt::UniqueConnection);
    connect(actionShowAsistantTip, &QAction::triggered,
            this, &MainWindow::onShowAsistantTip, Qt::UniqueConnection);
    connect(action_about, &QAction::triggered,
            this, &MainWindow::openAbout, Qt::UniqueConnection);

    // ----- crearea meniului
    ui->menuAssistance->addAction(actionSourceCode);
    ui->menuAssistance->addAction(actionReportBug);
    ui->menuAssistance->addSeparator();
    ui->menuAssistance->addAction(action_user_manual);
    ui->menuAssistance->addAction(actionOpenReleases);
    ui->menuAssistance->addAction(actionCheckUpdate);
    ui->menuAssistance->addAction(actionShowAsistantTip);
    ui->menuAssistance->addSeparator();
    ui->menuAssistance->addAction(action_about);
}

// **********************************************************************************
// --- procesarea actiunilor

void MainWindow::checkUpdateApp()
{
    connect(downloader_version, &DownloaderVersion::onReady,
            this, &MainWindow::onReadyVersion, Qt::UniqueConnection);
    // Conectarea este instalată înaintea solicitării, inclusiv pentru un
    // răspuns foarte rapid furnizat din cache.
    downloader_version->getData();
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
// --- determinarea/setarea versiunii schemei bazei de date

QString MainWindow::databaseSchemaVersion()
{
    QString error;
    const QString version = m_db.databaseSchemaVersion(&error);
    if (version.isEmpty())
        qCritical(logCritical())
            << tr("Nu s-a putut determina versiunea schemei bazei de date:")
            << error;
    return version;
}

bool MainWindow::setDatabaseSchemaVersion(const QString &version)
{
    QString error;
    if (m_db.setDatabaseSchemaVersion(version, &error))
        return true;

    qCritical(logCritical())
        << tr("Nu s-a putut salva versiunea schemei bazei de date:")
        << error;
    return false;
}

void MainWindow::closeDatabases()
{
    QSqlDatabase database = m_db.getDatabase();
    if (database.isValid() && database.isOpen())
        database.close();

    QSqlDatabase imageDatabase = m_db.getDatabaseImage();
    if (imageDatabase.isValid() && imageDatabase.isOpen())
        imageDatabase.close();
}

bool MainWindow::createAutomaticSqliteArchive()
{
    if (!SettingsService::instance().user().archiveSqliteOnExit
        || !MainDatabaseConnectionContext::instance().isSqlite()) {
        return true;
    }

#if defined(Q_OS_WIN)
    const QStringList executableNames{QStringLiteral("7zz.exe"),
                                      QStringLiteral("7za.exe"),
                                      QStringLiteral("7z.exe")};
#else
    const QStringList executableNames{QStringLiteral("7zz"),
                                      QStringLiteral("7za"),
                                      QStringLiteral("7z")};
#endif
    QString sevenZip;
    for (const QString &name : executableNames) {
        sevenZip = QStandardPaths::findExecutable(name);
        if (!sevenZip.isEmpty())
            break;
    }
    if (sevenZip.isEmpty()) {
        qCritical(logCritical())
            << tr("Arhivarea automată nu poate fi efectuată: 7-Zip nu este instalat.");
        return false;
    }

    const MainDatabaseConnectionData connection =
        MainDatabaseConnectionContext::instance().data();
    QStringList sourceFiles;
    for (const QString &path : {connection.sqliteDatabasePath,
                                connection.imageDatabasePath}) {
        if (!path.trimmed().isEmpty() && QFileInfo::exists(path))
            sourceFiles.append(QDir::cleanPath(path));
    }
    if (sourceFiles.isEmpty()) {
        qCritical(logCritical())
            << tr("Arhivarea automată nu poate fi efectuată: fișierele SQLite lipsesc.");
        return false;
    }

    const QString archiveDirectory =
        QDir(QDir::homePath()).filePath(QStringLiteral("Database_usg"));
    if (!QDir().mkpath(archiveDirectory)) {
        qCritical(logCritical())
            << tr("Directorul arhivei nu poate fi creat:") << archiveDirectory;
        return false;
    }

    QString databaseName = connection.sqliteDatabaseName.trimmed();
    if (databaseName.isEmpty())
        databaseName = QFileInfo(connection.sqliteDatabasePath).completeBaseName();
    databaseName.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]+")),
                         QStringLiteral("_"));
    const QString archivePath = QDir(archiveDirectory).filePath(
        QStringLiteral("%1_%2.7z")
            .arg(databaseName,
                 QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))));

    QStringList arguments{QStringLiteral("a"), QStringLiteral("-t7z"), archivePath,
                          QStringLiteral("-mx=9"), QStringLiteral("-mmt=on"),
                          QStringLiteral("-ms=on"), QStringLiteral("-bso0"),
                          QStringLiteral("-bsp0"), QStringLiteral("-bd")};
    arguments.append(sourceFiles);
    const int exitCode = QProcess::execute(sevenZip, arguments);
    if (exitCode != 0) {
        qCritical(logCritical())
            << tr("Arhivarea automată a eșuat. Cod proces:") << exitCode;
        return false;
    }

    qInfo(logInfo()) << tr("Arhiva automată SQLite a fost creată:") << archivePath;
    return true;
}

/*!
 * \brief Functia de inchidere a subWindows + salvarea automata a setarilor ferestrei,
 * sectiilor etc.
 * În ferestrele de listă și rapoarte apelează evenimentul closeEvent
 * in care se petrece salvarea setarilor in fisierul extern .json
 */
bool MainWindow::closeAndSaveSettingsSubwindows()
{
    const auto subWindows = mdiArea->subWindowList();
    for (QMdiSubWindow *subWindow : subWindows) {
        if (subWindow && !subWindow->close())
            return false;
    }
    return mdiArea->subWindowList().isEmpty();
}

// **********************************************************************************
// --- actualizarea la lansare aplicatiei

bool MainWindow::completeStartup()
{
    updateWindowTitle();

    // Întâi aducem schema la versiunea curentă. MainWindow este afișată de
    // AppController doar pentru progres, iar bucla normală de evenimente nu a
    // început încă, deci utilizatorul nu poate deschide documente pe schema veche.
    const QString version_app = databaseSchemaVersion();
    if (version_app.trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Actualizarea bazei de date"),
                              tr("Versiunea schemei bazei de date nu a putut fi determinată."));
        return false;
    }
    confirmedDatabaseVersion = version_app.trimmed();
    updateWindowTitle();

        // verificam daca versiunea aplicatiei e actuala
    if (version_app != VERSION_FULL){
            bool migrationCredentialsReady = true;
            const QVersionNumber databaseVersion =
                QVersionNumber::fromString(version_app);

            // De la 4.0.1, UUID-urile SQLite trebuie transferate în baza cloud.
            // Dacă există configurația, dar parola veche nu mai poate fi
            // decriptată cu cheia actuală, cerem resalvarea ei înaintea migrării.
            if (MainDatabaseConnectionContext::instance().isSqlite()
                && !databaseVersion.isNull()
                && databaseVersion < QVersionNumber(4, 0, 1)
                && CloudConnectionContext::instance().data().configured
                && CloudConnectionContext::instance().data().password.isEmpty()) {
                progress->hide();
                txt_title_bar->setText(
                    tr("Pentru actualizare este necesară resalvarea parolei cloud."));
                txt_title_bar->show();

                CloudServerConfig cloudServer(this);
                cloudServer.setProperty("ID_user", SessionContext::instance().userId());
                cloudServer.setProperty("ID_Organization",
                    SettingsService::instance().organization().organizationId);
                migrationCredentialsReady =
                    cloudServer.exec() == QDialog::Accepted
                    && !CloudConnectionContext::instance().data().password.isEmpty();
            }

            txt_title_bar->setText(tr("Se actualizează baza de date de la versiunea %1 la %2...")
                                       .arg(version_app, VERSION_FULL));
            txt_title_bar->show();
            appendMigrationMessage(txt_title_bar->text());
            progress->setRange(0, 0);
            progress->show();
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

            const bool migrationSucceeded = migrationCredentialsReady
                && update_app->execUpdateCurrentRelease(version_app)
                && setDatabaseSchemaVersion(QStringLiteral(VERSION_FULL));
            if (migrationSucceeded) {
                confirmedDatabaseVersion = VERSION_FULL;
                updateWindowTitle();
                progress->setRange(0, 100);
                progress->setValue(100);
                txt_title_bar->setText(tr("Actualizarea bazei de date la versiunea %1 s-a finalizat.")
                                           .arg(VERSION_FULL));
                appendMigrationMessage(txt_title_bar->text());
                openDescriptionRealease();
                // prezentam informatia de actualizarea in panoul informativ
                // textEdit_dockWidget->clear();
                textEdit_dockWidget->append(tr("%1  Aplicația a fost actualizată până la versiunea: USG v" VERSION_FULL)
                                                 .arg(m_db.getHTMLImageInfo()));
                if (MainDatabaseConnectionContext::instance().isSqlite()
                    && databaseVersion < QVersionNumber(4, 0, 1)
                    && QVersionNumber(VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE)
                           >= QVersionNumber(4, 0, 1)) {
                    textEdit_dockWidget->append(
                        tr("%1  Pentru o sincronizare corectă, este necesar să actualizați "
                           "și baza de date MariaDB/MySQL la versiunea 4.0.1 sau mai nouă.")
                            .arg(m_db.getHTMLImageInfo()));
                }
                textEdit_dockWidget->setFixedHeight(220);
                dock_widget->show();
                // Păstrăm rezultatul suficient pentru a putea fi observat și
                // când ultimele etape ale migrării se execută rapid.
                QTimer::singleShot(10000, progress, &QWidget::hide);
            } else {
                progress->setRange(0, 100);
                progress->setValue(0);
                progress->hide();
                txt_title_bar->setText(
                    migrationCredentialsReady
                        ? tr("Actualizarea bazei de date a eșuat. Verificați jurnalul aplicației.")
                        : tr("Actualizarea a fost amânată: parola cloud nu a fost resalvată."));
                appendMigrationMessage(txt_title_bar->text());
                if (migrationCredentialsReady)
                    appendMigrationMessage(tr("Actualizarea nu este confirmată integral. Etapele din tranzacțiile anulate nu au fost păstrate."));
                QMessageBox::critical(
                    this, tr("Actualizarea bazei de date"), txt_title_bar->text());
                return false;
            }
    }

    // Bazele MariaDB istorice pot avea reportVideo fără legătura cu raportul.
    // Verificarea rulează și pentru bazele marcate deja 4.2.0.
    if (!update_app->ensureReportVideoSchema()) {
        qCritical(logCritical())
            << tr("Verificarea schemei video a raportului a eșuat.");
        QMessageBox::critical(this, tr("Verificarea bazei de date"),
                              tr("Tabela video a rapoartelor nu a putut fi actualizată. "
                                 "Verificați jurnalul aplicației."));
        return false;
    }

    // Repară bazele deja marcate de o versiune intermediară a migrării.
    if (!update_app->ensureRequiredViews()) {
        qCritical(logCritical())
            << tr("Verificarea view-urilor obligatorii ale aplicației a eșuat.");
        QMessageBox::critical(this, tr("Verificarea bazei de date"),
                              tr("View-urile obligatorii nu au putut fi verificate."));
        return false;
    }

    // Asistentul folosește numai schema curentă.
    if (globals().firstLaunch){
        textEdit_dockWidget->append(tr("%1  S-a depistat lansarea primara a aplicatiei.")
                                     .arg(m_db.getHTMLImageInfo()));
        textEdit_dockWidget->setFixedHeight(100);
        dock_widget->show();
        launchFirstRunWizard();
    }

    if (SettingsService::instance().application().checkForUpdatesOnStartup)
        checkUpdateApp();

    if (SettingsService::instance().application().showUserManualOnStartup)
        openUserManual();

    if (SettingsService::instance().application().showAssistantOnStartup)
        onShowAsistantTip();

    return true;
}

void MainWindow::launchFirstRunWizard()
{
    FirstRunWizard firstRunWizard(m_db, this);
    if (firstRunWizard.exec() != QDialog::Accepted) {
        qInfo(logInfo())
            << tr("Configurarea inițială a fost întreruptă și va fi reluată la următoarea lansare.");
        return;
    }

    if (!AppSettings::saveInitialSetupComplete(true)) {
        qCritical(logCritical())
            << tr("Finalizarea configurării inițiale nu a putut fi salvată în profil.");
        QMessageBox::warning(
            this,
            tr("Configurarea inițială"),
            tr("Pașii au fost finalizați, dar starea nu a putut fi salvată. "
               "Asistentul va fi prezentat din nou la următoarea lansare."),
            QMessageBox::Ok);
        return;
    }

    qInfo(logInfo()) << tr("Configurarea inițială s-a finalizat cu succes.");
}

// **********************************************************************************
// --- procesarea slot-urilor

void MainWindow::openAbout()
{
    About* win_about = new About(this);
    win_about->exec();
    delete win_about;
}

void MainWindow::openCatalogDoctors()
{
    const QString key = "CatalogView_Doctors";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogView *cat = new CatalogView(m_db, CatalogType::Type::Doctors);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/doctor.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openCatalogNurses()
{
    const QString key = "CatalogView_Nurses";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogView *cat = new CatalogView(m_db, CatalogType::Type::Nurses);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/nurse.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openCatalogPacients()
{
    const QString key = "CatalogView_Pacients";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogView *cat = new CatalogView(m_db, CatalogType::Type::Patients);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/pacient.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openCatalogUsers()
{
    const QString key = "CatalogView_Users";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogView *cat = new CatalogView(m_db, CatalogType::Type::Users);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/user.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openCatalogOrganizations()
{
    const QString key = "CatalogView_Organizations";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogView *cat = new CatalogView(m_db, CatalogType::Type::Organizations);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/company.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openNormograms()
{
    normograms = new Normograms(this);
    normograms->setAttribute(Qt::WA_DeleteOnClose);
    normograms->show();
}

void MainWindow::openInvestigations()
{
    const QString key = "CatalogTableEditor_Investigations";

    if (mdiAreaCont->activateIfExists(key))
        return;

    // nu transmitem parent din cauza ca e introdus in mdiAreaCont
    CatalogTableEditor *investigation = new CatalogTableEditor(m_db,
                                                               CatalogType::FormType::List,
                                                               CatalogType::Type::Investigations);
    investigation->setObjectName(key);
    investigation->setWindowIcon(QIcon(":/img/catalogs/investigations.png"));
    mdiAreaCont->addWidget(investigation);
}

void MainWindow::openGroupInvestigation()
{
    group_investigation = new GroupInvestigationList(m_db);
    group_investigation->setWindowIcon(QIcon(":/img/catalogs/tree_yellow.png"));
    mdiAreaCont->addWidget(group_investigation);
}

void MainWindow::openConcluzionTemplets()
{
    const QString key = "CatalogTableEditor_ConclusionTemplates";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogTableEditor *cat = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::List,
                                                     CatalogType::Type::ConclusionTemplates);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/templates.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openTypesPrices()
{
    const QString key = "CatalogTableEditor_TypesPrices";

    if (mdiAreaCont->activateIfExists(key))
        return;

    CatalogTableEditor *cat = new CatalogTableEditor(m_db,
                                                     CatalogType::FormType::List,
                                                     CatalogType::Type::TypesPrices);
    cat->setObjectName(key);
    cat->setWindowIcon(QIcon(":/img/catalogs/types_prices.png"));
    mdiAreaCont->addWidget(cat);
}

void MainWindow::openPatientAppointments()
{
    const QString key = QStringLiteral("AppointmentDialog");
    if (mdiAreaCont->activateIfExists(key))
        return;

    registration_patients = new AppointmentDialog(m_db, this);
    registration_patients->setObjectName(key);
    registration_patients->setAttribute(Qt::WA_DeleteOnClose);
    mdiAreaCont->addWidget(registration_patients);
    registration_patients->show();
}

void MainWindow::openOrderView()
{
    const QString key = "OrderView";

    if (mdiAreaCont->activateIfExists(key))
        return;

    OrderView *view = new OrderView(m_db);
    view->setObjectName(key);
    view->setWindowIcon(QIcon(":/img/documents/orderEcho.png"));
    mdiAreaCont->addWidget(view);
}

void MainWindow::openHistoryPatient()
{
    patient_history = new PatientHistory(m_db, this);
    patient_history->setAttribute(Qt::WA_DeleteOnClose);
    patient_history->show();
}

void MainWindow::openDocExamen()
{
    const QString key = QStringLiteral("ReportView");
    if (mdiAreaCont->activateIfExists(key))
        return;

    list_report = new ReportView(m_db);
    list_report->setObjectName(key);
    list_report->setAttribute(Qt::WA_DeleteOnClose);
    list_report->setWindowIcon(QIcon(":/img/documents/reportEcho.png"));
    mdiAreaCont->addWidget(list_report);
}

void MainWindow::openReports()
{
    reports = new Reports(m_db, this);
    reports->setAttribute(Qt::WA_DeleteOnClose);
    reports->loadSettingsReport();
    mdiAreaCont->addWidget(reports);
    reports->show();
}

void MainWindow::openAppSettings()
{
    if (appSett) {
        appSett->showNormal();
        appSett->raise();
        appSett->activateWindow();
        return;
    }

    appSett = new AppSettings(this);
    appSett->setAttribute(Qt::WA_DeleteOnClose);
    appSett->setWindowIcon(QIcon(":/img/catalogs/settings.png"));
    connect(appSett, &AppSettings::restartRequested,
            this, &MainWindow::restartAfterLanguageChange,
            Qt::QueuedConnection);
    appSett->readSettings();
    appSett->show();
}

void MainWindow::restartAfterLanguageChange()
{
    m_restartRequested = true;
    const bool closed = close();
    m_restartRequested = false;
    if (closed) {
        emit restartApproved();
    } else {
        QMessageBox::warning(this,
                             tr("Relansarea aplicației"),
                             tr("Relansarea a fost anulată deoarece o fereastră nu a putut fi închisă."));
    }
}

void MainWindow::openUserSettings()
{
    auto *settingsDialog = new SettingsDialog(m_db, this);
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    settingsDialog->setUserId(SessionContext::instance().userId());
    connect(settingsDialog, &SettingsDialog::settingsApplied,
            this, [this]() {
                const bool enabled =
                    SettingsService::instance().user().printMenuMode
                    == Settings::PrintMenuMode::PreviewAndDesigner;
                const auto orderViews = findChildren<OrderView *>();
                for (OrderView *view : orderViews)
                    view->updatePrintButtons(enabled);
                const auto reportViews = findChildren<ReportView *>();
                for (ReportView *view : reportViews)
                    view->refreshPrintButton();
            });
    settingsDialog->exec();
}

void MainWindow::openPricing()
{
    const QString key = "PricingView";

    if (mdiAreaCont->activateIfExists(key))
        return;

    PricingView *pricingView = new PricingView(m_db);
    pricingView->setObjectName(key);
    pricingView->setWindowIcon(QIcon(":/img/documents/pricing.png"));
    mdiAreaCont->addWidget(pricingView);
}

void MainWindow::removeSubWindow()
{
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

    const QString version_online = QString::fromUtf8(file.readAll()).trimmed();
    qsizetype suffixIndex = 0;
    const QVersionNumber onlineVersion =
        QVersionNumber::fromString(version_online, &suffixIndex);
    const QVersionNumber applicationVersion(VERSION_MAJOR, VERSION_MINOR,
                                             VERSION_RELEASE);

    if (onlineVersion.isNull() || suffixIndex != version_online.size()) {
        qWarning(logWarning()) << tr("Versiunea online nu este validă:")
                               << version_online;
        QFile::remove(path_file_version);
        return;
    }

    if (applicationVersion < onlineVersion){
        textEdit_dockWidget->setHtml(tr("%1   %2: Exist\304\203 o versiune nou\304\203 a aplica\310\233iei <b><u>%3</u></b>.")
                                         .arg(m_db.getHTMLImageInfo(),
                                              QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"),
                                              version_online));
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
        txt_title_bar->setText(tr("Folosiți cea mai recentă versiune \"%1\".")
                                   .arg(USG_VERSION_FULL));
    }

    if (!QFile::remove(path_file_version))
        qWarning(logWarning()) << tr("Fișierul temporar de versiune nu a putut fi eliminat:")
                               << path_file_version;
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
    autorization = new AuthorizationUser(m_db, this);
    if (autorization->exec() == QDialog::Accepted)
        this->show();
}

void MainWindow::openOnlineAccountView()
{
    const QString key = "OnlineAccountView";

    if (mdiAreaCont->activateIfExists(key))
        return;

    OnlineAccountView *view = new OnlineAccountView(m_db);
    view->setObjectName(key);
    view->setWindowIcon(QIcon(":/img/toolBar/email.png"));
    mdiAreaCont->addWidget(view);
}

void MainWindow::openCloudServerView()
{
    const QString key = QStringLiteral("CloudServerView");
    if (mdiAreaCont->activateIfExists(key))
        return;

    auto *view = new CloudServerView(m_db);
    view->setObjectName(key);
    view->setWindowIcon(QIcon(QStringLiteral(":/img/database/cloud-server.png")));
    mdiAreaCont->addWidget(view);
}

void MainWindow::openFirstRunWizard()
{
    FirstRunWizard *first_wizard = new FirstRunWizard(m_db, this);
    connect(first_wizard, &FirstRunWizard::finishLoadClassifier,
            this, &MainWindow::mDockWidgetShowTex, Qt::UniqueConnection);
    first_wizard->exec();
}

void MainWindow::openArchiveHandler()
{
    auto archive_handler = new ArchiveCreationHandler(this);
    archive_handler->exec();
}

// **********************************************************************************
// --- descarcarea fisierului cu aplicatia noua si prezentarea progress bar

void MainWindow::downloadNewVersionApp(const QString str_new_version)
{
    txt_title_bar->setText(tr("Se descarc\304\203 fi\310\231ierul ... "));

    progress->setRange(0, 100);
    progress->setValue(0);
    progress->show();
    txt_title_bar->show();

    connect(&downloader, &Downloader::updateDownloadProgress,
            this, &MainWindow::onUpdateProgress);
    connect(&downloader, &Downloader::finishedDownload,
            this, &MainWindow::onNewAppFinishedDownload);

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
    progress->hide();
    txt_title_bar->setText(tr("Fi\310\231ierul este desc\304\203rcat cu succes."));
}

// **********************************************************************************
// --- minimizarea in tray

void MainWindow::initMinimizeAppToTray()
{
    if (trayIcon)
        return;
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning(logWarning())
            << tr("Minimizarea în tray nu este disponibilă în sesiunea curentă.");
        return;
    }

    /* initierea iconitei in tray */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/img/app_ico/eco_systemTray.png"));
    trayIcon->setToolTip(APPLICATION_NAME + " v" + USG_VERSION_FULL);

    /* meniu */
    QMenu   *menu = new QMenu(this);
    QAction *viewWindow = new QAction(tr("Maximiza\310\233i fereastra"), this);
    QAction *quitAction = new QAction(tr("Ie\310\231ire"), this);

    connect(viewWindow, &QAction::triggered,
            this, &MainWindow::show);
    connect(quitAction, &QAction::triggered,
            this, [this]()
            {
                Settings::UserPreferencesData preferences =
                    SettingsService::instance().user();
                preferences.minimizeToTray = false;
                SettingsService::instance().setUser(preferences);
                this->show();
                this->close();
            });

    menu->addAction(viewWindow);
    menu->addAction(quitAction);

    /* setam meniu contextual */
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::iconActivated);
}

void MainWindow::applyTrayPreference()
{
    if (SettingsService::instance().user().minimizeToTray) {
        initMinimizeAppToTray();
        return;
    }

    if (trayIcon) {
        trayIcon->hide();
        trayIcon->deleteLater();
        trayIcon = nullptr;
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        if (SettingsService::instance().user().minimizeToTray) {
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
    progress->hide();
    txt_title_bar->setText(textTitle);
}

// **********************************************************************************
// --- evenimentele formei

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_restartRequested) {
        if (!closeAndSaveSettingsSubwindows()
            || (appSett && !appSett->close())) {
            event->ignore();
            return;
        }
        closeDatabases();
        if (!createAutomaticSqliteArchive())
            qWarning(logWarning()) << tr("Aplicația se închide fără arhiva automată.");
        qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicația.")
                                .arg(globals().nameUserApp);
        event->accept();
        return;
    }

    if (SettingsService::instance().user().minimizeToTray
        && trayIcon && trayIcon->isVisible() && this->isVisible()) {

        event->ignore();
        this->hide();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);

        trayIcon->showMessage(APPLICATION_NAME,
                              tr("Aplica\310\233ia este minimizat\304\203 în tray. "
                                 "Pentru a maximiza fereastra aplica\310\233iei, "
                                 "face\310\233i clic pe pictograma aplica\310\233iei din tray."),
                              icon,
                              2000);
        return;

    }

    if (!SettingsService::instance().user().confirmOnExit) {
        if (!closeAndSaveSettingsSubwindows()) {
            event->ignore();
            return;
        }
        closeDatabases();
        if (!createAutomaticSqliteArchive())
            qWarning(logWarning()) << tr("Aplicația se închide fără arhiva automată.");
        qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicatia.")
                                .arg(globals().nameUserApp);
        event->accept();
        return;
    }

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Finisarea lucrului"),
                             tr("Dori\310\233i s\304\203 \303\256nchide\310\233i aplica\310\233ia ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    noButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton){
        if (!closeAndSaveSettingsSubwindows()) {
            event->ignore();
            return;
        }
        closeDatabases();
        if (!createAutomaticSqliteArchive()) {
            QMessageBox::warning(this, tr("Arhivarea automată"),
                                 tr("Arhiva SQLite nu a putut fi creată. "
                                    "Detaliile sunt disponibile în jurnal."));
        }
        qInfo(logInfo()) << tr("Utilizatorul '%1' a finisat lucru cu aplicația.").arg(globals().nameUserApp);
        event->accept();
    } else if (messange_box.clickedButton() == noButton) {
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

void MainWindow::appendMigrationMessage(const QString &message)
{
    if (message.isEmpty())
        return;
    textEdit_dockWidget->append(QStringLiteral("%1 %2").arg(m_db.getHTMLImageInfo(), message.toHtmlEscaped()));
    textEdit_dockWidget->setFixedHeight(220);
    dock_widget->show();
    textEdit_dockWidget->ensureCursorVisible();
}

void MainWindow::updateWindowTitle()
{
    QString title = APPLICATION_NAME;
    // Change the displayed version only after migration and version persistence succeed.
    if (!confirmedDatabaseVersion.isEmpty())
        title += " v." + confirmedDatabaseVersion;

    const MainDatabaseConnectionData connection =
        MainDatabaseConnectionContext::instance().data();
    if (connection.backend == MainDatabaseBackend::MariaDb) {
        title += tr(" (MySQL: %1@%2): utilizator (%3)")
                     .arg(connection.databaseName, connection.hostName, globals().nameUserApp);
    } else if (connection.backend == MainDatabaseBackend::SQLite)
        title += tr(" (.sqlite3): base - '%1', utilizator (%2)")
                     .arg(connection.sqliteDatabaseName, globals().nameUserApp);
    else
        title += tr(": utilizator (%1)").arg(globals().nameUserApp);
    setWindowTitle(title);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);    // traducem
        updateWindowTitle();
        updateTextBtn();
    }
    QMainWindow::changeEvent(event);
}
