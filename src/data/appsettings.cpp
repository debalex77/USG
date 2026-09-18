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

#include "appsettings.h"
#include "appsettingsstore.h"
#include "appsettingsvalidator.h"
#include "ui_appsettings.h"

#include <QSignalBlocker>

static const int width_textLog = 915;  // inaltimea si latimea tabelei 'textLog'
static const int height_textLog = 430; // sa fie fixat la lansarea setarilor

namespace ConfigKey = AppSettingsStore::Key;
namespace ConfigDefault = AppSettingsStore::Default;


AppSettings::AppSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppSettings)
{
    ui->setupUi(this);
    captureSettingsState();

    setWindowTitle(tr("Setările aplicației %1").arg("[*]")); // setam titlu

    ui->numberLogFile->setValue(ConfigDefault::retainedLogFiles);

    const QFileInfo profileInfo(globals().settingsPath);
    const QString profileBaseName = profileInfo.completeBaseName();
    if (!profileBaseName.isEmpty()) {
        const QString logFileName = profileBaseName + QStringLiteral(".log");
        fileLogPath = QDir::toNativeSeparators(QDir(dirLogPath).filePath(logFileName));
    }

    if (!profileInfo.isFile())
        globals().unknowModeLaunch = true;

    initBtnForm(); // initializam butoanele formei

    // setam combo...
    ui->comboBoxLangApp->addItems(QStringList() << "ru-RU" << "ro-RO");
    ui->comboBoxTypeSQL->addItems(QStringList() << tr("<- Alege ->") << "MySQL(MariaDB)" << "Sqlite");
    ui->comboBoxUnitMeasure->addItems(QStringList() << tr("milimetru") << tr("centimetru"));

    ui->tabSqlite->setEnabled(false);
    ui->tabMySQL->setEnabled(false);

    setLanguageApp(); // setam limba aplicatiei
    setDefaultPath(); // setam localizarea fisierelor
    changeIndexTypeSQL((globals().indexTypeSQL == -1) ? idx_Unknow : globals().indexTypeSQL);

    initConnections(); // connectari

    ui->textLog->resize(width_textLog, height_textLog);
    setWindowModified(false);
}

AppSettings::~AppSettings()
{
    delete ui;
}

void AppSettings::captureSettingsState()
{
    m_initialProfile = profileFromGlobals();
    m_initialSettingsPath = globals().settingsPath;
}

void AppSettings::restoreSettingsState()
{
    applyProfileToGlobals(m_initialProfile, m_initialSettingsPath);

    if (translator.load(QLocale(globals().langApp), QLatin1String("USG"),
                        QLatin1String("_"), QLatin1String(":/i18n")))
        qApp->installTranslator(&translator);
}

AppSettingsStore::ProfileData AppSettings::profileFromGlobals() const
{
    AppSettingsStore::ProfileData data;

    // index
    data.languageIndex    = globals().langApp == QStringLiteral("ru-RU") ? 0 : 1;
    data.databaseIndex    = globals().indexTypeSQL;
    data.unitMeasureIndex = globals().unitMeasure == QStringLiteral("centimetru")
                                    || globals().unitMeasure == QStringLiteral("cm")
                                ? 1 : 0;

    // paths
    data.pathTemplates = globals().docsTemplatesPath;
    data.pathReports   = globals().reportsPath;
    data.pathVideo     = globals().videoDirectory;

    // mysql/nariadb
    data.mysqlHost     = globals().mySQLhost;
    data.mysqlDatabase = globals().mySQLnameBase;
    bool portValid     = false;
    data.mysqlPort = globals().mySQLport.toInt(&portValid);
    if (!portValid || data.mysqlPort < 1 || data.mysqlPort > 65535)
        data.mysqlPort = AppSettingsStore::Default::mysqlPort;
    data.mysqlUser     = globals().mySQLuser;
    data.mysqlPassword = globals().mySQLpasswdUser;
    data.mysqlOptions  = globals().mySQLoptionConnect;

    // sqlite
    data.sqliteDatabase    = globals().sqliteDatabaseName;
    data.sqlitePath        = globals().sqliteDatabasePath;
    data.imageDatabasePath = globals().imageDatabasePath;
    data.logPath           = globals().logPath;

    // remember
    data.rememberUser       = globals().memoryUser;
    data.rememberedUserId   = globals().idUserApp;
    data.rememberedUserName = globals().nameUserApp;

    // log & firstLaunch
    data.retainedLogFiles     = globals().numSavedFilesLog;
    data.initialSetupComplete = !globals().firstLaunch;

    // shows
    data.showVideoMessage   = globals().show_content_info_video;
    data.showReportsMessage = globals().show_info_reports;

    return data;
}

AppSettingsStore::ProfileData AppSettings::profileFromForm() const
{
    AppSettingsStore::ProfileData data;
    data.languageIndex    = ui->comboBoxLangApp->currentIndex();
    data.databaseIndex    = ui->comboBoxTypeSQL->currentIndex();
    data.unitMeasureIndex = ui->comboBoxUnitMeasure->currentIndex();

    data.pathTemplates = lineEditPathTemplatesPrint->text();
    data.pathReports   = lineEditPathReports->text();
    data.pathVideo     = lineEditPathVideo->text();

    data.mysqlHost     = ui->mySQLhost->text();
    data.mysqlDatabase = ui->mySQLnameBase->text();
    data.mysqlPort     = ui->mySQLport->text().toInt();
    data.mysqlUser     = ui->mySQLuser->text();
    data.mysqlPassword = ui->mySQLpasswdUser->text();
    data.mysqlOptions  = ui->mySQLoptionConnect->text();

    data.sqliteDatabase    = ui->nameBaseSqlite->text();
    data.sqlitePath        = lineEditPathDBSqlite->text();
    data.imageDatabasePath = lineEditPathDBImage->text();
    data.logPath           = ui->txtPathLog->text();

    data.rememberUser       = globals().memoryUser;
    data.rememberedUserId   = globals().idUserApp;
    data.rememberedUserName = globals().nameUserApp;

    data.retainedLogFiles     = ui->numberLogFile->value();
    data.initialSetupComplete = !globals().firstLaunch;

    data.showVideoMessage   = globals().show_content_info_video;
    data.showReportsMessage = globals().show_info_reports;

    return data;
}

void AppSettings::applyProfileToForm(const AppSettingsStore::ProfileData &data)
{
    ui->comboBoxLangApp->setCurrentIndex(data.languageIndex);
    ui->comboBoxTypeSQL->setCurrentIndex(data.databaseIndex);
    changeIndexTypeSQL(data.databaseIndex);
    ui->comboBoxUnitMeasure->setCurrentIndex(data.unitMeasureIndex);
    ui->txtPathAppSettings->setText(globals().settingsPath);

    // Profilul păstrează configurațiile ambelor motoare. Comutarea motorului activ
    // trebuie să schimbe doar disponibilitatea taburilor, nu să golească datele
    // celuilalt motor înainte de salvare.
    ui->mySQLhost->setText(data.mysqlHost);
    ui->mySQLnameBase->setText(data.mysqlDatabase);
    ui->mySQLport->setText(QString::number(data.mysqlPort));
    ui->mySQLoptionConnect->setText(data.mysqlOptions);
    ui->mySQLuser->setText(data.mysqlUser);
    ui->mySQLpasswdUser->setText(data.mysqlPassword);
    ui->nameBaseSqlite->setText(data.sqliteDatabase);
    lineEditPathDBSqlite->setText(data.sqlitePath);
    lineEditPathDBImage->setText(data.imageDatabasePath);

    if (data.databaseIndex == idx_MySQL) {
        ui->tabSqlite->setEnabled(false);
        ui->tabMySQL->setEnabled(true);
    } else if (data.databaseIndex == idx_Sqlite) {
        ui->tabSqlite->setEnabled(true);
        ui->tabMySQL->setEnabled(false);
    }

    ui->txtPathLog->setText(data.logPath);
    lineEditPathTemplatesPrint->setText(data.pathTemplates);
    lineEditPathReports->setText(data.pathReports);
    lineEditPathVideo->setText(data.pathVideo);
    ui->numberLogFile->setValue(data.retainedLogFiles);
}

