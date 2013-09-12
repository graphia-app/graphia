#include "graph.h"
#include "grapharray.h"
#include "simplecomponentmanager.h"

#include <QtGlobal>

Graph::Graph() :
    nextNodeId(0),
    nextEdgeId(0),
    componentManager(new SimpleComponentManager(*this)),
    componentManagementEnabled(true)
{
}

Graph::~Graph()
{
    delete componentManager;
    componentManager = nullptr;
}

void Graph::clear()
{
    for(NodeId nodeId : nodeIds())
        removeNode(nodeId);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    nodesVector.resize(0);
    edgesVector.resize(0);
}

void Graph::setComponentManager(ComponentManager *componentManager)
{
    delete this->componentManager;

    this->componentManager = componentManager;
    this->componentManager->findComponents();
}

void Graph::enableComponentMangagement()
{
    componentManagementEnabled = true;
    componentManager->findComponents();
}

void Graph::disableComponentMangagement()
{
    componentManagementEnabled = false;
}

NodeId Graph::addNode()
{
    emit graphWillChange(this);

    NodeId newNodeId;

    if(!vacatedNodeIdQueue.isEmpty())
        newNodeId = vacatedNodeIdQueue.dequeue();
    else
        newNodeId = nextNodeId++;

    nodeIdsList.append(newNodeId);
    nodesVector.resize(nodeArrayCapacity());
    nodesVector[newNodeId]._id = newNodeId;

    for(ResizableGraphArray* nodeArray : nodeArrayList)
        nodeArray->resize(nodeArrayCapacity());

    if(componentManagementEnabled && componentManager != nullptr)
        componentManager->nodeAdded(newNodeId);

    emit nodeAdded(this, newNodeId);
    emit graphChanged(this);

    return newNodeId;
}

void Graph::removeNode(NodeId nodeId)
{
    if(componentManagementEnabled && componentManager != nullptr)
        componentManager->nodeWillBeRemoved(nodeId);

    emit graphWillChange(this);
    emit nodeWillBeRemoved(this, nodeId);

    // Remove all edges that touch this node
    const Node& node = nodesVector[nodeId];
    for(EdgeId edgeId : node.edges())
        removeEdge(edgeId);

    nodeIdsList.removeOne(nodeId);
    vacatedNodeIdQueue.enqueue(nodeId);

    emit graphChanged(this);
}

EdgeId Graph::addEdge(NodeId sourceId, NodeId targetId)
{
    emit graphWillChange(this);

    EdgeId newEdgeId;

    if(!vacatedEdgeIdQueue.isEmpty())
        newEdgeId = vacatedEdgeIdQueue.dequeue();
    else
        newEdgeId = nextEdgeId++;

    edgeIdsList.append(newEdgeId);
    edgesVector.resize(edgeArrayCapacity());
    edgesVector[newEdgeId]._id = newEdgeId;

    for(ResizableGraphArray* edgeArray : edgeArrayList)
        edgeArray->resize(edgeArrayCapacity());

    setEdgeNodes(newEdgeId, sourceId, targetId);

    if(componentManagementEnabled && componentManager != nullptr)
        componentManager->edgeAdded(newEdgeId);

    emit edgeAdded(this, newEdgeId);
    emit graphChanged(this);

    return newEdgeId;
}

void Graph::removeEdge(EdgeId edgeId)
{
    if(componentManagementEnabled && componentManager != nullptr)
        componentManager->edgeWillBeRemoved(edgeId);

    emit graphWillChange(this);
    emit edgeWillBeRemoved(this, edgeId);

    // Remove all node references to this edge
    const Edge& edge = edgesVector[edgeId];
    Node& source = nodesVector[edge.sourceId()];
    Node& target = nodesVector[edge.targetId()];
    source._outEdges.remove(edgeId);
    target._inEdges.remove(edgeId);

    edgeIdsList.removeOne(edgeId);
    vacatedEdgeIdQueue.enqueue(edgeId);

    emit graphChanged(this);
}

const QList<ComponentId> *Graph::componentIds() const
{
    if(componentManager != nullptr)
        return &componentManager->componentIds();

    return nullptr;
}

int Graph::numComponents() const
{
    if(componentManager != nullptr)
        return componentManager->componentIds().size();

    return 0;
}

const ReadOnlyGraph *Graph::componentById(ComponentId componentId) const
{
    if(componentManager != nullptr)
        return &componentManager->componentById(componentId);

    return nullptr;
}

void Graph::setEdgeNodes(Edge& edge, NodeId sourceId, NodeId targetId)
{
    emit graphWillChange(this);

    Q_ASSERT(sourceId != NullNodeId);
    Q_ASSERT(targetId != NullNodeId);

    if(edge.sourceId() != NullNodeId)
    {
        // Remove edge from source node out edges
        Node& source = nodesVector[edge.sourceId()];
        source._outEdges.remove(edge.id());
    }

    if(edge.targetId() != NullNodeId)
    {
        // Remove edge from target node in edges
        Node& target = nodesVector[edge.targetId()];
        target._inEdges.remove(edge.id());
    }

    edge._sourceId = sourceId;
    edge._targetId = targetId;

    Node& source = nodesVector[sourceId];
    source._outEdges.insert(edge.id());

    Node& target = nodesVector[targetId];
    target._inEdges.insert(edge.id());

    emit graphChanged(this);
}

void Graph::setEdgeNodes(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    setEdgeNodes(edgesVector[edgeId], sourceId, targetId);
}

