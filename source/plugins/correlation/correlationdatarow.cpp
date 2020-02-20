/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "correlationdatarow.h"

#include "shared/utils/container.h"

#include <algorithm>
#include <cmath>

void CorrelationDataRow::update()
{
    _sum = 0.0;
    _sumSq = 0.0;
    _sumAllSq = 0.0;
    _variability = 0.0;

    _mean = 0.0;
    _variance = 0.0;
    _stddev = 0.0;
    _coefVar = 0.0;

    _minValue = std::numeric_limits<double>::max();
    _maxValue = std::numeric_limits<double>::lowest();
    _largestColumnIndex = 0;

    bool allPositive = true;

    for(size_t columnIndex = 0; columnIndex < _data.size(); columnIndex++)
    {
        auto value = _data.at(columnIndex);

        allPositive = allPositive && !std::signbit(value);

        _sum += value;
        _sumSq += value * value;
        _mean += value / _numColumns;
        _minValue = std::min(_minValue, value);
        _maxValue = std::max(_maxValue, value);

        if(std::abs(value) > std::abs(_data.at(_largestColumnIndex)))
            _largestColumnIndex = columnIndex;
    }

    _sumAllSq = _sum * _sum;
    _variability = std::sqrt((_numColumns * _sumSq) - _sumAllSq);

    double sum = 0.0;
    for(auto value : *this)
    {
        double x = (value - _mean);
        x *= x;
        sum += x;
    }

    _variance = sum / _numColumns;
    _stddev = std::sqrt(_variance);
    _coefVar = (allPositive && _mean > 0.0) ? _stddev / _mean : std::nan("1");
}

void CorrelationDataRow::generateRanking() const
{
    _rankingRow = std::make_shared<CorrelationDataRow>(u::rankingOf(_data), _nodeId, _cost);
}

const CorrelationDataRow* CorrelationDataRow::ranking() const
{
    return _rankingRow.get();
}
