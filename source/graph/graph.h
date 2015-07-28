#ifndef GRAPH_H
#define GRAPH_H

#include "../utils/cpp1x_hacks.h"
#include "../utils/debugpauser.h"

#include <QObject>
#include <QDebug>

#include <vector>
#include <deque>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include <memory>
#include <mutex>

class ResizableGraphArray;
class GraphComponent;
class AbstractComponentManager;
class ComponentSplitSet;
class ComponentMergeSet;

template<typename T> class ElementId
{
    friend std::hash<ElementId<T>>;
private:
    static const int NullValue = -1;
    int _value;

public:
    explicit ElementId(int value = NullValue) :
        _value(value)
    {
        static_assert(sizeof(ElementId) == sizeof(_value), "ElementId should not be larger than an int");
    }

    inline operator int() const { return _value; }

    ElementId& operator=(const ElementId<T>& other)
    {
        _value = other._value;
        return *this;
    }

    inline T& operator++()
    {
        ++_value;
        return static_cast<T&>(*this);
    }

    inline T operator++(int)
    {
        T previous = static_cast<T&>(*this);
        operator++();
        return previous;
    }

    inline bool operator==(const ElementId<T>& other) const
    {
        return _value == other._value;
    }

    inline bool operator<(const ElementId<T>& other) const
    {
        return _value < other._value;
    }

    inline bool isNull() const
    {
        return _value == NullValue;
    }

    inline void setToNull()
    {
        _value = NullValue;
    }

    operator QString()
    {
        if(isNull())
            return "Null";
        else
            return QString::number(_value);
    }
};

template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id)
{
    if(id.isNull())
        d << "Null";
    else
        d << (int)id;

    return d;
}

class NodeId : public ElementId<NodeId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    explicit NodeId() : ElementId() {}
    explicit NodeId(int value) : ElementId(value) {}
#endif
};

class EdgeId : public ElementId<EdgeId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    explicit EdgeId() : ElementId() {}
    explicit EdgeId(int value) : ElementId(value) {}
#endif
};

namespace std
{
    template<typename T> struct hash<ElementId<T>>
    {
    public:
        size_t operator()(const ElementId<T>& x) const
        {
            return x._value;
        }
    };
}

template<typename T> using ElementIdSet = std::unordered_set<T, std::hash<ElementId<T>>>;

template<typename T> QDebug operator<<(QDebug d, const ElementIdSet<T>& idSet)
{
    d << "[";
    for(auto id : idSet)
        d << id;
    d << "]";

    return d;
}

class ComponentId : public ElementId<ComponentId>
{
#if __cplusplus >= 201103L
    using ElementId::ElementId;
#else
public:
    explicit ComponentId() : ElementId() {}
    explicit ComponentId(int value) : ElementId(value) {}
#endif
};

class Node
{
    friend class Graph;

private:
    NodeId _id;
    ElementIdSet<EdgeId> _inEdges;
    ElementIdSet<EdgeId> _outEdges;
    ElementIdSet<EdgeId> _edges;

public:
    Node() {}

    Node(const Node& other) :
        _id(other._id),
        _inEdges(other._inEdges),
        _outEdges(other._outEdges),
        _edges(other._edges)
    {}

    Node(Node&& other) noexcept :
        _id(other._id),
        _inEdges(std::move(other._inEdges)),
        _outEdges(std::move(other._outEdges)),
        _edges(std::move(other._edges))
    {}

    const ElementIdSet<EdgeId> inEdges() const { return _inEdges; }
    int inDegree() const { return static_cast<int>(_inEdges.size()); }
    const ElementIdSet<EdgeId> outEdges() const { return _outEdges; }
    int outDegree() const { return static_cast<int>(_outEdges.size()); }
    const ElementIdSet<EdgeId> edges() const { return _edges; }
    int degree() const { return static_cast<int>(_edges.size()); }

    NodeId id() const { return _id; }
};

class Edge
{
    friend class Graph;

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

class ImmutableGraph : public QObject
{
    Q_OBJECT

public:
    virtual ~ImmutableGraph();

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    virtual int numNodes() const = 0;
    virtual const Node& nodeById(NodeId nodeId) const = 0;
    NodeId firstNodeId() const { return nodeIds().size() > 0 ? nodeIds().at(0) : NodeId(); }

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    virtual int numEdges() const = 0;
    virtual const Edge& edgeById(EdgeId edgeId) const = 0;
    EdgeId firstEdgeId() const { return edgeIds().size() > 0 ? edgeIds().at(0) : EdgeId(); }

