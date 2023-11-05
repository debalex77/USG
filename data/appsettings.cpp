#include "appsettings.h"
#include "ui_appsettings.h"
#include <string.h>

static const int width_textLog = 915;  // inaltimea si latimea tabelei 'textLog'
static const int height_textLog = 430; // sa fie fixat la lansarea setarilor

AppSettings::AppSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppSettings)
{
    ui->setupUi(this);

    setWindowTitle(tr("Setările aplicației %1").arg("[*]")); // setam titlu

    popUp      = new PopUp();       // alocam memoria p-u mesaje
    translator = new QTranslator(); // alocam memoria p-u translator
    db         = new DataBase();
    model_logs = new QStandardItemModel(this);

    ui->numberLogFile->setValue(10);

#if defined(Q_OS_LINUX)

    if (! QFile(dirLogPath).exists()){
        QMessageBox::information(this, tr("Crearea directoriului de logare"),
                                 tr("Pentru crearea directoriului de logare a aplicației este necesar accesul administrativ."),
                                 QMessageBox::Ok);
        QString str_cmd = "pkexec mkdir /" NAME_DIR_LOG_PATH " && pkexec chmod 777 /" NAME_DIR_LOG_PATH " && touch /" NAME_FILE_LOG_PATH;
        system(str_cmd.toStdString().c_str()); // pkexec mkdir /var/log/usg && pkexec chmod 777 /var/log/usg && touch /var/log/usg/usg.log
        globals::pathLogAppSettings = fileLogPath;
    } else {
        processingLoggingFiles();
        globals::pathLogAppSettings = fileLogPath;
    }

    if (! QFile(globals::pathAppSettings).exists())
        globals::unknowModeLaunch = true; // globals::numSavedFilesLog

#elif defined(Q_OS_MACOS)

    if (! QFile(fileLogPath).exists()){
        QString str_cmd = "touch " + fileLogPath;
        system(str_cmd.toStdString().c_str());
        globals::pathLogAppSettings = fileLogPath;
    } else {
        processingLoggingFiles();
        globals::pathLogAppSettings = fileLogPath;
    }

    if (! QFile(globals::pathAppSettings).exists())
        globals::unknowModeLaunch = true; // globals::numSavedFilesLog

#elif defined(Q_OS_WIN)

    QDir dir;
    //********************************************
    // usg.log
    if (! QDir(dir.toNativeSeparators(dirLogPath)).exists()){
        if (! QDir().mkdir(dir.toNativeSeparators(dirLogPath))){
            QMessageBox::warning(this, tr("Crearea directoriului de logare"),
                                 tr("Directoria 'config' pentru păstrarea fișierelor de logare a aplicației nu a fost creată !!!<br>"
                                    "Adresați-vă administratorului aplicației."),
                                 QMessageBox::Ok);
            return;
        }
    } else {
        processingLoggingFiles();
    }

    //********************************************
    // setam variabile globale si alocam memoria pu
    // setarile aplicatiei
    globals::pathLogAppSettings = dir.toNativeSeparators(fileLogPath);

    //********************************************
    // setam variabile globale
    if (! QFile(globals::pathAppSettings).exists())
        globals::unknowModeLaunch = true;


#endif

    initBtnForm(); // initializam butoanele formei

    // setam combo...
    ui->comboBoxLangApp->addItems(QStringList() << "ru-RU" << "ro-RO");
    ui->comboBoxTypeSQL->addItems(QStringList() << tr("<- Alege ->") << "MySQL(MariaDB)" << "Sqlite");
    ui->comboBoxUnitMeasure->addItems(QStringList() << tr("milimetru") << tr("centimetru"));

    ui->tabSqlite->setEnabled(false);
    ui->tabMySQL->setEnabled(false);

    setLanguageApp(); // setam limba aplicatiei
    setDefaultPath(); // setam localizarea fisierelor
    changeIndexTypeSQL(idx_Unknow);

    initConnections(); // connectari

    ui->textLog->resize(width_textLog, height_textLog);
}

AppSettings::~AppSettings()
{
    delete translator;
    delete settApp;
    delete db;
    delete popUp;
    delete lineEditPathDBImage;
    delete lineEditPathDBSqlite;
    delete lineEditPathLog;
    delete lineEditPathSettings;
    delete lineEditPathReports;
    delete lineEditPathTemplatesPrint;
    delete btnAdd;
    delete btnEdit;
    delete btnClear;
    delete btnOpenSettings;
    delete btnRemoveSettings;
    delete btnOpenLog;
    delete btnRemoveLog;
    delete btnAddImage;
    delete btnEditImage;
    delete btnClearImage;
    delete model_logs;
    delete ui;
}

// *******************************************************************
// **************** INITIEREA SI SETAREA DATELOR *********************

void AppSettings::initBtnForm()
{
    QFile fileStyleBtn(":/styles/style_btn.css");
    fileStyleBtn.open(QFile::ReadOnly);
    QString appStyleBtn(fileStyleBtn.readAll());

    initBtnSettingsApp(appStyleBtn);
    initBtnLogApp(appStyleBtn);
    initBtnMainBase(appStyleBtn);
    initBtnImageBase(appStyleBtn);
    initBtnDirTemplets(appStyleBtn);
    initBtnDirReports(appStyleBtn);
}

void AppSettings::initBtnSettingsApp(const QString appStyleBtn)
{
    btnOpenSettings = new QToolButton(this);
    btnOpenSettings->setIcon(QIcon(":/img/open-search.png"));
    btnOpenSettings->setStyleSheet(appStyleBtn);
    btnOpenSettings->setCursor(Qt::PointingHandCursor);
    btnOpenSettings->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveSettings = new QToolButton(this);
    btnRemoveSettings->setIcon(QIcon(":/img/trash.png"));
    btnRemoveSettings->setStyleSheet(appStyleBtn);
    btnRemoveSettings->setCursor(Qt::PointingHandCursor);
    btnRemoveSettings->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathSettings = new QLineEdit(this);
    lineEditPathSettings->setStyleSheet(appStyleBtn);
    lineEditPathSettings->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathSettings = new QHBoxLayout;
    layoutPathSettings->setContentsMargins(0, 0, 0, 0);
    layoutPathSettings->setSpacing(0);
    layoutPathSettings->addWidget(lineEditPathSettings);
    layoutPathSettings->addWidget(btnOpenSettings);
    layoutPathSettings->addWidget(btnRemoveSettings);
    ui->txtPathAppSettings->setLayout(layoutPathSettings);

    connect(btnOpenSettings, &QToolButton::clicked, this, &AppSettings::openFileSettingsApp);
    connect(btnRemoveSettings, &QToolButton::clicked, this, &AppSettings::removeFileSettingsApp);
}

