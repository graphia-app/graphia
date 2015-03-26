#include "graph.h"
#include "grapharray.h"
#include "simplecomponentmanager.h"
#include "../utils/make_unique.h"

#include <QtGlobal>
#include <QMetaType>

Graph::Graph() :
    _lastNodeId(0),
    _lastEdgeId(0),
    _componentManager(std::make_unique<SimpleComponentManager>(*this)),
    _graphChangeDepth(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<ElementIdSet<NodeId>>("ElementIdSet<NodeId>");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<ElementIdSet<EdgeId>>("ElementIdSet<EdgeId>");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ElementIdSet<ComponentId>>("ElementIdSet<ComponentId>");
}

Graph::~Graph()
{
    // Ensure no transactions are in progress
    std::unique_lock<std::mutex>(_mutex);

    // Let the GraphArrays know that we're going away
    for(auto nodeArray : _nodeArrayList)
        nodeArray->invalidate();

    for(auto edgeArray : _edgeArrayList)
        edgeArray->invalidate();
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

NodeId Graph::addNode()
{
    if(!_unusedNodeIdsDeque.empty())
    {
        NodeId unusedNodeId = _unusedNodeIdsDeque.front();
        _unusedNodeIdsDeque.pop_front();

        return addNode(unusedNodeId);
    }

    return addNode(_lastNodeId);
}

NodeId Graph::addNode(NodeId nodeId)
{
    Q_ASSERT(!nodeId.isNull());

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(nodeId >= _lastNodeId || (nodeId < _lastNodeId && _nodeIdsInUse[nodeId]))
    {
        nodeId = _lastNodeId;
        _lastNodeId++;

        _nodeIdsInUse.resize(nodeArrayCapacity());
        _nodesVector.resize(nodeArrayCapacity());
        for(ResizableGraphArray* nodeArray : _nodeArrayList)
            nodeArray->resize(nodeArrayCapacity());
    }

    _nodeIdsInUse[nodeId] = true;
    Node& node = _nodesVector[nodeId];
    node._id = nodeId;
    node._inEdges.clear();
    node._outEdges.clear();
    node._edges.clear();

    emit nodeAdded(this, nodeId);
    endTransaction();

    return nodeId;
}

NodeId Graph::addNode(const Node& node)
{
    return addNode(node._id);
}

void Graph::addNodes(const ElementIdSet<NodeId>& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(NodeId nodeId : nodeIds)
        addNode(nodeId);

    endTransaction();
}

void Graph::removeNode(NodeId nodeId)
{
    beginTransaction();

    // Remove all edges that touch this node
    const Node& node = _nodesVector[nodeId];
    for(EdgeId edgeId : node.edges())
        removeEdge(edgeId);

    emit nodeWillBeRemoved(this, nodeId);

    _nodeIdsInUse[nodeId] = false;
    _unusedNodeIdsDeque.push_back(nodeId);

    endTransaction();
}

void Graph::removeNodes(const ElementIdSet<NodeId>& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(NodeId nodeId : nodeIds)
        removeNode(nodeId);

    endTransaction();
}

EdgeId Graph::addEdge(NodeId sourceId, NodeId targetId)
{
    if(!_unusedEdgeIdsDeque.empty())
    {
        EdgeId unusedEdgeId = _unusedEdgeIdsDeque.front();
        _unusedEdgeIdsDeque.pop_front();

        return addEdge(unusedEdgeId, sourceId, targetId);
    }

    return addEdge(_lastEdgeId, sourceId, targetId);
}

EdgeId Graph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!edgeId.isNull());

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(edgeId >= _lastEdgeId || (edgeId < _lastEdgeId && _edgeIdsInUse[edgeId]))
    {
        edgeId = _lastEdgeId;
        _lastEdgeId++;

        _edgeIdsInUse.resize(edgeArrayCapacity());
        _edgesVector.resize(edgeArrayCapacity());
        for(ResizableGraphArray* edgeArray : _edgeArrayList)
            edgeArray->resize(edgeArrayCapacity());
    }

    _edgeIdsInUse[edgeId] = true;
    Edge& edge = _edgesVector[edgeId];
    edge._id = edgeId;
    edge._sourceId = sourceId;
    edge._targetId = targetId;

    _nodesVector[sourceId]._outEdges.insert(edgeId);
    _nodesVector[sourceId]._edges.insert(edgeId);
    _nodesVector[targetId]._inEdges.insert(edgeId);
    _nodesVector[targetId]._edges.insert(edgeId);

    emit edgeAdded(this, edgeId);
    endTransaction();

    return edgeId;
}

EdgeId Graph::addEdge(const Edge& edge)
{
    return addEdge(edge._id, edge._sourceId, edge._targetId);
}

