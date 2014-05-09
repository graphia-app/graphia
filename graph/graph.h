#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QList>
#include <QSet>
#include <QVector>
#include <QMap>
#include <QQueue>
#include <QDebug>

class ResizableGraphArray;
class GraphComponent;
class ComponentManager;

template<typename T> class ElementId;
template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id);

template<typename T> class ElementId
{
private:
    static const int NullValue = -1;
    int value;

public:
    explicit ElementId(int _value = NullValue) :
        value(_value)
    {}

    inline operator int() const { return value; }

    ElementId& operator=(const ElementId<T>& other)
    {
        value = other.value;
        return *this;
    }

    explicit ElementId(const ElementId<T>& other)
    {
        value = other.value;
    }

    inline T& operator++()
    {
        ++value;
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
        return value == other.value;
    }

    inline bool isNull() const
    {
        return value == NullValue;
    }

    inline void setToNull()
    {
        value = NullValue;
    }

    friend QDebug operator<< <T>(QDebug d, const ElementId<T>& id);
};

template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id)
{
    if(id.isNull())
        d << "Null";
    else
        d << id.value;

    return d;
}

struct NodeId : ElementId<NodeId> { using ElementId::ElementId; };
struct EdgeId : ElementId<EdgeId> { using ElementId::ElementId; };
struct ComponentId : ElementId<ComponentId> { using ElementId::ElementId; };

class Node
{
    friend class Graph;

private:
    NodeId _id;
    QSet<EdgeId> _inEdges;
    QSet<EdgeId> _outEdges;

public:
    Node() {}
    Node(const Node& other) :
        _id(other._id),
        _inEdges(other._inEdges),
        _outEdges(other._outEdges)
    {}

    const QSet<EdgeId> inEdges() const { return _inEdges; }
    int inDegree() const { return _inEdges.size(); }
    const QSet<EdgeId> outEdges() const { return _outEdges; }
    int outDegree() const { return _outEdges.size(); }
    const QSet<EdgeId> edges() const { return _inEdges + _outEdges; }
    int degree() const { return edges().size(); }

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

class ReadOnlyGraph
{
public:
    virtual const QVector<NodeId>& nodeIds() const = 0;
    virtual int numNodes() const = 0;
    virtual const Node& nodeById(NodeId nodeId) const = 0;
    NodeId firstNodeId() const { return nodeIds().size() > 0 ? nodeIds().at(0) : NodeId(); }

    virtual const QVector<EdgeId>& edgeIds() const = 0;
    virtual int numEdges() const = 0;
    virtual const Edge& edgeById(EdgeId edgeId) const = 0;
    EdgeId firstEdgeId() const { return edgeIds().size() > 0 ? edgeIds().at(0) : EdgeId(); }

    virtual void dumpToQDebug(int detail) const
    {
        qDebug() << numNodes() << "nodes" << numEdges() << "edges";

        if(detail > 1)
        {
            for(NodeId nodeId : nodeIds())
                qDebug() << "Node" << nodeId;

            for(EdgeId edgeId : edgeIds())
            {
                const Edge& edge = edgeById(edgeId);
                qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
            }
        }
    }
};

class Graph : public QObject, public ReadOnlyGraph
{
    Q_OBJECT
public:
    Graph();
    virtual ~Graph();

private:
    QVector<NodeId> nodeIdsList;
    QVector<Node> nodesVector;
    NodeId nextNodeId;
    QQueue<NodeId> vacatedNodeIdQueue;

    QVector<EdgeId> edgeIdsList;
    QVector<Edge> edgesVector;
    EdgeId nextEdgeId;
    QQueue<EdgeId> vacatedEdgeIdQueue;

    template<typename> friend class NodeArray;
    QList<ResizableGraphArray*> nodeArrayList;
    int nodeArrayCapacity() const { return nextNodeId; }

    template<typename> friend class EdgeArray;
    QList<ResizableGraphArray*> edgeArrayList;
    int edgeArrayCapacity() const { return nextEdgeId; }

    template<typename> friend class ComponentArray;
    ComponentManager* componentManager;
    bool _componentManagementEnabled;

    void setEdgeNodes(Edge& edge, NodeId sourceId, NodeId targetId);
    void setEdgeNodes(EdgeId edgeId, NodeId sourceId, NodeId targetId);

public:
    void clear();
    void setComponentManager(ComponentManager* componentManager);
    void enableComponentMangagement();
    void disableComponentMangagement();
    bool componentManagementEnabled() const { return _componentManagementEnabled; }

    const QVector<NodeId>& nodeIds() const { return nodeIdsList; }
    int numNodes() const { return nodeIdsList.size(); }
    const Node& nodeById(NodeId nodeId) const { return nodesVector[nodeId]; }

    NodeId addNode();
    void removeNode(NodeId nodeId);
    void removeNodes(QSet<NodeId> nodeIds);
    void removeNodes(QList<NodeId> nodeIds);

    const QVector<EdgeId>& edgeIds() const { return edgeIdsList; }
    int numEdges() const { return edgeIdsList.size(); }
    const Edge& edgeById(EdgeId edgeId) const { return edgesVector[edgeId]; }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    void removeEdge(EdgeId edgeId);
    void removeEdges(QSet<EdgeId> edgeIds);
    void removeEdges(QList<EdgeId> edgeIds);

    const QList<ComponentId>* componentIds() const;
    ComponentId firstComponentId() const { return (*componentIds())[0]; }
    int numComponents() const;
    const ReadOnlyGraph* componentById(ComponentId componentId) const;
    const ReadOnlyGraph* firstComponent() const { return componentById((*componentIds())[0]); }
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;

    void dumpToQDebug(int detail) const;

private:
    int graphChangeDepth;
    void emitGraphWillChange();
    void emitGraphChanged();

signals:
    void graphWillChange(const Graph*) const;
    void graphChanged(const Graph*) const;
    void nodeAdded(const Graph*, NodeId) const;
    void nodeWillBeRemoved(const Graph*, NodeId) const;
    void edgeAdded(const Graph*, EdgeId) const;
    void edgeWillBeRemoved(const Graph*, EdgeId) const;
    void componentAdded(const Graph*, ComponentId) const;
    void componentWillBeRemoved(const Graph*, ComponentId) const;
    void componentSplit(const Graph*, ComponentId, const QSet<ComponentId>&) const;
    void componentsWillMerge(const Graph*, const QSet<ComponentId>&, ComponentId) const;
};

#endif // GRAPH_H
