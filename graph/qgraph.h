#ifndef QGRAPH_H
#define QGRAPH_H

#include "graph.h"

#include <QHash>

class QGraph : public Graph
{
public:
    QGraph();
    virtual ~QGraph() {}

    class QNode : public Graph::Node
    {
        friend class QGraph;

    private:
        NodeId _id;
        QSet<EdgeId> _inEdges;
        QSet<EdgeId> _outEdges;

    public:
        QNode(NodeId _id) : _id(_id) {}
        virtual ~QNode() {}

        const QSet<EdgeId> inEdges() { return _inEdges; }
        int inDegree() { return _inEdges.size(); }
        const QSet<EdgeId> outEdges() { return _outEdges; }
        int outDegree() { return _outEdges.size(); }
        const QSet<EdgeId> edges() { return _inEdges + _outEdges; }
        int degree() { return edges().size(); }

        NodeId id() { return _id; }
    };

    class QEdge : public Graph::Edge
    {
        friend class QGraph;

    private:
        EdgeId _id;
        NodeId _sourceId;
        NodeId _targetId;

    public:
        QEdge(EdgeId _id) :
            _id(_id),
            _sourceId(Graph::NullNodeId),
            _targetId(Graph::NullNodeId)
        {}
        virtual ~QEdge() {}

        NodeId sourceId() { return _sourceId; }
        NodeId targetId() { return _targetId; }

        virtual EdgeId id() { return _id; }
    };

private:
    QHash<NodeId,Node*> nodesMap;
    NodeId nextNodeId;
    QHash<EdgeId,Edge*> edgesMap;
    EdgeId nextEdgeId;

public:
    QList<Node*> nodes() { return nodesMap.values(); }
    QList<NodeId> nodeIds() { return nodesMap.keys(); }
    int numNodes() { return nodesMap.size(); }

    NodeId addNode();
    void removeNode(NodeId nodeId);
    Node* nodeById(NodeId nodeId) { return nodesMap[nodeId]; }

    QList<Edge*> edges() { return edgesMap.values(); }
    QList<EdgeId> edgeIds() { return edgesMap.keys(); }
    virtual int numEdges() { return edgesMap.size(); }

    EdgeId addEdge(NodeId sourceId, NodeId targetId);
    void removeEdge(EdgeId edgeId);
    Edge* edgeById(EdgeId edgeId) { return edgesMap[edgeId]; }

    void setNodeEdges(Edge* edge, NodeId sourceId, NodeId targetId);
    void setNodeEdges(EdgeId edgeId, NodeId sourceId, NodeId targetId);
};

#endif // QGRAPH_H
