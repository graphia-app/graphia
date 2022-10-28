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

#include "protograph.h"

ProtoGraph::ProtoGraph(const ContinuousDataVectors& vectors, CorrelationPolarity polarity, size_t k) :
    _protoNodes(vectors.size()), _begin(vectors.begin()), _polarity(polarity), _k(k)
{
    for(auto& protoNode : _protoNodes)
        protoNode._protoGraph = this;
}

ProtoGraph::iterator::iterator(const ProtoGraph* protoGraph, bool end) :
    _protoGraph(protoGraph)
{
    if(!end)
    {
        _protoNodeIt = _protoGraph->_protoNodes.begin();

        if(!_protoGraph->_protoNodes.empty())
            _protoEdgeIt = _protoNodeIt->_protoEdges.begin();
    }
    else
        _protoNodeIt = _protoGraph->_protoNodes.end();
}

ContinuousDataVectorRelation ProtoGraph::iterator::operator*() const
{
    Q_ASSERT(_protoNodeIt != _protoGraph->_protoNodes.end());

    auto nodeOffset = (_protoNodeIt - _protoGraph->_protoNodes.begin());
    auto a = _protoGraph->_begin + nodeOffset;
    auto b = _protoEdgeIt->_it;
    double r = _protoEdgeIt->_r;

    return {a, b, r};
}

ProtoGraph::iterator::self_type ProtoGraph::iterator::operator++()
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

void ProtoGraph::ProtoNode::add(CDVIt it, double r)
{
    std::unique_lock<std::mutex> lock(_mutex);

    auto minR = !_protoEdges.empty() ? _protoEdges.back()._r : 0.0;

    if(!correlationExceedsThreshold(_protoGraph->_polarity, r,  minR))
        return;

    _protoEdges.push_back({it, r});
    std::inplace_merge(_protoEdges.begin(), std::prev(_protoEdges.end()),
        _protoEdges.end(), std::greater());
    _protoEdges.resize(std::min(_protoEdges.size(), _protoGraph->_k));
}

void ProtoGraph::add(CDVIt a, CDVIt b, double r)
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