void AppSettings::initBtnLogApp(const QString appStyleBtn)
{
    btnOpenLog = new QToolButton(this);
    btnOpenLog->setIcon(QIcon(":/img/open-search.png"));
    btnOpenLog->setStyleSheet(appStyleBtn);
    btnOpenLog->setCursor(Qt::PointingHandCursor);
    btnOpenLog->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveLog = new QToolButton(this);
    btnRemoveLog->setIcon(QIcon(":/img/trash.png"));
    btnRemoveLog->setStyleSheet(appStyleBtn);
    btnRemoveLog->setCursor(Qt::PointingHandCursor);
    btnRemoveLog->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    lineEditPathLog = new QLineEdit(this);
    lineEditPathLog->setStyleSheet(appStyleBtn);
    lineEditPathLog->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    QHBoxLayout* layoutPathLog = new QHBoxLayout;
    layoutPathLog->setContentsMargins(0, 0, 0, 0);
    layoutPathLog->setSpacing(0);
    layoutPathLog->addWidget(lineEditPathLog);
    layoutPathLog->addWidget(btnOpenLog);
    layoutPathLog->addWidget(btnRemoveLog);
    ui->txtPathLog->setLayout(layoutPathLog);

    connect(btnOpenLog, &QToolButton::clicked, this, &AppSettings::openFileCurrentLogApp);
    connect(btnRemoveLog, &QToolButton::clicked, this, &AppSettings::removeFileCurrentLogApp);
}

void AppSettings::initBtnMainBase(const QString appStyleBtn)
{
    btnAdd = new QToolButton(this);
    btnAdd->setIcon(QIcon(":/img/add_x32.png"));
    btnAdd->setStyleSheet(appStyleBtn);
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnEdit = new QToolButton(this);
    btnEdit->setIcon(QIcon(":/img/edit_x32.png"));
    btnEdit->setStyleSheet(appStyleBtn);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnClear = new QToolButton(this);
    btnClear->setIcon(QIcon(":/img/clear_x32.png"));
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

    connect(btnAdd, &QAbstractButton::clicked, this, &AppSettings::onAddPathSqlite);
    connect(btnEdit, &QAbstractButton::clicked, this, &AppSettings::onEditPathSqlite);
    connect(btnClear, &QAbstractButton::clicked, this, &AppSettings::onClearPathSqlite);
}

void AppSettings::initBtnImageBase(const QString appStyleBtn)
{
    btnAddImage = new QToolButton(this);
    btnAddImage->setIcon(QIcon(":/img/add_x32.png"));
    btnAddImage->setStyleSheet(appStyleBtn);
    btnAddImage->setCursor(Qt::PointingHandCursor);
    btnAddImage->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnEditImage = new QToolButton(this);
    btnEditImage->setIcon(QIcon(":/img/edit_x32.png"));
    btnEditImage->setStyleSheet(appStyleBtn);
    btnEditImage->setCursor(Qt::PointingHandCursor);
    btnEditImage->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnClearImage = new QToolButton(this);
    btnClearImage->setIcon(QIcon(":/img/clear_x32.png"));
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

    connect(btnAddImage, &QAbstractButton::clicked, this, &AppSettings::onAddPathDBImage);
    connect(btnEditImage, &QAbstractButton::clicked, this, &AppSettings::onEditPathDBImage);
    connect(btnClearImage, &QAbstractButton::clicked, this, &AppSettings::onClearPathDBImage);
}

void AppSettings::initBtnDirTemplets(const QString appStyleBtn)
{
    btnOpenDirTemplates = new QToolButton(this);
    btnOpenDirTemplates->setIcon(QIcon(":/img/folder.png"));
    btnOpenDirTemplates->setStyleSheet(appStyleBtn);
    btnOpenDirTemplates->setCursor(Qt::PointingHandCursor);
    btnOpenDirTemplates->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirTemplates = new QToolButton(this);
    btnRemoveDirTemplates->setIcon(QIcon(":/img/trash.png"));
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

    connect(btnOpenDirTemplates, &QAbstractButton::clicked, this , &AppSettings::openDirTemplets);
    connect(btnRemoveDirTemplates, &QAbstractButton::clicked, this, [this]()
    {
        lineEditPathTemplatesPrint->clear();
    });
}

void AppSettings::initBtnDirReports(const QString appStyleBtn)
{
    btnOpenDirReports = new QToolButton(this);
    btnOpenDirReports->setIcon(QIcon(":/img/folder.png"));
    btnOpenDirReports->setStyleSheet(appStyleBtn);
    btnOpenDirReports->setCursor(Qt::PointingHandCursor);
    btnOpenDirReports->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirReports = new QToolButton(this);
    btnRemoveDirReports->setIcon(QIcon(":/img/trash.png"));
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

    connect(btnOpenDirReports, &QAbstractButton::clicked, this , &AppSettings::openDirReports);
    connect(btnRemoveDirReports, &QAbstractButton::clicked, this, [this]()
    {
        lineEditPathReports->clear();
    });
}

void AppSettings::initConnections()
{
    connect(lineEditPathDBSqlite, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathDBImage, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

    connect(ui->tabView, QOverload<int>::of(&QTabWidget::currentChanged),
            this, QOverload<int>::of(&AppSettings::slot_currentIndexChangedTab));

    connect(ui->tableViewLogs, QOverload<const QModelIndex&>::of(&QTableView::pressed),
            this, &AppSettings::slot_clickedTableLogs);

    // conectarea la modificarea limbei aplicatiei
    connect(this, &AppSettings::changeLanguage, this, &AppSettings::setLanguageApp);

    // conectari combo...
    connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
    connect(ui->comboBoxTypeSQL, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexTypeSQL));
    connect(ui->comboBoxUnitMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexUnitMeasure));

    // modificarea datelor conectarii sqlite
    connect(ui->nameBaseSqlite, &QLineEdit::editingFinished, this, &AppSettings::changeNameBaseSqlite);
    connect(lineEditPathDBSqlite, &QLineEdit::editingFinished, this, &AppSettings::changePathBaseSqlite);

    // modificarea datelor conectarii MySQL
    connect(ui->mySQLhost, &QLineEdit::editingFinished, this, &AppSettings::changeNameHostMySQL);
    connect(ui->mySQLnameBase, &QLineEdit::editingFinished, this, &AppSettings::changeNameBaseMySQL);
    connect(ui->mySQLport, &QLineEdit::editingFinished, this, &AppSettings::changePortConectionMySQL);
    connect(ui->mySQLoptionConnect, &QLineEdit::editingFinished, this, &AppSettings::changeConnectOptionsMySQL);
    connect(ui->mySQLuser, &QLineEdit::editingFinished, this, &AppSettings::changeUserNameMySQL);
    connect(ui->mySQLpasswdUser, &QLineEdit::editingFinished, this, &AppSettings::changePasswdMySQL);

    // setam drumul spre setarile aplicatiei
    connect(ui->nameBaseSqlite, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings);
    connect(ui->mySQLnameBase, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings);
    connect(ui->nameBaseSqlite, &QLineEdit::editingFinished, this, &AppSettings::setPathGlobalVariableAppSettings);
    connect(ui->mySQLnameBase, &QLineEdit::editingFinished, this, &AppSettings::setPathGlobalVariableAppSettings);

    // testarea conectarii la BD
    connect(ui->btnCreateNewBaseSqlite, &QAbstractButton::clicked, this, &AppSettings::createNewBaseSqlite);
    connect(ui->btnTestConnectionMySQL, &QAbstractButton::clicked, this, &AppSettings::onTestConnectionMySQL);

    // main btn form
    connect(ui->btnOK, &QAbstractButton::clicked, this, &AppSettings::onBtnOKSettings);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &AppSettings::onBtnWriteSettings);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &AppSettings::onBtnCancelSettings);
}

