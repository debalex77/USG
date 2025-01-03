#include "infowindow.h"
#include <QScreen>
#include "ui_infowindow.h"

InfoWindow::InfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow)
{
    ui->setupUi(this);

    appSettings = new AppSettings(this);

    ui->txt_title->clear();
    ui->textBrowser->clear();

    (globals::show_content_info_video) ? ui->notShow->setChecked(false)
                                       : ui->notShow->setChecked(true);

    ui->textBrowser->setFocus();

    connect(this, &InfoWindow::typeInfoChanged, this, &InfoWindow::slot_typeInfoChanged);

    // ********************************************************
    // resize and move windows

    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    this->resize(680, 420);
    int x = (screenWidth / 2) - (width() / 2);   //*0.1;
    int y = (screenHeight / 2) - (height() / 2); //*0.1;
    move(x, y);
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

InfoWindow::TypeInfo InfoWindow::getTypeInfo()
{
    return m_typeInfo;
}

void InfoWindow::setTypeInfo(TypeInfo typeInfo)
{
    m_typeInfo = typeInfo;
    emit typeInfoChanged();
}

void InfoWindow::setTitle(const QString title)
{
    ui->txt_title->setText(title);
}

void InfoWindow::setTex(const QString content)
{
    if (m_typeInfo == INFO_REALEASE)
        ui->textBrowser->setMarkdown(content);
    else if (m_typeInfo == INFO_VIDEO || m_typeInfo == INFO_REPORT)
        ui->textBrowser->setHtml(content);
}

void InfoWindow::slot_typeInfoChanged()
{
    if (m_typeInfo == INFO_REALEASE) {
        setWindowTitle(tr("Istoria versiunilor"));
        ui->txt_title->setText(tr("Descrierea versiunilor"));
        ui->label_image->setPixmap(QPixmap(QString::fromUtf8(":/img/history.png")));
        ui->label_image->setMinimumSize(QSize(32, 32));
        ui->label_image->setMaximumSize(QSize(32, 32));
        ui->notShow->setVisible(false);
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(11);
        ui->textBrowser->setFont(font);
        ui->textBrowser->setOpenExternalLinks(true);
    }
}

void InfoWindow::closeEvent(QCloseEvent *event)
{
    if (m_typeInfo == INFO_REALEASE) {
        appSettings->deleteLater();
        event->accept();
    } else {
        if (m_typeInfo == INFO_VIDEO) {
            globals::show_content_info_video = !ui->notShow->isChecked();
            appSettings->setKeyAndValue("show_msg",
                                        "showMsgVideo",
                                        (globals::show_content_info_video) ? 1 : 0);
        } else if (m_typeInfo == INFO_REPORT) {
            globals::show_info_reports = !ui->notShow->isChecked();
            appSettings->setKeyAndValue("show_msg",
                                        "showMsgReports",
                                        (globals::show_info_reports) ? 1 : 0);
        };

        appSettings->deleteLater();

        event->accept();
    }
}
