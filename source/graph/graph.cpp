#include "graph.h"
#include "grapharray.h"
#include "componentmanager.h"
#include "../utils/cpp1x_hacks.h"

#include <QtGlobal>
#include <QMetaType>

Graph::Graph(bool componentManagement) :
    _nextNodeId(0), _nextEdgeId(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<ElementIdSet<NodeId>>("ElementIdSet<NodeId>");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<ElementIdSet<EdgeId>>("ElementIdSet<EdgeId>");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ElementIdSet<ComponentId>>("ElementIdSet<ComponentId>");

    connect(this, &Graph::nodeAdded, [this](const Graph*, const Node* node)
    {
        if(node->id() >= nextNodeId())
            setNextNodeId(NodeId(node->id() + 1));
    });

    connect(this, &Graph::edgeAdded, [this](const Graph*, const Edge* edge)
    {
        if(edge->id() >= nextEdgeId())
            setNextEdgeId(EdgeId(edge->id() + 1));
    });

    if(componentManagement)
        _componentManager = std::make_unique<ComponentManager>(*this);
}

Graph::~Graph()
{
    // Let the GraphArrays know that we're going away
    for(auto nodeArray : _nodeArrayList)
        nodeArray->invalidate();

    for(auto edgeArray : _edgeArrayList)
        edgeArray->invalidate();
}

NodeId Graph::firstNodeId() const
{
    return nodeIds().size() > 0 ? nodeIds().at(0) : NodeId();
}

bool Graph::containsNodeId(NodeId nodeId) const
{
    return std::find(nodeIds().cbegin(), nodeIds().cend(), nodeId) != nodeIds().cend();
}

EdgeId Graph::firstEdgeId() const
{
    return edgeIds().size() > 0 ? edgeIds().at(0) : EdgeId();
}

bool Graph::containsEdgeId(EdgeId edgeId) const
{
    return std::find(edgeIds().cbegin(), edgeIds().cend(), edgeId) != edgeIds().cend();
}

const ElementIdSet<EdgeId> Graph::edgeIdsForNodes(const ElementIdSet<NodeId>& nodeIds) const
{
    ElementIdSet<EdgeId> edgeIds;

    for(NodeId nodeId : nodeIds)
    {
        const Node& node = nodeById(nodeId);
        for(EdgeId edgeId : node.edgeIds())
            edgeIds.insert(edgeId);
    }

    return edgeIds;
}

const std::vector<Edge> Graph::edgesForNodes(const ElementIdSet<NodeId>& nodeIds) const
{
    const ElementIdSet<EdgeId>& edgeIds = edgeIdsForNodes(nodeIds);
    std::vector<Edge> edges;

    for(EdgeId edgeId : edgeIds)
        edges.push_back(edgeById(edgeId));

    return edges;
}

const ElementIdSet<NodeId> Graph::adjacentNodeIds(NodeId nodeId) const
{
    auto node = nodeById(nodeId);
    ElementIdSet<NodeId> nodes;

    for(auto edgeId : node.edgeIds())
        nodes.insert(edgeById(edgeId).oppositeId(nodeId));

    return nodes;
}