void Graph::addEdges(const std::vector<Edge>& edges)
{
    if(edges.empty())
        return;

    beginTransaction();

    for(const Edge& edge : edges)
        addEdge(edge);

    endTransaction();
}

void Graph::removeEdge(EdgeId edgeId)
{
    beginTransaction();

    emit edgeWillBeRemoved(this, edgeId);

    // Remove all node references to this edge
    const Edge& edge = _edgesVector[edgeId];
    Node& source = _nodesVector[edge.sourceId()];
    Node& target = _nodesVector[edge.targetId()];
    source._outEdges.erase(edgeId);
    source._edges.erase(edgeId);
    target._inEdges.erase(edgeId);
    target._edges.erase(edgeId);

    _edgeIdsInUse[edgeId] = false;
    _unusedEdgeIdsDeque.push_back(edgeId);

    endTransaction();
}

void Graph::removeEdges(const ElementIdSet<EdgeId>& edgeIds)
{
    if(edgeIds.empty())
        return;

    beginTransaction();

    for(EdgeId edgeId : edgeIds)
        removeEdge(edgeId);

    endTransaction();
}

const std::vector<ComponentId>& Graph::componentIds() const
{
    if(_componentManager)
        return _componentManager->componentIds();

    static std::vector<ComponentId> emptyComponentIdList;

    return emptyComponentIdList;
}

int Graph::numComponents() const
{
    if(_componentManager)
        return static_cast<int>(_componentManager->componentIds().size());

    return 0;
}

const ImmutableGraph* Graph::componentById(ComponentId componentId) const
{
    if(_componentManager)
        return _componentManager->componentById(componentId);

    Q_ASSERT(nullptr);
    return nullptr;
}

ComponentId Graph::componentIdOfNode(NodeId nodeId) const
{
    if(_componentManager)
        return _componentManager->componentIdOfNode(nodeId);

    return ComponentId();
}

ComponentId Graph::componentIdOfEdge(EdgeId edgeId) const
{
    if(_componentManager)
        return _componentManager->componentIdOfEdge(edgeId);

    return ComponentId();
}

const ElementIdSet<EdgeId> Graph::edgeIdsForNodes(const ElementIdSet<NodeId>& nodeIds)
{
    ElementIdSet<EdgeId> edgeIds;

    for(NodeId nodeId : nodeIds)
    {
        const Node& node = _nodesVector[nodeId];
        for(EdgeId edgeId : node.edges())
            edgeIds.insert(edgeId);
    }

    return edgeIds;
}

const std::vector<Edge> Graph::edgesForNodes(const ElementIdSet<NodeId>& nodeIds)
{
    const ElementIdSet<EdgeId>& edgeIds = edgeIdsForNodes(nodeIds);
    std::vector<Edge> edges;

    for(EdgeId edgeId : edgeIds)
        edges.push_back(_edgesVector[edgeId]);

    return edges;
}

void Graph::dumpToQDebug(int detail) const
{
    ImmutableGraph::dumpToQDebug(detail);

    if(detail > 0)
    {
        if(_componentManager)
        {
            for(ComponentId componentId : _componentManager->componentIds())
            {
                auto component = _componentManager->componentById(componentId);
                qDebug() << "component" << componentId;
                component->dumpToQDebug(detail);
            }
        }
    }
}

void Graph::beginTransaction()
{
    if(_graphChangeDepth++ <= 0)
    {
        emit graphWillChange(this);
        _mutex.lock();
    }
}

void Graph::endTransaction()
{
    Q_ASSERT(_graphChangeDepth > 0);
    if(--_graphChangeDepth <= 0)
    {
        updateElementIdData();
        _mutex.unlock();
        emit graphChanged(this);
    }
}

void Graph::performTransaction(std::function<void(Graph&)> transaction)
{
    ScopedTransaction lock(*this);
    transaction(*this);
}

void Graph::updateElementIdData()
{
    _nodeIdsVector.clear();
    _unusedNodeIdsDeque.clear();
    for(NodeId nodeId(0); nodeId < _lastNodeId; nodeId++)
    {
        if(_nodeIdsInUse[nodeId])
            _nodeIdsVector.push_back(nodeId);
        else
            _unusedNodeIdsDeque.push_back(nodeId);
    }

    _edgeIdsVector.clear();
    _unusedEdgeIdsDeque.clear();
    for(EdgeId edgeId(0); edgeId < _lastEdgeId; edgeId++)
    {
        if(_edgeIdsInUse[edgeId])
            _edgeIdsVector.push_back(edgeId);
        else
            _unusedEdgeIdsDeque.push_back(edgeId);
    }
}

Graph::ScopedTransaction::ScopedTransaction(Graph& graph) :
    _graph(graph)
{
    _graph.beginTransaction();
}

Graph::ScopedTransaction::~ScopedTransaction()
{
    _graph.endTransaction();
}
