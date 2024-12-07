/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "tabulardatamodel.h"

bool TabularDataModel::transposed() const
{
    return _transposed;
}

void TabularDataModel::setTransposed(bool transposed)
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

bool TabularDataModel::columnIsNumerical(int column) const
{
    auto c = static_cast<size_t>(column);

    if(_data != nullptr && c < _data->numColumns())
    {
        auto type = _data->columnTypeIdentity(c, 0).type();

        return type == TypeIdentity::Type::Int || type == TypeIdentity::Type::Float;
    }

    return false;
}

int TabularDataModel::rowCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numRows());

    return 0;
}

int TabularDataModel::columnCount(const QModelIndex&) const
{
    if(_data != nullptr)
        return static_cast<int>(_data->numColumns());

    return 0;
}

QVariant TabularDataModel::data(const QModelIndex& index, int) const
{
    auto row = static_cast<size_t>(index.row());
    auto column = static_cast<size_t>(index.column());

    if(row >= _data->numRows() || column >= _data->numColumns())
        return  {};

    return _data->valueAt(column, row);
}

QHash<int, QByteArray> TabularDataModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

void TabularDataModel::setTabularData(TabularData& data)
{
    beginResetModel();
    _data = &data;
    _data->setTransposed(_transposed);
    endResetModel();
}
