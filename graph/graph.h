#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QSet>
#include <QDebug>

class Graph
{
public:
    virtual ~Graph() {}

    typedef int NodeId;
    const static NodeId NullNodeId = -1;
    typedef int EdgeId;
    const static EdgeId NullEdgeId = -1;

    class Node
    {
    public:
        virtual const QSet<EdgeId> inEdges() = 0;
        virtual int inDegree() = 0;
        virtual const QSet<EdgeId> outEdges() = 0;
        virtual int outDegree() = 0;
        virtual const QSet<EdgeId> edges() = 0;
        virtual int degree() = 0;

        virtual NodeId id() = 0;
    };

    class Edge
    {
    public:
        virtual NodeId sourceId() = 0;
        virtual NodeId targetId() = 0;

        bool isLoop() { return sourceId() == targetId(); }

        virtual EdgeId id() = 0;
    };

    virtual QList<Node*> nodes() = 0;
    virtual QList<NodeId> nodeIds() = 0;
    virtual int numNodes() = 0;

    virtual NodeId addNode() = 0;
    virtual void removeNode(NodeId nodeId) = 0;
    virtual Node* nodeById(NodeId nodeId) = 0;

    virtual QList<Edge*> edges() = 0;
    virtual QList<EdgeId> edgeIds() = 0;
    virtual int numEdges() = 0;

    virtual EdgeId addEdge(NodeId sourceId, NodeId targetId) = 0;
    virtual void removeEdge(EdgeId edgeId) = 0;
    virtual Edge* edgeById(EdgeId edgeId) = 0;

    virtual void setNodeEdges(Edge* edge, NodeId sourceId, NodeId targetId) = 0;
    virtual void setNodeEdges(EdgeId edgeId, NodeId sourceId, NodeId targetId) = 0;

    void dumpToQDebug(int detail)
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
};

#endif // GRAPH_H
