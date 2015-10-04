#ifndef MUTABLEGRAPH_H
#define MUTABLEGRAPH_H

#include "graph.h"

#include <deque>
#include <mutex>
#include <functional>

class MutableGraph : public Graph
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
    const Node& nodeById(NodeId nodeId) const;
    bool containsNodeId(NodeId nodeId) const;
    NodeIdDistinctSetCollection::Type typeOf(NodeId nodeId) const;
    NodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const;

    NodeId addNode();
    NodeId addNode(NodeId nodeId);
    NodeId addNode(const Node& node);
    template<typename C> void addNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            addNode(nodeId);

        endTransaction();
    }

    void removeNode(NodeId nodeId);
    template<typename C> void removeNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            removeNode(nodeId);

        endTransaction();
    }

    const std::vector<EdgeId>& edgeIds() const;
    int numEdges() const;
    const Edge& edgeById(EdgeId edgeId) const;
    bool containsEdgeId(EdgeId edgeId) const;
    EdgeIdDistinctSetCollection::Type typeOf(EdgeId edgeId) const;
    EdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const;
    EdgeIdDistinctSet edgeIdsForNodeId(NodeId nodeId) const;
    EdgeIdDistinctSet inEdgeIdsForNodeId(NodeId nodeId) const;
    EdgeIdDistinctSet outEdgeIdsForNodeId(NodeId nodeId) const;

    template<typename C> EdgeIdDistinctSet inEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSet set;

        for(auto nodeId : nodeIds)
            set.add(_n._nodes[nodeId]._inEdgeIds);

        return set;
    }

    template<typename C> EdgeIdDistinctSet outEdgeIdsForNodeIds(const C& nodeIds) const
    {
        EdgeIdDistinctSet set;

        for(auto nodeId : nodeIds)
            set.add(_n._nodes[nodeId]._outEdgeIds);

        return set;
    }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId);
    EdgeId addEdge(const Edge& edge);
    template<typename C> void addEdges(const C& edges)
    {
        if(edges.empty())
            return;

        beginTransaction();

        for(const auto& edge : edges)
            addEdge(edge);

        endTransaction();
    }

    void removeEdge(EdgeId edgeId);
    template<typename C> void removeEdges(const C& edgeIds)
    {
        if(edgeIds.empty())
            return;

        beginTransaction();

        for(auto edgeId : edgeIds)
            removeEdge(edgeId);

        endTransaction();
    }

    void contractEdge(EdgeId edgeId);
    void contractEdges(const EdgeIdSet& edgeIds);

    void reserve(const Graph& other);
    void cloneFrom(const Graph& other);

    void update();

private:
    int _graphChangeDepth = 0;
    std::mutex _mutex;

    void beginTransaction();
    void endTransaction();

public:
    class ScopedTransaction
    {
    public:
        ScopedTransaction(MutableGraph& graph);
        ~ScopedTransaction();

    private:
        MutableGraph& _graph;
    };

    void performTransaction(std::function<void(MutableGraph& graph)> transaction);
};

#endif // MUTABLEGRAPH_H