// *******************************************************************
// **************** SETAREA DATELOR **********************************

void AppSettings::setLanguageApp()
{
    if (globals::firstLaunch) {

        if (globals::langApp == nullptr){
            // determinam locale
            QString nameLocale = QLocale::system().name();
            nameLocale.replace(QString("_"), QString("-")); // schimbam simbolul

            globals::langApp = nameLocale;
            int indexLangApp = (nameLocale == "ru_RU" || nameLocale == "ru-RU") ? 0 : 1;
            disconnect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
            ui->comboBoxLangApp->setCurrentIndex(indexLangApp);
            connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
        } else {
            disconnect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
            ui->comboBoxLangApp->setCurrentIndex((globals::langApp == "ro_RO" || globals::langApp == "ro-RO") ? 1 : 0);
            connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
        }
        // traducem aplicatia
        if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
            qApp->installTranslator(translator);
        }
    } else if (globals::unknowModeLaunch){
        if (globals::langApp == nullptr){
            QString nameLocale = globals::langApp;
            int indexLangApp = (nameLocale == "ru_RU" || nameLocale == "ru-RU") ? 0 : 1;
            disconnect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
            ui->comboBoxLangApp->setCurrentIndex(indexLangApp);
            connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
        }
        // traducem aplicatia
        if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
            qApp->installTranslator(translator);
        }
    } else {
        if (!globals::langApp.isEmpty() && globals::langApp != nullptr){
            QString nameLocale = globals::langApp;
            int indexLangApp = (nameLocale == "ru_RU" || nameLocale == "ru-RU") ? 0 : 1;
            disconnect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
            ui->comboBoxLangApp->setCurrentIndex(indexLangApp);
            connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
                       this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
            // traducem aplicatia
            if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
                qApp->installTranslator(translator);
            }
        }
    }
}

void AppSettings::setDefaultPath()
{
    if (! globals::pathTemplatesDocs.isEmpty() && ! globals::pathReports.isEmpty())
        return;

    QDir dir;

    if (! lineEditPathSettings->text().isEmpty())
        lineEditPathSettings->setText(globals::pathAppSettings);
    lineEditPathLog->setText(globals::pathLogAppSettings);

    //******************************************************************************
    // setam localizarea sabloanelor de tipar si a rapoartelor
#if defined(Q_OS_LINUX)
    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators("/opt/USG/templets"));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.toNativeSeparators("/opt/USG/templets/reports"));
    globals::pathReports = lineEditPathReports->text();
#elif defined(Q_OS_MACOS)
    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators(dir.currentPath() + "/templets"));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.toNativeSeparators(dir.currentPath() + "/templets/reports"));
    globals::pathReports = lineEditPathReports->text();
#elif defined(Q_OS_WINDOWS)
    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators(dir.currentPath() + "/templets_win"));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.toNativeSeparators(dir.currentPath() + "/templets_win/reports"));
    globals::pathReports = lineEditPathReports->text();
#endif
}

void AppSettings::setDefaultPathSqlite()
{
    if (! globals::firstLaunch)
        return;

    QDir dir;

#if defined(Q_OS_LINUX)

    //******************************************************************************
    // crearea fisierelor bazelor de date si setarea
    QString str_dir_database = dir.toNativeSeparators(dir.homePath() + "/Database_usg");
    QString str_file_database = str_dir_database + "/base.sqlite3";
    QString str_file_database_image = str_dir_database + "/base_image.sqlite3";

#elif defined(Q_OS_MACOS)

    //******************************************************************************
    // crearea fisierelor bazelor de date si setarea
    QString str_dir_database = dir.toNativeSeparators(dir.homePath() + "/Database_usg");
    QString str_file_database = str_dir_database + "/base.sqlite3";
    QString str_file_database_image = str_dir_database + "/base_image.sqlite3";

#elif defined(Q_OS_WINDOWS)

    //******************************************************************************
    // crearea fisierelor bazelor de date si setam variabel globale
    QString str_dir_database        = dir.toNativeSeparators(dir.rootPath()) + "Database_usg";
    QString str_file_database       = str_dir_database + "\\base.sqlite3";
    QString str_file_database_image = str_dir_database + "\\base_image.sqlite3";

#endif
    if (! QFile(str_dir_database).exists()){
        if (QDir().mkdir(str_dir_database)){

            //---- baza principale
            lineEditPathDBSqlite->setText(str_file_database);                     // baza principala
            ui->nameBaseSqlite->setText("base");                                  // denumirea bazei de date
            globals::sqlitePathBase = dir.toNativeSeparators(str_file_database);  // variabile globale
            globals::sqliteNameBase = ui->nameBaseSqlite->text();

            //---- baza cu imagini
            lineEditPathDBImage->setText(str_file_database_image);         // baza de date cu imagini
            globals::pathImageBaseAppSettings = str_file_database_image;
        } else {
            QMessageBox::warning(this, tr("Crearea directoriei"),
                                 tr("Directoria <b>'%1'</b><br>"
                                    "pentru baza de date SQlite nu a fost creată !!! Lansarea aplicației nu este posibilă.<br>"
                                    "Adresați-vă administratorului aplicației.").arg(str_dir_database),
                                QMessageBox::Ok);
            qCritical(logCritical()) << tr("Directoria '%1' pentru baza de date SQlite nu a fost creată.").arg(str_dir_database);
        }
    } else {
        if (QFile(str_file_database).exists()){
            QMessageBox messange_box(QMessageBox::Question,
                                     tr("Determinarea existenței bazei de date"),
                                     tr("Baza de date '<u>%1</u>' există în sistem !!!<br><br>"
                                        "Pentru crearea bazei de date noi cu nume '<b><u>%2</u></b>' este necesar de eliminat fișierul vechi. "
                                        "Doriți să eliminați fișierul din sistem ?").arg(str_file_database, (ui->nameBaseSqlite->text().isEmpty()) ? "base" : ui->nameBaseSqlite->text()),
                                     QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
            messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//            messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//            messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
            if (messange_box.exec() == QMessageBox::Yes){
#if defined(Q_OS_LINUX)
                QString str_cmd = "rm \"" + str_file_database + "\"";
                system(str_cmd.toStdString().c_str());
                str_cmd = "rm \"" + str_file_database_image + "\"";
                system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_MACOS)
                QString str_cmd = "rm \"" + str_file_database + "\"";
                system(str_cmd.toStdString().c_str());
                str_cmd = "rm \"" + str_file_database_image + "\"";
                system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_WIN)
                QString str_cmd = "del " + str_file_database;
                system(str_cmd.toStdString().c_str());
#endif
            } else {
                return;
            }
        }
        ui->nameBaseSqlite->setText("base");
        lineEditPathDBSqlite->setText(str_file_database);
        globals::sqlitePathBase = dir.toNativeSeparators(str_file_database);  // variabile globale
        globals::sqliteNameBase = ui->nameBaseSqlite->text();

        lineEditPathDBImage->setText(str_file_database_image);
        globals::pathImageBaseAppSettings = str_file_database_image;
    }
}

