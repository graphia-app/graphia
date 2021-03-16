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

#ifndef CORRELATIONDATAROW_H
#define CORRELATIONDATAROW_H

#include "shared/graph/elementid.h"
#include "shared/utils/statistics.h"

#include <vector>
#include <limits>
#include <iterator>
#include <memory>

class ContinuousDataRow
{
public:
    using ConstDataIterator = std::vector<double>::const_iterator;
    using DataIterator = std::vector<double>::iterator;
    using DataOffset = std::vector<double>::size_type;

    ContinuousDataRow() = default;
    ContinuousDataRow(const ContinuousDataRow&) = default;

    template<typename T>
    ContinuousDataRow(const std::vector<T>& data, size_t row, size_t numColumns,
        NodeId nodeId, uint64_t computeCost = 1) :
        _nodeId(nodeId), _cost(computeCost)
    {
        auto cbegin = data.cbegin() + (row * numColumns);
        auto cend = cbegin + numColumns;
        _data = {cbegin, cend};
        _numColumns = std::distance(begin(), end());

        update();
    }

    template<typename T>
    ContinuousDataRow(const std::vector<T>& dataRow,
        NodeId nodeId, uint64_t computeCost = 1) :
        ContinuousDataRow(dataRow, 0, dataRow.size(), nodeId, computeCost)
    {}

    DataIterator begin() { return _data.begin(); }
    DataIterator end() { return _data.end(); }

    ConstDataIterator begin() const { return _data.begin(); }
    ConstDataIterator end() const { return _data.end(); }

    uint64_t computeCostHint() const { return _cost; }

    size_t numColumns() const { return _numColumns; }
    double valueAt(size_t column) const { return _data.at(column); }
    void setValueAt(size_t column, double value) { _data[column] = value; }

    NodeId nodeId() const { return _nodeId; }

    double sum() const { return _statistics._sum; }
    double variability() const { return _statistics._variability; }
    double mean() const { return _statistics._mean; }
    double variance() const { return _statistics._variance; }
    double stddev() const { return _statistics._stddev; }
    double coefVar() const { return _statistics._coefVar; }

    double minValue() const { return _statistics._min; }
    double maxValue() const { return _statistics._max; }
    size_t largestColumnIndex() const { return _statistics._largestIndex; }

    void update();

    void generateRanking() const;
    const ContinuousDataRow* ranking() const;

private:
    std::vector<double> _data;
    size_t _numColumns = 0;
    NodeId _nodeId;
    uint64_t _cost = 0;
    u::Statistics _statistics;
    mutable std::shared_ptr<ContinuousDataRow> _rankingRow;
};

using ContinuousDataRows = std::vector<ContinuousDataRow>;

#endif // CORRELATIONDATAROW_H