    virtual void dumpToQDebug(int detail) const;

    virtual void enableComponentManagement() { qFatal("enabledComponentManagement is not callable"); }

protected:
    template<typename> friend class NodeArray;
    std::unordered_set<ResizableGraphArray*> _nodeArrayList;
    virtual int nodeArrayCapacity() const { qFatal("nodeArrayCapacity is not callable"); }

    template<typename> friend class EdgeArray;
    std::unordered_set<ResizableGraphArray*> _edgeArrayList;
    virtual int edgeArrayCapacity() const { qFatal("edgeArrayCapacity is not callable"); }

    template<typename> friend class ComponentArray;
    std::unique_ptr<AbstractComponentManager> _componentManager;

public:
    const std::vector<ComponentId>& componentIds() const;
    int numComponents() const;
    const ImmutableGraph* componentById(ComponentId componentId) const;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
    ComponentId largestComponentId() const;

signals:
    // The signals are listed here in the order in which they are emitted
    void componentsWillMerge(const ImmutableGraph*, const ComponentMergeSet&) const;
    void componentWillBeRemoved(const ImmutableGraph*, ComponentId, bool) const;
    void componentAdded(const ImmutableGraph*, ComponentId, bool) const;
    void componentSplit(const ImmutableGraph*, const ComponentSplitSet&) const;
};

class Graph : public ImmutableGraph
{
    Q_OBJECT
public:
    Graph();
    virtual ~Graph();

private:
    std::vector<bool> _nodeIdsInUse;
    std::vector<NodeId> _nodeIdsVector;
    std::deque<NodeId> _unusedNodeIdsDeque;
    std::vector<Node> _nodesVector;
    NodeId _lastNodeId;

    std::vector<bool> _edgeIdsInUse;
    std::vector<EdgeId> _edgeIdsVector;
    std::deque<EdgeId> _unusedEdgeIdsDeque;
    std::vector<Edge> _edgesVector;
    EdgeId _lastEdgeId;

    int nodeArrayCapacity() const { return _lastNodeId; }
    int edgeArrayCapacity() const { return _lastEdgeId; }

    void updateElementIdData();

public:
    void clear();

    const std::vector<NodeId>& nodeIds() const { return _nodeIdsVector; }
    int numNodes() const { return static_cast<int>(_nodeIdsVector.size()); }
    const Node& nodeById(NodeId nodeId) const { return _nodesVector[nodeId]; }

    NodeId addNode();
    NodeId addNode(NodeId nodeId);
    NodeId addNode(const Node& node);
    void addNodes(const ElementIdSet<NodeId>& nodeIds);

    void removeNode(NodeId nodeId);
    void removeNodes(const ElementIdSet<NodeId>& nodeIds);

    const std::vector<EdgeId>& edgeIds() const { return _edgeIdsVector; }
    int numEdges() const { return static_cast<int>(_edgeIdsVector.size()); }
    const Edge& edgeById(EdgeId edgeId) const { return _edgesVector[edgeId]; }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId);
    EdgeId addEdge(const Edge& edge);
    void addEdges(const std::vector<Edge>& edges);

    void removeEdge(EdgeId edgeId);
    void removeEdges(const ElementIdSet<EdgeId>& edgeIds);

    const ElementIdSet<EdgeId> edgeIdsForNodes(const ElementIdSet<NodeId>& nodeIds);
    const std::vector<Edge> edgesForNodes(const ElementIdSet<NodeId>& nodeIds);

    void dumpToQDebug(int detail) const;

    void enableComponentManagement();

    mutable DebugPauser debugPauser;

private:
    int _graphChangeDepth;
    std::mutex _mutex;
    void beginTransaction();
    void endTransaction();

public:
    class ScopedTransaction
    {
    public:
        ScopedTransaction(Graph& graph);
        ~ScopedTransaction();

    private:
        Graph& _graph;
    };

    void performTransaction(std::function<void(Graph& graph)> transaction);

signals:
    // The signals are listed here in the order in which they are emitted
    void graphWillChange(const Graph*) const;

    void nodeAdded(const Graph*, NodeId) const;
    void nodeWillBeRemoved(const Graph*, NodeId) const;
    void edgeAdded(const Graph*, EdgeId) const;
    void edgeWillBeRemoved(const Graph*, EdgeId) const;

    void graphChanged(const Graph*) const;
};

#endif // GRAPH_H
