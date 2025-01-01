#include "appsettings.h"
#include "ui_appsettings.h"

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

    QFileInfo info_file_config(globals::pathAppSettings);
    const QString baseName_file_config = info_file_config.baseName();
    const QString nameMatches = baseName_file_config + ".log";
    fileLogPath = QDir::rootPath() + NAME_DIR_LOG_PATH "/" + nameMatches;

    if (! QFile(dirLogPath).exists()){
        QMessageBox::information(this, tr("Crearea directoriului de logare"),
                                 tr("Pentru crearea directoriului de logare a aplica\310\233iei este necesar accesul administrativ."),
                                 QMessageBox::Ok);
        QString str_cmd = "pkexec mkdir /" NAME_DIR_LOG_PATH " && pkexec chmod 777 /" NAME_DIR_LOG_PATH " && touch " + fileLogPath;
        system(str_cmd.toStdString().c_str()); // pkexec mkdir /var/log/usg && pkexec chmod 777 /var/log/usg && touch /var/log/usg/usg.log
        globals::pathLogAppSettings = fileLogPath;
    } else {
        processingLoggingFiles(nameMatches);
        globals::pathLogAppSettings = fileLogPath;
    }

    if (! QFile(globals::pathAppSettings).exists())
        globals::unknowModeLaunch = true; // globals::numSavedFilesLog

#elif defined(Q_OS_MACOS)

    if (! QFile(dirLogPath).exists())
        QDir().mkpath(dirLogPath);

    QFileInfo info_file_config(globals::pathAppSettings);
    const QString baseName_file_config = info_file_config.baseName();
    const QString nameMatches = baseName_file_config + ".log";
    fileLogPath = dirLogPath + "/" + nameMatches;

    processingLoggingFiles(fileLogPath);
    globals::pathLogAppSettings = fileLogPath;

    if (! QFile(globals::pathAppSettings).exists())
        globals::unknowModeLaunch = true; // globals::numSavedFilesLog

#elif defined(Q_OS_WIN)

    QDir dir;

    QFileInfo info_file_config(globals::pathAppSettings);
    const QString baseName_file_config = info_file_config.baseName();
    const QString nameMatches = baseName_file_config + ".log";
    fileLogPath = dir.toNativeSeparators(dirLogPath + "/" + nameMatches);

    //********************************************
    // usg.log
    if (! QDir(dir.toNativeSeparators(dirLogPath)).exists()){
        if (! QDir().mkdir(dir.toNativeSeparators(dirLogPath))){
            QMessageBox::warning(this, tr("Crearea directoriului de logare"),
                                 tr("Directoria 'config' pentru p\304\203strarea fi\310\231ierelor de logare a aplica\310\233iei nu a fost creat\304\203 !!!<br>"
                                    "Adresa\310\233i-v\304\203 administratorului aplica\310\233iei."),
                                 QMessageBox::Ok);
            return;
        }
    } else {
        processingLoggingFiles(nameMatches);
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
    // delete settApp;
    delete db;
    delete popUp;
    delete lineEditPathDBImage;
    delete lineEditPathDBSqlite;
    delete lineEditPathReports;
    delete lineEditPathTemplatesPrint;
    delete btnAdd;
    delete btnEdit;
    delete btnClear;
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
                                       "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);}");

    ui->txtPathAppSettings->setEnabled(false);

    connect(ui->btnOpenSettings, &QToolButton::clicked, this, &AppSettings::openFileSettingsApp);
}

void AppSettings::initBtnLogApp()
{

    ui->btnOpenLog->setStyleSheet("QToolButton {border: 1px solid #8f8f91; "
                                  "border-radius: 4px;}"
                                  "QToolButton:hover {background-color: rgb(234,243,250);}"
                                  "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);}");

    ui->txtPathLog->setEnabled(false);

    connect(ui->btnOpenLog, &QToolButton::clicked, this, &AppSettings::openFileCurrentLogApp);
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

void AppSettings::initBtnDirVideo(const QString appStyleBtn)
{
    btnOpenDirVideo = new QToolButton(this);
    btnOpenDirVideo->setIcon(QIcon(":/img/folder.png"));
    btnOpenDirVideo->setStyleSheet(appStyleBtn);
    btnOpenDirVideo->setCursor(Qt::PointingHandCursor);
    btnOpenDirVideo->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);

    btnRemoveDirVideo = new QToolButton(this);
    btnRemoveDirVideo->setIcon(QIcon(":/img/trash.png"));
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

    connect(btnOpenDirVideo, &QAbstractButton::clicked, this , &AppSettings::openDirVideo);
    connect(btnRemoveDirVideo, &QAbstractButton::clicked, this, [this]()
            {
                lineEditPathVideo->clear();
            });
}

