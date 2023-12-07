#include "initlaunch.h"
#include "loggingcategories.h"
#include "ui_initlaunch.h"

InitLaunch::InitLaunch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitLaunch)
{
    ui->setupUi(this);

    setWindowTitle(tr("Lansarea aplicației %1").arg("[*]"));

    nameLocale = QLocale::system().name();
//    nameLocale.replace(QString("_"), QString("-")); // schimbam simbolul

    // traducem aplicatia
    translator = new QTranslator(this);

    ui->comboLangApp->addItems(QStringList() << "ro_RO" << "ru_RU");

    int indexLangApp = (nameLocale == "ro-RO") ? 0 : 1;
    ui->comboLangApp->setCurrentIndex(indexLangApp);
    changeIndexComboLangApp(indexLangApp);

    connect(ui->comboLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InitLaunch::changeIndexComboLangApp);
    connect(ui->comboLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InitLaunch::dataWasModified);

    connect(ui->radBtnFirstLaunch, &QRadioButton::clicked, this, &InitLaunch::onClickedRadBtnFirstLaunch);
    connect(ui->radBtnFirstLaunch, &QRadioButton::clicked, this, &InitLaunch::dataWasModified);
    connect(ui->radBtnAppMove, &QRadioButton::clicked, this, &InitLaunch::onClickedRadBtnAppMove);
    connect(ui->radBtnAppMove, &QRadioButton::clicked, this, &InitLaunch::dataWasModified);

    connect(ui->btnOK, &QPushButton::clicked, this, &InitLaunch::onWritingData);
    connect(ui->btnClose, &QPushButton::clicked, this, &InitLaunch::onClose);

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif
}

InitLaunch::~InitLaunch()
{
    delete translator;
    delete ui;
}

void InitLaunch::dataWasModified()
{
    setWindowModified(true);
}

void InitLaunch::changeIndexComboLangApp(const int _index)
{
    nameLocale = (_index == 0) ? "ro_RO" : "ru_RU"; // setam nameLocal dupa index alex
    globals::langApp = nameLocale;                  // setam variabila globala
    if (translator->load(QLocale(globals::langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        QCoreApplication::installTranslator(translator);
    }
}

void InitLaunch::onClickedRadBtnFirstLaunch()
{
    globals::firstLaunch = true;
    globals::moveApp = 0;
}

void InitLaunch::onClickedRadBtnAppMove()
{
    globals::firstLaunch = false;
    globals::moveApp = 1;
}

void InitLaunch::onWritingData()
{
    globals::unknowModeLaunch = false;
    globals::firstLaunch = ui->radBtnFirstLaunch->isChecked();
    globals::moveApp     = (ui->radBtnAppMove->isChecked()) ? 1 : 0;
    if (globals::moveApp == 1)
        qInfo(logInfo()) << tr("Aplicatia a fost transferata.");
    else
        qInfo(logInfo()) << tr("Prima lansare a aplicatiei.");

    QDialog::accept();
}

void InitLaunch::onClose()
{
    this->close();
}

void InitLaunch::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox::StandardButton answer;
        answer = QMessageBox::warning(this, tr("Modificarea datelor"),
                                      tr("Datele au fost modificate.\n"
                                      "Doriți să salvați aceste modificări ?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            onWritingData();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void InitLaunch::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Lansarea aplicației %1").arg("[*]"));
    }
}
