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

typedef int NodeId;
const static NodeId NullNodeId = -1;
typedef int EdgeId;
const static EdgeId NullEdgeId = -1;
typedef int ComponentId;
const static ComponentId NullComponentId = -1;

class Node
{
    friend class Graph;

private:
    NodeId _id;
    QSet<EdgeId> _inEdges;
    QSet<EdgeId> _outEdges;

public:
    Node() : _id(NullNodeId) {}
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
    Edge() :
        _id(NullNodeId),
        _sourceId(NullNodeId),
        _targetId(NullNodeId)
    {}
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

        return NullNodeId;
    }

    bool isLoop() const { return sourceId() == targetId(); }

    EdgeId id() const { return _id; }
};

class ReadOnlyGraph
{
public:
    virtual const QList<NodeId>& nodeIds() const = 0;
    virtual int numNodes() const = 0;
    virtual const Node& nodeById(NodeId nodeId) const = 0;

    virtual const QList<EdgeId>& edgeIds() const = 0;
    virtual int numEdges() const = 0;
    virtual const Edge& edgeById(EdgeId edgeId) const = 0;
};

class Graph : public QObject, public ReadOnlyGraph
{
    Q_OBJECT
public:
    Graph();
    virtual ~Graph();

private:
    QList<NodeId> nodeIdsList;
    QVector<Node> nodesVector;
    NodeId nextNodeId;
    QQueue<NodeId> vacatedNodeIdQueue;

    QList<EdgeId> edgeIdsList;
    QVector<Edge> edgesVector;
    EdgeId nextEdgeId;
    QQueue<EdgeId> vacatedEdgeIdQueue;

    template<typename> friend class NodeArray;
    QList<ResizableGraphArray*> nodeArrayList;
    int nodeArrayCapacity() const { return nextNodeId; }

    template<typename> friend class EdgeArray;
    QList<ResizableGraphArray*> edgeArrayList;
    int edgeArrayCapacity() const { return nextEdgeId; }

    ComponentManager* componentManager;

    void setEdgeNodes(Edge& edge, NodeId sourceId, NodeId targetId);
    void setEdgeNodes(EdgeId edgeId, NodeId sourceId, NodeId targetId);

public:
    void clear();
    void setComponentManager(ComponentManager* componentManager);

    const QList<NodeId>& nodeIds() const { return nodeIdsList; }
    int numNodes() const { return nodeIdsList.size(); }
    const Node& nodeById(NodeId nodeId) const { return nodesVector[nodeId]; }

    NodeId addNode();
    void removeNode(NodeId nodeId);

    const QList<EdgeId>& edgeIds() const { return edgeIdsList; }
    int numEdges() const { return edgeIdsList.size(); }
    const Edge& edgeById(EdgeId edgeId) const { return edgesVector[edgeId]; }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    void removeEdge(EdgeId edgeId);

    const QList<ComponentId>* componentIds() const;
    int numComponents() const;
    const ReadOnlyGraph* componentById(ComponentId componentId) const;

    void dumpToQDebug(int detail) const
    {
        qDebug() << numNodes() << "nodes" << numEdges() << "edges";

        if(detail > 0)
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

signals:
    void graphWillChange(const Graph*) const;
    void graphChanged(const Graph*) const;
    void nodeAdded(const Graph*, NodeId) const;
    void nodeWillBeRemoved(const Graph*, NodeId) const;
    void edgeAdded(const Graph*, EdgeId) const;
    void edgeWillBeRemoved(const Graph*, EdgeId) const;
    void componentAdded(const Graph*, ComponentId) const;
    void componentWillBeRemoved(const Graph*, ComponentId) const;
    void componentSplit(const Graph*, ComponentId, QSet<ComponentId>) const;
    void componentsWillMerge(const Graph*, QSet<ComponentId>, ComponentId) const;
};

#endif // GRAPH_H
