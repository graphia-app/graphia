#ifndef GRAPH_H
#define GRAPH_H

#include "elementid.h"

#include "../utils/cpp1x_hacks.h"
#include "../utils/debugpauser.h"

#include <QObject>
#include <QDebug>

#include <vector>
#include <deque>
#include <type_traits>
#include <functional>
#include <memory>
#include <mutex>

class GraphArray;
class GraphComponent;
class ComponentManager;
class ComponentSplitSet;
class ComponentMergeSet;

class Node
{
    friend class MutableGraph;

private:
    NodeId _id;
    EdgeIdSet _inEdgeIds;
    EdgeIdSet _outEdgeIds;
    NodeIdSet _adjacentNodeIds;

public:
    Node() {}

    Node(const Node& other) :
        _id(other._id),
        _inEdgeIds(other._inEdgeIds),
        _outEdgeIds(other._outEdgeIds),
        _adjacentNodeIds(other._adjacentNodeIds)
    {}

    Node(Node&& other) noexcept :
        _id(other._id),
        _inEdgeIds(std::move(other._inEdgeIds)),
        _outEdgeIds(std::move(other._outEdgeIds)),
        _adjacentNodeIds(std::move(other._adjacentNodeIds))
    {}

    Node& operator=(const Node& other)
    {
        if(this != &other)
        {
            _id                 = other._id;
            _inEdgeIds          = other._inEdgeIds;
            _outEdgeIds         = other._outEdgeIds;
            _adjacentNodeIds    = other._adjacentNodeIds;
        }

        return *this;
    }

    const EdgeIdSet edgeIds() const
    {
        EdgeIdSet edgeIds;
        edgeIds.insert(_inEdgeIds.cbegin(), _inEdgeIds.cend());
        edgeIds.insert(_outEdgeIds.cbegin(), _outEdgeIds.cend());
        return edgeIds;
    }

    const EdgeIdSet inEdgeIds() const { return _inEdgeIds; }
    const EdgeIdSet outEdgeIds() const { return _outEdgeIds; }
    const NodeIdSet adjacentNodeIds() const { return _adjacentNodeIds; }
    int degree() const { return static_cast<int>(_inEdgeIds.size() + _outEdgeIds.size()); }

    NodeId id() const { return _id; }
};

class Edge
{
    friend class MutableGraph;

private:
    EdgeId _id;
    NodeId _sourceId;
    NodeId _targetId;

public:
    Edge() {}

    Edge(const Edge& other) :
        _id(other._id),
        _sourceId(other._sourceId),
        _targetId(other._targetId)
    {}

    Edge(Edge&& other) noexcept :
        _id(other._id),
        _sourceId(other._sourceId),
        _targetId(other._targetId)
    {}

    Edge& operator=(const Edge& other)
    {
        if(this != &other)
        {
            _id         = other._id;
            _sourceId   = other._sourceId;
            _targetId   = other._targetId;
        }

        return *this;
    }

    NodeId sourceId() const { return _sourceId; }
    NodeId targetId() const { return _targetId; }
    NodeId oppositeId(NodeId nodeId) const
    {
        if(nodeId == _sourceId)
            return _targetId;
        else if(nodeId == _targetId)
            return _sourceId;

        return NodeId();
    }

    bool isLoop() const { return sourceId() == targetId(); }

    EdgeId id() const { return _id; }
};

class Graph : public QObject
{
    Q_OBJECT

public:
    Graph();
    virtual ~Graph();

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    virtual int numNodes() const = 0;
    virtual const Node& nodeById(NodeId nodeId) const = 0;
    NodeId firstNodeId() const;
    virtual bool containsNodeId(NodeId nodeId) const;
    virtual MultiNodeId::Type typeOf(NodeId nodeId) const = 0;

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    virtual int numEdges() const = 0;
    virtual const Edge& edgeById(EdgeId edgeId) const = 0;
    EdgeId firstEdgeId() const;
    virtual bool containsEdgeId(EdgeId edgeId) const;
    virtual MultiEdgeId::Type typeOf(EdgeId edgeId) const = 0;

    template<typename C, typename EdgesIdFn>
    EdgeIdSet edgeIdsForNodes(const C& nodeIds, EdgesIdFn edgeIdsFn) const
    {
        EdgeIdSet edgeIds;

        for(auto nodeId : nodeIds)
        {
            auto& node = nodeById(nodeId);
            for(auto edgeId : edgeIdsFn(node))
                edgeIds.insert(edgeId);
        }

        return edgeIds;
    }

    template<typename C> EdgeIdSet edgeIdsForNodes(const C& nodeIds) const
    {
        return edgeIdsForNodes(nodeIds, [](const Node& node) { return node.edgeIds(); });
    }

