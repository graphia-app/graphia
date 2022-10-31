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

#ifndef CORRELATIONDATAVECTOR_H
#define CORRELATIONDATAVECTOR_H

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
class CorrelationDataVector
{
protected:
    std::vector<T> _data;

    NodeId _nodeId;
    uint64_t _cost = 0;

public:
    using ConstDataIterator = typename decltype(_data)::const_iterator;
    using DataIterator = typename decltype(_data)::iterator;
    using DataOffset = typename decltype(_data)::size_type;

    CorrelationDataVector() = default;
    CorrelationDataVector(const CorrelationDataVector&) = default;
    CorrelationDataVector& operator=(const CorrelationDataVector&) = default;
    virtual ~CorrelationDataVector() = default;

    // Column from row-wise input
    template<typename U>
    CorrelationDataVector(const std::vector<U>& data, size_t column,
        size_t numColumns, size_t numRows,
        uint64_t computeCost = 1) :
        _cost(computeCost)
    {
        _data.reserve(numRows);

        for(size_t row = 0; row < numRows; row++)
        {
            auto index = (row * numColumns) + column;
            _data.push_back(data.at(index));
        }
    }

    // Row from row-wise input
    template<typename U>
    CorrelationDataVector(const std::vector<U>& data, size_t row, size_t numColumns,
        NodeId nodeId, uint64_t computeCost = 1) :
        _data({data.cbegin() + static_cast<typename std::vector<U>::difference_type>(row * numColumns),
            data.cbegin() + static_cast<typename std::vector<U>::difference_type>((row * numColumns) + numColumns)}),
        _nodeId(nodeId), _cost(computeCost)
    {}

    template<typename U>
    CorrelationDataVector(const std::vector<U>& dataVector,
        NodeId nodeId, uint64_t computeCost = 1) :
        CorrelationDataVector(dataVector, 0, dataVector.size(), nodeId, computeCost)
    {}

    DataIterator begin() { return _data.begin(); }
    DataIterator end() { return _data.end(); }

    ConstDataIterator begin() const { return _data.begin(); }
    ConstDataIterator end() const { return _data.end(); }

    const std::vector<T>& data() const { return _data; }

    uint64_t computeCostHint() const { return _cost; }

    size_t size() const { return _data.size(); }
    const T& valueAt(size_t index) const { return _data.at(index); }
    void setValueAt(size_t index, const T& value) { _data[index] = value; }

    NodeId nodeId() const { return _nodeId; }

    virtual void update() {}
};

class ContinuousDataVector : public CorrelationDataVector<double>
{
private:
    u::Statistics _statistics;
    mutable std::shared_ptr<ContinuousDataVector> _rankingVector;

public:
    using CorrelationDataVector::CorrelationDataVector;

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
    const ContinuousDataVector* ranking() const;
};

class DiscreteDataVector : public CorrelationDataVector<QString>
{
public:
    using CorrelationDataVector::CorrelationDataVector;
};

class TokenisedDataVector : public CorrelationDataVector<size_t>
{
public:
    using CorrelationDataVector::CorrelationDataVector;
};

using ContinuousDataVectors = std::vector<ContinuousDataVector>;
using CDVIt = ContinuousDataVectors::const_iterator;

struct ContinuousDataVectorRelation
{
    CDVIt _a;
    CDVIt _b;
    double _r = 0.0;
};

using CorrelationVector = std::vector<ContinuousDataVectorRelation>;

using DiscreteDataVectors = std::vector<DiscreteDataVector>;
using TokenisedDataVectors = std::vector<TokenisedDataVector>;

template<typename DataVectors>
TokenisedDataVectors tokeniseDataVectors(const DataVectors& dataVectors)
{
    using T = typename DataVectors::value_type::ConstDataIterator::value_type;

    TokenisedDataVectors tokenisedDataVectors;

    size_t token = 1;
    std::map<T, size_t> valueMap;

    // Map various falsey values to the 0 token
    if constexpr(std::is_same_v<T, QString>)
        valueMap[""] = valueMap["0"] = valueMap["false"] = 0;
    else
        valueMap[0] = 0;

    for(const auto& dataVector : dataVectors)
    {
        std::vector<size_t> tokens;
        tokens.reserve(dataVector.size());

        for(const auto& value : dataVector)
        {
            if(!u::contains(valueMap, value))
            {
                valueMap[value] = token;
                token++;
            }

            tokens.emplace_back(valueMap.at(value));
        }

        tokenisedDataVectors.emplace_back(tokens, dataVector.nodeId(), dataVector.computeCostHint());
    }

    return tokenisedDataVectors;
}

#endif // CORRELATIONDATAVECTOR_H
