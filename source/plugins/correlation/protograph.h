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

#ifndef PROTOGRAPH_H
#define PROTOGRAPH_H

#include "correlationtype.h"
#include "correlationdatavector.h"

#include <cstddef>
#include <mutex>
#include <vector>
#include <compare>

class ProtoGraph
{
private:
    struct ProtoNode
    {
        std::mutex _mutex;
        const ProtoGraph* _protoGraph = nullptr;

        struct ProtoEdge
        {
            CDVIt _it;
            double _r = 0.0;

            auto operator<=>(const ProtoEdge& other) const { return _r <=> other._r; }
        };

        std::vector<ProtoEdge> _protoEdges;

        void add(CDVIt it, double r);
    };

    std::vector<ProtoNode> _protoNodes;

    CDVIt _begin;
    CorrelationPolarity _polarity = CorrelationPolarity::Positive;
    size_t _k = 0;

public:
    ProtoGraph(const ContinuousDataVectors& vectors, CorrelationPolarity polarity, size_t k);

    class iterator
    {
    private:
        const ProtoGraph* _protoGraph = nullptr;
        std::vector<ProtoNode>::const_iterator _protoNodeIt;
        std::vector<ProtoNode::ProtoEdge>::const_iterator _protoEdgeIt;

    public:
        using self_type = iterator;
        using value_type = ContinuousDataVectorRelation;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = size_t;

        iterator(const ProtoGraph* protoGraph, bool end);

        self_type operator++();

        ContinuousDataVectorRelation operator*() const;

        bool operator!=(const self_type& other) const { return !operator==(other); }
        bool operator==(const self_type& other) const { return _protoNodeIt == other._protoNodeIt; }
    };

    iterator begin() const { return {this, false}; }
    iterator end() const   { return {this, true}; }

    void add(CDVIt a, CDVIt b, double r);
};

#endif // PROTOGRAPH_H
