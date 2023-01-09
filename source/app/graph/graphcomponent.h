/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef GRAPHCOMPONENT_H
#define GRAPHCOMPONENT_H

#include "shared/graph/igraphcomponent.h"
#include "graph.h"

class GraphComponent : public IGraphComponent
{
    friend class ComponentManager;

public:
    explicit GraphComponent(const Graph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
        _graph(other._graph),
        _nodeIds(other._nodeIds),
        _edgeIds(other._edgeIds)
    {}

    GraphComponent& operator=(const GraphComponent& other)
    {
        if(&other == this)
            return *this;

        _graph = other._graph;
        _nodeIds = other._nodeIds;
        _edgeIds = other._edgeIds;

        return *this;
    }

private:
    const Graph* _graph;
    std::vector<NodeId> _nodeIds;
    std::vector<EdgeId> _edgeIds;

public:
    const std::vector<NodeId>& nodeIds() const override { return _nodeIds; }
    const std::vector<EdgeId>& edgeIds() const override { return _edgeIds; }
    const IGraph& graph() const override { return *_graph; }
};

#endif // GRAPHCOMPONENT_H
