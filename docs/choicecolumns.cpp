#include "choicecolumns.h"
#include <QGuiApplication>
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QMessageBox>
#include <QScreen>
#endif

ChoiceColumns::ChoiceColumns(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Alege colonițe"));

    createListWidget();
    createOtherWidgets();
    createLayout();
    createConnections();

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    int x = (screenWidth / 2) - (318 / 2);
    int y = (screenHeight / 2) - (302 / 2);
    move(x, y);
}

void ChoiceColumns::highlightChecked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked)
        item->setBackground(QColor(255,255,178));
    else
        item->setBackground(QColor(255,255,255));
}

void ChoiceColumns::save()
{

}

void ChoiceColumns::setListWidget()
{

}

void ChoiceColumns::createListWidget()
{

}

void ChoiceColumns::createOtherWidgets()
{
    viewBox   = new QGroupBox(tr("Lista colonițelor"));
    buttonBox = new QDialogButtonBox;
    btnOK    = buttonBox->addButton(QDialogButtonBox::Ok);
    btnClose = buttonBox->addButton(QDialogButtonBox::Close);
}

void ChoiceColumns::createLayout()
{

}

void ChoiceColumns::createConnections()
{

}
