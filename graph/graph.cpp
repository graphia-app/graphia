#include "graph.h"
#include "grapharray.h"
#include "simplecomponentmanager.h"

#include <QtGlobal>
#include <QMetaType>

Graph::Graph() :
    nextNodeId(0),
    nextEdgeId(0),
    componentManager(new SimpleComponentManager(*this)),
    _componentManagementEnabled(true),
    graphChangeDepth(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<EdgeId>("EdgeId");
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
    emitGraphWillChange();
    delete this->componentManager;
    this->componentManager = componentManager;
    emitGraphChanged();
}

void Graph::enableComponentMangagement()
{
    emitGraphWillChange();
    _componentManagementEnabled = true;
    emitGraphChanged();
}

void Graph::disableComponentMangagement()
{
    _componentManagementEnabled = false;
}

NodeId Graph::addNode()
{
    emitGraphWillChange();

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

    if(_componentManagementEnabled && componentManager != nullptr)
        componentManager->nodeAdded(newNodeId);

    emit nodeAdded(this, newNodeId);

    emitGraphChanged();

    return newNodeId;
}

void Graph::removeNode(NodeId nodeId)
{
    emitGraphWillChange();

    // Remove all edges that touch this node
    const Node& node = nodesVector[nodeId];
    for(EdgeId edgeId : node.edges())
        removeEdge(edgeId);

    if(_componentManagementEnabled && componentManager != nullptr)
        componentManager->nodeWillBeRemoved(nodeId);

    emit nodeWillBeRemoved(this, nodeId);

    nodeIdsList.removeOne(nodeId);
    vacatedNodeIdQueue.enqueue(nodeId);

    emitGraphChanged();
}

void Graph::removeNodes(QSet<NodeId> nodeIds)
{
    removeNodes(nodeIds.toList());
}

void Graph::removeNodes(QList<NodeId> nodeIds)
{
    if(nodeIds.isEmpty())
        return;

    emitGraphWillChange();

    for(NodeId nodeId : nodeIds)
        removeNode(nodeId);

    emitGraphChanged();
}

EdgeId Graph::addEdge(NodeId sourceId, NodeId targetId)
{
    emitGraphWillChange();

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

    if(_componentManagementEnabled && componentManager != nullptr)
        componentManager->edgeAdded(newEdgeId);

    emit edgeAdded(this, newEdgeId);

    emitGraphChanged();

    return newEdgeId;
}

void Graph::removeEdge(EdgeId edgeId)
{
    emitGraphWillChange();

    if(_componentManagementEnabled && componentManager != nullptr)
        componentManager->edgeWillBeRemoved(edgeId);

    emit edgeWillBeRemoved(this, edgeId);

    // Remove all node references to this edge
    const Edge& edge = edgesVector[edgeId];
    Node& source = nodesVector[edge.sourceId()];
    Node& target = nodesVector[edge.targetId()];
    source._outEdges.remove(edgeId);
    target._inEdges.remove(edgeId);

    edgeIdsList.removeOne(edgeId);
    vacatedEdgeIdQueue.enqueue(edgeId);

    emitGraphChanged();
}

void Graph::removeEdges(QSet<EdgeId> edgeIds)
{
    removeEdges(edgeIds.toList());
}

void Graph::removeEdges(QList<EdgeId> edgeIds)
{
    if(edgeIds.isEmpty())
        return;

    emitGraphWillChange();

    for(EdgeId edgeId : edgeIds)
        removeEdge(edgeId);

    emitGraphChanged();
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
        return componentManager->componentById(componentId);

    Q_ASSERT(nullptr);
    return nullptr;
}

ComponentId Graph::componentIdOfNode(NodeId nodeId) const
{
    if(componentManager != nullptr)
        return componentManager->componentIdOfNode(nodeId);

    return ComponentId::Null();
}

ComponentId Graph::componentIdOfEdge(EdgeId edgeId) const
{
    if(componentManager != nullptr)
        return componentManager->componentIdOfEdge(edgeId);

    return ComponentId::Null();
}

void Graph::dumpToQDebug(int detail) const
{
    ReadOnlyGraph::dumpToQDebug(detail);

    if(detail > 0)
    {
        if(_componentManagementEnabled && componentManager != nullptr)
        {
            for(ComponentId componentId : componentManager->componentIds())
            {
                const ReadOnlyGraph* component = componentManager->componentById(componentId);
                qDebug() << "component" << componentId;
                component->dumpToQDebug(detail);
            }
        }
    }
}

void Graph::emitGraphWillChange()
{
    if(graphChangeDepth++ <= 0)
        emit graphWillChange(this);
}

void Graph::emitGraphChanged()
{
    Q_ASSERT(graphChangeDepth > 0);
    if(--graphChangeDepth <= 0)
    {
        if(_componentManagementEnabled && componentManager != nullptr)
            componentManager->graphChanged(this);

        emit graphChanged(this);
    }
}

void Graph::setEdgeNodes(Edge& edge, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!sourceId.IsNull());
    Q_ASSERT(!targetId.IsNull());

    if(!edge.sourceId().IsNull())
    {
        // Remove edge from source node out edges
        Node& source = nodesVector[edge.sourceId()];
        source._outEdges.remove(edge.id());
    }

    if(!edge.targetId().IsNull())
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
}

void Graph::setEdgeNodes(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    setEdgeNodes(edgesVector[edgeId], sourceId, targetId);
}
