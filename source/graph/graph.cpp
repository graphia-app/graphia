#include "graph.h"
#include "grapharray.h"
#include "simplecomponentmanager.h"

#include <QtGlobal>
#include <QMetaType>

Graph::Graph() :
    _nextNodeId(0),
    _nextEdgeId(0),
    _componentManager(new SimpleComponentManager(*this)),
    _componentManagementEnabled(true),
    _graphChangeDepth(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<EdgeId>("EdgeId");
}

Graph::~Graph()
{
    delete _componentManager;
    _componentManager = nullptr;
}

void Graph::clear()
{
    for(NodeId nodeId : nodeIds())
        removeNode(nodeId);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    _nodesVector.resize(0);
    _edgesVector.resize(0);
}

void Graph::setComponentManager(ComponentManager *componentManager)
{
    emitGraphWillChange();
    delete this->_componentManager;
    this->_componentManager = componentManager;
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
    if(!_vacatedNodeIdQueue.isEmpty())
        return addNode(_vacatedNodeIdQueue.head());

    return addNode(_nextNodeId);
}

NodeId Graph::addNode(NodeId nodeId)
{
    if(nodeId < _nextNodeId && !_vacatedNodeIdQueue.contains(nodeId))
        return NodeId(); // Already taken

    emitGraphWillChange();

    while(_nextNodeId < nodeId)
    {
        // Queue up intervening IDs that will be skipped
        _vacatedNodeIdQueue.enqueue(_nextNodeId);
        _nextNodeId++;
    }

    if(!_vacatedNodeIdQueue.removeOne(nodeId))
        _nextNodeId++;

    _nodeIdsVector.append(nodeId);
    _nodesVector.resize(nodeArrayCapacity());
    _nodesVector[nodeId]._id = nodeId;
    _nodesVector[nodeId]._inEdges.clear();
    _nodesVector[nodeId]._outEdges.clear();

    for(ResizableGraphArray* nodeArray : _nodeArrayList)
        nodeArray->resize(nodeArrayCapacity());

    if(_componentManagementEnabled && _componentManager != nullptr)
        _componentManager->nodeAdded(nodeId);

    emit nodeAdded(this, nodeId);
    emitGraphChanged();

    return nodeId;
}

NodeId Graph::addNode(const Node& node)
{
    return addNode(node._id);
}

void Graph::addNodes(const QSet<NodeId>& nodeIds)
{
    addNodes(nodeIds.toList());
}

void Graph::addNodes(const QList<NodeId>& nodeIds)
{
    if(nodeIds.isEmpty())
        return;

    emitGraphWillChange();

    for(NodeId nodeId : nodeIds)
        addNode(nodeId);

    emitGraphChanged();
}

void Graph::removeNode(NodeId nodeId)
{
    emitGraphWillChange();

    // Remove all edges that touch this node
    const Node& node = _nodesVector[nodeId];
    for(EdgeId edgeId : node.edges())
        removeEdge(edgeId);

    if(_componentManagementEnabled && _componentManager != nullptr)
        _componentManager->nodeWillBeRemoved(nodeId);

    emit nodeWillBeRemoved(this, nodeId);

    _nodeIdsVector.remove(_nodeIdsVector.indexOf(nodeId));
    _vacatedNodeIdQueue.enqueue(nodeId);

    emitGraphChanged();
}

void Graph::removeNodes(const QSet<NodeId>& nodeIds)
{
    removeNodes(nodeIds.toList());
}

void Graph::removeNodes(const QList<NodeId>& nodeIds)
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
    if(!_vacatedEdgeIdQueue.isEmpty())
        return addEdge(_vacatedEdgeIdQueue.head(), sourceId, targetId);

    return addEdge(_nextEdgeId, sourceId, targetId);
}

EdgeId Graph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    if(edgeId < _nextEdgeId && !_vacatedEdgeIdQueue.contains(edgeId))
        return EdgeId(); // Already taken

    emitGraphWillChange();

    while(_nextEdgeId < edgeId)
    {
        // Queue up intervening IDs that will be skipped
        _vacatedEdgeIdQueue.enqueue(_nextEdgeId);
        _nextEdgeId++;
    }

    if(!_vacatedEdgeIdQueue.removeOne(edgeId))
        _nextEdgeId++;

    _edgeIdsVector.append(edgeId);
    _edgesVector.resize(edgeArrayCapacity());
    _edgesVector[edgeId]._id = edgeId;
    setEdgeNodes(edgeId, sourceId, targetId);

    for(ResizableGraphArray* edgeArray : _edgeArrayList)
        edgeArray->resize(edgeArrayCapacity());

    if(_componentManagementEnabled && _componentManager != nullptr)
        _componentManager->edgeAdded(edgeId);

    emit edgeAdded(this, edgeId);
    emitGraphChanged();

    return edgeId;
}

EdgeId Graph::addEdge(const Edge& edge)
{
    return addEdge(edge._id, edge._sourceId, edge._targetId);
}

