#include "about.h"
#include "ui_about.h"
#include <QAction>
#include "mainwindow.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    db = new DataBase(this);

    QString str = tr("<p align=center><h3><b> %1 v%2</b> </h3>"
                     "Aplicația <b>USG</b> este o soluție modernă și eficientă destinată gestionării pacienților "
                     "supuși investigațiilor ecografice. Oferă funcționalități avansate pentru evidența completă "
                     "a datelor pacienților, incluzând: </p>"
                     "<ul style=\"margin-left: 40px;\">"
                     "  <li>Memorarea informațiilor personale și a datelor de contact;</li>"
                     "  <li>Atașarea imaginilor și fișierelor video rezultate în urma investigațiilor;</li>"
                     "  <li>Gestionarea centralizată a istoricului medical etc.</li>"
                     "</ul>"
                     "<p align=center><h3><b>Caracteristici principale:</b> </h3></p>"
                     "<ul>"
                     "  <li><b>Free și open-source</b> – aplicația este gratuită și codul sursă este disponibil public pentru personalizare și contribuții din partea comunității.</li>"
                     "  <li><b>Cross-platform</b> – funcționează pe principalele sisteme de operare: Linux, MacOS și Windows.</li>"
                     "  <li><b>Suport pentru baze de date</b> – compatibilă cu SQLite3, MySQL și MariaDB, oferind flexibilitate în funcție de nevoile utilizatorilor.</li>"
                     "</ul>"
                     "<p align=center><h3><b> Autorul aplicației:</b> </h3>"
                     "%3, Alovada-Med SRL</p>").arg(APPLICATION_NAME, USG_VERSION_FULL, USG_COMPANY_EMAIL);
    ui->textBrowser_about->setText(str);

    QString str_licenses = tr("<br>"
                              "<p align=center><h4>Acest program este software gratuit; <br>"
                              "îl poți redistribui și/sau modifica în concordanță cu termenii <br>"
                              "GNU Licență Publică Generală cum sunt publicați de <br>"
                              "Free Software Foundation; fie versiunea 3 a licenței, <br>sau(după alegerea ta) orice versiune mai actuală.</h4>");
    ui->textBrowser_licenses->setText(str_licenses);

#if defined(Q_OS_WIN)
    ui->textBrowser_about->setStyleSheet("font-size: 15px;");
    ui->textBrowser_licenses->setStyleSheet("font-size: 15px;");
#elif defined(Q_OS_LINUX)
    ui->textBrowser_about->setStyleSheet("font-size: 15px;");
    ui->textBrowser_licenses->setStyleSheet("font-size: 15px;");
#endif

    QDir dir;
    if (globals().mySQLhost.isEmpty()){
        ui->dir_app->setText(dir.toNativeSeparators(globals().sqlitePathBase));
        ui->dir_img->setText(dir.toNativeSeparators(globals().pathImageBaseAppSettings));
    } else {
        ui->dir_app->setVisible(false);
        ui->dir_img->setVisible(false);
        ui->label_2->setVisible(false);
        ui->label_4->setVisible(false);
    }
    ui->dir_settings->setText(dir.toNativeSeparators(globals().pathAppSettings));
    ui->dir_templates->setText(dir.toNativeSeparators(globals().pathTemplatesDocs));
    ui->dir_reports->setText(dir.toNativeSeparators(globals().pathReports));
    ui->dir_logs->setText(dir.toNativeSeparators(globals().pathLogAppSettings));

    ui->text_versionQt->setText("version Qt: " QT_VERSION_STR);
    if (! globals().mySQLhost.isEmpty())
        ui->text_version_SQLite->setText("version MySQL: " + db->getVersionMySQL());
    else
        ui->text_version_SQLite->setText("version SQLite: " + db->getVersionSQLite());

    connect(ui->pushButton, &QAbstractButton::clicked, this, &About::close);

    if (globals().isSystemThemeDark)
        ui->frame->setObjectName("customFrame");
}

About::~About()
{
    delete db;
    delete ui;
}