bool AppSettings::checkDataSettings()
{
    if (ui->comboBoxTypeSQL->currentIndex() == idx_Unknow){
        QMessageBox::warning(this,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat \"<b>%1</b>\" !!!").arg(tr("Tipul bazei de date")),
                                 QMessageBox::Ok);
        return false;
    }

    // MySQL
    if (ui->comboBoxTypeSQL->currentIndex() == idx_MySQL){
        if (ui->mySQLhost->text().isEmpty()){ // numele hostului (MySQL)
            QMessageBox::warning(this,
                                     tr("Verificarea datelor"),
                                     tr("Nu este indicat \"<b>%1</b>\" !!!.")
                                     .arg(tr("Numele hostului")),
                                     QMessageBox::Ok);
            return false;
        }
        if (ui->mySQLnameBase->text().isEmpty()){ // denumirea bazei de date (MySQL)
            QMessageBox::warning(this,
                                     tr("Verificarea datelor"),
                                     tr("Nu este indicată \"<b>%1</b>\" !!!.")
                                     .arg(tr("Denumirea bazei de date")),
                                     QMessageBox::Ok);
            return false;
        }
        if (ui->mySQLuser->text().isEmpty()){ // utilizatorul (MySQL)
            QMessageBox::warning(this,
                                     tr("Verificarea datelor"),
                                     tr("Nu este indicat \"<b>%1</b>\" !!!.")
                                     .arg(tr("Utilizatorul")),
                                     QMessageBox::Ok);
            return false;
        }
    }

    // Sqlite
    if (ui->comboBoxTypeSQL->currentIndex() == idx_Sqlite){
        if (ui->nameBaseSqlite->text().isEmpty()){ // denumirea bazei de date (sqlite)
            QMessageBox::warning(this,
                                     tr("Verificarea datelor"),
                                     tr("Nu este indicat \"<b>%1</b>\" !!!")
                                     .arg(tr("Denumirea bazei de date (.sqlite3)")),
                                     QMessageBox::Ok);
            return false;
        }

        if (lineEditPathDBSqlite->text().isEmpty()){ // localizarea bazei de date (sqlite)
            QMessageBox::warning(this,
                                     tr("Verificarea datelor"),
                                     tr("Nu este indicată \"<b>%1</b>\" !!!")
                                     .arg(tr("Localizarea bazei de date (.sqlite3)")),
                                     QMessageBox::Ok);
            return false;
        }
    }

    return true;

}

// *******************************************************************
// **************** EXTRAGEREA SI SALVAREA DATELOR *******************

void AppSettings::readSettings()
{
    loadSettings(); // citim din nou datele din fisierul 'settings.conf'

    disconnect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
    disconnect(ui->comboBoxTypeSQL, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexTypeSQL));
    disconnect(ui->comboBoxUnitMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexUnitMeasure));

    disconnect(lineEditPathDBSqlite, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathDBImage, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

    disconnect(ui->nameBaseSqlite, &QLineEdit::editingFinished, this, &AppSettings::changeNameBaseSqlite);
    disconnect(lineEditPathDBSqlite, &QLineEdit::editingFinished, this, &AppSettings::changePathBaseSqlite);

    ui->comboBoxLangApp->setCurrentIndex((globals::langApp == "ru-RU") ? 0 : 1);
    ui->comboBoxTypeSQL->setCurrentIndex(globals::indexTypeSQL);
    changeIndexTypeSQL(globals::indexTypeSQL);
    ui->comboBoxUnitMeasure->setCurrentText(globals::unitMeasure);

    if (ui->txtPathAppSettings->text().isEmpty() && ! globals::pathAppSettings.isEmpty()) // setam din variabila globala
        ui->txtPathAppSettings->setText(globals::pathAppSettings);

    if (globals::connectionMade == "MySQL"){

        ui->tabSqlite->setEnabled(false);  // accesarea tabului
        ui->tabMySQL->setEnabled(true);

        ui->mySQLhost->setText(globals::mySQLhost);

        disconnect(ui->mySQLnameBase, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings); // deconectam la setarea programata
        ui->mySQLnameBase->setText(globals::mySQLnameBase);
        connect(ui->mySQLnameBase, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings);    // conectarea la modificarea textului

        ui->mySQLport->setText(globals::mySQLport);
        ui->mySQLoptionConnect->setText(globals::mySQLoptionConnect);
        ui->mySQLuser->setText(globals::mySQLuser);
        ui->mySQLpasswdUser->setText(globals::mySQLpasswdUser);

    } else if (globals::connectionMade == "Sqlite"){

        ui->tabSqlite->setEnabled(true);
        ui->tabMySQL->setEnabled(false);

        disconnect(ui->nameBaseSqlite, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings); // deconectam la setarea programata
        ui->nameBaseSqlite->setText(globals::sqliteNameBase);
        connect(ui->nameBaseSqlite, &QLineEdit::textChanged, this, &AppSettings::setPathAppSettings);    // conectarea la modificarea textului

        lineEditPathDBSqlite->setText(globals::sqlitePathBase);
        lineEditPathDBImage->setText(globals::pathImageBaseAppSettings);

    }

    lineEditPathLog->setText(globals::pathLogAppSettings);   
    lineEditPathTemplatesPrint->setText(globals::pathTemplatesDocs);
    lineEditPathReports->setText(globals::pathReports);

    ui->numberLogFile->setValue(globals::numSavedFilesLog);

    // conectari combo...
    connect(ui->comboBoxLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexLangApp));
    connect(ui->comboBoxTypeSQL, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexTypeSQL));
    connect(ui->comboBoxUnitMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, QOverload<int>::of(&AppSettings::changeIndexUnitMeasure));

    connect(lineEditPathDBSqlite, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathDBImage, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

    connect(ui->nameBaseSqlite, &QLineEdit::editingFinished, this, &AppSettings::changeNameBaseSqlite);
    connect(lineEditPathDBSqlite, &QLineEdit::editingFinished, this, &AppSettings::changePathBaseSqlite);
}

