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

#ifndef CORRELATIONDATAROW_H
#define CORRELATIONDATAROW_H

#include "shared/graph/elementid.h"
#include "shared/utils/statistics.h"
#include "shared/utils/container.h"

#include <vector>
#include <limits>
#include <iterator>
#include <memory>
#include <map>
#include <type_traits>

#include <QString>

template<typename T>
class CorrelationDataRow
{
protected:
    std::vector<T> _data;

    size_t _numColumns = 0;
    NodeId _nodeId;
    uint64_t _cost = 0;

public:
    using ConstDataIterator = typename decltype(_data)::const_iterator;
    using DataIterator = typename decltype(_data)::iterator;
    using DataOffset = typename decltype(_data)::size_type;

    CorrelationDataRow() = default;
    CorrelationDataRow(const CorrelationDataRow&) = default;
    virtual ~CorrelationDataRow() = default;

    template<typename U>
    CorrelationDataRow(const std::vector<U>& data, size_t row, size_t numColumns,
        NodeId nodeId, uint64_t computeCost = 1) :
        _nodeId(nodeId), _cost(computeCost)
    {
        auto cbegin = data.cbegin() + (row * numColumns);
        auto cend = cbegin + numColumns;
        _data = {cbegin, cend};
        _numColumns = std::distance(begin(), end());
    }

    template<typename U>
    CorrelationDataRow(const std::vector<U>& dataRow,
        NodeId nodeId, uint64_t computeCost = 1) :
        CorrelationDataRow(dataRow, 0, dataRow.size(), nodeId, computeCost)
    {}

    DataIterator begin() { return _data.begin(); }
    DataIterator end() { return _data.end(); }

    ConstDataIterator begin() const { return _data.begin(); }
    ConstDataIterator end() const { return _data.end(); }

    const std::vector<T>& data() const { return _data; }

    uint64_t computeCostHint() const { return _cost; }

    size_t numColumns() const { return _numColumns; }
    const T& valueAt(size_t column) const { return _data.at(column); }
    void setValueAt(size_t column, const T& value) { _data[column] = value; }

    NodeId nodeId() const { return _nodeId; }

    virtual void update() {}
};

class ContinuousDataRow : public CorrelationDataRow<double>
{
private:
    u::Statistics _statistics;
    mutable std::shared_ptr<ContinuousDataRow> _rankingRow;

public:
    using CorrelationDataRow::CorrelationDataRow;

    double sum() const { return _statistics._sum; }
    double sumSq() const { return _statistics._sumSq; }
    double variability() const { return _statistics._variability; }
    double magnitude() const { return _statistics._magnitude; }
    double mean() const { return _statistics._mean; }
    double variance() const { return _statistics._variance; }
    double stddev() const { return _statistics._stddev; }
    double coefVar() const { return _statistics._coefVar; }

    double minValue() const { return _statistics._min; }
    double maxValue() const { return _statistics._max; }
    size_t largestColumnIndex() const { return _statistics._largestIndex; }

    void update() override;

    void generateRanking() const;
    const ContinuousDataRow* ranking() const;
};

class DiscreteDataRow : public CorrelationDataRow<QString>
{
public:
    using CorrelationDataRow::CorrelationDataRow;
};

class TokenisedDataRow : public CorrelationDataRow<size_t>
{
public:
    using CorrelationDataRow::CorrelationDataRow;
};

using ContinuousDataRows = std::vector<ContinuousDataRow>;
using DiscreteDataRows = std::vector<DiscreteDataRow>;
using TokenisedDataRows = std::vector<TokenisedDataRow>;

template<typename DataRows>
TokenisedDataRows tokeniseDataRows(const DataRows& dataRows)
{
    using T = typename DataRows::value_type::ConstDataIterator::value_type;

    TokenisedDataRows tokenisedDataRows;

    size_t token = 1;
    std::map<T, size_t> valueMap;

    // Map various falsey values to the 0 token
    if constexpr(std::is_same_v<T, QString>)
        valueMap[""] = valueMap["0"] = valueMap["false"] = 0;
    else
        valueMap[0] = 0;

    for(const auto& dataRow : dataRows)
    {
        std::vector<size_t> tokens;
        tokens.reserve(dataRow.numColumns());

        for(const auto& value : dataRow)
        {
            if(!u::contains(valueMap, value))
            {
                valueMap[value] = token;
                token++;
            }

            tokens.emplace_back(valueMap.at(value));
        }

        tokenisedDataRows.emplace_back(tokens, dataRow.nodeId(), dataRow.computeCostHint());
    }

    return tokenisedDataRows;
}

#endif // CORRELATIONDATAROW_H
