#include "mdiareacontainer.h"
#include <QMdiSubWindow>
#include <QDebug>

MdiAreaContainer::MdiAreaContainer(QMdiArea *widget, QObject *parent)
    : QObject(parent),
    m_mdiArea(widget)
{   
}

int MdiAreaContainer::count() const
{
    return m_mdiArea->subWindowList(QMdiArea::CreationOrder).count();
}

int MdiAreaContainer::currentIndex() const
{
    if (QMdiSubWindow *sub = m_mdiArea->activeSubWindow())
        return m_mdiArea->subWindowList(QMdiArea::CreationOrder).indexOf(sub);
    return -1; // daca nu este subwindow activ = -1
}

QWidget *MdiAreaContainer::widget(int index) const
{
    if (index < 0)
         return 0;
     return m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index)->widget();
}

void MdiAreaContainer::setCurrentIndex(int index)
{
    if (index < 0) {
        qDebug() << "** WARNING Attempt to MdiAreaContainer::setCurrentIndex(-1)";
        return;
    }
    QMdiSubWindow *subWindow = m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index);
    m_mdiArea->setActiveSubWindow(subWindow);
}

void MdiAreaContainer::addWidget(QWidget *widget)
{
    QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(widget, Qt::Window);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle(widget->windowTitle());
    subWindow->setWindowIcon(widget->windowIcon());
    subWindow->show();
}

void MdiAreaContainer::remove(int index)
{
    QList<QMdiSubWindow *> subWins = m_mdiArea->subWindowList(QMdiArea::CreationOrder);
    if (index >= 0 && index < subWins.size()) {
        QMdiSubWindow *f = subWins.at(index);
        auto internalWidget = f->widget();
        m_mdiArea->removeSubWindow(f->widget());
        delete f;
        delete internalWidget;
    }
}
