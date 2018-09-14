#ifndef CORRELATIONDATAROW_H
#define CORRELATIONDATAROW_H

#include "shared/graph/elementid.h"

#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>
#include <cmath>

struct CorrelationDataRow
{
    using ConstDataIterator = std::vector<double>::const_iterator;
    using DataIterator = std::vector<double>::iterator;
    using DataOffset = std::vector<double>::size_type;

    CorrelationDataRow() = default;
    CorrelationDataRow(const CorrelationDataRow&) = default;

    CorrelationDataRow(const std::vector<double>& data,
        size_t row, size_t numColumns,
        NodeId nodeId, int computeCost = 0) :
        _nodeId(nodeId), _cost(computeCost)
    {
        auto cbegin = data.cbegin() + (row * numColumns);
        auto cend = cbegin + numColumns;
        _data = {cbegin, cend};
        _numColumns = std::distance(begin(), end());

        update();
    }

    CorrelationDataRow(const std::vector<double>& dataRow,
        NodeId nodeId, int computeCost = 0) :
        CorrelationDataRow(dataRow, 0, dataRow.size(), nodeId, computeCost)
    {}

    std::vector<double> _data;
    std::vector<double> _sortedData;

    DataIterator begin() { return _data.begin(); }
    DataIterator end() { return _data.end(); }

    ConstDataIterator begin() const { return _data.begin(); }
    ConstDataIterator end() const { return _data.end(); }

    ConstDataIterator cbegin() const { return _data.cbegin(); }
    ConstDataIterator cend() const { return _data.cend(); }

    DataIterator sortedBegin() { return _sortedData.begin(); }
    DataIterator sortedEnd() { return _sortedData.end(); }

    size_t _numColumns = 0;

    NodeId _nodeId;

    int _cost = 0;
    int computeCostHint() const { return _cost; }

    double _sum = 0.0;
    double _sumSq = 0.0;
    double _sumAllSq = 0.0;
    double _variability = 0.0;

    double _mean = 0.0;
    double _variance = 0.0;
    double _stddev = 0.0;
    double _coefVar = 0.0;

    double _minValue = std::numeric_limits<double>::max();
    double _maxValue = std::numeric_limits<double>::lowest();

    void update()
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
};

#endif // CORRELATIONDATAROW_H
