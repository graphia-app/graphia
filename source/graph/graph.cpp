#include "graph.h"
#include "grapharray.h"
#include "componentmanager.h"
#include "../utils/cpp1x_hacks.h"

#include <QtGlobal>
#include <QMetaType>

#include <tuple>

Graph::Graph(bool componentManagement) :
    _nextNodeId(0), _nextEdgeId(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<NodeIdSet>("NodeIdSet");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<EdgeIdSet>("EdgeIdSet");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ComponentIdSet>("ComponentIdSet");

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

const EdgeIdSet Graph::edgeIdsForNodes(const NodeIdSet& nodeIds) const
{
    EdgeIdSet edgeIds;

    for(auto nodeId : nodeIds)
    {
        auto& node = nodeById(nodeId);
        for(auto edgeId : node.edgeIds())
            edgeIds.insert(edgeId);
    }

    return edgeIds;
}

const std::vector<Edge> Graph::edgesForNodes(const NodeIdSet& nodeIds) const
{
    auto& edgeIds = edgeIdsForNodes(nodeIds);
    std::vector<Edge> edges;

    for(auto edgeId : edgeIds)
        edges.push_back(edgeById(edgeId));

    return edges;
}

void Graph::dumpToQDebug(int detail) const
{
    qDebug() << numNodes() << "nodes" << numEdges() << "edges";

    if(detail > 0)
    {
        for(auto nodeId : nodeIds())
        {
            auto& node = nodeById(nodeId);
            qDebug() << "Node" << nodeId << node.edgeIds();
        }

        for(auto edgeId : edgeIds())
        {
            auto& edge = edgeById(edgeId);
            qDebug() << "Edge" << edgeId << "(" << edge.sourceId() << "->" << edge.targetId() << ")";
        }
    }

    if(detail > 1)
    {
        if(_componentManager)
        {
            for(auto componentId : _componentManager->componentIds())
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

    for(auto nodeId : nodeIds())
        removeNode(nodeId);

    endTransaction();

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    _nodes.resize(0);
    _edges.resize(0);
}

const std::vector<NodeId>& MutableGraph::nodeIds() const
{
    return _nodeIds;
}

int MutableGraph::numNodes() const
{
    return static_cast<int>(_nodeIds.size());
}

const Node& MutableGraph::nodeById(NodeId nodeId) const
{
    Q_ASSERT(_nodeIdsInUse[nodeId]);
    return _nodes[nodeId];
}

bool MutableGraph::containsNodeId(NodeId nodeId) const
{
    return _nodeIdsInUse[nodeId];
}

MultiNodeId::Type MutableGraph::typeOf(NodeId nodeId) const
{
    return MultiNodeId::typeOf(nodeId, _multiNodeIds);
}

NodeId MutableGraph::addNode()
{
    if(!_unusedNodeIds.empty())
    {
        auto unusedNodeId = _unusedNodeIds.front();
        _unusedNodeIds.pop_front();

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
    _multiNodeIds.resize(nextNodeId());
    _nodes.resize(nextNodeId());
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
    auto& node = _nodes[nodeId];
    node._id = nodeId;
    node._edgeIds.clear();
    node._adjacentNodeIds.clear();

    emit nodeAdded(this, &node);
    endTransaction();

    return nodeId;
}

NodeId MutableGraph::addNode(const Node& node)
{
    return addNode(node._id);
}

void MutableGraph::addNodes(const NodeIdSet& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(auto nodeId : nodeIds)
        addNode(nodeId);

    endTransaction();
}

void MutableGraph::removeNode(NodeId nodeId)
{
    beginTransaction();

    // Remove all edges that touch this node
    auto& node = _nodes[nodeId];
    for(auto edgeId : node.edgeIds())
        removeEdge(edgeId);

    emit nodeWillBeRemoved(this, &node);

    MultiNodeId::removeMultiElementId(nodeId, _multiNodeIds);
    _nodeIdsInUse[nodeId] = false;
    _unusedNodeIds.push_back(nodeId);

    endTransaction();
}

void MutableGraph::removeNodes(const NodeIdSet& nodeIds)
{
    if(nodeIds.empty())
        return;

    beginTransaction();

    for(auto nodeId : nodeIds)
        removeNode(nodeId);

    endTransaction();
}

const std::vector<EdgeId>& MutableGraph::edgeIds() const
{
    return _edgeIds;
}

int MutableGraph::numEdges() const
{
    return static_cast<int>(_edgeIds.size());
}

const Edge& MutableGraph::edgeById(EdgeId edgeId) const
{
    Q_ASSERT(_edgeIdsInUse[edgeId]);
    return _edges[edgeId];
}

bool MutableGraph::containsEdgeId(EdgeId edgeId) const
{
    return _edgeIdsInUse[edgeId];
}

MultiEdgeId::Type MutableGraph::typeOf(EdgeId edgeId) const
{
    return MultiEdgeId::typeOf(edgeId, _multiEdgeIds);
}

EdgeId MutableGraph::addEdge(NodeId sourceId, NodeId targetId)
{
    if(!_unusedEdgeIds.empty())
    {
        auto unusedEdgeId = _unusedEdgeIds.front();
        _unusedEdgeIds.pop_front();

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
    _multiEdgeIds.resize(nextEdgeId());
    _edges.resize(nextEdgeId());
}

NodeId MutableGraph::mergeNodes(NodeId nodeIdA, NodeId nodeIdB)
{
    return MultiNodeId::mergeElements(nodeIdA, nodeIdB, _multiNodeIds);
}

EdgeId MutableGraph::mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB)
{
    return MultiEdgeId::mergeElements(edgeIdA, edgeIdB, _multiEdgeIds);
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
    auto& edge = _edges[edgeId];
    edge._id = edgeId;
    edge._sourceId = sourceId;
    edge._targetId = targetId;

    _nodes[sourceId]._edgeIds.insert(edgeId);
    auto sourceInsert = _nodes[sourceId]._adjacentNodeIds.insert(targetId);
    _nodes[targetId]._edgeIds.insert(edgeId);
    auto targetInsert = _nodes[targetId]._adjacentNodeIds.insert(sourceId);

    Q_ASSERT(sourceId == targetId || sourceInsert.second == targetInsert.second);
    if(!sourceInsert.second && !targetInsert.second)
    {
        // No insert occurred, meaning there is already an edge between these nodes
        for(auto otherEdgeId : _nodes[sourceId]._edgeIds)
        {
            if(otherEdgeId == edgeId)
                continue;

            auto otherEdge = edgeById(otherEdgeId);

            if(std::minmax(sourceId, targetId) == std::minmax(otherEdge.sourceId(), otherEdge.targetId()))
            {
                mergeEdges(edgeId, otherEdgeId);
                break;
            }
        }
    }

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

    for(const auto& edge : edges)
        addEdge(edge);

    endTransaction();
}

void MutableGraph::removeEdge(EdgeId edgeId)
{
    beginTransaction();

    // Remove all node references to this edge
    const auto& edge = _edges[edgeId];

    emit edgeWillBeRemoved(this, &edge);

    auto& source = _nodes[edge.sourceId()];
    auto& target = _nodes[edge.targetId()];
    source._edgeIds.erase(edgeId);
    source._adjacentNodeIds.erase(edge.targetId());
    target._edgeIds.erase(edgeId);
    target._adjacentNodeIds.erase(edge.sourceId());

    MultiEdgeId::removeMultiElementId(edgeId, _multiEdgeIds);
    _edgeIdsInUse[edgeId] = false;
    _unusedEdgeIds.push_back(edgeId);

    endTransaction();
}

void MutableGraph::removeEdges(const EdgeIdSet& edgeIds)
{
    if(edgeIds.empty())
        return;

    beginTransaction();

    for(auto edgeId : edgeIds)
        removeEdge(edgeId);

    endTransaction();
}

void MutableGraph::contractEdge(EdgeId edgeId)
{
    // Can't contract an edge that doesn't exist
    if(!containsEdgeId(edgeId))
        return;

    auto& edge = edgeById(edgeId);
    NodeId nodeId, nodeIdToMerge;
    std::tie(nodeId, nodeIdToMerge) = std::minmax(edge.sourceId(), edge.targetId());
    auto& nodeToMerge = nodeById(nodeIdToMerge);

    removeEdge(edgeId);

    // Move all the edges that were connected to nodeIdToMerge to nodeId
    for(auto edgeIdToMove : nodeToMerge.edgeIds())
    {
        auto& edgeToMove = edgeById(edgeIdToMove);
        auto otherNodeId = edgeToMove.oppositeId(nodeIdToMerge);

        removeEdge(edgeIdToMove);

        if(edgeToMove.sourceId() == otherNodeId)
            addEdge(edgeIdToMove, otherNodeId, nodeId);
        else
            addEdge(edgeIdToMove, nodeId, otherNodeId);
    }

    mergeNodes(nodeId, nodeIdToMerge);
}

void MutableGraph::reserve(const Graph& other)
{
    const auto* mutableOther = dynamic_cast<const MutableGraph*>(&other);
    Q_ASSERT(mutableOther != nullptr);

    reserveNodeId(mutableOther->nextNodeId());
    reserveEdgeId(mutableOther->nextEdgeId());
}

void MutableGraph::cloneFrom(const Graph& other)
{
    const auto* mutableOther = dynamic_cast<const MutableGraph*>(&other);
    Q_ASSERT(mutableOther != nullptr);

    _nodeIdsInUse       = mutableOther->_nodeIdsInUse;
    _nodeIds      = mutableOther->_nodeIds;
    _unusedNodeIds = mutableOther->_unusedNodeIds;
    _nodes        = mutableOther->_nodes;
    _multiNodeIds       = mutableOther->_multiNodeIds;
    setNextNodeId(mutableOther->nextNodeId());

    _edgeIdsInUse       = mutableOther->_edgeIdsInUse;
    _edgeIds      = mutableOther->_edgeIds;
    _unusedEdgeIds = mutableOther->_unusedEdgeIds;
    _edges        = mutableOther->_edges;
    _multiEdgeIds       = mutableOther->_multiEdgeIds;
    setNextEdgeId(mutableOther->nextEdgeId());
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
    _nodeIds.clear();
    _unusedNodeIds.clear();
    for(NodeId nodeId(0); nodeId < nextNodeId(); nodeId++)
    {
        if(_nodeIdsInUse[nodeId])
            _nodeIds.emplace_back(nodeId);
        else
            _unusedNodeIds.emplace_back(nodeId);
    }

    _edgeIds.clear();
    _unusedEdgeIds.clear();
    for(EdgeId edgeId(0); edgeId < nextEdgeId(); edgeId++)
    {
        if(_edgeIdsInUse[edgeId])
            _edgeIds.emplace_back(edgeId);
        else
            _unusedEdgeIds.emplace_back(edgeId);
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
