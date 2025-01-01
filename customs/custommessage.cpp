#include "custommessage.h"
#include "ui_custommessage.h"

CustomMessage::CustomMessage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CustomMessage)
{
    ui->setupUi(this);
    ui->detailedText->hide();
    ui->label_img->setPixmap(QIcon(":/img/right-arrow_tool_box.png").pixmap(QSize(16,16)));
    this->adjustSize();

    connect(ui->btnOK, &QToolButton::clicked, this, &CustomMessage::close);
    connect(ui->btnDetailed, &QToolButton::clicked, this, &CustomMessage::onShowDetailedText);
}

CustomMessage::~CustomMessage()
{
    delete ui;
}

void CustomMessage::setTextTitle(QString text)
{
    ui->txt_title->setText(text);
}

void CustomMessage::setDetailedText(QString text)
{
    ui->detailedText->setPlainText(text);
}

void CustomMessage::onShowDetailedText()
{
    if (ui->detailedText->isVisible()) {
        ui->detailedText->hide();
        ui->label_img->setPixmap(QIcon(":/img/right-arrow_tool_box.png").pixmap(QSize(16,16)));
        this->adjustSize();
    } else {
        ui->detailedText->show();
        ui->label_img->setPixmap(QIcon(":/img/down-arrow_tool_box.png").pixmap(QSize(16,16)));
        this->adjustSize();
    }
}