void AppSettings::saveSettings()
{
    QDir dir_conf;

    globals::pathAppSettings = dir_conf.toNativeSeparators(ui->txtPathAppSettings->text());

    settApp = new QSettings(globals::pathAppSettings, QSettings::IniFormat, this);

    settApp->beginGroup("index_init");
    settApp->setValue("indexLangApp",     ui->comboBoxLangApp->currentIndex());
    settApp->setValue("indexTypeSQL",     ui->comboBoxTypeSQL->currentIndex());
    settApp->setValue("indexUnitMeasure", ui->comboBoxUnitMeasure->currentIndex());
    settApp->endGroup();

    settApp->beginGroup("path_app");
    settApp->setValue("pathTemplatesDocs", lineEditPathTemplatesPrint->text());
    settApp->setValue("pathReports",       lineEditPathReports->text());
    settApp->endGroup();

    settApp->beginGroup("connect");
    if (globals::connectionMade == "MySQL"){
        settApp->setValue("MySQL_host",           db->encode_string(ui->mySQLhost->text()));
        settApp->setValue("MySQL_name_base",      db->encode_string(ui->mySQLnameBase->text()));
        settApp->setValue("MySQL_port",           ui->mySQLport->text());
        settApp->setValue("MySQL_user",           db->encode_string(ui->mySQLuser->text()));
        settApp->setValue("MySQL_passwd_user",    db->encode_string(ui->mySQLpasswdUser->text()));
        settApp->setValue("MySQL_option_connect", ui->mySQLoptionConnect->text());
    } else if (globals::connectionMade == "Sqlite") {
        settApp->setValue("sqliteNameBase", ui->nameBaseSqlite->text());
        settApp->setValue("sqlitePathBase", lineEditPathDBSqlite->text());
        settApp->setValue("pathDBImage",    lineEditPathDBImage->text());
        settApp->setValue("pathLogApp",     lineEditPathLog->text());
    }
    settApp->endGroup();

    settApp->beginGroup("on_start");
    settApp->setValue("numSavedFilesLog", ui->numberLogFile->value());
    settApp->endGroup();

    qInfo(logInfo()) << tr("Setările aplicației sunt salvate/modificate în fișierul - %1.").arg(globals::pathAppSettings);
}

void AppSettings::loadDataFromTableSettingsUsers()
{
    db->updateVariableFromTableSettingsUser();
}

void AppSettings::loadSettings()
{
    settApp = new QSettings(globals::pathAppSettings, QSettings::IniFormat, this);

    settApp->beginGroup("index_init");
    globals::langApp      = (settApp->value("indexLangApp", 1).toInt() == 0) ? "ru-RU" : "ro-RO";
    globals::indexTypeSQL = settApp->value("indexTypeSQL", idx_Unknow).toInt();
    globals::unitMeasure  = (settApp->value("indexUnitMeasure", 0).toInt() == 0) ? "mm" : "cm";
    settApp->endGroup();

    settApp->beginGroup("path_app");
    globals::pathTemplatesDocs  = settApp->value("pathTemplatesDocs", "").toString();
    globals::pathReports        = settApp->value("pathReports", "").toString();
    settApp->endGroup();

    settApp->beginGroup("connect");
    if (globals::indexTypeSQL == idx_MySQL){
        globals::thisMySQL  = true;
        globals::thisSqlite = false;
        globals::connectionMade = "MySQL";

        globals::mySQLhost          = db->decode_string(settApp->value("MySQL_host", "").toString());
        globals::mySQLnameBase      = db->decode_string(settApp->value("MySQL_name_base", "").toString());
        globals::mySQLport          = settApp->value("MySQL_port", "3306").toString();
        globals::mySQLoptionConnect = settApp->value("MySQL_option_connect", "").toString();
        globals::mySQLuser          = db->decode_string(settApp->value("MySQL_user", "").toString());
        globals::mySQLpasswdUser    = db->decode_string(settApp->value("MySQL_passwd_user", "").toString());
    } else if (globals::indexTypeSQL == idx_Sqlite){
        globals::thisMySQL  = false;
        globals::thisSqlite = true;
        globals::connectionMade = "Sqlite";

        globals::pathImageBaseAppSettings = settApp->value("pathDBImage", "").toString();
        globals::pathLogAppSettings       = settApp->value("pathLogApp", fileLogPath).toString();
        globals::sqliteNameBase           = settApp->value("sqliteNameBase", "").toString();
        globals::sqlitePathBase           = settApp->value("sqlitePathBase", "").toString();
    } else {
        globals::thisMySQL  = false;
        globals::thisSqlite = false;
        globals::connectionMade = nullptr;
    }
    settApp->endGroup();

    settApp->beginGroup("on_start");
    globals::idUserApp        = db->decode_string(settApp->value("idUserApp", "").toString()).toInt();
    globals::nameUserApp      = db->decode_string(settApp->value("nameUserApp", "").toString());
    globals::memoryUser       = settApp->value("memoryUser", false).toBool();
    globals::numSavedFilesLog = settApp->value("numSavedFilesLog", 10).toInt();  // determinam nr.fisierelor pu pastrare
    removeFilesLogOnStartApp();                                                  // eliminarea din sistem
    settApp->endGroup();

    globals::firstLaunch = false;

    // traducem aplicatia
    if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        qApp->installTranslator(translator);
    }

    if (! db->getDatabase().open()){
        if(! db->connectToDataBase()){
            QMessageBox::warning(this,
                                  tr("Conectarea la baza de date"),
                                  tr("Conectarea la baza de date <b><u>%1</u></b> lipsește !!! <br><br>Adresați-vă administratorului aplicației.")
                                         .arg((globals::thisMySQL) ? ("host: " + globals::mySQLhost + ", base: " + globals::mySQLnameBase) : globals::sqliteNameBase),
                                     QMessageBox::Ok);
            qWarning(logWarning()) << tr("%1: loadSettings()<br> Nu a fost efectuata conectarea la BD '%2'")
                                      .arg(metaObject()->className(), globals::sqliteNameBase);
            qApp->quit();
        }
    }

    loadDataFromTableSettingsUsers();
}

