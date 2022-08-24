/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "distancematrix.h"

#include <QtGlobal>

DistanceMatrix::DistanceMatrix(size_t size) : _size(size)
{
    auto square = size * size;
    auto diagonal = size;
    auto twoHalves = square - diagonal;

    _values.resize(twoHalves / 2);
}

static size_t indexOf(size_t column, size_t row, size_t size)
{
    Q_ASSERT(column != row);

    if(row > column)
        std::swap(column, row);

    auto tri = (row * (row + 1)) / 2;
    auto index = (row * (size - 1)) + (column - 1);

    return index - tri;
}

double DistanceMatrix::valueAt(size_t column, size_t row) const
{
    if(column == row)
        return 1.0;

    return _values.at(indexOf(column, row, _size));
}

void DistanceMatrix::setValueAt(size_t column, size_t row, double value)
{
    if(column == row)
        return;

    _values[indexOf(column, row, _size)] = value;
}
