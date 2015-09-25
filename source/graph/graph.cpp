#include "graph.h"
#include "grapharray.h"
#include "componentmanager.h"
#include "../utils/utils.h"
#include "../utils/cpp1x_hacks.h"

#include <QtGlobal>
#include <QMetaType>

#include <tuple>

Graph::Graph() :
    _nextNodeId(0), _nextEdgeId(0)
{
    qRegisterMetaType<NodeId>("NodeId");
    qRegisterMetaType<NodeIdSet>("NodeIdSet");
    qRegisterMetaType<EdgeId>("EdgeId");
    qRegisterMetaType<EdgeIdSet>("EdgeIdSet");
    qRegisterMetaType<ComponentId>("ComponentId");
    qRegisterMetaType<ComponentIdSet>("ComponentIdSet");

    connect(this, &Graph::nodeAdded, [this](const Graph*, const Node* node) { reserveNodeId(node->id()); });
    connect(this, &Graph::edgeAdded, [this](const Graph*, const Edge* edge) { reserveEdgeId(edge->id()); });
}

Graph::~Graph()
{
    // Let the GraphArrays know that we're going away
    for(auto nodeArray : _nodeArrays)
        nodeArray->invalidate();

    for(auto edgeArray : _edgeArrays)
        edgeArray->invalidate();
}

NodeId Graph::firstNodeId() const
{
    return nodeIds().size() > 0 ? nodeIds().at(0) : NodeId();
}

bool Graph::containsNodeId(NodeId nodeId) const
{
    return u::contains(nodeIds(), nodeId);
}

EdgeId Graph::firstEdgeId() const
{
    return edgeIds().size() > 0 ? edgeIds().at(0) : EdgeId();
}

bool Graph::containsEdgeId(EdgeId edgeId) const
{
    return u::contains(edgeIds(), edgeId);
}

void Graph::enableComponentManagement()
{
    if(_componentManager == nullptr)
    {
        _componentManager = std::make_unique<ComponentManager>(*this);

        connect(_componentManager.get(), &ComponentManager::componentAdded,         this, &Graph::componentAdded, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentWillBeRemoved, this, &Graph::componentWillBeRemoved, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentSplit,         this, &Graph::componentSplit, Qt::DirectConnection);
        connect(_componentManager.get(), &ComponentManager::componentsWillMerge,    this, &Graph::componentsWillMerge, Qt::DirectConnection);
    }
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

int Graph::numComponentArrays() const
{
    if(_componentManager)
        return _componentManager->componentArrayCapacity();

    return 0;
}

void Graph::insertComponentArray(GraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->_componentArrays.insert(componentArray);
}

void Graph::eraseComponentArray(GraphArray* componentArray) const
{
    if(_componentManager)
        _componentManager->_componentArrays.erase(componentArray);
}

NodeId Graph::nextNodeId() const
{
    return _nextNodeId;
}

EdgeId Graph::nextEdgeId() const
{
    return _nextEdgeId;
}

void Graph::reserveNodeId(NodeId nodeId)
{
    if(nodeId < nextNodeId())
        return;

    _nextNodeId = nodeId + 1;

    for(auto nodeArray : _nodeArrays)
        nodeArray->resize(_nextNodeId);
}

void Graph::reserveEdgeId(EdgeId edgeId)
{
    if(edgeId < nextEdgeId())
        return;

    _nextEdgeId = edgeId + 1;

    for(auto edgeArray : _edgeArrays)
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
    return static_cast<int>(componentIds().size());
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
    return componentIdOfLargestComponent(componentIds());
}

void Graph::setPhase(const QString& phase) const
{
    if(phase != _phase)
    {
        _phase = phase;
        emit phaseChanged();
    }
}

void Graph::clearPhase() const
{
    setPhase("");
}

const QString&Graph::phase() const
{
    return _phase;
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

    _updateRequired = true;
    endTransaction();

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    _n.clear();
    _e.clear();
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
    Q_ASSERT(_n._nodeIdsInUse[nodeId]);
    return _n._nodes[nodeId];
}

bool MutableGraph::containsNodeId(NodeId nodeId) const
{
    return _n._nodeIdsInUse[nodeId];
}

MultiNodeId::Type MutableGraph::typeOf(NodeId nodeId) const
{
    return MultiNodeId::typeOf(nodeId, _n._mergedNodeIds);
}

NodeIdSet MutableGraph::multiNodesForNodeId(NodeId nodeId) const
{
    return MultiNodeId::elements(nodeId, _n._mergedNodeIds);
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

    Graph::reserveNodeId(nodeId);
    _n.resize(nextNodeId());
}

NodeId MutableGraph::addNode(NodeId nodeId)
{
    Q_ASSERT(!nodeId.isNull());

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(nodeId >= nextNodeId() || (nodeId < nextNodeId() && _n._nodeIdsInUse[nodeId]))
    {
        nodeId = nextNodeId();
        reserveNodeId(nodeId);
    }

    _n._nodeIdsInUse[nodeId] = true;
    auto& node = _n._nodes[nodeId];
    node._id = nodeId;
    node._inEdgeIds.clear();
    node._outEdgeIds.clear();
    node._adjacentNodeIds.clear();

    emit nodeAdded(this, &node);
    _updateRequired = true;
    endTransaction();

    return nodeId;
}

NodeId MutableGraph::addNode(const Node& node)
{
    return addNode(node._id);
}

void MutableGraph::removeNode(NodeId nodeId)
{
    beginTransaction();

    // Remove all edges that touch this node
    auto& node = _n._nodes[nodeId];
    for(auto edgeId : node.edgeIds())
        removeEdge(edgeId);

    emit nodeWillBeRemoved(this, &node);

    MultiNodeId::remove(nodeId, _n._mergedNodeIds);
    _n._nodeIdsInUse[nodeId] = false;
    _unusedNodeIds.push_back(nodeId);

    _updateRequired = true;
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
    Q_ASSERT(_e._edgeIdsInUse[edgeId]);
    return _e._edges[edgeId];
}

bool MutableGraph::containsEdgeId(EdgeId edgeId) const
{
    return _e._edgeIdsInUse[edgeId];
}

MultiEdgeId::Type MutableGraph::typeOf(EdgeId edgeId) const
{
    return MultiEdgeId::typeOf(edgeId, _e._mergedEdgeIds);
}

EdgeIdSet MutableGraph::multiEdgesForEdgeId(EdgeId edgeId) const
{
    return MultiEdgeId::elements(edgeId, _e._mergedEdgeIds);
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

    Graph::reserveEdgeId(edgeId);
    _e.resize(nextEdgeId());
}

NodeId MutableGraph::mergeNodes(NodeId nodeIdA, NodeId nodeIdB)
{
    return MultiNodeId::add(nodeIdA, nodeIdB, _n._mergedNodeIds);
}

EdgeId MutableGraph::mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB)
{
    return MultiEdgeId::add(edgeIdA, edgeIdB, _e._mergedEdgeIds);
}