void AppSettings::applyProfileToGlobals(const AppSettingsStore::ProfileData &data,
                                        const QString &settingsPath)
{
    globals().langApp = data.languageIndex == 0 ? QStringLiteral("ru-RU")
                                                : QStringLiteral("ro-RO");
    globals().unitMeasure = data.unitMeasureIndex == 0 ? QStringLiteral("milimetru")
                                                       : QStringLiteral("centimetru");
    globals().indexTypeSQL   = data.databaseIndex;
    globals().thisMySQL      = data.databaseIndex == idx_MySQL;
    globals().thisSqlite     = data.databaseIndex == idx_Sqlite;
    globals().connectionMade = globals().thisMySQL ? QStringLiteral("MySQL")
                               : globals().thisSqlite ? QStringLiteral("Sqlite")
                                                      : QString();

    if (!settingsPath.isEmpty())
        globals().settingsPath = QDir::toNativeSeparators(settingsPath);

    globals().logPath = QDir::toNativeSeparators(data.logPath);
    globals().docsTemplatesPath  = data.pathTemplates;
    globals().reportsPath        = data.pathReports;
    globals().videoDirectory     = data.pathVideo;

    globals().numSavedFilesLog = data.retainedLogFiles;
    globals().memoryUser       = data.rememberUser;
    globals().idUserApp        = data.rememberedUserId;
    globals().nameUserApp      = data.rememberedUserName;
    globals().show_content_info_video = data.showVideoMessage;
    globals().show_info_reports = data.showReportsMessage;

    globals().mySQLhost          = data.mysqlHost;
    globals().mySQLnameBase      = data.mysqlDatabase;
    globals().mySQLport          = QString::number(data.mysqlPort);
    globals().mySQLoptionConnect = data.mysqlOptions;
    globals().mySQLuser          = data.mysqlUser;
    globals().mySQLpasswdUser    = data.mysqlPassword;

    globals().sqliteDatabaseName = data.sqliteDatabase;
    globals().sqliteDatabasePath = data.sqlitePath;
    globals().imageDatabasePath  = data.imageDatabasePath;

    globals().firstLaunch = !data.initialSetupComplete;
}

// *******************************************************************
// **************** INITIEREA SI SETAREA DATELOR *********************

void AppSettings::initBtnForm()
{
    QFile fileStyleBtn(":/styles/style_btn.css");
    fileStyleBtn.open(QFile::ReadOnly);
    QString appStyleBtn(fileStyleBtn.readAll());

    initBtnSettingsApp();
    initBtnLogApp();
    initBtnMainBase(appStyleBtn);
    initBtnImageBase(appStyleBtn);
    initBtnDirTemplets(appStyleBtn);
    initBtnDirReports(appStyleBtn);
    initBtnDirVideo(appStyleBtn);
}

void AppSettings::initBtnSettingsApp()
{

    ui->btnOpenSettings->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                       "border-radius: 4px;}"
                                       "QToolButton:hover {background-color: rgb(234,243,250);}"
                                       "QToolButton:pressed {"
                                       "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);"
                                       "}");

    ui->txtPathAppSettings->setEnabled(false);

    connect(ui->btnOpenSettings, &QToolButton::clicked,
            this, &AppSettings::openFileSettingsApp, Qt::UniqueConnection);
}

void AppSettings::initBtnLogApp()
{

    ui->btnOpenLog->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                  "border-radius: 4px;}"
                                  "QToolButton:hover {background-color: rgb(234,243,250);}"
                                  "QToolButton:pressed {"
                                  "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);"
                                  "}");

    ui->txtPathLog->setEnabled(false);

    connect(ui->btnOpenLog, &QToolButton::clicked,
            this, &AppSettings::openFileCurrentLogApp, Qt::UniqueConnection);
}

void AppSettings::initBtnMainBase(const QString appStyleBtn)
{
    btnAdd = new QToolButton(this);
    btnAdd->setIcon(QIcon(":/img/toolBar/add.png"));
    btnAdd->setStyleSheet(appStyleBtn);
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnEdit = new QToolButton(this);
    btnEdit->setIcon(QIcon(":/img/toolBar/edit.png"));
    btnEdit->setStyleSheet(appStyleBtn);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnClear = new QToolButton(this);
    btnClear->setIcon(QIcon(":/img/toolBar/delete.png"));
    btnClear->setStyleSheet(appStyleBtn);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathDBSqlite = new QLineEdit(this);
    lineEditPathDBSqlite->setStyleSheet(appStyleBtn);
    lineEditPathDBSqlite->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathDBSqlite = new QHBoxLayout;
    layoutPathDBSqlite->setContentsMargins(0, 0, 0, 0);
    layoutPathDBSqlite->setSpacing(0);
    layoutPathDBSqlite->addWidget(lineEditPathDBSqlite);
    layoutPathDBSqlite->addWidget(btnAdd);
    layoutPathDBSqlite->addWidget(btnEdit);
    layoutPathDBSqlite->addWidget(btnClear);
    ui->txtPathBaseSQLITE->setLayout(layoutPathDBSqlite);

    connect(btnAdd, &QAbstractButton::clicked,
            this, &AppSettings::onAddPathSqlite, Qt::UniqueConnection);
    connect(btnEdit, &QAbstractButton::clicked,
            this, &AppSettings::onEditPathSqlite, Qt::UniqueConnection);
    connect(btnClear, &QAbstractButton::clicked,
            this, &AppSettings::onClearPathSqlite, Qt::UniqueConnection);
}

void AppSettings::initBtnImageBase(const QString appStyleBtn)
{
    btnAddImage = new QToolButton(this);
    btnAddImage->setIcon(QIcon(":/img/toolBar/add.png"));
    btnAddImage->setStyleSheet(appStyleBtn);
    btnAddImage->setCursor(Qt::PointingHandCursor);
    btnAddImage->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnEditImage = new QToolButton(this);
    btnEditImage->setIcon(QIcon(":/img/toolBar/edit.png"));
    btnEditImage->setStyleSheet(appStyleBtn);
    btnEditImage->setCursor(Qt::PointingHandCursor);
    btnEditImage->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnClearImage = new QToolButton(this);
    btnClearImage->setIcon(QIcon(":/img/toolBar/delete.png"));
    btnClearImage->setStyleSheet(appStyleBtn);
    btnClearImage->setCursor(Qt::PointingHandCursor);
    btnClearImage->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathDBImage = new QLineEdit(this);
    lineEditPathDBImage->setStyleSheet(appStyleBtn);
    lineEditPathDBImage->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathDBImage = new QHBoxLayout;
    layoutPathDBImage->setContentsMargins(0, 0, 0, 0);
    layoutPathDBImage->setSpacing(0);
    layoutPathDBImage->addWidget(lineEditPathDBImage);
    layoutPathDBImage->addWidget(btnAddImage);
    layoutPathDBImage->addWidget(btnEditImage);
    layoutPathDBImage->addWidget(btnClearImage);
    ui->txtPathImage->setLayout(layoutPathDBImage);

    connect(btnAddImage, &QAbstractButton::clicked,
            this, &AppSettings::onAddPathDBImage, Qt::UniqueConnection);
    connect(btnEditImage, &QAbstractButton::clicked,
            this, &AppSettings::onEditPathDBImage, Qt::UniqueConnection);
    connect(btnClearImage, &QAbstractButton::clicked,
            this, &AppSettings::onClearPathDBImage, Qt::UniqueConnection);
}

