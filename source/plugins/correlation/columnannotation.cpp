/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "columnannotation.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"

ColumnAnnotation::ColumnAnnotation(const QString& name, const std::vector<QString>& values) :
    _name(name), _values(values)
{
    int index = 0;
    for(const auto& value : _values)
    {
        if(_uniqueValues.emplace(value, index).second)
            index++;

        if(_hasOnlyNumericValues)
        {
            if(u::isNumeric(value))
            {
                auto d = u::toNumber(value);
                _minValue = std::min(d, _minValue);
                _maxValue = std::max(d, _maxValue);
            }
            else if(!value.isEmpty())
            {
                _hasOnlyNumericValues = false;
                _minValue = std::numeric_limits<double>::max();
                _maxValue = std::numeric_limits<double>::lowest();
            }
        }
    }
}

ColumnAnnotation::ColumnAnnotation(const QString& name,
    const ColumnAnnotation::Iterator& begin,
    const ColumnAnnotation::Iterator& end) :
    ColumnAnnotation(name, {begin, end})
{}

size_t ColumnAnnotation::uniqueIndexOf(const QString& value) const
{
    if(u::contains(_uniqueValues, value))
        return _uniqueValues.at(value);

    qDebug() << "ColumnAnnotation::uniqueIndexOf: unique value" << value << "not found";
    return 0;
}

double ColumnAnnotation::normalisedNumericValueAt(size_t index) const
{
    Q_ASSERT(isNumeric());
    if(!isNumeric())
        return 0.0;

    Q_ASSERT(_minValue <= _maxValue);

    auto value = u::toNumber(valueAt(index));

    return (value - _minValue) / (_maxValue - _minValue);
}

bool ColumnAnnotation::operator==(const ColumnAnnotation& other) const
{
    return _name == other._name && _values == other._values;
}