    template<typename C> EdgeIdSet inEdgeIdsForNodes(const C& nodeIds) const
    {
        return edgeIdsForNodes(nodeIds, [](const Node& node) { return node.inEdgeIds(); });
    }

    template<typename C> EdgeIdSet outEdgeIdsForNodes(const C& nodeIds) const
    {
        return edgeIdsForNodes(nodeIds, [](const Node& node) { return node.outEdgeIds(); });
    }

    template<typename C> std::vector<Edge> edgesForNodes(const C& nodeIds) const
    {
        auto edgeIds = edgeIdsForNodes(nodeIds);
        std::vector<Edge> edges;

        for(auto edgeId : edgeIds)
            edges.emplace_back(edgeById(edgeId));

        return edges;
    }

    virtual void reserve(const Graph& other) = 0;
    virtual void cloneFrom(const Graph& other) = 0;

    void enableComponentManagement();

    virtual const std::vector<ComponentId>& componentIds() const;
    int numComponents() const;
    const Graph* componentById(ComponentId componentId) const;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
    ComponentId componentIdOfLargestComponent() const;

    template<typename C> ComponentId componentIdOfLargestComponent(const C& componentIds) const
    {
        ComponentId largestComponentId;
        int maxNumNodes = 0;
        for(auto componentId : componentIds)
        {
            auto component = componentById(componentId);
            if(component->numNodes() > maxNumNodes)
            {
                maxNumNodes = component->numNodes();
                largestComponentId = componentId;
            }
        }

        return largestComponentId;
    }

    // Call this to ensure the Graph is in a consistent state
    // Usually it is called automatically and is generally only
    // necessary when accessing the Graph before changes have
    // been completed
    virtual void update() {}

    void setPhase(const QString& phase) const;
    void clearPhase() const;
    const QString& phase() const;

    mutable DebugPauser debugPauser;
    void dumpToQDebug(int detail) const;

private:
    template<typename> friend class NodeArray;
    template<typename> friend class EdgeArray;
    template<typename> friend class ComponentArray;

    NodeId _nextNodeId;
    EdgeId _nextEdgeId;

    std::unordered_set<GraphArray*> _nodeArrays;
    std::unordered_set<GraphArray*> _edgeArrays;

    std::unique_ptr<ComponentManager> _componentManager;

    mutable QString _phase;

    int numComponentArrays();
    void insertComponentArray(GraphArray* componentArray);
    void eraseComponentArray(GraphArray* componentArray);

protected:
    NodeId nextNodeId() const;
    NodeId largestNodeId() const { return NodeId(nextNodeId() - 1); }
    virtual void reserveNodeId(NodeId nodeId);
    EdgeId nextEdgeId() const;
    EdgeId largestEdgeId() const { return EdgeId(nextEdgeId() - 1); }
    virtual void reserveEdgeId(EdgeId edgeId);

signals:
    // The signals are listed here in the order in which they are emitted
    void graphWillChange(const Graph*) const;

    void nodeAdded(const Graph*, const Node*) const;
    void nodeWillBeRemoved(const Graph*, const Node*) const;
    void edgeAdded(const Graph*, const Edge*) const;
    void edgeWillBeRemoved(const Graph*, const Edge*) const;

    void componentsWillMerge(const Graph*, const ComponentMergeSet&) const;
    void componentWillBeRemoved(const Graph*, ComponentId, bool) const;
    void componentAdded(const Graph*, ComponentId, bool) const;
    void componentSplit(const Graph*, const ComponentSplitSet&) const;

    void graphChanged(const Graph*) const;

    void phaseChanged() const;
};

class MutableGraph : public Graph
{
    Q_OBJECT

public:
    virtual ~MutableGraph();

private:
    std::vector<bool> _nodeIdsInUse;
    std::vector<NodeId> _nodeIds;
    std::deque<NodeId> _unusedNodeIds;
    std::vector<Node> _nodes;
    std::vector<MultiNodeId> _multiNodeIds;

    std::vector<bool> _edgeIdsInUse;
    std::vector<EdgeId> _edgeIds;
    std::deque<EdgeId> _unusedEdgeIds;
    std::vector<Edge> _edges;
    std::vector<MultiEdgeId> _multiEdgeIds;

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
    MultiNodeId::Type typeOf(NodeId nodeId) const;

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
    MultiEdgeId::Type typeOf(EdgeId edgeId) const;

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
    template<typename C> void contractEdges(const C& edgeIds)
    {
        for(auto edgeId : edgeIds)
            contractEdge(edgeId);
    }

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

#endif // GRAPH_H