void Graph::addEdges(const QSet<Edge>& edges)
{
    addEdges(edges.toList());
}

void Graph::addEdges(const QList<Edge>& edges)
{
    if(edges.isEmpty())
        return;

    emitGraphWillChange();

    for(const Edge& edge : edges)
        addEdge(edge);

    emitGraphChanged();
}

void Graph::removeEdge(EdgeId edgeId)
{
    emitGraphWillChange();

    if(_componentManagementEnabled && _componentManager != nullptr)
        _componentManager->edgeWillBeRemoved(edgeId);

    emit edgeWillBeRemoved(this, edgeId);

    // Remove all node references to this edge
    const Edge& edge = _edgesVector[edgeId];
    Node& source = _nodesVector[edge.sourceId()];
    Node& target = _nodesVector[edge.targetId()];
    source._outEdges.remove(edgeId);
    target._inEdges.remove(edgeId);

    _edgeIdsVector.remove(_edgeIdsVector.indexOf(edgeId));
    _vacatedEdgeIdQueue.enqueue(edgeId);

    emitGraphChanged();
}

void Graph::removeEdges(const QSet<EdgeId>& edgeIds)
{
    removeEdges(edgeIds.toList());
}

void Graph::removeEdges(const QList<EdgeId>& edgeIds)
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
    if(_componentManager != nullptr)
        return &_componentManager->componentIds();

    return nullptr;
}

int Graph::numComponents() const
{
    if(_componentManager != nullptr)
        return _componentManager->componentIds().size();

    return 0;
}

const ReadOnlyGraph *Graph::componentById(ComponentId componentId) const
{
    if(_componentManager != nullptr)
        return _componentManager->componentById(componentId);

    Q_ASSERT(nullptr);
    return nullptr;
}

ComponentId Graph::componentIdOfNode(NodeId nodeId) const
{
    if(_componentManager != nullptr)
        return _componentManager->componentIdOfNode(nodeId);

    return ComponentId();
}

ComponentId Graph::componentIdOfEdge(EdgeId edgeId) const
{
    if(_componentManager != nullptr)
        return _componentManager->componentIdOfEdge(edgeId);

    return ComponentId();
}

const QList<EdgeId> Graph::edgeIdsForNodes(const QSet<NodeId>& nodeIds)
{
    return edgeIdsForNodes(nodeIds.toList());
}

const QList<EdgeId> Graph::edgeIdsForNodes(const QList<NodeId>& nodeIds)
{
    QSet<EdgeId> edgeIds;

    for(NodeId nodeId : nodeIds)
    {
        const Node& node = _nodesVector[nodeId];
        for(EdgeId edgeId : node.edges())
            edgeIds.insert(edgeId);
    }

    return edgeIds.toList();
}

const QList<Edge> Graph::edgesForNodes(const QSet<NodeId>& nodeIds)
{
    return edgesForNodes(nodeIds.toList());
}

const QList<Edge> Graph::edgesForNodes(const QList<NodeId>& nodeIds)
{
    const QList<EdgeId>& edgeIds = edgeIdsForNodes(nodeIds);
    QList<Edge> edges;

    for(EdgeId edgeId : edgeIds)
        edges.append(_edgesVector[edgeId]);

    return edges;
}

void Graph::dumpToQDebug(int detail) const
{
    ReadOnlyGraph::dumpToQDebug(detail);

    if(detail > 0)
    {
        if(_componentManagementEnabled && _componentManager != nullptr)
        {
            for(ComponentId componentId : _componentManager->componentIds())
            {
                const ReadOnlyGraph* component = _componentManager->componentById(componentId);
                qDebug() << "component" << componentId;
                component->dumpToQDebug(detail);
            }
        }
    }
}

void Graph::emitGraphWillChange()
{
    if(_graphChangeDepth++ <= 0)
        emit graphWillChange(this);
}

void Graph::emitGraphChanged()
{
    Q_ASSERT(_graphChangeDepth > 0);
    if(--_graphChangeDepth <= 0)
    {
        if(_componentManagementEnabled && _componentManager != nullptr)
            _componentManager->graphChanged(this);

        emit graphChanged(this);
    }
}

void Graph::setEdgeNodes(Edge& edge, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!sourceId.isNull());
    Q_ASSERT(!targetId.isNull());

    if(!edge.sourceId().isNull())
    {
        // Remove edge from source node out edges
        Node& source = _nodesVector[edge.sourceId()];
        source._outEdges.remove(edge.id());
    }

    if(!edge.targetId().isNull())
    {
        // Remove edge from target node in edges
        Node& target = _nodesVector[edge.targetId()];
        target._inEdges.remove(edge.id());
    }

    edge._sourceId = sourceId;
    edge._targetId = targetId;

    Node& source = _nodesVector[sourceId];
    source._outEdges.insert(edge.id());

    Node& target = _nodesVector[targetId];
    target._inEdges.insert(edge.id());
}

void Graph::setEdgeNodes(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    setEdgeNodes(_edgesVector[edgeId], sourceId, targetId);
}