void AppSettings::initBtnDirTemplets(const QString appStyleBtn)
{
    btnOpenDirTemplates = new QToolButton(this);
    btnOpenDirTemplates->setIcon(QIcon(":/img/common/folder.png"));
    btnOpenDirTemplates->setStyleSheet(appStyleBtn);
    btnOpenDirTemplates->setCursor(Qt::PointingHandCursor);
    btnOpenDirTemplates->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirTemplates = new QToolButton(this);
    btnRemoveDirTemplates->setIcon(QIcon(":/img/actions/trash.png"));
    btnRemoveDirTemplates->setStyleSheet(appStyleBtn);
    btnRemoveDirTemplates->setCursor(Qt::PointingHandCursor);
    btnRemoveDirTemplates->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathTemplatesPrint = new QLineEdit(this);
    lineEditPathTemplatesPrint->setStyleSheet(appStyleBtn);
    lineEditPathTemplatesPrint->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathTemplates = new QHBoxLayout;
    layoutPathTemplates->setContentsMargins(0, 0, 0, 0);
    layoutPathTemplates->setSpacing(0);
    layoutPathTemplates->addWidget(lineEditPathTemplatesPrint);
    layoutPathTemplates->addWidget(btnOpenDirTemplates);
    layoutPathTemplates->addWidget(btnRemoveDirTemplates);
    ui->txtPathTemplatesPrint->setLayout(layoutPathTemplates);

    connect(btnOpenDirTemplates, &QAbstractButton::clicked,
            this , &AppSettings::openDirTemplets, Qt::UniqueConnection);
    connect(btnRemoveDirTemplates, &QAbstractButton::clicked, this, [this]()
    {
        lineEditPathTemplatesPrint->clear();
    });
}

void AppSettings::initBtnDirReports(const QString appStyleBtn)
{
    btnOpenDirReports = new QToolButton(this);
    btnOpenDirReports->setIcon(QIcon(":/img/common/folder.png"));
    btnOpenDirReports->setStyleSheet(appStyleBtn);
    btnOpenDirReports->setCursor(Qt::PointingHandCursor);
    btnOpenDirReports->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirReports = new QToolButton(this);
    btnRemoveDirReports->setIcon(QIcon(":/img/actions/trash.png"));
    btnRemoveDirReports->setStyleSheet(appStyleBtn);
    btnRemoveDirReports->setCursor(Qt::PointingHandCursor);
    btnRemoveDirReports->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathReports = new QLineEdit(this);
    lineEditPathReports->setStyleSheet(appStyleBtn);
    lineEditPathReports->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathReports = new QHBoxLayout;
    layoutPathReports->setContentsMargins(0, 0, 0, 0);
    layoutPathReports->setSpacing(0);
    layoutPathReports->addWidget(lineEditPathReports);
    layoutPathReports->addWidget(btnOpenDirReports);
    layoutPathReports->addWidget(btnRemoveDirReports);
    ui->txtPathReports->setLayout(layoutPathReports);

    connect(btnOpenDirReports, &QAbstractButton::clicked,
            this , &AppSettings::openDirReports, Qt::UniqueConnection);
    connect(btnRemoveDirReports, &QAbstractButton::clicked, this, [this]()
    {
        lineEditPathReports->clear();
    });
}

void AppSettings::initBtnDirVideo(const QString appStyleBtn)
{
    btnOpenDirVideo = new QToolButton(this);
    btnOpenDirVideo->setIcon(QIcon(":/img/common/folder.png"));
    btnOpenDirVideo->setStyleSheet(appStyleBtn);
    btnOpenDirVideo->setCursor(Qt::PointingHandCursor);
    btnOpenDirVideo->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirVideo = new QToolButton(this);
    btnRemoveDirVideo->setIcon(QIcon(":/img/actions/trash.png"));
    btnRemoveDirVideo->setStyleSheet(appStyleBtn);
    btnRemoveDirVideo->setCursor(Qt::PointingHandCursor);
    btnRemoveDirVideo->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathVideo = new QLineEdit(this);
    lineEditPathVideo->setStyleSheet(appStyleBtn);
    lineEditPathVideo->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathVideo = new QHBoxLayout;
    layoutPathVideo->setContentsMargins(0, 0, 0, 0);
    layoutPathVideo->setSpacing(0);
    layoutPathVideo->addWidget(lineEditPathVideo);
    layoutPathVideo->addWidget(btnOpenDirVideo);
    layoutPathVideo->addWidget(btnRemoveDirVideo);
    ui->txtPathVideo->setLayout(layoutPathVideo);

    connect(btnOpenDirVideo, &QAbstractButton::clicked,
            this , &AppSettings::openDirVideo, Qt::UniqueConnection);
    connect(btnRemoveDirVideo, &QAbstractButton::clicked, this, [this]()
            {
                lineEditPathVideo->clear();
            });
}

void AppSettings::initConnections()
{
    // data modified
    connect(lineEditPathDBSqlite, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(lineEditPathDBImage, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->nameBaseSqlite, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLhost, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLnameBase, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLport, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLoptionConnect, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLuser, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->mySQLpasswdUser, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->txtPathAppSettings, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->txtPathLog, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(lineEditPathReports, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(lineEditPathTemplatesPrint, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(lineEditPathVideo, &QLineEdit::textChanged,
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);
    connect(ui->numberLogFile, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AppSettings::dataWasModified, Qt::UniqueConnection);

    // tables
    connect(ui->tabView, QOverload<int>::of(&QTabWidget::currentChanged),
            this, QOverload<int>::of(&AppSettings::slot_currentIndexChangedTab), Qt::UniqueConnection);

    connect(ui->tableViewLogs, QOverload<const QModelIndex&>::of(&QTableView::pressed),
            this, &AppSettings::slot_clickedTableLogs, Qt::UniqueConnection);

    // conectarea la modificarea limbei aplicatiei
    // conectari combo...
    connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexLangApp), Qt::UniqueConnection);
    connect(ui->comboBoxTypeSQL, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexTypeSQL), Qt::UniqueConnection);
    connect(ui->comboBoxUnitMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexUnitMeasure), Qt::UniqueConnection);

    // setam drumul spre setarile aplicatiei
    connect(ui->nameBaseSqlite, &QLineEdit::textChanged,
            this, &AppSettings::setPathAppSettings, Qt::UniqueConnection);
    connect(ui->mySQLnameBase, &QLineEdit::textChanged,
            this, &AppSettings::setPathAppSettings, Qt::UniqueConnection);
    // testarea conectarii la BD
    connect(ui->btnCreateNewBaseSqlite, &QAbstractButton::clicked,
            this, &AppSettings::createNewBaseSqlite, Qt::UniqueConnection);
    connect(ui->btnTestConnectionMySQL, &QAbstractButton::clicked,
            this, &AppSettings::onTestConnectionMySQL, Qt::UniqueConnection);

    // main btn form
    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &AppSettings::onBtnOKSettings, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &AppSettings::onBtnWriteSettings, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &AppSettings::onBtnCancelSettings, Qt::UniqueConnection);

    connect(ui->btnLogLevel, &LogLevelButton::selectedLevel,
            this, &AppSettings::selectedLevelLog, Qt::UniqueConnection);
}

