#include "infowindow.h"
#include "ui_infowindow.h"

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QScreen>
#endif

InfoWindow::InfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow)
{
    ui->setupUi(this);

    appSettings = new AppSettings(this);

    ui->txt_title->clear();
    ui->textBrowser->clear();

    (globals::show_content_info_video) ? ui->notShow->setChecked(false) : ui->notShow->setChecked(true);

    ui->textBrowser->setFocus();

    // ********************************************************
    // resize and move windows

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    this->resize(680, 420);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

void InfoWindow::setTitle(const QString title)
{
    ui->txt_title->setText(title);
}

void InfoWindow::setTex(const QString content)
{
    ui->textBrowser->setHtml(content);
}

void InfoWindow::closeEvent(QCloseEvent *event)
{
    globals::show_content_info_video = ! ui->notShow->isChecked();

    appSettings->setKeyAndValue("show_msg", "showMsgVideo", (globals::show_content_info_video) ? 1 : 0);
    appSettings->deleteLater();

    event->accept();
    // event->ignore();
}
