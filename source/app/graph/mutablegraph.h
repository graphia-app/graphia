#ifndef MUTABLEGRAPH_H
#define MUTABLEGRAPH_H

#include "graph.h"
#include "shared/graph/imutablegraph.h"

#include <deque>
#include <mutex>
#include <vector>
#include <map>

class UndirectedEdge
{
private:
    NodeId lo;
    NodeId hi;

public:
    UndirectedEdge(NodeId a, NodeId b)
    {
        std::tie(lo, hi) = std::minmax(a, b);
    }

    bool operator<(const UndirectedEdge& other) const
    {
        if(lo == other.lo)
            return hi < other.hi;

        return lo < other.lo;
    }
};

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
        std::vector<Node>           _nodes;

        void resize(std::size_t size)
        {
            _nodeIdsInUse.resize(size);
            _mergedNodeIds.resize(size);
            _nodes.resize(size);
        }

        void clear()
        {
            _nodeIdsInUse.clear();
            _mergedNodeIds.clear();
            _nodes.clear();
        }
    } _n;

    std::vector<NodeId> _nodeIds;
    std::deque<NodeId> _unusedNodeIds;

    struct
    {
        std::vector<bool>           _edgeIdsInUse;
        EdgeIdDistinctSetCollection _mergedEdgeIds;
        std::vector<Edge>           _edges;

        EdgeIdDistinctSetCollection _inEdgeIdsCollection;
        EdgeIdDistinctSetCollection _outEdgeIdsCollection;

        std::map<UndirectedEdge, EdgeIdDistinctSet> _connections;

        void resize(std::size_t size)
        {
            _edgeIdsInUse.resize(size);
            _mergedEdgeIds.resize(size);
            _edges.resize(size);
            _inEdgeIdsCollection.resize(size);
            _outEdgeIdsCollection.resize(size);
        }

        void clear()
        {
            _edgeIdsInUse.clear();
            _mergedEdgeIds.clear();
            _edges.clear();
            _inEdgeIdsCollection.clear();
            _outEdgeIdsCollection.clear();
            _connections.clear();
        }
    } _e;

    std::vector<EdgeId> _edgeIds;
    std::deque<EdgeId> _unusedEdgeIds;

    bool _updateRequired = false;

    void reserveNodeId(NodeId nodeId) override;
    void reserveEdgeId(EdgeId edgeId) override;

    NodeId mergeNodes(NodeId nodeIdA, NodeId nodeIdB);
    EdgeId mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB);

    MutableGraph& clone(const MutableGraph& other);

public:
    void clear() override;

    const std::vector<NodeId>& nodeIds() const override;
    int numNodes() const override;
    const INode& nodeById(NodeId nodeId) const override;
    bool containsNodeId(NodeId nodeId) const override;
    MultiElementType typeOf(NodeId nodeId) const override;
    ConstNodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const override;

    NodeId addNode() override;
    NodeId addNode(NodeId nodeId) override;
    NodeId addNode(const INode& node) override;
    void removeNode(NodeId nodeId) override;

    const std::vector<EdgeId>& edgeIds() const override;
    int numEdges() const override;
    const IEdge& edgeById(EdgeId edgeId) const override;
    bool containsEdgeId(EdgeId edgeId) const override;
    MultiElementType typeOf(EdgeId edgeId) const override;
    ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const override;
    EdgeIdDistinctSets edgeIdsForNodeId(NodeId nodeId) const override;
    EdgeIdDistinctSet inEdgeIdsForNodeId(NodeId nodeId) const;
    EdgeIdDistinctSet outEdgeIdsForNodeId(NodeId nodeId) const;

    template<typename C> EdgeIdDistinctSets inEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSets set;

        for(auto nodeId : nodeIds)
            set.add(_n._nodes[nodeId]._inEdgeIds);

        return set;
    }

    template<typename C> EdgeIdDistinctSets outEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSets set;

        for(auto nodeId : nodeIds)
            set.add(_n._nodes[nodeId]._outEdgeIds);

        return set;
    }

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

    void update() override;

private:
    int _graphChangeDepth = 0;
    bool _graphChangeOccurred = false;
    std::mutex _mutex;

    void beginTransaction() override;
    void endTransaction(bool graphChangeOccurred = true) override;
};

#endif // MUTABLEGRAPH_H

