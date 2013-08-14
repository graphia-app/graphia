#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QSet>
#include <QVector>
#include <QQueue>
#include <QDebug>

class Graph
{
public:
    Graph();
    virtual ~Graph();

    typedef int NodeId;
    const static NodeId NullNodeId = -1;
    typedef int EdgeId;
    const static EdgeId NullEdgeId = -1;

    class Node
    {
        friend class Graph;

    private:
        NodeId _id;
        QSet<EdgeId> _inEdges;
        QSet<EdgeId> _outEdges;

    public:
        Node() : _id(Graph::NullNodeId) {}

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
            _id(Graph::NullNodeId),
            _sourceId(Graph::NullNodeId),
            _targetId(Graph::NullNodeId)
        {}

        NodeId sourceId() const { return _sourceId; }
        NodeId targetId() const { return _targetId; }

        bool isLoop() const { return sourceId() == targetId(); }

        EdgeId id() const { return _id; }
    };

private:
    QList<NodeId> nodeIdsList;
    QVector<Node> nodesVector;
    NodeId nextNodeId;
    QQueue<NodeId> vacatedNodeIdQueue;

    QList<EdgeId> edgeIdsList;
    QVector<Edge> edgesVector;
    EdgeId nextEdgeId;
    QQueue<EdgeId> vacatedEdgeIdQueue;

public:
    QList<NodeId>& nodeIds() { return nodeIdsList; }
    const QList<NodeId>& nodeIds() const { return nodeIdsList; }
    int numNodes() const { return nodeIdsList.size(); }
    int nodeArrayCapacity() const { return nextNodeId; }

    NodeId addNode();
    void removeNode(NodeId nodeId);
    Node& nodeById(NodeId nodeId) { return nodesVector[nodeId]; }
    const Node& nodeById(NodeId nodeId) const { return nodesVector[nodeId]; }

    QList<EdgeId>& edgeIds() { return edgeIdsList; }
    const QList<EdgeId>& edgeIds() const { return edgeIdsList; }
    int numEdges() const { return edgeIdsList.size(); }
    int edgeArrayCapacity() const { return nextEdgeId; }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    void removeEdge(EdgeId edgeId);
    Edge& edgeById(EdgeId edgeId) { return edgesVector[edgeId]; }
    const Edge& edgeById(EdgeId edgeId) const { return edgesVector[edgeId]; }

    void setNodeEdges(Edge& edge, NodeId sourceId, NodeId targetId);
    void setNodeEdges(EdgeId edgeId, NodeId sourceId, NodeId targetId);

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

    class ChangeListener
    {
    public:
        virtual void onNodeAdded(NodeId) {}
        virtual void onNodeRemoved(NodeId) {}
        virtual void onEdgeAdded(EdgeId) {}
        virtual void onEdgeRemoved(EdgeId) {}
    };

protected:
    QList<ChangeListener*> changeListeners;

public:
    void addChangeListener(ChangeListener* changeListener)
    {
        changeListeners.append(changeListener);
    }

    void removeChangeListener(ChangeListener* changeListener)
    {
        changeListeners.removeAll(changeListener);
    }
};

#endif // GRAPH_H
