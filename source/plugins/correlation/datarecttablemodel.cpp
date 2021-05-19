/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datarecttablemodel.h"
#include <QDebug>

bool DataRectTableModel::transposed() const
{
    return _transposed;
}

void DataRectTableModel::setTransposed(bool transposed)
{
    if(_transposed != transposed)
    {
        beginResetModel();
        _transposed = transposed;
        if(_data != nullptr)
            _data->setTransposed(_transposed);
        endResetModel();
    }
}

int DataRectTableModel::rowCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numRows());
    return 0;
}

int DataRectTableModel::columnCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numColumns());
    return 0;
}

QVariant DataRectTableModel::data(const QModelIndex& index, int) const
{
    auto row = static_cast<size_t>(index.row());
    auto column = static_cast<size_t>(index.column());

    if(row >= _data->numRows() || column >= _data->numColumns())
        return  {};

    return _data->valueAt(column, row);
}

QHash<int, QByteArray> DataRectTableModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

void DataRectTableModel::setTabularData(TabularData& data)
{
    beginResetModel();
    _data = &data;
    _data->setTransposed(_transposed);
    endResetModel();
}
