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

#ifndef IMUTABLEGRAPH_H
#define IMUTABLEGRAPH_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

#include "shared/graph/igraph.h"

class IMutableGraph : public virtual IGraph
{
public:
    ~IMutableGraph() override = default;

    virtual void clear() = 0;

    virtual void reserveNodeId(NodeId nodeId) = 0;

    virtual NodeId addNode() = 0;
    virtual NodeId addNode(NodeId nodeId) = 0;
    virtual NodeId addNode(const INode& node) = 0;
    template<typename C> void addNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            addNode(nodeId);

        endTransaction();
    }

    virtual void removeNode(NodeId nodeId) = 0;
    template<typename C> void removeNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            removeNode(nodeId);

        endTransaction();
    }

    virtual void reserveEdgeId(EdgeId edgeId) = 0;

    virtual EdgeId addEdge(NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(const IEdge& edge) = 0;
    template<typename C> void addEdges(const C& edges)
    {
        if(edges.empty())
            return;

        beginTransaction();

        for(const auto& edge : edges)
            addEdge(edge);

        endTransaction();
    }

    virtual void removeEdge(EdgeId edgeId) = 0;
    template<typename C> void removeEdges(const C& edgeIds)
    {
        if(edgeIds.empty())
            return;

        beginTransaction();

        for(auto edgeId : edgeIds)
            removeEdge(edgeId);

        endTransaction();
    }

    virtual void contractEdge(EdgeId edgeId) = 0;
    virtual void contractEdges(const EdgeIdSet& edgeIds) = 0;

protected:
    virtual void beginTransaction() = 0;
    virtual void endTransaction(bool graphChangeOccurred = true) = 0;

public:
    class ScopedTransaction
    {
    public:
        explicit ScopedTransaction(IMutableGraph& graph) : _graph(&graph)
        { _graph->beginTransaction(); }
        ~ScopedTransaction() { _graph->endTransaction(false); }

    private:
        IMutableGraph* _graph;
    };

    template<typename Fn>
    void performTransaction(const Fn& transaction)
    {
        const ScopedTransaction lock(*this);
        transaction(*this);
    }
};

#endif // IMUTABLEGRAPH_H
