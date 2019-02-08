#ifndef CORRELATIONDATAROW_H
#define CORRELATIONDATAROW_H

#include "shared/graph/elementid.h"

#include <vector>
#include <limits>
#include <iterator>
#include <memory>

class CorrelationDataRow
{
public:
    using ConstDataIterator = std::vector<double>::const_iterator;
    using DataIterator = std::vector<double>::iterator;
    using DataOffset = std::vector<double>::size_type;

    CorrelationDataRow() = default;
    CorrelationDataRow(const CorrelationDataRow&) = default;

    template<typename T>
    CorrelationDataRow(const std::vector<T>& data, size_t row, size_t numColumns,
        NodeId nodeId, int computeCost = 0) :
        _nodeId(nodeId), _cost(computeCost)
    {
        auto cbegin = data.cbegin() + (row * numColumns);
        auto cend = cbegin + numColumns;
        _data = {cbegin, cend};
        _numColumns = std::distance(begin(), end());

        update();
    }

    template<typename T>
    CorrelationDataRow(const std::vector<T>& dataRow,
        NodeId nodeId, int computeCost = 0) :
        CorrelationDataRow(dataRow, 0, dataRow.size(), nodeId, computeCost)
    {}

    DataIterator begin() { return _data.begin(); }
    DataIterator end() { return _data.end(); }

    ConstDataIterator begin() const { return _data.begin(); }
    ConstDataIterator end() const { return _data.end(); }

    int computeCostHint() const { return _cost; }

    size_t numColumns() const { return _numColumns; }
    double valueAt(size_t column) const { return _data.at(column); }
    void setValueAt(size_t column, double value) { _data[column] = value; }

    NodeId nodeId() const { return _nodeId; }

    double sum() const { return _sum; }
    double variability() const { return _variability; }
    double mean() const { return _mean; }
    double variance() const { return _variance; }
    double stddev() const { return _stddev; }
    double coefVar() const { return _coefVar; }

    double minValue() const { return _minValue; }
    double maxValue() const { return _maxValue; }

    void update();

    void generateRanking() const;
    const CorrelationDataRow* ranking() const;

private:
    std::vector<double> _data;

    size_t _numColumns = 0;

    NodeId _nodeId;

    int _cost = 0;

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

    mutable std::shared_ptr<CorrelationDataRow> _rankingRow;
};

#endif // CORRELATIONDATAROW_H