// *******************************************************************
// **************** SETAREA DATELOR - KEY & VALUE ********************

void AppSettings::setBeginGroup(const QString nameGroup)
{
    settApp->beginGroup(nameGroup);
}

void AppSettings::setEndGroup()
{
    settApp->endGroup();
}

void AppSettings::setKeyAndValue(const QString nameGroup, const QString &_key, const QVariant &_value)
{
    settApp = new QSettings(globals::pathAppSettings, QSettings::IniFormat, this);

    if (_key.isEmpty())
        return;

    if (nameGroup != nullptr){
        settApp->beginGroup(nameGroup);
    }

    settApp->setValue(_key, _value);

    if (nameGroup != nullptr){
        settApp->endGroup();
    }
}

QVariant AppSettings::getValuesSettings(const QString nameGroup, const QString &_key, const QVariant &_defaultValue)
{
    if (nameGroup != nullptr){
        settApp->beginGroup(nameGroup);
    }

    return settApp->value(_key, _defaultValue);

    if (nameGroup != nullptr){
        settApp->endGroup();
    }
}

// *******************************************************************
// **************** PROCESAREA LOG-FISIERELOR ************************

void AppSettings::processingLoggingFiles()
{
    QDir dir(dirLogPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();

    QDate mCurrentDate = QDate::currentDate(); // variabila pu determinarea datei curente

    for (int n = 0; n < listFiles.size(); n++) {

        QFileInfo fileInfo = listFiles.at(n);

#if defined(Q_OS_LINUX)

        // verificam data curenta si ultima modificare a fisierului
        // copierea fisierului si redenumirea cu atribuirea datei
        if (fileInfo.fileName() == "usg.log"){
            QString log_file = fileInfo.filePath();
            QString str_cmd;
            QString renamed_file;
            if (mCurrentDate > fileInfo.lastModified().date()){
                renamed_file = "/" NAME_DIR_LOG_PATH "/" + fileInfo.baseName() + QString("_%1.log").arg(fileInfo.lastModified().toString("dd.MM.yyyy"));
                str_cmd = "mv \"" + log_file + "\" \"" + renamed_file + "\"";
                system(str_cmd.toStdString().c_str());
            }
        }

#elif defined(Q_OS_MACOS)

        // verificam data curenta si ultima modificare a fisierului
        // copierea fisierului si redenumirea cu atribuirea datei
        if (fileInfo.fileName() == "usg.log"){
            QString log_file = fileInfo.filePath();
            QString str_cmd;
            QString renamed_file;
            if (mCurrentDate > fileInfo.lastModified().date()){
                renamed_file = "/" NAME_DIR_LOG_PATH "/" + fileInfo.baseName() + QString("_%1.log").arg(fileInfo.lastModified().toString("dd.MM.yyyy"));
                str_cmd = "mv \"" + log_file + "\" \"" + renamed_file + "\"";
                system(str_cmd.toStdString().c_str());
            }
        }

#elif defined(Q_OS_WIN)

        // verificam data curenta si ultima modificare a fisierului
        // copierea fisierului si redenumirea cu atasarea datei ultimei
        // modificari
        if (fileInfo.fileName() == "usg.log" && mCurrentDate > fileInfo.lastModified().date()){
            QDir file_init;
            QString nFile = file_init.toNativeSeparators(fileInfo.filePath());
            QString nNameFile = file_init.toNativeSeparators(dirLogPath + "\\" + fileInfo.baseName() + QString("_%1.log").arg(fileInfo.lastModified().toString("dd.MM.yyyy")));
            QString str_cmd;
            if (n > ui->numberLogFile->text().toInt())
                str_cmd = "del \"" + nFile + "\"";
            else
                str_cmd = "move \"" + nFile + "\" \"" + nNameFile + "\"";
            system(str_cmd.toStdString().c_str());
            qInfo(logInfo()) << tr("Fișierul de logare este redenumit în '%1'.").arg(fileInfo.lastModified().toString("dd.MM.yyyy"));
        }

#endif

    }
}

void AppSettings::removeFilesLogOnStartApp()
{
    if (dirLogPath.isEmpty())
        return;

    if (! QFile(dirLogPath).exists())
        return;

    int count_file = 0; // variabila pu num.fisierelor

    QDir dir(dirLogPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();

    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);

        // verificam fisiere si eliminam din sistem
        if (fileInfo.fileName() != "usg.log"){             // fisierul curent = usg.log
            count_file += 1;
            if (count_file > globals::numSavedFilesLog){  // valoarea in SpinBox - numberLogFile
#if defined(Q_OS_LINUX)
                system(QString("rm -f " + fileInfo.filePath()).toStdString().c_str());
#elif defined(Q_OS_MACOS)
                system(QString("rm -f " + fileInfo.filePath()).toStdString().c_str());
#elif defined(Q_OS_WIN)
                system(QString("del /f " + fileInfo.filePath()).toStdString().c_str());
#endif
            }
            continue;
        }
    }
}

void AppSettings::dataWasModified()
{
    setWindowModified(true);
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void AppSettings::slot_currentIndexChangedTab(const int index)
{
    if (index == 0)
        return;

    model_logs->clear();

    QDir dir(dirLogPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);
        QString m_file_log = dir.toNativeSeparators(fileInfo.filePath());

        QStandardItem *item = new QStandardItem;
        item->setData(n, Qt::UserRole);
        item->setData(m_file_log, Qt::DisplayRole);
        model_logs->appendRow(item);
    }

    ui->tableViewLogs->setModel(model_logs);
    ui->tableViewLogs->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewLogs->verticalHeader()->setDefaultSectionSize(13);
    model_logs->setHeaderData(0, Qt::Horizontal, tr("Localizarea fisierelor de logare"));
    qInfo(logInfo()) << tr("Deschisa forma 'Vizualizarea fisierului de logare'.");
}

