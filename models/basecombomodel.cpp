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

#include "basecombomodel.h"
#include <QSqlQuery>

namespace
{
    enum Columns // Depends with 'query.prepare( QString( "SELECT ... '
    {
        Id,
        Data,
    };
}

BaseComboModel::BaseComboModel( const QString& visualColumn, const QString& queryTail, QObject *parent ) :QSqlQueryModel( parent )
{
    QSqlQuery query;
    query.prepare(QString( "SELECT %1.id, %2 FROM %3" ).arg(queryTail.split( ' ' ).first(), visualColumn, queryTail));
    // I.e. query.prepare( "SELECT country.id, countryname || ' - ' || countrycode  FROM country" );
    query.exec();
    QSqlQueryModel::setQuery( std::move(query) );
}

QVariant BaseComboModel::dataFromParent( QModelIndex index, int column ) const
{
    return QSqlQueryModel::data( QSqlQueryModel::index( index.row() - 1 // "- 1" because make first row empty
                                                        , column ) );
}

int BaseComboModel::rowCount(const QModelIndex &parent) const
{
    return QSqlQueryModel::rowCount( parent ) + 1; // Add info about first empty row
}

QVariant BaseComboModel::data(const QModelIndex & item, int role /* = Qt::DisplayRole */) const
{
    QVariant result;

    if( item.row() == 0 ){ // Make first row empty
        switch( role ){
        case Qt::UserRole:
            result = 0;
            break;
        case Qt::DisplayRole:
            result = "(please select)";
            break;
        default:
            break;
        }
    } else {
        switch( role ){
        case Qt::UserRole:
            result = dataFromParent( item, Id );
            break;
        case Qt::DisplayRole:
            result = dataFromParent( item, Data );
            break;
        default:
            break;
        }
    }
    return result;
}
