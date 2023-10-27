#include "historyversion.h"
#include "loggingcategories.h"
#include "ui_historyversion.h"

HistoryVersion::HistoryVersion(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryVersion)
{
    ui->setupUi(this);
    setWindowTitle(tr("Istoria versiunilor 'USG'"));
    ui->view->setAttribute(Qt::WA_DeleteOnClose);
    ui->view->load(QUrl("qrc:/html/history_version.html"));

    qWarning(logInfo()) << tr("Deschisă forma 'Istoria versiunilor'.");
}

HistoryVersion::~HistoryVersion()
{
    delete ui;
}