void Graph::dumpToQDebug(int detail) const
{
    qDebug() << numNodes() << "nodes" << numEdges() << "edges";

    if(detail > 0)
    {
        for(NodeId nodeId : nodeIds())
        {
            const Node& node = nodeById(nodeId);
            qDebug() << "Node" << nodeId << "in" << node.inEdgeIds() << "out" << node.outEdgeIds();
        }

        for(EdgeId edgeId : edgeIds())
        {
            const Edge& edge = edgeById(edgeId);
            qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
        }
    }

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

NodeId Graph::nextNodeId() const
{
    return _nextNodeId;
}

EdgeId Graph::nextEdgeId() const
{
    return _nextEdgeId;
}

void Graph::setNextNodeId(NodeId nextNodeId)
{
    _nextNodeId = nextNodeId;

    for(auto nodeArray : _nodeArrayList)
        nodeArray->resize(_nextNodeId);
}

void Graph::setNextEdgeId(EdgeId lastEdgeId)
{
    _nextEdgeId = lastEdgeId;

    for(auto edgeArray : _edgeArrayList)
        edgeArray->resize(_nextEdgeId);
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

ComponentId Graph::componentIdOfLargestComponent() const
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

MutableGraph::MutableGraph(bool componentManagement) :
    Graph(componentManagement)
{
}

MutableGraph::~MutableGraph()
{
    // Ensure no transactions are in progress
    std::unique_lock<std::mutex>(_mutex);
}

void MutableGraph::clear()
{
    beginTransaction();

    for(NodeId nodeId : nodeIds())
        removeNode(nodeId);

    endTransaction();

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

    return addNode(nextNodeId());
}

void MutableGraph::reserveNodeId(NodeId nodeId)
{
    if(nodeId < nextNodeId())
        return;

    setNextNodeId(NodeId(nodeId + 1));
    _nodeIdsInUse.resize(nextNodeId());
    _nodesVector.resize(nextNodeId());
}

NodeId MutableGraph::addNode(NodeId nodeId)
{
    Q_ASSERT(!nodeId.isNull());

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(nodeId >= nextNodeId() || (nodeId < nextNodeId() && _nodeIdsInUse[nodeId]))
    {
        nodeId = nextNodeId();
        reserveNodeId(nodeId);
    }

    _nodeIdsInUse[nodeId] = true;
    Node& node = _nodesVector[nodeId];
    node._id = nodeId;
    node._inEdgeIds.clear();
    node._outEdgeIds.clear();
    node._edgeIds.clear();

    emit nodeAdded(this, &node);
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
    for(EdgeId edgeId : node.edgeIds())
        removeEdge(edgeId);

    emit nodeWillBeRemoved(this, &node);

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

    return addEdge(nextEdgeId(), sourceId, targetId);
}

void MutableGraph::reserveEdgeId(EdgeId edgeId)
{
    if(edgeId < nextEdgeId())
        return;

    setNextEdgeId(EdgeId(edgeId + 1));
    _edgeIdsInUse.resize(nextEdgeId());
    _edgesVector.resize(nextEdgeId());
}

EdgeId MutableGraph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!edgeId.isNull());
    Q_ASSERT(_nodeIdsInUse[sourceId]);
    Q_ASSERT(_nodeIdsInUse[targetId]);

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(edgeId >= nextEdgeId() || (edgeId < nextEdgeId() && _edgeIdsInUse[edgeId]))
    {
        edgeId = nextEdgeId();
        reserveEdgeId(edgeId);
    }

    _edgeIdsInUse[edgeId] = true;
    Edge& edge = _edgesVector[edgeId];
    edge._id = edgeId;
    edge._sourceId = sourceId;
    edge._targetId = targetId;

    _nodesVector[sourceId]._outEdgeIds.insert(edgeId);
    _nodesVector[sourceId]._edgeIds.insert(edgeId);
    _nodesVector[targetId]._inEdgeIds.insert(edgeId);
    _nodesVector[targetId]._edgeIds.insert(edgeId);

    emit edgeAdded(this, &edge);
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

    // Remove all node references to this edge
    const Edge& edge = _edgesVector[edgeId];

    emit edgeWillBeRemoved(this, &edge);

    Node& source = _nodesVector[edge.sourceId()];
    Node& target = _nodesVector[edge.targetId()];
    source._outEdgeIds.erase(edgeId);
    source._edgeIds.erase(edgeId);
    target._inEdgeIds.erase(edgeId);
    target._edgeIds.erase(edgeId);

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

void MutableGraph::reserve(const MutableGraph& other)
{
    reserveNodeId(other.nextNodeId());
    reserveEdgeId(other.nextEdgeId());
}

void MutableGraph::cloneFrom(const MutableGraph& other)
{
    _nodeIdsInUse       = other._nodeIdsInUse;
    _nodeIdsVector      = other._nodeIdsVector;
    _unusedNodeIdsDeque = other._unusedNodeIdsDeque;
    _nodesVector        = other._nodesVector;
    setNextNodeId(other.nextNodeId());

    _edgeIdsInUse       = other._edgeIdsInUse;
    _edgeIdsVector      = other._edgeIdsVector;
    _unusedEdgeIdsDeque = other._unusedEdgeIdsDeque;
    _edgesVector        = other._edgesVector;
    setNextEdgeId(other.nextEdgeId());
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
    for(NodeId nodeId(0); nodeId < nextNodeId(); nodeId++)
    {
        if(_nodeIdsInUse[nodeId])
            _nodeIdsVector.emplace_back(nodeId);
        else
            _unusedNodeIdsDeque.emplace_back(nodeId);
    }

    _edgeIdsVector.clear();
    _unusedEdgeIdsDeque.clear();
    for(EdgeId edgeId(0); edgeId < nextEdgeId(); edgeId++)
    {
        if(_edgeIdsInUse[edgeId])
            _edgeIdsVector.emplace_back(edgeId);
        else
            _unusedEdgeIdsDeque.emplace_back(edgeId);
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
