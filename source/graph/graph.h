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
class AbstractComponentManager;
class ComponentSplitSet;
class ComponentMergeSet;

class Node
{
    friend class MutableGraph;

private:
    NodeId _id;
    EdgeIdSet _edgeIds;
    NodeIdSet _adjacentNodeIds;

public:
    Node() {}

    Node(const Node& other) :
        _id(other._id),
        _edgeIds(other._edgeIds),
        _adjacentNodeIds(other._adjacentNodeIds)
    {}

    Node(Node&& other) noexcept :
        _id(other._id),
        _edgeIds(std::move(other._edgeIds)),
        _adjacentNodeIds(std::move(other._adjacentNodeIds))
    {}

    Node& operator=(const Node& other)
    {
        if(this != &other)
        {
            _id                 = other._id;
            _edgeIds            = other._edgeIds;
            _adjacentNodeIds    = other._adjacentNodeIds;
        }

        return *this;
    }

    const EdgeIdSet edgeIds() const { return _edgeIds; }
    const NodeIdSet adjacentNodeIds() const { return _adjacentNodeIds; }
    int degree() const { return static_cast<int>(_edgeIds.size()); }

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

    const EdgeIdSet edgeIdsForNodes(const NodeIdSet& nodeIds) const;
    const std::vector<Edge> edgesForNodes(const NodeIdSet& nodeIds) const;

    virtual void reserve(const Graph& other) = 0;
    virtual void cloneFrom(const Graph& other) = 0;

    void enableComponentManagement(const Graph* other = nullptr);

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

    std::shared_ptr<AbstractComponentManager> _componentManager;

protected:
    NodeId nextNodeId() const;
    void setNextNodeId(NodeId nextNodeId);
    EdgeId nextEdgeId() const;
    void setNextEdgeId(EdgeId nextEdgeId);

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

    void reserveNodeId(NodeId nodeId);
    void reserveEdgeId(EdgeId edgeId);

    NodeId mergeNodes(NodeId nodeIdA, NodeId nodeIdB);
    EdgeId mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB);

    void updateElementIdData();

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
    void addNodes(const NodeIdSet& nodeIds);

    void removeNode(NodeId nodeId);
    void removeNodes(const NodeIdSet& nodeIds);

    const std::vector<EdgeId>& edgeIds() const;
    int numEdges() const;
    const Edge& edgeById(EdgeId edgeId) const;
    bool containsEdgeId(EdgeId edgeId) const;
    MultiEdgeId::Type typeOf(EdgeId edgeId) const;

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId);
    EdgeId addEdge(const Edge& edge);
    void addEdges(const std::vector<Edge>& edges);

    void removeEdge(EdgeId edgeId);
    void removeEdges(const EdgeIdSet& edgeIds);

    void contractEdge(EdgeId edgeId);

    void reserve(const Graph& other);
    void cloneFrom(const Graph& other);

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