void AppSettings::slot_clickedTableLogs(const QModelIndex &index)
{
    if (! index.isValid())
        return;

    QDir dir;
    int row = index.row();
    QString m_file_log = model_logs->data(model_logs->index(row, 0), Qt::DisplayRole).toString();
    QFile file(dir.toNativeSeparators(m_file_log));
    if(! file.exists())
        return;

    ui->textLog->clear();

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qInfo(logInfo()) << tr("Vizualizarea fisierului de logare '%1'.").arg(dir.toNativeSeparators(m_file_log));
        ui->textLog->setPlainText(file.readAll());
    }
    file.close();
    ui->textLog->setStyleSheet("font-size: 13px;");
}

// *******************************************************************
// *********** MODIFICAREA DATELOR CONECTARII - SQLITE ***************

void AppSettings::changeNameBaseSqlite()
{
    if (!ui->nameBaseSqlite->text().isEmpty()){
        globals::sqliteNameBase = ui->nameBaseSqlite->text();
        dataWasModified();
    }
}

void AppSettings::changePathBaseSqlite()
{
    if (!lineEditPathDBSqlite->text().isEmpty()){
        globals::sqlitePathBase = lineEditPathDBSqlite->text();
        dataWasModified();
    }
}

// *******************************************************************
// *********** MODIFICAREA DATELOR CONECTARII - MYSQL ****************

void AppSettings::changeNameHostMySQL()
{
    globals::mySQLhost = ui->mySQLhost->text();
    dataWasModified();
}

void AppSettings::changeNameBaseMySQL()
{
    globals::mySQLnameBase = ui->mySQLnameBase->text();
    dataWasModified();
}

void AppSettings::changePortConectionMySQL()
{
    globals::mySQLport = ui->mySQLport->text();
    dataWasModified();
}

void AppSettings::changeConnectOptionsMySQL()
{
    globals::mySQLoptionConnect = ui->mySQLoptionConnect->text();
    dataWasModified();
}

void AppSettings::changeUserNameMySQL()
{
    globals::mySQLuser = ui->mySQLuser->text();
    dataWasModified();
}

void AppSettings::changePasswdMySQL()
{
    globals::mySQLpasswdUser = ui->mySQLpasswdUser->text();
    dataWasModified();
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
        globals::sqlitePathBase = file_database.toNativeSeparators(fileName);
        globals::sqliteNameBase = ui->nameBaseSqlite->text();

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
        globals::sqlitePathBase = file_database.toNativeSeparators(fileName);
        globals::sqliteNameBase = ui->nameBaseSqlite->text();

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
        globals::pathImageBaseAppSettings = lineEditPathDBImage->text();
        globals::pathImageBaseAppSettings = lineEditPathDBImage->text();
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
        globals::pathImageBaseAppSettings = lineEditPathDBImage->text();
        globals::pathImageBaseAppSettings = lineEditPathDBImage->text();
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
                             tr("Verificarea fișierului"),
                             tr("Fișierul cu setările aplicației nu a fost găsit !!!."),
                             QMessageBox::Ok);
        return ;
    }
#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl(ui->txtPathAppSettings->text()));
#elif defined(Q_OS_MACOS)
    QString str_cmd = "open -a \"TextEdit\" " + ui->txtPathAppSettings->text();
    system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_WIN)
    QProcess* proc = new QProcess(this);
    QStringList arguments;
    arguments << ui->txtPathAppSettings->text();
    proc->start("notepad.exe", arguments);
    proc->waitForFinished();
    proc->deleteLater();
#endif

}