// *******************************************************************
// **************** SETAREA DATELOR **********************************

void AppSettings::setLanguageApp()
{
    QString localeName = globals().langApp.trimmed();
    if (localeName.isEmpty())
        localeName = QLocale::system().name();

    localeName.replace(QLatin1Char('_'), QLatin1Char('-'));
    const bool russian = localeName.startsWith(QLatin1String("ru"),
                                                Qt::CaseInsensitive);
    const QString language = russian ? QStringLiteral("ru-RU")
                                     : QStringLiteral("ro-RO");

    // blocam signale pu declansarea modificarii
    const QSignalBlocker blocker(ui->comboBoxLangApp);
    ui->comboBoxLangApp->setCurrentIndex(russian ? 0 : 1);

    if (translator.load(QLocale(language), QLatin1String("USG"),
                        QLatin1String("_"), QLatin1String(":/i18n")))
        qApp->installTranslator(&translator);
}

void AppSettings::setDefaultPath()
{
    if (ui->txtPathAppSettings->text().isEmpty())
        ui->txtPathAppSettings->setText(globals().settingsPath);

    ui->txtPathLog->setText(globals().logPath.isEmpty()
                                ? fileLogPath
                                : globals().logPath);

    // Șabloanele instalate sunt localizate relativ la executabil, independent
    // de directorul curent din care a fost pornită aplicația.
    const QDir applicationDirectory(QCoreApplication::applicationDirPath());
    const QString defaultTemplatesPath = QDir::toNativeSeparators(
        applicationDirectory.filePath(QStringLiteral("templets")));
    const QString defaultReportsPath = QDir::toNativeSeparators(
        QDir(defaultTemplatesPath).filePath(QStringLiteral("reports")));

    lineEditPathTemplatesPrint->setText(globals().docsTemplatesPath.isEmpty()
                                            ? defaultTemplatesPath
                                            : globals().docsTemplatesPath);
    lineEditPathReports->setText(globals().reportsPath.isEmpty()
                                     ? defaultReportsPath
                                     : globals().reportsPath);
}

void AppSettings::setDefaultPathSqlite()
{
    if (! globals().firstLaunch)
        return;

    QDir dir;

#if defined(Q_OS_LINUX)

    //******************************************************************************
    // crearea fisierelor bazelor de date si setarea
    QString str_dir_database = dir.toNativeSeparators(dir.homePath() + "/Database_usg");
    QString str_file_database = str_dir_database + "/base.sqlite3";
    QString str_file_database_image = str_dir_database + "/base_image.sqlite3";

#elif defined(Q_OS_MACOS)

    // Șabloanele rămân în directorul aplicației; aici se inițializează numai
    // locația implicită a bazelor SQLite.
    QString str_dir_database = dir.toNativeSeparators(dir.homePath() + "/USG/Database_usg");
    QString str_file_database = str_dir_database + "/base.sqlite3";
    QString str_file_database_image = str_dir_database + "/base_image.sqlite3";

#elif defined(Q_OS_WIN)

    //******************************************************************************
    // crearea fisierelor bazelor de date si setam variabel globale
    QString str_dir_database        = dir.toNativeSeparators(dir.rootPath()) + "Database_usg";
    QString str_file_database       = str_dir_database + "\\base.sqlite3";
    QString str_file_database_image = str_dir_database + "\\base_image.sqlite3";

#endif
    const QDir databaseDirectory(str_dir_database);
    if (!databaseDirectory.exists()) {
        // mkpath creează și directorul intermediar (de exemplu ~/USG pe macOS).
        if (QDir().mkpath(str_dir_database)) {

            //---- baza principale
            lineEditPathDBSqlite->setText(str_file_database);                     // baza principala
            ui->nameBaseSqlite->setText("base");                                  // denumirea bazei de date

            //---- baza cu imagini
            lineEditPathDBImage->setText(str_file_database_image);         // baza de date cu imagini

            setPathAppSettings();

        } else {
            QMessageBox::warning(this, tr("Crearea directoriei"),
                                 tr("Directoria <b>'%1'</b><br>"
                                    "pentru baza de date SQlite nu a fost creat\304\203 !!! Lansarea aplica\310\233iei nu este posibil\304\203.<br>"
                                    "Adresa\310\233i-v\304\203 administratorului aplica\310\233iei.").arg(str_dir_database),
                                QMessageBox::Ok);
            qCritical(logCritical()) << tr("Directoria '%1' pentru baza de date SQlite nu a fost creată.").arg(str_dir_database);
        }
    } else {
//         if (QFile(str_file_database).exists()){
//             QMessageBox messange_box(QMessageBox::Question,
//                                      tr("Determinarea existen\310\233ei bazei de date"),
//                                      tr("Baza de date '<u>%1</u>' exist\304\203 \303\256n sistem !!!<br><br>"
//                                         "Pentru crearea bazei de date noi cu nume '<b><u>%2</u></b>' este necesar de eliminat fi\310\231ierul vechi. "
//                                         "Dori\310\233i s\304\203 elimina\310\233i fi\310\231ierul din sistem ?").arg(str_file_database, (ui->nameBaseSqlite->text().isEmpty()) ? "base" : ui->nameBaseSqlite->text()),
//                                      QMessageBox::NoButton, this);
//             QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//             QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
//             yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
//             noButton->setStyleSheet(db->getStyleForButtonMessageBox());
//             messange_box.exec();

//             if (messange_box.clickedButton() == yesButton){
// #if defined(Q_OS_LINUX)
//                 QString str_cmd = "rm \"" + str_file_database + "\"";
//                 system(str_cmd.toStdString().c_str());
//                 str_cmd = "rm \"" + str_file_database_image + "\"";
//                 system(str_cmd.toStdString().c_str());
// #elif defined(Q_OS_MACOS)
//                 QString str_cmd = "rm \"" + str_file_database + "\"";
//                 system(str_cmd.toStdString().c_str());
//                 str_cmd = "rm \"" + str_file_database_image + "\"";
//                 system(str_cmd.toStdString().c_str());
// #elif defined(Q_OS_WIN)
//                 QString str_cmd = "del " + str_file_database;
//                 system(str_cmd.toStdString().c_str());
// #endif
//             } else if (messange_box.clickedButton() == noButton) {
//                 return;
//             }
//         }
        ui->nameBaseSqlite->setText("base");
        lineEditPathDBSqlite->setText(str_file_database);

        lineEditPathDBImage->setText(str_file_database_image);
    }
}

