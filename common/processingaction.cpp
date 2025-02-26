#include "processingaction.h"
#include "ui_processingaction.h"

ProcessingAction::ProcessingAction(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProcessingAction)
{
    ui->setupUi(this);

    setWindowTitle(tr("Procesarea ..."));
    QMovie *movie = new QMovie(":/img/Spinner200px-200px.gif");
    movie->setScaledSize(ui->loader->size()); // Setează dimensiunea GIF-ului la QLabel
    ui->loader->setMovie(movie);
    movie->start();

    // Șterge movie când QLabel este distrus
    connect(ui->loader, &QObject::destroyed, movie, &QObject::deleteLater);
    connect(this, &ProcessingAction::txtInfoChanged, this, &ProcessingAction::slot_txtInfoChanged);
}

ProcessingAction::~ProcessingAction()
{
    delete ui;
}

QString ProcessingAction::getTxtInfo()
{
    return m_txtInfo;
}

void ProcessingAction::setTxtInfo(QString txtInfo)
{
    m_txtInfo = txtInfo;
    emit txtInfoChanged();
}

void ProcessingAction::slot_txtInfoChanged()
{
    if (m_txtInfo.isEmpty())
        return;

    ui->txt_info->setText(m_txtInfo);
}
