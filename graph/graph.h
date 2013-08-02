#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QSet>
#include <QHash>
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
        Node(NodeId _id) : _id(_id) {}
        virtual ~Node() {}

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
        Edge(EdgeId _id) :
            _id(_id),
            _sourceId(Graph::NullNodeId),
            _targetId(Graph::NullNodeId)
        {}
        virtual ~Edge() {}

        NodeId sourceId() const { return _sourceId; }
        NodeId targetId() const { return _targetId; }

        bool isLoop() const { return sourceId() == targetId(); }

        EdgeId id() const { return _id; }
    };

private:
    QHash<NodeId,Node*> nodesMap;
    NodeId nextNodeId;
    QQueue<NodeId> vacatedNodeIdQueue;

    QHash<EdgeId,Edge*> edgesMap;
    EdgeId nextEdgeId;
    QQueue<EdgeId> vacatedEdgeIdQueue;

public:
    QList<Node*> nodes() { return nodesMap.values(); }
    const QList<Node*> nodes() const { return nodesMap.values(); }
    QList<NodeId> nodeIds() { return nodesMap.keys(); }
    const QList<NodeId> nodeIds() const { return nodesMap.keys(); }
    int numNodes() const { return nodesMap.size(); }
    int nodeArrayCapactity() const { return nextNodeId; }

    NodeId addNode();
    void removeNode(NodeId nodeId);
    Node* nodeById(NodeId nodeId) { return nodesMap[nodeId]; }
    const Node* nodeById(NodeId nodeId) const { return nodesMap[nodeId]; }

    QList<Edge*> edges() { return edgesMap.values(); }
    const QList<Edge*> edges() const { return edgesMap.values(); }
    QList<EdgeId> edgeIds() { return edgesMap.keys(); }
    const QList<EdgeId> edgeIds() const { return edgesMap.keys(); }
    int numEdges() const { return edgesMap.size(); }
    int edgeArrayCapactity() const { return nextEdgeId; }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    void removeEdge(EdgeId edgeId);
    Edge* edgeById(EdgeId edgeId) { return edgesMap[edgeId]; }
    const Edge* edgeById(EdgeId edgeId) const { return edgesMap[edgeId]; }

    void setNodeEdges(Edge* edge, NodeId sourceId, NodeId targetId);
    void setNodeEdges(EdgeId edgeId, NodeId sourceId, NodeId targetId);

    void dumpToQDebug(int detail) const
    {
        qDebug() << numNodes() << "nodes" << numEdges() << "edges";

        if(detail > 0)
        {
            for(Node* node : nodes())
                qDebug() << "Node" << node->id();

            for(Edge* edge : edges())
                qDebug() << "Edge" << edge->id() << "(" << edge->sourceId() << "->" << edge->targetId() << ")";
        }
    }

    class ChangeListener
    {
    public:
        virtual void onNodeAdded(NodeId) const {}
        virtual void onNodeRemoved(NodeId) const {}
        virtual void onEdgeAdded(EdgeId) const {}
        virtual void onEdgeRemoved(EdgeId) const {}
    };

protected:
    QList<const ChangeListener*> changeListeners;

public:
    void addChangeListener(const ChangeListener* changeListener)
    {
        changeListeners.append(changeListener);
    }

    void removeChangeListener(const ChangeListener* changeListener)
    {
        changeListeners.removeAll(changeListener);
    }
};

#endif // GRAPH_H
