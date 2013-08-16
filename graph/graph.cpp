#include "graph.h"

#include <QtGlobal>

Graph::Graph() :
    nextNodeId(0),
    nextEdgeId(0)
{
}

Graph::~Graph()
{
    for(Graph::NodeId nodeId : nodeIds())
        removeNode(nodeId);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);
}

Graph::NodeId Graph::addNode()
{
    emit graphWillChange(*this);

    Graph::NodeId newNodeId;

    if(!vacatedNodeIdQueue.isEmpty())
        newNodeId = vacatedNodeIdQueue.dequeue();
    else
        newNodeId = nextNodeId++;

    nodeIdsList.append(newNodeId);
    nodesVector.resize(nodeArrayCapacity());
    nodesVector[newNodeId]._id = newNodeId;

    emit nodeAdded(*this, newNodeId);
    emit graphChanged(*this);

    return newNodeId;
}

void Graph::removeNode(Graph::NodeId nodeId)
{
    emit graphWillChange(*this);
    emit nodeWillBeRemoved(*this, nodeId);

    // Remove all edges that touch this node
    Node& node = nodeById(nodeId);
    for(Graph::EdgeId edgeId : node.edges())
        removeEdge(edgeId);

    nodeIdsList.removeOne(nodeId);
    vacatedNodeIdQueue.enqueue(nodeId);

    emit graphChanged(*this);
}

Graph::EdgeId Graph::addEdge(Graph::NodeId sourceId, Graph::NodeId targetId)
{
    emit graphWillChange(*this);

    Graph::EdgeId newEdgeId;

    if(!vacatedEdgeIdQueue.isEmpty())
        newEdgeId = vacatedEdgeIdQueue.dequeue();
    else
        newEdgeId = nextEdgeId++;

    edgeIdsList.append(newEdgeId);
    edgesVector.resize(edgeArrayCapacity());
    edgesVector[newEdgeId]._id = newEdgeId;

    setNodeEdges(newEdgeId, sourceId, targetId);

    emit edgeAdded(*this, newEdgeId);
    emit graphChanged(*this);

    return newEdgeId;
}

void Graph::removeEdge(Graph::EdgeId edgeId)
{
    emit graphWillChange(*this);
    emit edgeWillBeRemoved(*this, edgeId);

    // Remove all node references to this edge
    Edge& edge = edgeById(edgeId);
    Node& source = nodeById(edge.sourceId());
    Node& target = nodeById(edge.targetId());
    source._outEdges.remove(edgeId);
    target._inEdges.remove(edgeId);

    edgeIdsList.removeOne(edgeId);
    vacatedEdgeIdQueue.enqueue(edgeId);

    emit graphChanged(*this);
}

void Graph::setNodeEdges(Graph::Edge& edge, Graph::NodeId sourceId, Graph::NodeId targetId)
{
    emit graphWillChange(*this);

    Q_ASSERT(sourceId != Graph::NullNodeId);
    Q_ASSERT(targetId != Graph::NullNodeId);

    if(edge.sourceId() != Graph::NullNodeId)
    {
        // Remove edge from source node out edges
        Node& source = nodeById(edge.sourceId());
        source._outEdges.remove(edge.id());
    }

    if(edge.targetId() != Graph::NullNodeId)
    {
        // Remove edge from target node in edges
        Node& target = nodeById(edge.targetId());
        target._inEdges.remove(edge.id());
    }

    edge._sourceId = sourceId;
    edge._targetId = targetId;

    Node& source = nodeById(sourceId);
    source._outEdges.insert(edge.id());

    Node& target = nodeById(targetId);
    target._inEdges.insert(edge.id());

    emit graphChanged(*this);
}

void Graph::setNodeEdges(Graph::EdgeId edgeId, Graph::NodeId sourceId, Graph::NodeId targetId)
{
    setNodeEdges(edgeById(edgeId), sourceId, targetId);
}

