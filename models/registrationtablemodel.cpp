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

#include "registrationtablemodel.h"
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QFont>

RegistrationTableModel::RegistrationTableModel(int rows, int cols, QObject *parent)
    : QAbstractTableModel(parent), row_count(rows), col_count(cols)
{

}

int RegistrationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return row_count;
}

int RegistrationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return col_count;
}

QVariant RegistrationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return cell_data.value(index, QVariant());
    case Qt::EditRole:
        return cell_data.value(index, QVariant());
    case Qt::UserRole:
        return cell_data.value(index, QVariant());
    case Qt::BackgroundRole:
        if (! cell_data.value(index).isNull())
            return QBrush(QColor(247,246,202));
        return QVariant();
    case Qt::FontRole:
        if (! cell_data.value(RegistrationTableModel::index(index.row(), 1), Qt::DisplayRole).toString().isEmpty() &&
            cell_data.value(RegistrationTableModel::index(index.row(), 1), Qt::DisplayRole).toInt() == 1 &&
            ! cell_data.value(index).toString().isEmpty()){
            QFont font = cell_data.value(index, Qt::FontRole).value<QFont>();
            font.setStrikeOut(true);
            return font;
        }
        return QVariant();
    default:
        return QVariant();
    }

    return QVariant();
}

bool RegistrationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role == Qt::DisplayRole || role == Qt::EditRole){
        cell_data.insert(index, value);
        emit dataChanged(index, index);
        emit m_data_changed();
    }
    return false;
}

Qt::ItemFlags RegistrationTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

RegistrationPatientsModel::RegistrationPatientsModel(int rows, int cols, QObject *parent) : RegistrationTableModel(rows, cols, parent)
{

}

void RegistrationPatientsModel::setDataPatient(int time_num, int column_num, QString _data)
{
    setData(index(time_num - 1, column_num - 1), _data, Qt::EditRole);
    emit m_data_changed();
}


QVariant RegistrationPatientsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return init_time.addSecs(60 * 15 * section).toString("hh:mm");
    else
        switch (section) {
        case section_id:
            return tr("id");
        case section_execute:
            return tr("Efectuat");
        case section_data_patient:
            return tr("Nume, prenume pacientului");
        case section_investigation:
            return tr("Investigația");
        case section_organization:
            return tr("Organizația");
        case section_doctor:
            return tr("Doctor");
        case section_comment:
            return tr("Comentariu / note");
        default:
            return QAbstractTableModel::headerData(section, orientation, role);
        }

    return QAbstractTableModel::headerData(section, orientation, role);
}