EdgeId MutableGraph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!edgeId.isNull());
    Q_ASSERT(_n._nodeIdsInUse[sourceId]);
    Q_ASSERT(_n._nodeIdsInUse[targetId]);

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(edgeId >= nextEdgeId() || (edgeId < nextEdgeId() && _e._edgeIdsInUse[edgeId]))
    {
        edgeId = nextEdgeId();
        reserveEdgeId(edgeId);
    }

    _e._edgeIdsInUse[edgeId] = true;
    auto& edge = _e._edges[edgeId];
    edge._id = edgeId;
    edge._sourceId = sourceId;
    edge._targetId = targetId;

    _n._nodes[sourceId]._outEdgeIds.insert(edgeId);
    auto sourceInsert = _n._nodes[sourceId]._adjacentNodeIds.insert({targetId, edgeId});
    _n._nodes[targetId]._inEdgeIds.insert(edgeId);
    auto targetInsert = _n._nodes[targetId]._adjacentNodeIds.insert({sourceId, edgeId});

    Q_ASSERT(sourceId == targetId || sourceInsert.second == targetInsert.second);
    if(!sourceInsert.second && !targetInsert.second)
        mergeEdges(edgeId, (*sourceInsert.first).second);

    emit edgeAdded(this, &edge);
    _updateRequired = true;
    endTransaction();

    return edgeId;
}

EdgeId MutableGraph::addEdge(const Edge& edge)
{
    return addEdge(edge._id, edge._sourceId, edge._targetId);
}

void MutableGraph::removeEdge(EdgeId edgeId)
{
    beginTransaction();

    // Remove all node references to this edge
    const auto& edge = _e._edges[edgeId];

    emit edgeWillBeRemoved(this, &edge);

    auto& source = _n._nodes[edge.sourceId()];
    auto& target = _n._nodes[edge.targetId()];
    source._outEdgeIds.erase(edgeId);
    source._adjacentNodeIds.erase(edge.targetId());
    target._inEdgeIds.erase(edgeId);
    target._adjacentNodeIds.erase(edge.sourceId());

    MultiEdgeId::remove(edgeId, _e._mergedEdgeIds);
    _e._edgeIdsInUse[edgeId] = false;
    _unusedEdgeIds.push_back(edgeId);

    _updateRequired = true;
    endTransaction();
}

