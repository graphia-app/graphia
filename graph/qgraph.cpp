#include "qgraph.h"

#include <QtGlobal>

QGraph::QGraph() :
    nextNodeId(0),
    nextEdgeId(0)
{
}

QGraph::~QGraph()
{
    for(Graph::NodeId nodeId : nodeIds())
        removeNode(nodeId);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);
}

Graph::NodeId QGraph::addNode()
{
    Graph::NodeId newNodeId;

    if(!vacatedNodeIdQueue.isEmpty())
        newNodeId = vacatedNodeIdQueue.dequeue();
    else
        newNodeId = nextNodeId++;

    QNode* node = new QNode(newNodeId);
    nodesMap.insert(newNodeId, node);

    for(const Graph::ChangeListener* changeListener : changeListeners)
        changeListener->onNodeAdded(newNodeId);

    return newNodeId;
}

void QGraph::removeNode(Graph::NodeId nodeId)
{
    for(const Graph::ChangeListener* changeListener : changeListeners)
        changeListener->onNodeRemoved(nodeId);

    // Remove all edges that touch this node
    QNode* node = static_cast<QNode*>(nodeById(nodeId));
    for(Graph::EdgeId edgeId : node->edges())
        removeEdge(edgeId);

    delete node;
    nodesMap.remove(nodeId);
    vacatedNodeIdQueue.enqueue(nodeId);
}

Graph::EdgeId QGraph::addEdge(Graph::NodeId sourceId, Graph::NodeId targetId)
{
    Graph::EdgeId newEdgeId;

    if(!vacatedEdgeIdQueue.isEmpty())
        newEdgeId = vacatedEdgeIdQueue.dequeue();
    else
        newEdgeId = nextEdgeId++;

    QEdge* edge = new QEdge(newEdgeId);
    edgesMap.insert(newEdgeId, edge);
    setNodeEdges(edge, sourceId, targetId);

    for(const Graph::ChangeListener* changeListener : changeListeners)
        changeListener->onEdgeAdded(newEdgeId);

    return newEdgeId;
}

void QGraph::removeEdge(Graph::EdgeId edgeId)
{
    for(const Graph::ChangeListener* changeListener : changeListeners)
        changeListener->onEdgeRemoved(edgeId);

    // Remove all node references to this edge
    QEdge* edge = static_cast<QEdge*>(edgeById(edgeId));
    QNode* source = static_cast<QNode*>(nodeById(edge->sourceId()));
    QNode* target = static_cast<QNode*>(nodeById(edge->targetId()));
    source->_outEdges.remove(edgeId);
    target->_inEdges.remove(edgeId);

    delete edge;
    edgesMap.remove(edgeId);
    vacatedEdgeIdQueue.enqueue(edgeId);
}

void QGraph::setNodeEdges(Graph::Edge *edge, Graph::NodeId sourceId, Graph::NodeId targetId)
{
    Q_ASSERT(sourceId != Graph::NullNodeId);
    Q_ASSERT(targetId != Graph::NullNodeId);

    if(edge->sourceId() != Graph::NullNodeId)
    {
        // Remove edge from source node out edges
        QNode* source = static_cast<QNode*>(nodeById(edge->sourceId()));
        source->_outEdges.remove(edge->id());
    }

    if(edge->targetId() != Graph::NullNodeId)
    {
        // Remove edge from target node in edges
        QNode* target = static_cast<QNode*>(nodeById(edge->targetId()));
        target->_inEdges.remove(edge->id());
    }

    QEdge* qEdge = static_cast<QEdge*>(edge);
    qEdge->_sourceId = sourceId;
    qEdge->_targetId = targetId;

    QNode* source = static_cast<QNode*>(nodeById(sourceId));
    source->_outEdges.insert(edge->id());

    QNode* target = static_cast<QNode*>(nodeById(targetId));
    target->_inEdges.insert(edge->id());
}

void QGraph::setNodeEdges(Graph::EdgeId edgeId, Graph::NodeId sourceId, Graph::NodeId targetId)
{
    setNodeEdges(edgeById(edgeId), sourceId, targetId);
}