void AppSettings::removeFileSettingsApp()
{
    QMessageBox::StandardButton YesNo;
    YesNo = QMessageBox::question(this,
                                  tr("Eliminarea fișierului"),
                                  tr("Sunteți siguri că doriți să eliminați fișierul cu setările aplicației din sistem ?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    QString str_cmd;
#if defined(Q_OS_LINUX)
    str_cmd = "rm -r " + lineEditPathSettings->text();
#elif defined(Q_OS_MACOS)
  str_cmd = "rm -r " + lineEditPathSettings->text();
#elif defined(Q_OS_WIN)
    str_cmd = "del /f " + lineEditPathSettings->text();
#endif
    if (YesNo == QMessageBox::Yes){
        system(str_cmd.toStdString().c_str());
        if (! QFile(lineEditPathSettings->text()).exists()){
            popUp->setPopupText(tr("Fișierul cu setările aplicației<br> a fost eliminat cu succes."));
            popUp->show();
        }
    }
}

void AppSettings::openFileCurrentLogApp()
{
    if (! QFile(fileLogPath).exists()){
        QMessageBox::warning(this,
                             tr("Verificarea fișierului de logare"),
                             tr("Fișierul de logare nu a fost găsit !!!<br>"
                                "Creați fișierul nou sau restartați aplicația."),
                             QMessageBox::Ok);
        return ;
    }

#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl(lineEditPathLog->text()));
#elif defined(Q_OS_MACOS)
    QString str_cmd = "open -a \"TextEdit\" " + lineEditPathLog->text();
    system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_WIN)
    QProcess* proc = new QProcess(this);
    QStringList arguments;
    arguments << lineEditPathLog->text();
    proc->start("notepad.exe", arguments);
#elif defined(Q_OS_MACOS)
#endif

}

void AppSettings::removeFileCurrentLogApp()
{
    QMessageBox::StandardButton YesNo;
    YesNo = QMessageBox::question(this,
                                  tr("Eliminarea fișierului"),
                                  tr("Doriți să eliminați fișierul de logare a aplicației din sistem ?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    QString str_cmd;
#if defined(Q_OS_LINUX)
    str_cmd = "rm -r " + lineEditPathLog->text();
#elif defined(Q_OS_MACOS)
    str_cmd = "rm -r " + lineEditPathLog->text();
#elif defined(Q_OS_WIN)
    str_cmd = "del /f " + lineEditPathLog->text(); // nu se elimina pu ca e deschis !!!
#endif
    if (YesNo == QMessageBox::Yes){
        system(str_cmd.toStdString().c_str());
        if (! QFile(fileLogPath).exists()){
            popUp->setPopupText(tr("Fișierul de logare a aplicației<br> a fost eliminat cu succes."));
            popUp->show();
        }
    }
}

void AppSettings::openDirTemplets()
{
    QDir dir;
    QString pathTemplets;
    if (lineEditPathTemplatesPrint->text().isEmpty())
        pathTemplets = dir.toNativeSeparators(QDir::currentPath());
    else
        pathTemplets = dir.toNativeSeparators(lineEditPathTemplatesPrint->text());

    QString dir_templates = QFileDialog::getExistingDirectory(this, tr("Alegeti directoriu"),
                                                    pathTemplets,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators(dir_templates));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();
}

void AppSettings::openDirReports()
{
    QDir dir;
    QString pathReports;
    if (lineEditPathReports->text().isEmpty())
        pathReports = dir.toNativeSeparators(QDir::currentPath());
    else
        pathReports = dir.toNativeSeparators(lineEditPathReports->text());

    QString dir_reports = QFileDialog::getExistingDirectory(this, tr("Alegeti directoriu"),
                                                    pathReports,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    lineEditPathReports->setText(dir.toNativeSeparators(dir_reports));
    globals::pathReports = lineEditPathReports->text();
}

// *******************************************************************
// **************** BUTOANE - formei principale **********************

void AppSettings::onBtnOKSettings()
{
    if (checkDataSettings()){
        saveSettings();
        QDialog::accept();
    }
}

void AppSettings::onBtnWriteSettings()
{
    if (checkDataSettings()){
        saveSettings();
    }
}

void AppSettings::onBtnCancelSettings()
{
    this->close();
}

// *******************************************************************
// **************** PROCESAREA INDEXULUI COMBOBOX ********************

void AppSettings::changeIndexLangApp(const int _index)
{
    if (_index < -1 || _index > 1)
        return;

    ui->comboBoxLangApp->setCurrentIndex(_index);

    if (_index == 0)
        globals::langApp = "ru_RU";
    else if (_index == 1)
        globals::langApp = "ro_RO";

    if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        qApp->installTranslator(translator);
    }

    dataWasModified();
    emit changeLanguage();

    QMessageBox::StandardButton YesNo;
    YesNo = QMessageBox::question(this, tr("Traducerea aplicației."),
                                  tr("Pentru traducerea completă este necesar de relansat aplicația.<br><br>"
                                     "Doriți relansarea ?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (YesNo == QMessageBox::Yes){
        saveSettings();
        // restart:
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    }
}

void AppSettings::changeIndexTypeSQL(const int _index)
{
    if (_index == idx_MySQL){ // MySQL

        globals::indexTypeSQL   = _index;
        globals::thisMySQL      = true;
        globals::connectionMade = "MySQL";

        ui->tabLogs->setEnabled(false);
        ui->tabSqlite->setEnabled(false);
        ui->tabMySQL->setEnabled(true);
        ui->txtPathImage->setEnabled(false);
        ui->tabConnections->setCurrentIndex(1);

        if (! lineEditPathDBSqlite->text().isEmpty())
            lineEditPathDBSqlite->setText("");
        if (! ui->nameBaseSqlite->text().isEmpty())
            ui->nameBaseSqlite->setText("");

        lineEditPathDBImage->setText("");

        setDefaultPath(); // setam localizarea fisierelor implicite a bazei de date

        ui->tableViewLogs->setVisible(false);
        ui->textLog->setVisible(false);

    } else if (_index == idx_Sqlite){ //sqlite

        globals::indexTypeSQL   = _index;
        globals::thisMySQL      = false;
        globals::connectionMade = "Sqlite";

        ui->tabLogs->setEnabled(true);
        ui->tabSqlite->setEnabled(true);
        ui->tabMySQL->setEnabled(false);
        ui->txtPathImage->setEnabled(true);
        ui->tabConnections->setCurrentIndex(0);

        ui->mySQLhost->setText("");
        ui->mySQLnameBase->setText("");
        ui->mySQLoptionConnect->setText("");
        ui->mySQLport->setText("");
        ui->mySQLuser->setText("");
        ui->mySQLpasswdUser->setText("");

        setDefaultPathSqlite(); // setam localizarea fisierelor implicite a bazei de date sqlite

        ui->tableViewLogs->setVisible(true);
        ui->textLog->setVisible(true);

    } else {

        globals::indexTypeSQL   = _index;
        globals::thisMySQL      = false;
        globals::connectionMade = nullptr;

        ui->tabLogs->setEnabled(false);
        ui->tabSqlite->setEnabled(false);
        ui->tabMySQL->setEnabled(false);
        ui->txtPathImage->setEnabled(false);
        ui->tabConnections->setCurrentIndex(0);

        if (! lineEditPathDBSqlite->text().isEmpty())
            lineEditPathDBSqlite->setText("");
        if (! ui->nameBaseSqlite->text().isEmpty())
            ui->nameBaseSqlite->setText("");

        ui->mySQLhost->setText("");
        ui->mySQLnameBase->setText("");
        ui->mySQLoptionConnect->setText("");
        ui->mySQLport->setText("");
        ui->mySQLuser->setText("");
        ui->mySQLpasswdUser->setText("");
    }
}

void AppSettings::changeIndexUnitMeasure(const int _index)
{
    if (_index == 0) {
        globals::unitMeasure = "milimetru";
    } else {
        globals::unitMeasure = "centimetru";
    }
}

// *******************************************************************
// **************** OTHER ********************************************

void AppSettings::createNewBaseSqlite()
{
    if (!checkDataSettings())
        return;

    QString txtMSG; // pentru mesaj/debug
    db->createConnectBaseSqlite(txtMSG);

    QMessageBox::warning(this, tr("Crearea bazei de date (.sqlite)"), txtMSG, QMessageBox::Ok);
}

void AppSettings::onTestConnectionMySQL()
{
    if (!checkDataSettings())
        return;

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(globals::mySQLhost);         // localhost
    db.setDatabaseName(globals::mySQLnameBase); // USGdb
    db.setPort(globals::mySQLport.toInt());
    db.setConnectOptions(globals::mySQLoptionConnect);
    db.setUserName(globals::mySQLuser);
    db.setPassword(globals::mySQLpasswdUser);
    if (db.open()){
        QMessageBox::information(this,
                                 tr("Testarea conectării"),
                                 tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> este realizată cu succes.").arg(globals::mySQLnameBase),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::warning(this,
                             tr("Testarea conectării"),
                             tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> lipsește.").arg(globals::mySQLnameBase),
                             QMessageBox::Ok);
    }
}

void AppSettings::setPathAppSettings()
{
    QDir file_path;
    if (globals::thisMySQL){
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->mySQLnameBase->text() + ".conf"));
    } else {
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->nameBaseSqlite->text() + ".conf"));
    }
}

void AppSettings::setPathGlobalVariableAppSettings()
{
    globals::pathAppSettings = ui->txtPathAppSettings->text();
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void AppSettings::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        const QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Modificarea datelor"),
                                                                        tr("Setările au fost modificate.\n"
                                                                           "Doriți să salvați aceste modificări ?"),
                                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            onBtnOKSettings();
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
        ui->comboBoxUnitMeasure->clear();
        ui->comboBoxUnitMeasure->addItems(QStringList() << tr("milimetru") << tr("centimetru"));
        ui->comboBoxUnitMeasure->setCurrentIndex(indexMeasure); // instalam indexul memorizat
    }
}
