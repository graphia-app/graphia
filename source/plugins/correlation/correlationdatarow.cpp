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
