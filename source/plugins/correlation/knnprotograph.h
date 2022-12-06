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

#ifndef KNNPROTOGRAPH_H
#define KNNPROTOGRAPH_H

#include "correlationtype.h"
#include "correlationdatavector.h"

#include <cstddef>
#include <mutex>
#include <vector>
#include <compare>

#include <QVariantMap>

template<typename DataVectors>
class KnnProtoGraph
{
private:
    struct ProtoNode
    {
        std::mutex _mutex;
        const KnnProtoGraph* _protoGraph = nullptr;

        struct ProtoEdge
        {
            typename DataVectors::const_iterator _it;
            double _r = 0.0;

            auto operator<=>(const ProtoEdge& other) const { return _r <=> other._r; }
        };

        std::vector<ProtoEdge> _protoEdges;

        void add(typename DataVectors::const_iterator it, double r)
        {
            const std::unique_lock<std::mutex> lock(_mutex);

            auto minR = !_protoEdges.empty() ? _protoEdges.back()._r : 0.0;

            if(!correlationExceedsThreshold(_protoGraph->_polarity, r, minR))
                return;

            _protoEdges.push_back({it, r});
            std::inplace_merge(_protoEdges.begin(), std::prev(_protoEdges.end()),
                _protoEdges.end(), std::greater());
            _protoEdges.resize(std::min(_protoEdges.size(), _protoGraph->_k));
        }
    };

    std::vector<ProtoNode> _protoNodes;

    typename DataVectors::const_iterator _begin;
    CorrelationPolarity _polarity = CorrelationPolarity::Positive;
    size_t _k = 0;

public:
    KnnProtoGraph(const DataVectors& vectors, const QVariantMap& parameters) :
        _protoNodes(vectors.size()), _begin(vectors.begin())
    {
        _k = static_cast<size_t>(parameters[QStringLiteral("threshold")].toInt());
        _polarity = NORMALISE_QML_ENUM(CorrelationPolarity, parameters[QStringLiteral("correlationPolarity")].toInt());

        for(auto& protoNode : _protoNodes)
            protoNode._protoGraph = this;
    }

    class iterator
    {
    private:
        const KnnProtoGraph* _protoGraph = nullptr;
        typename std::vector<ProtoNode>::const_iterator _protoNodeIt;
        typename std::vector<typename ProtoNode::ProtoEdge>::const_iterator _protoEdgeIt;

    public:
        using self_type = iterator;
        using value_type = CorrelationDataVectorRelation<DataVectors>;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = size_t;

        iterator(const KnnProtoGraph* protoGraph, bool end) :
            _protoGraph(protoGraph)
        {
            if(!end)
            {
                _protoNodeIt = _protoGraph->_protoNodes.begin();
                _protoEdgeIt = _protoNodeIt->_protoEdges.begin();

                while(_protoEdgeIt == _protoNodeIt->_protoEdges.end())
                {
                    _protoNodeIt++;

                    if(_protoNodeIt == _protoGraph->_protoNodes.end())
                    {
                        _protoEdgeIt = {};
                        break;
                    }

                    _protoEdgeIt = _protoNodeIt->_protoEdges.begin();
                }
            }
            else
            {
                _protoNodeIt = _protoGraph->_protoNodes.end();
                _protoEdgeIt = {};
            }
        }

        self_type operator++()
        {
            self_type i = *this;

            do
            {
                if(_protoEdgeIt != _protoNodeIt->_protoEdges.end())
                    _protoEdgeIt++;

                while(_protoEdgeIt == _protoNodeIt->_protoEdges.end())
                {
                    _protoNodeIt++;
                    if(_protoNodeIt == _protoGraph->_protoNodes.end())
                        return i;

                    _protoEdgeIt = _protoNodeIt->_protoEdges.begin();
                }
            }
            // Skip edges we've already iterated over (in the other direction)
            while((**this)._a > (**this)._b);

            return i;
        }

        value_type operator*() const
        {
            Q_ASSERT(_protoNodeIt != _protoGraph->_protoNodes.end());

            auto nodeOffset = (_protoNodeIt - _protoGraph->_protoNodes.begin());
            auto a = _protoGraph->_begin + nodeOffset;
            auto b = _protoEdgeIt->_it;
            const double r = _protoEdgeIt->_r;

            return {a, b, r};
        }

        bool operator!=(const self_type& other) const { return !operator==(other); }
        bool operator==(const self_type& other) const { return _protoNodeIt == other._protoNodeIt; }
    };

    iterator begin() const { return {this, false}; }
    iterator end() const   { return {this, true}; }

    void add(typename DataVectors::const_iterator a, typename DataVectors::const_iterator b, double r)
    {
        auto aOffset = static_cast<size_t>(a - _begin);
        auto bOffset = static_cast<size_t>(b - _begin);

        Q_ASSERT(aOffset < _protoNodes.size() && bOffset < _protoNodes.size());
        if(aOffset >= _protoNodes.size() || bOffset >= _protoNodes.size())
        {
            qDebug() << "ProtoGraph source or target out of range";
            return;
        }

        _protoNodes.at(aOffset).add(b, r);
        _protoNodes.at(bOffset).add(a, r);
    }
};

#endif // KNNPROTOGRAPH_H
