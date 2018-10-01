#include "correlationdatarow.h"

#include <algorithm>
#include <cmath>

CorrelationDataRow::CorrelationDataRow(const std::vector<double>& data, size_t row,
    size_t numColumns, NodeId nodeId, int computeCost) :
    _nodeId(nodeId), _cost(computeCost)
{
    auto cbegin = data.cbegin() + (row * numColumns);
    auto cend = cbegin + numColumns;
    _data = {cbegin, cend};
    _numColumns = std::distance(begin(), end());

    update();
}

CorrelationDataRow::CorrelationDataRow(const std::vector<double>& dataRow,
    NodeId nodeId, int computeCost) :
    CorrelationDataRow(dataRow, 0, dataRow.size(), nodeId, computeCost)
{}

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

    _sortedData = _data;
    std::sort(_sortedData.begin(), _sortedData.end());

    bool allPositive = true;

    for(auto value : *this)
    {
        allPositive = allPositive && !std::signbit(value);

        _sum += value;
        _sumSq += value * value;
        _mean += value / _numColumns;
        _minValue = std::min(_minValue, value);
        _maxValue = std::max(_maxValue, value);
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
