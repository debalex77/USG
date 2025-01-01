#include "databaseselection.h"
#include "ui_databaseselection.h"

DatabaseSelection::DatabaseSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseSelection)
{
    ui->setupUi(this);

    db = new DataBase(this);
    timer = new QTimer(this);

    setWindowTitle(tr("Alege/creează baza de date"));

    QDir file_path;
    QString txt_path_config = file_path.toNativeSeparators(dirConfigPath);
    if (! QFile(txt_path_config).exists()){
        if (QDir().mkpath(txt_path_config)){
            globals::pathAppSettings = file_path.toNativeSeparators(fileConfigPath);
            connect(timer, &QTimer::timeout, this, &DatabaseSelection::updateTimer);
            timer->start(1000);
        } else {
            QMessageBox::warning(this, tr("Crearea directoriei"),
                                 tr("Directoria <b>'USG'</b> pentru păstrarea setărilor aplicației nu a fost creată !!!<br>"
                                    "Adresați-vă administratorului aplicației."), QMessageBox::Ok);
            qApp->quit();
        }
    }

    QDir dir(txt_path_config);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);
        ui->listWidget->addItem(fileInfo.baseName());
    }

    connect(ui->btnConnect, &QAbstractButton::clicked, this, &DatabaseSelection::onConnectToBase);
    connect(ui->btnAddDatabase, &QAbstractButton::clicked, this, &DatabaseSelection::onAddDatabase);
    connect(ui->btnRemove, &QAbstractButton::clicked, this, &DatabaseSelection::onRemoveRowListWidget);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &DatabaseSelection::close);
    connect(ui->listWidget, QOverload<int>::of(&QListWidget::currentRowChanged), this, QOverload<int>::of(&DatabaseSelection::onCurrentRowChanged));

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif

}

DatabaseSelection::~DatabaseSelection()
{
    delete ui;
    delete db;
}

void DatabaseSelection::readFileSettings(const QString pathToFile)
{
    QFile file(pathToFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line; //= in.readLine();
        in >> line;
        if (line.contains("MySQL_host=", Qt::CaseSensitive)){
            ui->txtPath->setText("<b><u>host:</u></b> " + db->decode_string(line.mid(11, line.size() - 11)));
        }
        if (line.contains("MySQL_name_base=", Qt::CaseSensitive)){
            ui->txtPath->setText(ui->txtPath->text() +
                                 "; <b><u>base:</u></b> " + db->decode_string(line.mid(16, line.size() - 16)));
        }
        if (line.contains("indexTypeSQL=", Qt::CaseSensitive)){
            if (line.mid(13, 1) == "1"){
                ui->txtTypeConnection->setText(tr("Tipul conectarii:<br><b><u>MySQL</u></b>"));
            } else if (line.mid(13, 1) == "2"){
                ui->txtTypeConnection->setText(tr("Tipul conectarii:<br><b><u>Sqlite</u></b>"));
            } else {
                ui->txtTypeConnection->setText(tr("Tipul conectarii:<br<b><u>>Unknow</u></b>"));
            }
        }
    }
}

void DatabaseSelection::onCurrentRowChanged(const int row)
{
    if (row == -1)
        return;

    QDir file_conf;
    const QString file_name = dirConfigPath + "/" + ui->listWidget->item(row)->data(Qt::DisplayRole).toString() + ".conf";
    ui->txtPath->setText(file_conf.toNativeSeparators(file_name));
    readFileSettings(file_name);
}

void DatabaseSelection::onConnectToBase()
{
    QDir file_conf;
    QString file_name = dirConfigPath + "/" + ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString() + ".conf";
    globals::pathAppSettings = file_conf.toNativeSeparators(file_name);
    qInfo(logInfo()) << "Selection of connection to base - " + ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString();
    QDialog::accept();
}

void DatabaseSelection::onAddDatabase()
{
    globals::firstLaunch = true;
    QDialog::accept();
}

void DatabaseSelection::onRemoveRowListWidget()
{
    if (ui->listWidget->currentRow() == -1)
        return;

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Eliminarea set\304\203rilor"),
                             tr("Dori\310\233i s\304\203 elimina\310\233i fi\310\231ierul:<br>%1 ?").arg(ui->txtPath->text()),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton) {
        QDir file_remove;
        QString file_name = dirConfigPath + "/" + ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString() + ".conf";
#if defined(Q_OS_LINUX)
        QString str_cmd = "rm " + file_remove.toNativeSeparators(file_name);
        system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_MACOS)
        QString str_cmd = "rm " + file_remove.toNativeSeparators(file_name);
        system(str_cmd.toStdString().c_str());
#elif defined(Q_OS_WIN)
        QString str_cmd = "del " + file_remove.toNativeSeparators(file_name);
        qDebug() << str_cmd;
        system(str_cmd.toStdString().c_str());
#endif
    } else if (messange_box.clickedButton() == noButton) {

    }

    const int row = ui->listWidget->currentRow();
    ui->listWidget->removeItemWidget(ui->listWidget->takeItem(row));
}

void DatabaseSelection::updateTimer()
{
    if (this->isVisible())
        timer->stop();
    else
        return;

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Crearea bazei de date"),
                             tr("Adaugarea/crearea bazei de date ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton)
        onAddDatabase();
    else if (messange_box.clickedButton() == noButton)
        return;

}