bool AppSettings::checkDataSettings()
{
    // Spațiile marginale nu fac parte din identificatorii/căile configurației.
    // Parola MariaDB nu este normalizată: spațiile pot fi caractere valide în parolă.
    ui->mySQLhost->setText(ui->mySQLhost->text().trimmed());
    ui->mySQLnameBase->setText(ui->mySQLnameBase->text().trimmed());
    ui->mySQLport->setText(ui->mySQLport->text().trimmed());
    ui->mySQLuser->setText(ui->mySQLuser->text().trimmed());
    ui->nameBaseSqlite->setText(ui->nameBaseSqlite->text().trimmed());
    lineEditPathDBSqlite->setText(lineEditPathDBSqlite->text().trimmed());
    lineEditPathDBImage->setText(lineEditPathDBImage->text().trimmed());
    ui->txtPathAppSettings->setText(ui->txtPathAppSettings->text().trimmed());
    ui->txtPathLog->setText(ui->txtPathLog->text().trimmed());
    lineEditPathTemplatesPrint->setText(lineEditPathTemplatesPrint->text().trimmed());
    lineEditPathReports->setText(lineEditPathReports->text().trimmed());
    lineEditPathVideo->setText(lineEditPathVideo->text().trimmed());

    const AppSettingsStore::ProfileData data = profileFromForm();
    const AppSettingsValidator::ValidationResult validation =
        AppSettingsValidator::validateProfile(data, ui->txtPathAppSettings->text());
    if (validation.isValid())
        return true;

    QWidget *invalidField = nullptr;
    using Field = AppSettingsValidator::Field;
    switch (validation.field) {
    case Field::DatabaseType:      invalidField = ui->comboBoxTypeSQL; break;
    case Field::ProfileName:
        invalidField = data.databaseIndex == idx_Sqlite
                           ? static_cast<QWidget *>(ui->nameBaseSqlite)
                           : static_cast<QWidget *>(ui->mySQLnameBase);
        break;
    case Field::MysqlDatabase:     invalidField = ui->mySQLnameBase; break;
    case Field::SettingsPath:      invalidField = ui->txtPathAppSettings; break;
    case Field::LogPath:           invalidField = ui->txtPathLog; break;
    case Field::TemplatesPath:     invalidField = lineEditPathTemplatesPrint; break;
    case Field::ReportsPath:       invalidField = lineEditPathReports; break;
    case Field::VideoPath:         invalidField = lineEditPathVideo; break;
    case Field::MysqlHost:         invalidField = ui->mySQLhost; break;
    case Field::MysqlPort:         invalidField = ui->mySQLport; break;
    case Field::MysqlUser:         invalidField = ui->mySQLuser; break;
    case Field::SqliteDatabase:    invalidField = ui->nameBaseSqlite; break;
    case Field::SqlitePath:        invalidField = lineEditPathDBSqlite; break;
    case Field::ImageDatabasePath: invalidField = lineEditPathDBImage; break;
    case Field::None: break;
    }

    if (invalidField)
        invalidField->setFocus();

    const QString valueDetails = validation.value.isEmpty()
                                     ? QString()
                                     : tr("<br><br>Valoare: %1")
                                           .arg(validation.value.toHtmlEscaped());
    QMessageBox::warning(this, tr("Verificarea datelor"),
                         tr("Configurația nu este validă: %1%2")
                             .arg(validation.reason.toHtmlEscaped(), valueDetails),
                         QMessageBox::Ok);
    return false;
}

// *******************************************************************
// **************** EXTRAGEREA SI SALVAREA DATELOR *******************

void AppSettings::readSettings()
{
    if (!loadSettings())
        return; // fisierul selectat lipseste sau invalid

    m_populatingForm = true;

    // blocam signale pu declansarea modificarii
    const QSignalBlocker languageBlocker(ui->comboBoxLangApp);
    const QSignalBlocker databaseTypeBlocker(ui->comboBoxTypeSQL);
    const QSignalBlocker unitMeasureBlocker(ui->comboBoxUnitMeasure);
    const QSignalBlocker sqliteNameBlocker(ui->nameBaseSqlite);
    const QSignalBlocker sqlitePathBlocker(lineEditPathDBSqlite);
    const QSignalBlocker imagePathBlocker(lineEditPathDBImage);
    const QSignalBlocker mysqlHostBlocker(ui->mySQLhost);
    const QSignalBlocker mysqlDatabaseBlocker(ui->mySQLnameBase);
    const QSignalBlocker mysqlPortBlocker(ui->mySQLport);
    const QSignalBlocker mysqlOptionsBlocker(ui->mySQLoptionConnect);
    const QSignalBlocker mysqlUserBlocker(ui->mySQLuser);
    const QSignalBlocker mysqlPasswordBlocker(ui->mySQLpasswdUser);
    const QSignalBlocker settingsPathBlocker(ui->txtPathAppSettings);
    const QSignalBlocker logPathBlocker(ui->txtPathLog);
    const QSignalBlocker templatesPathBlocker(lineEditPathTemplatesPrint);
    const QSignalBlocker reportsPathBlocker(lineEditPathReports);
    const QSignalBlocker videoPathBlocker(lineEditPathVideo);
    const QSignalBlocker retainedLogsBlocker(ui->numberLogFile);

    applyProfileToForm(m_loadedProfile);

    m_populatingForm = false;
    setWindowModified(false);
}

bool AppSettings::saveSettings()
{
    QDir dir_conf;

    const QString settingsPath = dir_conf.toNativeSeparators(ui->txtPathAppSettings->text());

    const AppSettingsStore::ProfileData data = profileFromForm();

    const AppSettingsStore::WriteError writeError = AppSettingsStore::writeProfile(settingsPath, data);

    if (writeError != AppSettingsStore::WriteError::None) {
        const QString reason = writeError == AppSettingsStore::WriteError::Access
                                   ? tr("Fișierul nu poate fi scris. Verificați calea și drepturile de acces.")
                                   : tr("Formatul fișierului de configurare nu este valid.");

        qCritical(logCritical()) << tr("Salvarea setărilor în '%1' a eșuat: %2")
                                        .arg(settingsPath, reason);

        QMessageBox::critical(this, tr("Salvarea setărilor"),
                              tr("Setările nu au putut fi salvate în:\n%1\n\n%2")
                                  .arg(settingsPath, reason));

        restoreSettingsState();
        setWindowModified(true);
        return false;
    }

    applyProfileToGlobals(data, settingsPath);
    m_loadedProfile = data;

    qInfo(logInfo()) << tr("Setarile aplicatiei sunt salvate/modificate in fisierul - %1.")
                            .arg(settingsPath);
    setWindowModified(false);
    captureSettingsState();
    return true;
}

void AppSettings::updateTableLog(QString level_log, QStringList level_exclude)
{
    const QModelIndex index = ui->tableViewLogs->currentIndex();

    QDir dir;
    int row = index.row();
    QString m_file_log = m_logModel.data(m_logModel.index(row, 0), Qt::DisplayRole).toString();
    QFile file(dir.toNativeSeparators(m_file_log));
    if (!file.exists())
        return;

    ui->textLog->clear();

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream in(&file);
        QString html;
        html.append("<p style='color:#a6a6a6'>");

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            // controlam nivelul log-lui
            if (! level_log.isEmpty() && ! line.contains(level_log))
                continue;

            // exludem nivel ce este introdus in lista
            bool shouldExclude = false;
            for (const QString &exclude : level_exclude) {
                if (line.contains(exclude)) {
                    shouldExclude = true;
                    break;
                }
            }

            if (shouldExclude)
                continue;

            QString icon;
            if (line.contains("SYNC"))
                icon = ":/img/actions/sync.png";
            else if (line.contains("THREAD"))
                icon = ":/img/actions/thread.png";
            else if (line.contains("WRN"))
                icon = ":/img/common/warning.png";
            else if (line.contains("INF"))
                icon = ":/img/common/info.png";
            else if (line.contains("CRT"))
                icon = ":/img/common/critical.png";
            else if (line.contains("FTL"))
                icon = ":/img/common/error.png";
            else if (line.contains("DBG"))
                icon = ":/img/common/bug.png";
            else
                icon = "";

            if (!icon.isEmpty()) {
                html += QString("<img src='%1' width='16' height='16'> ").arg(icon);
            }

            html += line.toHtmlEscaped() + "<br>";
        }

        html.append("</p>");

        ui->textLog->setHtml(html);
        ui->textLog->setStyleSheet("font-size: 13px;");
    }

    file.close();
}

