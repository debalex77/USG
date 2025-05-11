/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "mdiareacontainer.h"
#include <QMdiSubWindow>
#include <QDebug>

MdiAreaContainer::MdiAreaContainer(QMdiArea *widget, QObject *parent)
    : QObject(parent),
    m_mdiArea(widget)
{
     updateCachedSubWindowList();
}

int MdiAreaContainer::count() const
{
    updateCachedSubWindowList(); // Ensure cache is up to date
    return cachedSubWindowList.count();
}

int MdiAreaContainer::currentIndex() const
{
    updateCachedSubWindowList(); // Ensure cache is up to date
    if (QMdiSubWindow *sub = m_mdiArea->activeSubWindow())
        return cachedSubWindowList.indexOf(sub);
    return -1; // daca nu este subwindow activ = -1
}

QWidget *MdiAreaContainer::widget(int index) const
{
    updateCachedSubWindowList(); // Ensure cache is up to date
    if (index < 0 || index >= cachedSubWindowList.size())
        return nullptr;
    return cachedSubWindowList.at(index)->widget();
}

void MdiAreaContainer::setCurrentIndex(int index)
{
    updateCachedSubWindowList(); // Ensure cache is up to date
    if (index < 0 || index >= cachedSubWindowList.size()) {
        qDebug() << "** WARNING Attempt to MdiAreaContainer::setCurrentIndex(-1 or out of range)";
        return;
    }
    QMdiSubWindow *subWindow = cachedSubWindowList.at(index);
    m_mdiArea->setActiveSubWindow(subWindow);
}

void MdiAreaContainer::addWidget(QWidget *widget)
{
    QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(widget, Qt::Window);
    // Ensure attribute aligns with manual deletion policy in remove()
    if (! subWindow->testAttribute(Qt::WA_DeleteOnClose)) {
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
    }
    subWindow->setWindowTitle(widget->windowTitle());
    subWindow->setWindowIcon(widget->windowIcon());
    subWindow->show();
    updateCachedSubWindowList(); // Update cache after adding
}

void MdiAreaContainer::remove(int index)
{
    updateCachedSubWindowList(); // Ensure cache is up to date
    if (index >= 0 && index < cachedSubWindowList.size()) {
        QMdiSubWindow *f = cachedSubWindowList.at(index);
        auto internalWidget = f->widget();
        m_mdiArea->removeSubWindow(internalWidget);
        delete f; // Subfereastra șterge widget-ul copil automat
    }
    updateCachedSubWindowList(); // Update cache after removal
}

QMdiSubWindow *MdiAreaContainer::currentSubWindow()
{
    return m_mdiArea->activeSubWindow();
}

void MdiAreaContainer::updateCachedSubWindowList() const
{
    cachedSubWindowList = m_mdiArea->subWindowList(QMdiArea::CreationOrder);
}