void AppSettings::initConnections()
{
    connect(lineEditPathDBSqlite, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathDBImage, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(ui->txtPathAppSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(ui->txtPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathVideo, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

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

    if (! ui->txtPathAppSettings->text().isEmpty())
        ui->txtPathAppSettings->setText(globals::pathAppSettings);

    ui->txtPathLog->setText(globals::pathLogAppSettings);

    //******************************************************************************
    // setam localizarea sabloanelor de tipar si a rapoartelor
#if defined(Q_OS_LINUX)
    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators("/opt/USG/templets"));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.toNativeSeparators("/opt/USG/templets/reports"));
    globals::pathReports = lineEditPathReports->text();
#elif defined(Q_OS_MACOS)
    lineEditPathTemplatesPrint->setText(dir.homePath() + "/USG/templets");
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.homePath() + "/USG/templets/reports");
    globals::pathReports = lineEditPathReports->text();
#elif defined(Q_OS_WINDOWS)
    lineEditPathTemplatesPrint->setText(dir.toNativeSeparators(dir.currentPath() + "/templets"));
    globals::pathTemplatesDocs = lineEditPathTemplatesPrint->text();

    lineEditPathReports->setText(dir.toNativeSeparators(dir.currentPath() + "/templets/reports"));
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

    if (! QFile(dir.homePath() + "/USG").exists()) {
        dir.mkpath(dir.homePath() + "/USG");
        dir.mkpath(dir.homePath() + "/USG/Database_usg");
    }

    if (globals::pathLogAppSettings.isEmpty())
        globals::pathLogAppSettings = ui->txtPathLog->text();

    //******************************************************************************
    // crearea fisierelor bazelor de date si setarea
    QString str_dir_database = dir.toNativeSeparators(dir.homePath() + "/USG/Database_usg");
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
        if (QFile(str_file_database).exists()){
            QMessageBox messange_box(QMessageBox::Question,
                                     tr("Determinarea existen\310\233ei bazei de date"),
                                     tr("Baza de date '<u>%1</u>' exist\304\203 \303\256n sistem !!!<br><br>"
                                        "Pentru crearea bazei de date noi cu nume '<b><u>%2</u></b>' este necesar de eliminat fi\310\231ierul vechi. "
                                        "Dori\310\233i s\304\203 elimina\310\233i fi\310\231ierul din sistem ?").arg(str_file_database, (ui->nameBaseSqlite->text().isEmpty()) ? "base" : ui->nameBaseSqlite->text()),
                                     QMessageBox::NoButton, this);
            QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
            QPushButton *noButton = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
            yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
            noButton->setStyleSheet(db->getStyleForButtonMessageBox());
            messange_box.exec();

            if (messange_box.clickedButton() == yesButton){
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
            } else if (messange_box.clickedButton() == noButton) {
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
                                     tr("Nu este indicat\304\203 \"<b>%1</b>\" !!!.")
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
                                     tr("Nu este indicat\304\203 \"<b>%1</b>\" !!!")
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
    disconnect(ui->txtPathAppSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(ui->txtPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    disconnect(lineEditPathVideo, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

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

    ui->txtPathLog->setText(globals::pathLogAppSettings);
    lineEditPathTemplatesPrint->setText(globals::pathTemplatesDocs);
    lineEditPathReports->setText(globals::pathReports);
    lineEditPathVideo->setText(globals::pathDirectoryVideo);

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
    connect(ui->txtPathAppSettings, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(ui->txtPathLog, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathReports, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathTemplatesPrint, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);
    connect(lineEditPathVideo, &QLineEdit::textChanged, this, &AppSettings::dataWasModified);

    connect(ui->nameBaseSqlite, &QLineEdit::editingFinished, this, &AppSettings::changeNameBaseSqlite);
    connect(lineEditPathDBSqlite, &QLineEdit::editingFinished, this, &AppSettings::changePathBaseSqlite);
}

void AppSettings::saveSettings()
{
    QDir dir_conf;

#if defined(Q_OS_MACOS)
    if (! QFile(ui->txtPathLog->text()).exists()) {
        QString str_cmd = "touch \"" + ui->txtPathLog->text() + "\"";
        system(str_cmd.toStdString().c_str());
        qInfo(logInfo()) << "A fost creat fisierul de logare " << ui->txtPathLog->text();

    }
#endif

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
    settApp->setValue("pathVideo",         lineEditPathVideo->text());
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
        settApp->setValue("pathLogApp",     ui->txtPathLog->text());
    }
    settApp->endGroup();

    settApp->beginGroup("on_start");
    settApp->setValue("numSavedFilesLog", ui->numberLogFile->value());
    settApp->endGroup();

    settApp->beginGroup("show_msg");
    settApp->setValue("showMsgVideo",   (globals::show_content_info_video) ? 1 : 0);
    settApp->setValue("showMsgReports", (globals::show_info_reports) ? 1 : 0);
    settApp->endGroup();

    qInfo(logInfo()) << tr("Setarile aplicatiei sunt salvate/modificate in fisierul - %1.").arg(globals::pathAppSettings);
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
    globals::pathDirectoryVideo = settApp->value("pathVideo", "").toString();
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

    settApp->beginGroup("show_msg");
    globals::show_content_info_video = settApp->value("showMsgVideo", true).toBool();
    globals::show_info_reports       = settApp->value("showMsgReports", true).toBool();
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
                                  tr("Conectarea la baza de date <b><u>%1</u></b> lipse\310\231te !!! <br><br>Adresa\310\233i-v\304\203 administratorului aplica\310\233iei.")
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

void AppSettings::processingLoggingFiles(const QString nameMatches)
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
        if (fileInfo.fileName() == nameMatches){
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
        if (fileInfo.fileName() == nameMatches){
            QString log_file = fileInfo.filePath();
            QString renamed_file;
            if (mCurrentDate > fileInfo.lastModified().date()){
                renamed_file = QDir::cleanPath(dirLogPath + "/" + fileInfo.baseName() +
                                               QString("_%1.log").arg(fileInfo.lastModified().toString("dd.MM.yyyy")));
                // Renumirea fișierului folosind QFile
                QFile file(log_file);
                if (file.rename(renamed_file)) {
                    qInfo() << "Fișierul de logare a fost redenumit: " << renamed_file;
                } else {
                    qWarning() << "Nu s-a putut redenumi fișierul de logare: " << log_file;
                }
            }
        }

#elif defined(Q_OS_WIN)

        // verificam data curenta si ultima modificare a fisierului
        // copierea fisierului si redenumirea cu atasarea datei ultimei
        // modificari
        if (fileInfo.fileName() == nameMatches && mCurrentDate > fileInfo.lastModified().date()){
            QDir file_init;
            QString nFile = file_init.toNativeSeparators(fileInfo.filePath());
            QString nNameFile = file_init.toNativeSeparators(dirLogPath + "\\" + fileInfo.baseName() + QString("_%1.log").arg(fileInfo.lastModified().toString("dd.MM.yyyy")));
            QString str_cmd;
            if (n > ui->numberLogFile->text().toInt())
                str_cmd = "del \"" + nFile + "\"";
            else
                str_cmd = "move \"" + nFile + "\" \"" + nNameFile + "\"";
            system(str_cmd.toStdString().c_str());
            qInfo(logInfo()) << tr("Fisierul de logare este redenumit în '%1'.").arg(fileInfo.lastModified().toString("dd.MM.yyyy"));
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
    dir.setSorting(QDir::Time);
    QFileInfoList listFiles = dir.entryInfoList();

    QRegularExpression regx(globals::sqliteNameBase + "_[0-9]{1,2}\\.[0-9]{1,2}\\.[0-9]{1,4}");

    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);

        // verificam fisiere si eliminam din sistem
        if (fileInfo.fileName().contains(regx)){             // fisierul curent = usg.log
            count_file += 1;
            if (count_file > globals::numSavedFilesLog){  // valoarea in SpinBox - numberLogFile
#if defined(Q_OS_LINUX)
                system(QString("rm -f " + fileInfo.filePath()).toStdString().c_str());
#elif defined(Q_OS_MACOS)
                system(QString("rm -f " + fileInfo.filePath()).toStdString().c_str());
#elif defined(Q_OS_WIN)
                QDir dir_file;
                system(QString("del /f " + dir_file.toNativeSeparators(fileInfo.filePath())).toStdString().c_str());
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

    QRegularExpression regx((ui->comboBoxTypeSQL->currentIndex() == idx_Sqlite) ?
                     globals::sqliteNameBase : globals::mySQLnameBase, QRegularExpression::CaseInsensitiveOption);

    QDir dir(dirLogPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);

        if (fileInfo.fileName().contains(regx)){
            QString m_file_log = dir.toNativeSeparators(fileInfo.filePath());

            QStandardItem *item = new QStandardItem;
            item->setData(n, Qt::UserRole);
            item->setData(m_file_log, Qt::DisplayRole);
            model_logs->appendRow(item);
        }
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
                             tr("Verificarea fi\310\231ierului"),
                             tr("Fi\310\231ierul cu set\304\203rile aplica\310\233iei nu a fost g\304\203sit !!!."),
                             QMessageBox::Ok);
        return ;
    }
#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl(ui->txtPathAppSettings->text()));
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
    if (! QFile(fileLogPath).exists()){
        QMessageBox::warning(this,
                             tr("Verificarea fi\310\231ierului de logare"),
                             tr("Fi\310\231ierul de logare nu a fost g\304\203sit !!!<br>"
                                "Crea\310\233i fi\310\231ierul nou sau restarta\310\233i aplica\310\233ia."),
                             QMessageBox::Ok);
        return ;
    }

#if defined(Q_OS_LINUX)
    QDesktopServices::openUrl(QUrl(ui->txtPathLog->text()));
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

    QString dir_templates = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
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

    QString dir_reports = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
                                                    pathReports,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    lineEditPathReports->setText(dir.toNativeSeparators(dir_reports));
    globals::pathReports = lineEditPathReports->text();
}

void AppSettings::openDirVideo()
{
    QDir dir;
    QString pathVideo;
    if (lineEditPathVideo->text().isEmpty())
        pathVideo = dir.toNativeSeparators(QDir::currentPath());
    else
        pathVideo = dir.toNativeSeparators(lineEditPathVideo->text());

    QString dir_video = QFileDialog::getExistingDirectory(this, tr("Alege\310\233i directoriu"),
                                                          pathVideo,
                                                          QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks);

    lineEditPathVideo->setText(dir.toNativeSeparators(dir_video));
    globals::pathDirectoryVideo = lineEditPathVideo->text();
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
    YesNo = QMessageBox::question(this, tr("Traducerea aplica\310\233iei."),
                                  tr("Pentru traducerea complet\304\203 este necesar de relansat aplica\310\233ia.<br><br>"
                                     "Dori\310\233i relansarea ?"),
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
        globals::thisSqlite     = false;
        globals::connectionMade = "MySQL";

        ui->tabLogs->setEnabled(true);
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

        ui->tableViewLogs->setVisible(true);
        ui->textLog->setVisible(true);

    } else if (_index == idx_Sqlite){ //sqlite

        globals::indexTypeSQL   = _index;
        globals::thisSqlite     = true;
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
                                 tr("Testarea conect\304\203rii"),
                                 tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> este realizat\304\203 cu succes.").arg(globals::mySQLnameBase),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::warning(this,
                             tr("Testarea conect\304\203rii"),
                             tr("Conectarea cu baza de date <b>'%1'(MySQL)</b> lipse\310\231te.").arg(globals::mySQLnameBase),
                             QMessageBox::Ok);
    }
}

void AppSettings::setPathAppSettings()
{
    QDir file_path;
    if (globals::thisMySQL){
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->mySQLnameBase->text() + ".conf"));
        ui->txtPathLog->setText(file_path.toNativeSeparators(dirLogPath + "/" + ui->nameBaseSqlite->text() + ".log"));
    } else {
        ui->txtPathAppSettings->setText(file_path.toNativeSeparators(dirConfigPath + "/" + ui->nameBaseSqlite->text() + ".conf"));
        ui->txtPathLog->setText(file_path.toNativeSeparators(dirLogPath + "/" + ui->nameBaseSqlite->text() + ".log"));
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
                                                                        tr("Set\304\203rile au fost modificate.\n"
                                                                           "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
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
