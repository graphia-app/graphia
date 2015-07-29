#include "graph.h"
#include "grapharray.h"
#include "componentmanager.h"
#include "../utils/cpp1x_hacks.h"

#include <QtGlobal>
#include <QMetaType>

Graph::~Graph()
{
    // Let the GraphArrays know that we're going away
    for(auto nodeArray : _nodeArrayList)
        nodeArray->invalidate();

    for(auto edgeArray : _edgeArrayList)
        edgeArray->invalidate();
}

void Graph::dumpToQDebug(int detail) const
{
    qDebug() << numNodes() << "nodes" << numEdges() << "edges";

    if(detail > 0)
    {
        for(NodeId nodeId : nodeIds())
        {
            const Node& node = nodeById(nodeId);
            qDebug() << "Node" << nodeId << "in" << node.inEdges() << "out" << node.outEdges();
        }

        for(EdgeId edgeId : edgeIds())
        {
            const Edge& edge = edgeById(edgeId);
            qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
        }
    }
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

const Graph* Graph::componentById(ComponentId componentId) const
{
    if(_componentManager)
        return _componentManager->componentById(componentId);

    Q_ASSERT(!"Graph::componentById returning nullptr");
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

ComponentId Graph::largestComponentId() const
{
    ComponentId largestComponentId;
    int maxNumNodes = 0;
    for(auto componentId : componentIds())
    {
        auto component = componentById(componentId);
        if(component->numNodes() > maxNumNodes)
        {
            maxNumNodes = component->numNodes();
            largestComponentId = componentId;
        }
    }

    return largestComponentId;
}

MutableGraph::MutableGraph() :
    _lastNodeId(0),
    _lastEdgeId(0),
    _graphChangeDepth(0)
{
    enableComponentManagement(); //FIXME remove eventually

    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<ElementIdSet<NodeId>>("ElementIdSet<NodeId>");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<ElementIdSet<EdgeId>>("ElementIdSet<EdgeId>");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ElementIdSet<ComponentId>>("ElementIdSet<ComponentId>");
}

MutableGraph::~MutableGraph()
{
    // Ensure no transactions are in progress
    std::unique_lock<std::mutex>(_mutex);
}

void MutableGraph::clear()
{
    for(NodeId nodeId : nodeIds())
        removeNode(nodeId);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    _nodesVector.resize(0);
    _edgesVector.resize(0);
}

NodeId MutableGraph::addNode()
{
    if(!_unusedNodeIdsDeque.empty())
    {
        NodeId unusedNodeId = _unusedNodeIdsDeque.front();
        _unusedNodeIdsDeque.pop_front();

        return addNode(unusedNodeId);
    }

    return addNode(_lastNodeId);
}

NodeId MutableGraph::addNode(NodeId nodeId)
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

NodeId MutableGraph::addNode(const Node& node)
{
    return addNode(node._id);
}

void MutableGraph::addNodes(const ElementIdSet<NodeId>& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(NodeId nodeId : nodeIds)
        addNode(nodeId);

    endTransaction();
}

void MutableGraph::removeNode(NodeId nodeId)
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

void MutableGraph::removeNodes(const ElementIdSet<NodeId>& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(NodeId nodeId : nodeIds)
        removeNode(nodeId);

    endTransaction();
}

EdgeId MutableGraph::addEdge(NodeId sourceId, NodeId targetId)
{
    if(!_unusedEdgeIdsDeque.empty())
    {
        EdgeId unusedEdgeId = _unusedEdgeIdsDeque.front();
        _unusedEdgeIdsDeque.pop_front();

        return addEdge(unusedEdgeId, sourceId, targetId);
    }

    return addEdge(_lastEdgeId, sourceId, targetId);
}

EdgeId MutableGraph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
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

EdgeId MutableGraph::addEdge(const Edge& edge)
{
    return addEdge(edge._id, edge._sourceId, edge._targetId);
}

void MutableGraph::addEdges(const std::vector<Edge>& edges)
{
    if(edges.empty())
        return;

    beginTransaction();

    for(const Edge& edge : edges)
        addEdge(edge);

    endTransaction();
}

void MutableGraph::removeEdge(EdgeId edgeId)
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

void MutableGraph::removeEdges(const ElementIdSet<EdgeId>& edgeIds)
{
    if(edgeIds.empty())
        return;

    beginTransaction();

    for(EdgeId edgeId : edgeIds)
        removeEdge(edgeId);

    endTransaction();
}

const ElementIdSet<EdgeId> MutableGraph::edgeIdsForNodes(const ElementIdSet<NodeId>& nodeIds)
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

const std::vector<Edge> MutableGraph::edgesForNodes(const ElementIdSet<NodeId>& nodeIds)
{
    const ElementIdSet<EdgeId>& edgeIds = edgeIdsForNodes(nodeIds);
    std::vector<Edge> edges;

    for(EdgeId edgeId : edgeIds)
        edges.push_back(_edgesVector[edgeId]);

    return edges;
}

void MutableGraph::dumpToQDebug(int detail) const
{
    Graph::dumpToQDebug(detail);

    if(detail > 1)
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

void MutableGraph::enableComponentManagement()
{
    if(_componentManager == nullptr)
        _componentManager = std::make_unique<ComponentManager>(*this);
}

void MutableGraph::beginTransaction()
{
    if(_graphChangeDepth++ <= 0)
    {
        emit graphWillChange(this);
        _mutex.lock();
        debugPauser.pause("Begin Graph Transaction");
    }
}

void MutableGraph::endTransaction()
{
    Q_ASSERT(_graphChangeDepth > 0);
    if(--_graphChangeDepth <= 0)
    {
        updateElementIdData();
        _mutex.unlock();
        debugPauser.pause("End Graph Transaction");
        emit graphChanged(this);
    }
}

void MutableGraph::performTransaction(std::function<void(MutableGraph&)> transaction)
{
    ScopedTransaction lock(*this);
    transaction(*this);
}

void MutableGraph::updateElementIdData()
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

MutableGraph::ScopedTransaction::ScopedTransaction(MutableGraph& graph) :
    _graph(graph)
{
    _graph.beginTransaction();
}

MutableGraph::ScopedTransaction::~ScopedTransaction()
{
    _graph.endTransaction();
}