bool AppSettings::loadSettings()
{
    // verificam existenta fisierului si il citim
    const QFileInfo settingsFileInfo(globals().settingsPath);
    if (globals().settingsPath.isEmpty() ||
        !settingsFileInfo.isFile() ||
        !settingsFileInfo.isReadable()) {
        const QString reason = tr("Fișierul de configurare lipsește sau nu poate fi citit:\n%1")
                                   .arg(globals().settingsPath);
        qCritical(logCritical()) << reason;
        QMessageBox::critical(this, tr("Citirea setărilor"), reason);
        return false;
    }

    // verificam formatul corect a fisierului
    const AppSettingsStore::ReadResult readResult = AppSettingsStore::readProfile(globals().settingsPath, fileLogPath);
    if (readResult.error == AppSettingsStore::ReadError::Access ||
        readResult.error == AppSettingsStore::ReadError::Format) {
        const QString reason = readResult.error == AppSettingsStore::ReadError::Format
                                   ? tr("Fișierul de configurare are un format invalid:\n%1")
                                         .arg(globals().settingsPath)
                                   : tr("Fișierul de configurare nu poate fi accesat:\n%1")
                                         .arg(globals().settingsPath);
        qCritical(logCritical()) << reason;
        QMessageBox::critical(this, tr("Citirea setărilor"), reason);
        return false;
    }

    // validam codificarea
    if (readResult.error == AppSettingsStore::ReadError::InvalidEncodedValue) {
        qCritical(logCritical()) << tr("Valori codificate invalide în fișierul de configurare:")
                                 << readResult.invalidEncodedKeys;
        QMessageBox::critical(
            this, tr("Citirea setărilor"),
            tr("Fișierul de configurare conține valori codificate deteriorate:\n%1\n\n"
               "Selectați alt profil sau corectați setările.")
                .arg(readResult.invalidEncodedKeys.join(QLatin1Char('\n'))));
        return false;
    }

    AppSettingsStore::ProfileData data = readResult.data;
    // Profilurile MariaDB istorice nu păstrează pathLogApp în grupul connect.
    // În acest caz rămâne valabilă calea calculată anterior din numele profilului.
    if (data.logPath.isEmpty())
        data.logPath = fileLogPath;
    applyProfileToGlobals(data, globals().settingsPath);
    m_loadedProfile = data;
    if (data.rememberedUserDataIncomplete)
        qWarning(logWarning())
            << tr("Datele utilizatorului memorat sunt incomplete; memorarea a fost dezactivată.");

    // traducem aplicatia
    if (translator.load(QLocale(globals().langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        qApp->installTranslator(&translator);
    }

    // Anulare/esecul salvarii revine la profilul valid incarcat,
    // nu la starea incompleta existenta înainte de loadSettings().
    captureSettingsState();
    return true;
}

bool AppSettings::saveRememberedUser(int userId, const QString &userName, bool remember)
{
    const QString normalizedUserName = userName.trimmed();
    if (remember && (userId <= 0 || normalizedUserName.isEmpty())) {
        qWarning(logWarning())
            << tr("Datele utilizatorului memorat nu sunt valide; setările nu au fost scrise.");
        return false;
    }

    return AppSettingsStore::writeGroup(globals().settingsPath, ConfigKey::groupStartup,
        {
            {
                ConfigKey::rememberedUserId,
                remember ? DataBase::encode_string(QString::number(userId))  : QString()
            },
            {
                ConfigKey::rememberedUserName,
                remember ? DataBase::encode_string(normalizedUserName) : QString()
            },
            {ConfigKey::rememberUser, remember}
        });
}

bool AppSettings::saveInitialSetupComplete(bool complete)
{
    const bool saved = AppSettingsStore::writeGroup(
        globals().settingsPath,
        ConfigKey::groupStartup,
        {{ConfigKey::initialSetupComplete, complete}});
    if (saved)
        globals().firstLaunch = !complete;
    return saved;
}

bool AppSettings::saveInfoMessageVisibility(InfoMessage message, bool visible)
{
    const QString &key = message == InfoMessage::Video
                             ? ConfigKey::showVideoMessage
                             : ConfigKey::showReportsMessage;
    return AppSettingsStore::writeGroup(globals().settingsPath,
                                        ConfigKey::groupMessages, {{key, visible}});
}

// *******************************************************************
// **************** PROCESAREA LOG-FISIERELOR ************************

void AppSettings::dataWasModified()
{
    if (!m_populatingForm)
        setWindowModified(true);
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void AppSettings::slot_currentIndexChangedTab(const int index)
{
    if (index == 0)
        return;

    m_logModel.clear();

    const QFileInfo activeLogInfo(globals().logPath);
    const QString baseName = activeLogInfo.completeBaseName();
    if (baseName.isEmpty())
        return;

    const QRegularExpression regx(
        QStringLiteral("^%1(?:_[0-9]{1,2}\\.[0-9]{1,2}\\.[0-9]{4}(?:_[0-9]+)?)?\\.log$")
            .arg(QRegularExpression::escape(baseName)),
        QRegularExpression::CaseInsensitiveOption);

    QDir dir(activeLogInfo.absolutePath());
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Time);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);

        if (regx.match(fileInfo.fileName()).hasMatch()) {
            QString m_file_log = dir.toNativeSeparators(fileInfo.filePath());

            QStandardItem *item = new QStandardItem;
            item->setData(n, Qt::UserRole);
            item->setData(m_file_log, Qt::DisplayRole);
            m_logModel.appendRow(item);
        }
    }

    ui->tableViewLogs->setModel(&m_logModel);
    ui->tableViewLogs->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewLogs->verticalHeader()->setDefaultSectionSize(13);
    m_logModel.setHeaderData(0, Qt::Horizontal, tr("Localizarea fisierelor de logare"));
    qInfo(logInfo()) << tr("Deschisa forma 'Vizualizarea fisierului de logare'.");
}

void AppSettings::slot_clickedTableLogs(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    QDir dir;
    int row = index.row();
    QString m_file_log = m_logModel.data(m_logModel.index(row, 0), Qt::DisplayRole).toString();
    QFile file(dir.toNativeSeparators(m_file_log));
    if (!file.exists())
        return;

    ui->textLog->clear();

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo(logInfo()) << tr("Vizualizarea fisierului de logare '%1'.")
                                .arg(dir.toNativeSeparators(m_file_log));

        QTextStream in(&file);
        QString html;
        html.append("<p style='color:#a6a6a6'>");

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            QString icon;
            if (line.contains("[SYNC]"))
                icon = ":/img/actions/sync.png";
            else if (line.contains("THREAD"))
                icon = ":/img/actions/thread.png";
            else if (line.contains("WRN"))
                icon = ":/img/common/warning.png";
            else if (line.contains("INF"))
                icon = ":/img/common/info.png";
            else if (line.contains("CRT"))
                icon = ":/img/common/critical.png";
            else if (line.contains("FTL"))
                icon = ":/img/common/error.png";
            else if (line.contains("DBG"))
                icon = ":/img/common/bug.png";
            else
                icon = ""; // sau nimic

            if (!icon.isEmpty()) {
                html += QString("<img src='%1' width='16' height='16'> ").arg(icon);
            }

            html += line.toHtmlEscaped() + "<br>";
        }

        html.append("</p>");

        ui->textLog->setHtml(html);
        ui->textLog->setStyleSheet("font-size: 13px;");
    }

    file.close();
}

// *******************************************************************
// **************** BUTOANE - (base.sqlite3) *************************

