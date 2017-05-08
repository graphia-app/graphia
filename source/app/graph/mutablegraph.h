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
    virtual ~MutableGraph();

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

    void reserveNodeId(NodeId nodeId);
    void reserveEdgeId(EdgeId edgeId);

    NodeId mergeNodes(NodeId nodeIdA, NodeId nodeIdB);
    EdgeId mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB);

public:
    void clear();

    const std::vector<NodeId>& nodeIds() const;
    int numNodes() const;
    const INode& nodeById(NodeId nodeId) const;
    bool containsNodeId(NodeId nodeId) const;
    MultiElementType typeOf(NodeId nodeId) const;
    ConstNodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const;

    NodeId addNode();
    NodeId addNode(NodeId nodeId);
    NodeId addNode(const INode& node);
    void removeNode(NodeId nodeId);

    const std::vector<EdgeId>& edgeIds() const;
    int numEdges() const;
    const IEdge& edgeById(EdgeId edgeId) const;
    bool containsEdgeId(EdgeId edgeId) const;
    MultiElementType typeOf(EdgeId edgeId) const;
    ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const;
    EdgeIdDistinctSets edgeIdsForNodeId(NodeId nodeId) const;
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

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId);
    EdgeId addEdge(const IEdge& edge);
    void removeEdge(EdgeId edgeId);

    void contractEdge(EdgeId edgeId);
    void contractEdges(const EdgeIdSet& edgeIds);

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

    void update();

private:
    int _graphChangeDepth = 0;
    bool _graphChangeOccurred = false;
    std::mutex _mutex;

    void beginTransaction();
    void endTransaction(bool graphChangeOccurred = true);
};

#endif // MUTABLEGRAPH_H