// Move the edges to connect to nodeId
template<typename C> static void moveEdgesTo(MutableGraph& graph, NodeId nodeId,
                                             const C& inEdgeIds,
                                             const C& outEdgeIds)
{
    for(auto edgeIdToMove : inEdgeIds)
    {
        auto sourceId = graph.edgeById(edgeIdToMove).sourceId();
        graph.removeEdge(edgeIdToMove);
        graph.addEdge(edgeIdToMove, sourceId, nodeId);
    }

    for(auto edgeIdToMove : outEdgeIds)
    {
        auto targetId = graph.edgeById(edgeIdToMove).targetId();
        graph.removeEdge(edgeIdToMove);
        graph.addEdge(edgeIdToMove, nodeId, targetId);
    }
}

void MutableGraph::contractEdge(EdgeId edgeId)
{
    // Can't contract an edge that doesn't exist
    if(!containsEdgeId(edgeId))
        return;

    beginTransaction();

    auto& edge = edgeById(edgeId);
    NodeId nodeId, nodeIdToMerge;
    std::tie(nodeId, nodeIdToMerge) = std::minmax(edge.sourceId(), edge.targetId());
    auto& nodeToMerge = nodeById(nodeIdToMerge);

    removeEdge(edgeId);
    moveEdgesTo(*this, nodeId, nodeToMerge.inEdgeIds(), nodeToMerge.outEdgeIds());
    mergeNodes(nodeId, nodeIdToMerge);

    _updateRequired = true;
    endTransaction();
}

void MutableGraph::contractEdges(const EdgeIdSet& edgeIds)
{
    if(edgeIds.empty())
        return;

    beginTransaction();

    // Divide into components, but ignore any edges that aren't being contracted,
    // so that each component represents a set of nodes that will be merged
    ComponentManager componentManager(*this, nullptr,
    [edgeIds](EdgeId edgeId)
    {
        return !u::contains(edgeIds, edgeId);
    });

    removeEdges(edgeIds);

    for(auto componentId : componentManager.componentIds())
    {
        auto component = componentManager.componentById(componentId);

        // Nothing to contract
        if(component->numEdges() == 0)
            continue;

        auto nodeId = *std::min_element(component->nodeIds().cbegin(),
                                        component->nodeIds().cend());

        moveEdgesTo(*this, nodeId, inEdgeIdsForNodes(component->nodeIds()),
                    outEdgeIdsForNodes(component->nodeIds()));

        for(auto nodeIdToMerge : component->nodeIds())
            mergeNodes(nodeId, nodeIdToMerge);
    }

    _updateRequired = true;
    endTransaction();
}

void MutableGraph::reserve(const Graph& other)
{
    const auto* mutableOther = dynamic_cast<const MutableGraph*>(&other);
    Q_ASSERT(mutableOther != nullptr);

    reserveNodeId(mutableOther->largestNodeId());
    reserveEdgeId(mutableOther->largestEdgeId());
}

void MutableGraph::cloneFrom(const Graph& other)
{
    beginTransaction();

    const auto* mutableOther = dynamic_cast<const MutableGraph*>(&other);
    Q_ASSERT(mutableOther != nullptr);

    _n             = mutableOther->_n;
    _nodeIds       = mutableOther->_nodeIds;
    _unusedNodeIds = mutableOther->_unusedNodeIds;
    reserveNodeId(mutableOther->largestNodeId());

    _e             = mutableOther->_e;
    _edgeIds       = mutableOther->_edgeIds;
    _unusedEdgeIds = mutableOther->_unusedEdgeIds;
    reserveEdgeId(mutableOther->largestEdgeId());

    _updateRequired = true;
    endTransaction();
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
        update();
        _mutex.unlock();
        debugPauser.pause("End Graph Transaction");
        emit graphChanged(this);
        clearPhase();
    }
}

void MutableGraph::performTransaction(std::function<void(MutableGraph&)> transaction)
{
    ScopedTransaction lock(*this);
    transaction(*this);
}

void MutableGraph::update()
{
    if(!_updateRequired)
        return;

    _updateRequired = false;

    _nodeIds.clear();
    _unusedNodeIds.clear();
    for(NodeId nodeId(0); nodeId < nextNodeId(); nodeId++)
    {
        if(_n._nodeIdsInUse[nodeId])
            _nodeIds.emplace_back(nodeId);
        else
            _unusedNodeIds.emplace_back(nodeId);
    }

    _edgeIds.clear();
    _unusedEdgeIds.clear();
    for(EdgeId edgeId(0); edgeId < nextEdgeId(); edgeId++)
    {
        if(_e._edgeIdsInUse[edgeId])
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