void AppSettings::onAddPathSqlite()
{
    QDir dir;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Crearea fișierului"),
                                                    dir.toNativeSeparators(QDir::rootPath() + "/base"),
                                                    tr("SQLite3 file (*.sqlite3)"));

    if (!fileName.isEmpty()){
        QDir file_database;
        lineEditPathDBSqlite->setText(file_database.toNativeSeparators(fileName));

        if (ui->nameBaseSqlite->text().isEmpty()){
            QFileInfo fileInfo(dir.toNativeSeparators(fileName));
            QString m_nameBase = fileInfo.fileName();
            m_nameBase.remove(m_nameBase.size() - 8, 8); // 8 simboluri = (.sqlite3)
            ui->nameBaseSqlite->setText(m_nameBase);
        }
        dataWasModified();
    }
}

void AppSettings::onEditPathSqlite()
{
    QDir dir;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Deschide fișierul"),
                                                    dir.toNativeSeparators(QDir::currentPath()),
                                                    tr("SQLite3 file (*.sqlite3)"));
    if (!fileName.isEmpty()){
        QDir file_database;
        lineEditPathDBSqlite->setText(file_database.toNativeSeparators(fileName));

        if (ui->nameBaseSqlite->text().isEmpty()){
            QFileInfo fileInfo(dir.toNativeSeparators(fileName));
            QString m_nameBase = fileInfo.fileName();
            m_nameBase.remove(m_nameBase.size() - 8, 8); // 8 simboluri = (.sqlite3)
            ui->nameBaseSqlite->setText(m_nameBase);
        }
        dataWasModified();
    }
}

void AppSettings::onClearPathSqlite()
{
    lineEditPathDBSqlite->clear();
    dataWasModified();
}

// *******************************************************************
// **************** BUTOANE - (base_image.sqlite3) *******************

void AppSettings::onAddPathDBImage()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Crearea fișierului"),
                                                    QDir::currentPath() + "/database/imagesUSG.sqlite3",
                                                    tr("SQLite3 file (*.sqlite3)"));

    if (!fileName.isEmpty()){
        QDir file_img;
        lineEditPathDBImage->setText(file_img.toNativeSeparators(fileName));
        dataWasModified();
    }
}

void AppSettings::onEditPathDBImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Deschide fișierul"),
                                                    QDir::currentPath() + "/database/imagesUSG.sqlite3",
                                                    tr("SQLite3 file (*.sqlite3)"));
    if (!fileName.isEmpty()){
        QDir file_img;
        lineEditPathDBImage->setText(file_img.toNativeSeparators(fileName));
        dataWasModified();
    }
}

void AppSettings::onClearPathDBImage()
{
    lineEditPathDBImage->clear();
    dataWasModified();
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void AppSettings::openFileSettingsApp()
{
    if (! QFile(ui->txtPathAppSettings->text()).exists()){
        QMessageBox::warning(this,
                             tr("Verificarea fi\310\231ierului"),
                             tr("Fi\310\231ierul cu set\304\203rile aplica\310\233iei nu a fost g\304\203sit !!!."),
                             QMessageBox::Ok);
        return ;
    }
#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->txtPathAppSettings->text()));
#elif defined(Q_OS_MACOS)
    QProcess process;
    QStringList arguments;
    arguments << "-a" << "TextEdit" << ui->txtPathAppSettings->text();

    if (!process.startDetached("open", arguments)) {
        QMessageBox::warning(nullptr,
                             QObject::tr("Eroare"),
                             QObject::tr("Nu s-a putut deschide fișierul cu TextEdit."),
                             QMessageBox::Ok);
    }
#elif defined(Q_OS_WIN)
    QProcess* proc = new QProcess(this);
    QStringList arguments;
    arguments << ui->txtPathAppSettings->text();
    proc->start("notepad.exe", arguments);
    proc->waitForFinished();
    proc->deleteLater();
#endif

}

void AppSettings::openFileCurrentLogApp()
{
    const QString selectedLogPath = ui->txtPathLog->text();
    if (!QFileInfo::exists(selectedLogPath)){
        QMessageBox::warning(this,
                             tr("Verificarea fi\310\231ierului de logare"),
                             tr("Fi\310\231ierul de logare nu a fost g\304\203sit !!!<br>"
                                "Crea\310\233i fi\310\231ierul nou sau restarta\310\233i aplica\310\233ia."),
                             QMessageBox::Ok);
        return ;
    }

#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->txtPathLog->text()));
#elif defined(Q_OS_MACOS)
    if (! QDesktopServices::openUrl(QUrl::fromLocalFile(ui->txtPathLog->text()))) {
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Nu s-a putut deschide fișierul de logare a aplicației."),
                             QMessageBox::Ok);
    }
#elif defined(Q_OS_WIN)
    QProcess* proc = new QProcess(this);
    QStringList arguments;
    arguments << ui->txtPathLog->text();
    proc->start("notepad.exe", arguments);
#elif defined(Q_OS_MACOS)
#endif

}

void AppSettings::openDirTemplets()
{
    QDir dir;
    QString pathTemplets;
    if (lineEditPathTemplatesPrint->text().isEmpty())
        pathTemplets = dir.toNativeSeparators(QDir::currentPath());
    else
        pathTemplets = dir.toNativeSeparators(lineEditPathTemplatesPrint->text());

    const QString dir_templates = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
                                                                     pathTemplets,
                                                                     QFileDialog::ShowDirsOnly
                                                                         | QFileDialog::DontResolveSymlinks);

    if (!dir_templates.isEmpty())
        lineEditPathTemplatesPrint->setText(dir.toNativeSeparators(dir_templates));
}

void AppSettings::openDirReports()
{
    QDir dir;
    QString pathReports;
    if (lineEditPathReports->text().isEmpty())
        pathReports = dir.toNativeSeparators(QDir::currentPath());
    else
        pathReports = dir.toNativeSeparators(lineEditPathReports->text());

    const QString dir_reports = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
                                                                   pathReports,
                                                                   QFileDialog::ShowDirsOnly
                                                                       | QFileDialog::DontResolveSymlinks);

    if (!dir_reports.isEmpty())
        lineEditPathReports->setText(dir.toNativeSeparators(dir_reports));
}

void AppSettings::openDirVideo()
{
    QDir dir;
    QString pathVideo;
    if (lineEditPathVideo->text().isEmpty())
        pathVideo = dir.toNativeSeparators(QDir::currentPath());
    else
        pathVideo = dir.toNativeSeparators(lineEditPathVideo->text());

    const QString dir_video = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
                                                                 pathVideo,
                                                                 QFileDialog::ShowDirsOnly
                                                                     | QFileDialog::DontResolveSymlinks);

    if (!dir_video.isEmpty())
        lineEditPathVideo->setText(dir.toNativeSeparators(dir_video));
}

// *******************************************************************
// **************** BUTOANE - formei principale **********************

void AppSettings::onBtnOKSettings()
{
    if (checkDataSettings() && saveSettings())
        QDialog::accept();
}

void AppSettings::onBtnWriteSettings()
{
    if (checkDataSettings())
        saveSettings();
}

void AppSettings::onBtnCancelSettings()
{
    restoreSettingsState();
    setWindowModified(false);
    this->close();
}

// *******************************************************************
// **************** PROCESAREA INDEXULUI COMBOBOX ********************

