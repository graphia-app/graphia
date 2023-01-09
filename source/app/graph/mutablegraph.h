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

#ifndef MUTABLEGRAPH_H
#define MUTABLEGRAPH_H

#include "graph.h"

#include "shared/graph/imutablegraph.h"
#include "shared/graph/undirectededge.h"

#include <deque>
#include <mutex>
#include <vector>
#include <map>

class MutableGraph : public Graph, public virtual IMutableGraph
{
    Q_OBJECT

public:
    MutableGraph() = default;
    MutableGraph(const MutableGraph& other);

    ~MutableGraph() override;

private:
    struct
    {
        std::vector<bool>           _nodeIdsInUse;
        NodeIdDistinctSetCollection _mergedNodeIds;
        std::vector<int>            _multiplicities;
        std::vector<Node>           _nodes;

        void resize(std::size_t size)
        {
            _nodeIdsInUse.resize(size);
            _mergedNodeIds.resize(size);
            _multiplicities.resize(size);
            _nodes.resize(size);
        }

        void clear()
        {
            _nodeIdsInUse.clear();
            _mergedNodeIds.clear();
            _multiplicities.clear();
            _nodes.clear();
        }
    } _n;

    std::vector<NodeId> _nodeIds;
    std::deque<NodeId> _unusedNodeIds;

    struct
    {
        std::vector<bool>           _edgeIdsInUse;
        EdgeIdDistinctSetCollection _mergedEdgeIds;
        std::vector<int>            _multiplicities;
        std::vector<Edge>           _edges;

        EdgeIdDistinctSetCollection _inEdgeIdsCollection;
        EdgeIdDistinctSetCollection _outEdgeIdsCollection;

        std::map<UndirectedEdge, EdgeIdDistinctSet> _connections;

        void resize(std::size_t size)
        {
            _edgeIdsInUse.resize(size);
            _mergedEdgeIds.resize(size);
            _multiplicities.resize(size);
            _edges.resize(size);
            _inEdgeIdsCollection.resize(size);
            _outEdgeIdsCollection.resize(size);
        }

        void clear()
        {
            _edgeIdsInUse.clear();
            _mergedEdgeIds.clear();
            _multiplicities.clear();
            _edges.clear();
            _inEdgeIdsCollection.clear();
            _outEdgeIdsCollection.clear();
            _connections.clear();
        }
    } _e;

    std::vector<EdgeId> _edgeIds;
    std::deque<EdgeId> _unusedEdgeIds;

    bool _updateRequired = false;

    Node& nodeBy(NodeId nodeId);
    const Node& nodeBy(NodeId nodeId) const;
    void claimNodeId(NodeId nodeId);
    void releaseNodeId(NodeId nodeId);
    Edge& edgeBy(EdgeId edgeId);
    const Edge& edgeBy(EdgeId edgeId) const;
    void claimEdgeId(EdgeId edgeId);
    void releaseEdgeId(EdgeId edgeId);

    NodeId mergeNodes(NodeId nodeIdA, NodeId nodeIdB);
    EdgeId mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB);

    NodeId mergeNodes(const std::vector<NodeId>& nodeIds);
    EdgeId mergeEdges(const std::vector<EdgeId>& edgeIds);

    MutableGraph& clone(const MutableGraph& other);

public:
    void clear() override;

    const std::vector<NodeId>& nodeIds() const override;
    size_t numNodes() const override;
    const Node& nodeById(NodeId nodeId) const override;
    bool containsNodeId(NodeId nodeId) const override;
    MultiElementType typeOf(NodeId nodeId) const override;
    ConstNodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const override;
    int multiplicityOf(NodeId nodeId) const override;

    std::vector<EdgeId> edgeIdsBetween(NodeId nodeIdA, NodeId nodeIdB) const override;
    EdgeId firstEdgeIdBetween(NodeId nodeIdA, NodeId nodeIdB) const override;
    bool edgeExistsBetween(NodeId nodeIdA, NodeId nodeIdB) const override;

    void reserveNodeId(NodeId nodeId) override;

    NodeId addNode() override;
    NodeId addNode(NodeId nodeId) override;
    NodeId addNode(const INode& node) override;
    void removeNode(NodeId nodeId) override;

    const std::vector<EdgeId>& edgeIds() const override;
    size_t numEdges() const override;
    const Edge& edgeById(EdgeId edgeId) const override;
    bool containsEdgeId(EdgeId edgeId) const override;
    MultiElementType typeOf(EdgeId edgeId) const override;
    ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const override;
    int multiplicityOf(EdgeId edgeId) const override;
    EdgeIdDistinctSets edgeIdsForNodeId(NodeId nodeId) const override;
    EdgeIdDistinctSet inEdgeIdsForNodeId(NodeId nodeId) const;
    EdgeIdDistinctSet outEdgeIdsForNodeId(NodeId nodeId) const;

    template<typename C> EdgeIdDistinctSets inEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSets set;

        for(auto nodeId : nodeIds)
            set.add(nodeBy(nodeId)._inEdgeIds);

        return set;
    }

    template<typename C> EdgeIdDistinctSets outEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSets set;

        for(auto nodeId : nodeIds)
            set.add(nodeBy(nodeId)._outEdgeIds);

        return set;
    }

    void reserveEdgeId(EdgeId edgeId) override;

    EdgeId addEdge(NodeId sourceId, NodeId targetId) override;
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) override;
    EdgeId addEdge(const IEdge& edge) override;
    void removeEdge(EdgeId edgeId) override;

    void contractEdge(EdgeId edgeId) override;
    void contractEdges(const EdgeIdSet& edgeIds) override;

    MutableGraph& operator=(const MutableGraph& other);

    struct Diff
    {
        std::vector<NodeId> _nodesAdded;
        std::vector<NodeId> _nodesRemoved;
        std::vector<EdgeId> _edgesAdded;
        std::vector<EdgeId> _edgesRemoved;

        bool empty() const
        {
            return
                _nodesAdded.empty() &&
                _nodesRemoved.empty() &&
                _edgesAdded.empty() &&
                _edgesRemoved.empty();
        }
    };

    Diff diffTo(const MutableGraph& other);

    bool update() override;

    std::unique_lock<std::mutex> tryLock();

private:
    int _graphChangeDepth = 0;
    bool _graphChangeOccurred = false;
    std::mutex _mutex;

    void beginTransaction() final;
    void endTransaction(bool graphChangeOccurred = true) final;

signals:
    void transactionWillBegin(const MutableGraph*);
    void transactionEnded(const MutableGraph*);
};

#endif // MUTABLEGRAPH_H

