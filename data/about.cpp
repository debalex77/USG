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

    QString str = tr("<br>"
                     "<p align=center><b> %1 v%2</b> <br>"
                     "Aplicația pentru evidența pacienților care au trecut investigații ecografice, "
                     "cu memorarea datelor de contact și a documentelor de identitate. <br>"
                     "<p align=center><b>Informații suplimentare:</b> <br>"
                     "Free, open-source, cross-platform,<br>database SQLite3/MySQL/MariaDB"
                     "<br>"
                     "<p align=center><b> Autorul aplicației:</b> <br>"
                     "Codreanu Alexandru").arg(APPLICATION_NAME, APPLICATION_VERSION);
    ui->textBrowser_about->setText(str);

#if defined(Q_OS_WIN)
    ui->textBrowser_about->setStyleSheet("font-size: 15px;");
#endif

    QDir dir;
    if (globals::mySQLhost.isEmpty()){
        ui->dir_app->setText(dir.toNativeSeparators(globals::sqlitePathBase));
        ui->dir_img->setText(dir.toNativeSeparators(globals::pathImageBaseAppSettings));
    } else {
        ui->dir_app->setVisible(false);
        ui->dir_img->setVisible(false);
        ui->label_2->setVisible(false);
        ui->label_4->setVisible(false);
    }
    ui->dir_settings->setText(dir.toNativeSeparators(globals::pathAppSettings));
    ui->dir_templates->setText(dir.toNativeSeparators(globals::pathTemplatesDocs));
    ui->dir_reports->setText(dir.toNativeSeparators(globals::pathReports));
    ui->dir_logs->setText(dir.toNativeSeparators(globals::pathLogAppSettings));

    ui->text_versionQt->setText("version Qt: " QT_VERSION_STR);
    if (! globals::mySQLhost.isEmpty())
        ui->text_version_SQLite->setText("version MySQL: " + db->getVersionMySQL());
    else
        ui->text_version_SQLite->setText("version SQLite: " + db->getVersionSQLite());

    connect(ui->pushButton, &QAbstractButton::clicked, this, &About::close);
}

About::~About()
{
    delete db;
    delete ui;
}