void AppSettings::changeIndexLangApp(const int _index)
{
    if (_index < 0 || _index > 1)
        return;

    ui->comboBoxLangApp->setCurrentIndex(_index);
    const QString selectedLanguage = _index == 0 ? QStringLiteral("ru-RU")
                                                  : QStringLiteral("ro-RO");

    if (translator.load(QLocale(selectedLanguage),
                        QLatin1String("USG"),
                        QLatin1String("_"),
                        QLatin1String(":/i18n")))
    {
        qApp->installTranslator(&translator);
    }

    dataWasModified();

    QMessageBox::StandardButton YesNo;
    YesNo = QMessageBox::question(this, tr("Traducerea aplica\310\233iei."),
                                  tr("Pentru traducerea complet\304\203 este necesar de relansat aplica\310\233ia.<br><br>"
                                     "Dori\310\233i relansarea ?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (YesNo == QMessageBox::Yes){
        if (checkDataSettings() && saveSettings()) {
            QStringList arguments = qApp->arguments();
            if (!arguments.isEmpty())
                arguments.removeFirst(); // programul nu trebuie transmis și ca argv[1]

            const QString program = qApp->applicationFilePath();
            if (QProcess::startDetached(program, arguments)) {
                qApp->quit();
            } else {
                qCritical(logCritical())
                    << tr("Relansarea aplicației după schimbarea limbii a eșuat:")
                    << program;
                QMessageBox::critical(
                    this, tr("Relansarea aplicației"),
                    tr("Aplicația nu a putut fi relansată. Aplicația curentă va rămâne deschisă."));
            }
        }
    }
}

void AppSettings::changeIndexTypeSQL(const int _index)
{
    if (_index == idx_MySQL){ // MySQL

#if defined(Q_OS_MACOS)

        QMessageBox::warning(this,
                             tr("Controlul driverelor"),
                             tr("Pentru sistemul de operare MacOS nu este inclus driverul MySQL !!! "
                                "V-a fi inclus in actualizarile ulterioare."),
                             QMessageBox::Ok);
        ui->comboBoxTypeSQL->setCurrentIndex(idx_Sqlite);
        return;

#endif

        ui->tabLogs->setEnabled(true);
        ui->tabSqlite->setEnabled(false);
        ui->tabMySQL->setEnabled(true);
        ui->txtPathImage->setEnabled(false);
        ui->tabConnections->setCurrentIndex(1);

        setDefaultPath(); // setam localizarea fisierelor implicite a bazei de date

        ui->tableViewLogs->setVisible(true);
        ui->textLog->setVisible(true);

    } else if (_index == idx_Sqlite){ //sqlite

        ui->tabLogs->setEnabled(true);
        ui->tabSqlite->setEnabled(true);
        ui->tabMySQL->setEnabled(false);
        ui->txtPathImage->setEnabled(true);
        ui->tabConnections->setCurrentIndex(0);

        setDefaultPathSqlite(); // setam localizarea fisierelor implicite a bazei de date sqlite

        ui->tableViewLogs->setVisible(true);
        ui->textLog->setVisible(true);

    } else {

        ui->tabLogs->setEnabled(false);
        ui->tabSqlite->setEnabled(false);
        ui->tabMySQL->setEnabled(false);
        ui->txtPathImage->setEnabled(false);
        ui->tabConnections->setCurrentIndex(0);

    }

    dataWasModified();
}

void AppSettings::changeIndexUnitMeasure(const int _index)
{
    if (_index >= 0 && _index <= 1)
        dataWasModified();
}

// *******************************************************************
// **************** OTHER ********************************************

void AppSettings::createNewBaseSqlite()
{
    if (!checkDataSettings())
        return;

    QString txtMSG; // pentru mesaj/debug
    DataBase database;
    // Aici se creează numai fișierul și se verifică dacă poate fi deschis.
    // Schema este inițializată într-un singur loc, în AppController, după ce
    // utilizatorul confirmă setările primei lansări.
    const bool created = database.createConnectBaseSqlite(ui->nameBaseSqlite->text(),
                                                           lineEditPathDBSqlite->text(),
                                                           false,
                                                           txtMSG);

    if (created) {
        QMessageBox::information(this,
                                 tr("Crearea bazei de date (.sqlite)"),
                                 txtMSG,
                                 QMessageBox::Ok);
    } else {
        QMessageBox::critical(this,
                              tr("Crearea bazei de date (.sqlite)"),
                              txtMSG,
                              QMessageBox::Ok);
    }
}

void AppSettings::onTestConnectionMySQL()
{
    if (!checkDataSettings())
        return;

    const QString connectionName = QStringLiteral("app_settings_test_%1")
                                       .arg(reinterpret_cast<quintptr>(this));
    bool connected = false;
    QString connectionError;
    {
        QSqlDatabase testDb = QSqlDatabase::addDatabase("QMYSQL", connectionName);
        testDb.setHostName(ui->mySQLhost->text());
        testDb.setDatabaseName(ui->mySQLnameBase->text());
        testDb.setPort(ui->mySQLport->text().toInt());
        testDb.setConnectOptions(ui->mySQLoptionConnect->text());
        testDb.setUserName(ui->mySQLuser->text());
        testDb.setPassword(ui->mySQLpasswdUser->text());
        connected = testDb.open();
        if (!connected)
            connectionError = testDb.lastError().text();
        testDb.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (connected){
        QMessageBox::information(this,
                                 tr("Testarea conect\304\203rii"),
                                 tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> este realizat\304\203 cu succes.")
                                     .arg(ui->mySQLnameBase->text()),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::warning(this,
                             tr("Testarea conect\304\203rii"),
                             tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> lipse\310\231te.<br><br>%2")
                                 .arg(ui->mySQLnameBase->text(), connectionError.toHtmlEscaped()),
                             QMessageBox::Ok);
    }
}

void AppSettings::setPathAppSettings()
{
    QDir file_path;
    if (ui->comboBoxTypeSQL->currentIndex() == idx_MySQL){
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->mySQLnameBase->text() + ".conf"));
        ui->txtPathLog->setText(file_path.toNativeSeparators(dirLogPath + "/" + ui->mySQLnameBase->text() + ".log"));
    } else if (ui->comboBoxTypeSQL->currentIndex() == idx_Sqlite) {
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->nameBaseSqlite->text() + ".conf"));
        ui->txtPathLog->setText(file_path.toNativeSeparators(dirLogPath + "/" + ui->nameBaseSqlite->text() + ".log"));
    }
}

void AppSettings::selectedLevelLog(const QString level)
{
    if (level == "Filtrare [ALL]")
        updateTableLog(QString(), QStringList());
    if (level == "Filtrare [THREAD]")
        updateTableLog("THREAD", QStringList());
    if (level == "Filtrare [SYNC]")
        updateTableLog("SYNC", QStringList());
    if (level == "Filtrare [INFO]")
        updateTableLog("INF", {"THREAD", "SYNC"});
    if (level == "Filtrare [CRITICAL]")
        updateTableLog("CRT", {"THREAD", "SYNC"});
    if (level == "Filtrare [WARNING]")
        updateTableLog("WRN", {"THREAD", "SYNC"});
    if (level == "Filtrare [DEBUG]")
        updateTableLog("DBG", {"THREAD", "SYNC"});
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void AppSettings::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        const QMessageBox::StandardButton answer =
            QMessageBox::warning(this, tr("Modificarea datelor"),
                                 tr("Set\304\203rile au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            if (checkDataSettings() && saveSettings()) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (answer == QMessageBox::No) {
            restoreSettingsState();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void AppSettings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Setările aplicației %1").arg("[*]"));
        //traducem unitatea de masura
        const int indexMeasure = ui->comboBoxUnitMeasure->currentIndex(); // memorizam
        const QSignalBlocker unitMeasureBlocker(ui->comboBoxUnitMeasure);
        ui->comboBoxUnitMeasure->clear();
        ui->comboBoxUnitMeasure->addItems(QStringList() << tr("milimetru") << tr("centimetru"));
        ui->comboBoxUnitMeasure->setCurrentIndex(indexMeasure); // instalam indexul memorizat
    }
}
