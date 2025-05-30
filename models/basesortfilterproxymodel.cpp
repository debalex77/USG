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

#include "basesortfilterproxymodel.h"


BaseSortFilterProxyModel::BaseSortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{    
}

bool BaseSortFilterProxyModel::compareDates(const QVariant &left, const QVariant &right, const QString &format) const
{
    QDateTime leftDateTime = QDateTime::fromString(left.toString(), format);
    QDateTime rightDateTime = QDateTime::fromString(right.toString(), format);

    return leftDateTime.isValid() && rightDateTime.isValid() && leftDateTime < rightDateTime;
}

bool BaseSortFilterProxyModel::compareInts(const QVariant &left, const QVariant &right) const
{
    bool leftOk, rightOk;
    int leftInt = left.toInt(&leftOk);
    int rightInt = right.toInt(&rightOk);
    return leftOk && rightOk && leftInt < rightInt;
}

QVariant BaseSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //redefinim sortarea numeratiei randurilor tabelei
    if (role == Qt::DisplayRole){
        if (orientation == Qt::Vertical || section == 0){
            return section + 1;
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool BaseSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // const QAbstractItemModel* source = sourceModel(); // Obține modelul sursă într-un mod const
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    // SORTAREA - număr documente (QString reprezentând un int)
    if (leftData.typeId() == QMetaType::QString) {
        if (left.column() == Enums::PROXY_MODEL::PROXY_NUMBER_DOC && m_listFormType == ListFormType::ListDocuments) {
            return compareInts(leftData, rightData);
        }
    }

    // SORTAREA - QDateTime
    if (leftData.typeId() == QMetaType::QDateTime) {
        if (left.column() == Enums::PROXY_MODEL::PROXY_DATE_DOC && m_listFormType == ListFormType::ListDocuments) {
            return compareDates(leftData, rightData, "dd.MM.yyyy hh:mm:ss");
        }
    }

    // SORTAREA - QDate
    if (leftData.typeId() == QMetaType::QDate) {
        if (compareDates(leftData, rightData, "dd.MM.yyyy")) {
            return true;
        }
    }

    // SORTAREA - QDateTime fallback
    if (leftData.typeId() == QMetaType::QDateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    }

    // SORTAREA - QString (inclusiv sortare specială pentru emailuri)
    QString leftString = leftData.toString();
    QString rightString = rightData.toString();
    if (left.column() == 1) {
        static const QRegularExpression emailPattern("[\\w\\.]*@[\\w\\.]*");
        QRegularExpressionMatch matchLeft = emailPattern.match(leftString);
        QRegularExpressionMatch matchRight = emailPattern.match(rightString);
        if (matchLeft.hasMatch()) leftString = matchLeft.captured(0);
        if (matchRight.hasMatch()) rightString = matchRight.captured(0);
    }

    // Fallback implicit - sortare lexicografică
    return QString::localeAwareCompare(leftString, rightString) < 0;
}
