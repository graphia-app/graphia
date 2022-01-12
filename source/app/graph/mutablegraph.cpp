/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mutablegraph.h"

#include "graphcomponent.h"
#include "componentmanager.h"

#include "shared/utils/container.h"

MutableGraph::MutableGraph(const MutableGraph& other) // NOLINT bugprone-copy-constructor-init
{
    clone(other);
}

MutableGraph::~MutableGraph() // NOLINT modernize-use-equals-default
{
    // Ensure no transactions are in progress
    std::unique_lock<std::mutex> lock(_mutex);
}

void MutableGraph::clear()
{
    beginTransaction();

    bool changed = numNodes() > 0;

    for(auto nodeId : nodeIds())
        removeNode(nodeId);

    _updateRequired = true;
    endTransaction(changed);

    // Removing all the nodes should remove all the edges
    Q_ASSERT(numEdges() == 0);

    _n.clear();
    _n.resize(0);
    _e.clear();
    _e.resize(0);

    Graph::clear();
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
    Q_ASSERT(_n._nodeIdsInUse[static_cast<int>(nodeId)]);
    return _n._nodes[static_cast<int>(nodeId)];
}

bool MutableGraph::containsNodeId(NodeId nodeId) const
{
    return static_cast<int>(nodeId) < static_cast<int>(_n._nodeIdsInUse.size()) &&
        _n._nodeIdsInUse[static_cast<int>(nodeId)];
}

MultiElementType MutableGraph::typeOf(NodeId nodeId) const
{
    return _n._mergedNodeIds.typeOf(nodeId);
}

ConstNodeIdDistinctSet MutableGraph::mergedNodeIdsForNodeId(NodeId nodeId) const
{
    return {nodeId, &_n._mergedNodeIds};
}

int MutableGraph::multiplicityOf(NodeId nodeId) const
{
    return _n._multiplicities[static_cast<int>(nodeId)];
}

std::vector<EdgeId> MutableGraph::edgeIdsBetween(NodeId nodeIdA, NodeId nodeIdB) const
{
    std::vector<EdgeId> edgeIds;

    auto undirectedEdge = UndirectedEdge(nodeIdA, nodeIdB);
    if(u::contains(_e._connections, undirectedEdge))
    {
        const auto& edgeIdDistinctSet = _e._connections.at(undirectedEdge);
        std::copy(edgeIdDistinctSet.begin(), edgeIdDistinctSet.end(), std::back_inserter(edgeIds));
    }

    return edgeIds;
}

EdgeId MutableGraph::firstEdgeIdBetween(NodeId nodeIdA, NodeId nodeIdB) const
{
    const auto& nodeA = nodeById(nodeIdA);

    for(auto edgeId : nodeA._outEdgeIds)
    {
        if(edgeById(edgeId).targetId() == nodeIdB)
            return edgeId;
    }

    for(auto edgeId : nodeA._inEdgeIds)
    {
        if(edgeById(edgeId).sourceId() == nodeIdB)
            return edgeId;
    }

    return {};
}

bool MutableGraph::edgeExistsBetween(NodeId nodeIdA, NodeId nodeIdB) const
{
    return !firstEdgeIdBetween(nodeIdA, nodeIdB).isNull();
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

Node& MutableGraph::nodeBy(NodeId nodeId)
{
    return _n._nodes[static_cast<int>(nodeId)];
}

const Node& MutableGraph::nodeBy(NodeId nodeId) const
{
    return _n._nodes[static_cast<int>(nodeId)];
}

void MutableGraph::claimNodeId(NodeId nodeId)
{
    _n._nodeIdsInUse[static_cast<int>(nodeId)] = true;
}

void MutableGraph::releaseNodeId(NodeId nodeId)
{
    _n._nodeIdsInUse[static_cast<int>(nodeId)] = false;
}

Edge& MutableGraph::edgeBy(EdgeId edgeId)
{
    return _e._edges[static_cast<int>(edgeId)];
}

const Edge& MutableGraph::edgeBy(EdgeId edgeId) const
{
    return _e._edges[static_cast<int>(edgeId)];
}

void MutableGraph::claimEdgeId(EdgeId edgeId)
{
    _e._edgeIdsInUse[static_cast<int>(edgeId)] = true;
}

void MutableGraph::releaseEdgeId(EdgeId edgeId)
{
    _e._edgeIdsInUse[static_cast<int>(edgeId)] = false;
}

void MutableGraph::reserveNodeId(NodeId nodeId)
{
    if(nodeId < nextNodeId())
        return;

    auto unusedNodeId = nextNodeId();

    Graph::reserveNodeId(nodeId);
    _n.resize(static_cast<int>(nextNodeId()));

    while(unusedNodeId < nodeId)
        _unusedNodeIds.push_back(unusedNodeId++);
}

NodeId MutableGraph::addNode(NodeId nodeId)
{
    Q_ASSERT(!nodeId.isNull());

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(nodeId >= nextNodeId() || containsNodeId(nodeId))
    {
        nodeId = nextNodeId();
        reserveNodeId(nodeId);
    }

    claimNodeId(nodeId);
    auto& node = nodeBy(nodeId);
    node._id = nodeId;
    node._inEdgeIds.setCollection(&_e._inEdgeIdsCollection);
    node._outEdgeIds.setCollection(&_e._outEdgeIdsCollection);

    emit nodeAdded(this, nodeId);
    _updateRequired = true;
    endTransaction();

    return nodeId;
}

NodeId MutableGraph::addNode(const INode& node)
{
    return addNode(node.id());
}

void MutableGraph::removeNode(NodeId nodeId)
{
    Q_ASSERT(containsNodeId(nodeId));

    beginTransaction();

    // Remove all edges that touch this node
    for(auto edgeId : inEdgeIdsForNodeId(nodeId).copy())
        removeEdge(edgeId);

    // ...do in and out separately in case an edge is in both
    for(auto edgeId : outEdgeIdsForNodeId(nodeId).copy())
        removeEdge(edgeId);

    _n._mergedNodeIds.remove({}, nodeId);

    releaseNodeId(nodeId);
    _unusedNodeIds.push_back(nodeId);

    emit nodeRemoved(this, nodeId);
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
    Q_ASSERT(containsEdgeId(edgeId));
    return _e._edges[static_cast<int>(edgeId)];
}

bool MutableGraph::containsEdgeId(EdgeId edgeId) const
{
    return static_cast<int>(edgeId) < static_cast<int>(_e._edgeIdsInUse.size()) &&
        _e._edgeIdsInUse[static_cast<int>(edgeId)];
}

MultiElementType MutableGraph::typeOf(EdgeId edgeId) const
{
    return _e._mergedEdgeIds.typeOf(edgeId);
}

ConstEdgeIdDistinctSet MutableGraph::mergedEdgeIdsForEdgeId(EdgeId edgeId) const
{
    return {edgeId, &_e._mergedEdgeIds};
}

int MutableGraph::multiplicityOf(EdgeId edgeId) const
{
    return _e._multiplicities[static_cast<int>(edgeId)];
}

EdgeIdDistinctSets MutableGraph::edgeIdsForNodeId(NodeId nodeId) const
{
    EdgeIdDistinctSets set;
    const auto& node = nodeBy(nodeId);

    set.add(node._inEdgeIds);
    set.add(node._outEdgeIds);

    return set;
}

EdgeIdDistinctSet MutableGraph::inEdgeIdsForNodeId(NodeId nodeId) const
{
    return nodeBy(nodeId)._inEdgeIds;
}

EdgeIdDistinctSet MutableGraph::outEdgeIdsForNodeId(NodeId nodeId) const
{
    return nodeBy(nodeId)._outEdgeIds;
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

    auto unusedEdgeId = nextEdgeId();

    Graph::reserveEdgeId(edgeId);
    _e.resize(static_cast<int>(nextEdgeId()));

    while(unusedEdgeId < edgeId)
        _unusedEdgeIds.push_back(unusedEdgeId++);
}

NodeId MutableGraph::mergeNodes(NodeId nodeIdA, NodeId nodeIdB)
{
    return _n._mergedNodeIds.add(nodeIdA, nodeIdB);
}

EdgeId MutableGraph::mergeEdges(EdgeId edgeIdA, EdgeId edgeIdB)
{
    return _e._mergedEdgeIds.add(edgeIdA, edgeIdB);
}

NodeId MutableGraph::mergeNodes(const std::vector<NodeId>& nodeIds)
{
    auto setId = *std::min_element(nodeIds.begin(), nodeIds.end());

    for(auto nodeId : nodeIds)
        _n._mergedNodeIds.add(setId, nodeId);

    return setId;
}

EdgeId MutableGraph::mergeEdges(const std::vector<EdgeId>& edgeIds)
{
    auto setId = *std::min_element(edgeIds.begin(), edgeIds.end());

    for(auto edgeId : edgeIds)
        _e._mergedEdgeIds.add(setId, edgeId);

    return setId;
}

EdgeId MutableGraph::addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId)
{
    Q_ASSERT(!edgeId.isNull());
    Q_ASSERT(_n._nodeIdsInUse[static_cast<int>(sourceId)]);
    Q_ASSERT(_n._nodeIdsInUse[static_cast<int>(targetId)]);

    beginTransaction();

    // The requested ID is not available or is out of range, so resize and append
    if(edgeId >= nextEdgeId() || containsEdgeId(edgeId))
    {
        edgeId = nextEdgeId();
        reserveEdgeId(edgeId);
    }

    claimEdgeId(edgeId);
    auto& edge = edgeBy(edgeId);
    edge._id = edgeId;
    edge._sourceId = sourceId;
    edge._targetId = targetId;

    nodeBy(sourceId)._outEdgeIds.add(edgeId);
    nodeBy(targetId)._inEdgeIds.add(edgeId);

    auto undirectedEdge = UndirectedEdge(sourceId, targetId);
    if(!u::contains(_e._connections, undirectedEdge))
        _e._connections.emplace(undirectedEdge, EdgeIdDistinctSet(&_e._mergedEdgeIds));

    _e._connections[undirectedEdge].add(edgeId);

    emit edgeAdded(this, edgeId);
    _updateRequired = true;
    endTransaction();

    return edgeId;
}

EdgeId MutableGraph::addEdge(const IEdge& edge)
{
    return addEdge(edge.id(), edge.sourceId(), edge.targetId());
}

void MutableGraph::removeEdge(EdgeId edgeId)
{
    Q_ASSERT(containsEdgeId(edgeId));

    beginTransaction();

    // Remove all node references to this edge
    const auto& edge = edgeBy(edgeId);

    nodeBy(edge.sourceId())._outEdgeIds.remove(edgeId);
    nodeBy(edge.targetId())._inEdgeIds.remove(edgeId);

    auto undirectedEdge = UndirectedEdge(edge.sourceId(), edge.targetId());
    auto& connection = _e._connections[undirectedEdge];
    Q_ASSERT(!connection.empty());
    connection.remove(edgeId);

    if(connection.empty())
        _e._connections.erase(undirectedEdge);

    releaseEdgeId(edgeId);
    _unusedEdgeIds.push_back(edgeId);

    emit edgeRemoved(this, edgeId);
    _updateRequired = true;
    endTransaction();
}

// Move the edges to connect to nodeId
template<typename C> static void moveEdgesTo(MutableGraph& graph, NodeId nodeId,
                                             const C& inEdgeIds,
                                             const C& outEdgeIds)
{
    // Don't bother emitting signals for edges that are moving
    bool wasBlocked = graph.blockSignals(true);

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

    graph.blockSignals(wasBlocked);
}

void MutableGraph::contractEdge(EdgeId edgeId)
{
    // Can't contract an edge that doesn't exist
    if(!containsEdgeId(edgeId))
        return;

    beginTransaction();

    const auto& edge = edgeById(edgeId);
    auto [nodeId, nodeIdToMerge] = std::minmax(edge.sourceId(), edge.targetId());

    removeEdge(edgeId);
    moveEdgesTo(*this, nodeId,
        inEdgeIdsForNodeId(nodeIdToMerge).copy(),
        outEdgeIdsForNodeId(nodeIdToMerge).copy());
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
    [&edgeIds](EdgeId edgeId)
    {
        return !u::contains(edgeIds, edgeId);
    });

    removeEdges(edgeIds);

    for(auto componentId : componentManager.componentIds())
    {
        const auto* component = componentManager.componentById(componentId);

        // Nothing to contract
        if(component->numEdges() == 0)
            continue;

        const auto& nodeIds = component->nodeIds();
        auto nodeId = *std::min_element(nodeIds.begin(), nodeIds.end());

        moveEdgesTo(*this, nodeId,
            inEdgeIdsForNodeIds(nodeIds).copy(),
            outEdgeIdsForNodeIds(nodeIds).copy());

        mergeNodes(nodeIds);
    }

    _updateRequired = true;
    endTransaction();
}

MutableGraph& MutableGraph::clone(const MutableGraph& other)
{
    beginTransaction();

    // Store the differences between the graphs
    auto diff = diffTo(other);

    _n             = other._n;
    _nodeIds       = other._nodeIds;
    _unusedNodeIds = other._unusedNodeIds;
    Graph::reserveNodeId(other.largestNodeId());
    _n.resize(static_cast<int>(nextNodeId()));

    _e             = other._e;
    _edgeIds       = other._edgeIds;
    _unusedEdgeIds = other._unusedEdgeIds;
    Graph::reserveEdgeId(other.largestEdgeId());
    _e.resize(static_cast<int>(nextEdgeId()));

    // Reset collection pointers to collections in this
    for(auto& node : _n._nodes)
    {
        node._inEdgeIds.setCollection(&_e._inEdgeIdsCollection);
        node._outEdgeIds.setCollection(&_e._outEdgeIdsCollection);
    }

    for(auto& connection : _e._connections)
        connection.second.setCollection(&_e._mergedEdgeIds);

    // Signal all the changes based on the diff before we cloned
    for(NodeId nodeId : diff._nodesAdded)
        emit nodeAdded(this, nodeId);

    for(EdgeId edgeId : diff._edgesAdded)
        emit edgeAdded(this, edgeId);

    for(EdgeId edgeId : diff._edgesRemoved)
        emit edgeRemoved(this, edgeId);

    for(NodeId nodeId : diff._nodesRemoved)
        emit nodeRemoved(this, nodeId);

    _updateRequired = true;
    endTransaction(!diff.empty());

    return *this;
}

MutableGraph& MutableGraph::operator=(const MutableGraph& other)
{
    if(this == &other)
        return *this;

    clone(other);
    return *this;
}

MutableGraph::Diff MutableGraph::diffTo(const MutableGraph& other)
{
    MutableGraph::Diff diff;

    auto maxNodeId = std::max(nextNodeId(), other.nextNodeId());
    for(NodeId nodeId(0); nodeId < maxNodeId; ++nodeId)
    {
        if(nodeId < nextNodeId() && nodeId < other.nextNodeId())
        {
            if(containsNodeId(nodeId) && !other.containsNodeId(nodeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
                diff._nodesRemoved.push_back(nodeId);
            else if(!containsNodeId(nodeId) && other.containsNodeId(nodeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
                diff._nodesAdded.push_back(nodeId);
        }
        else if(nodeId < nextNodeId() && containsNodeId(nodeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
            diff._nodesRemoved.push_back(nodeId);
        else if(nodeId < other.nextNodeId() && other.containsNodeId(nodeId))
            diff._nodesAdded.push_back(nodeId);
    }

    auto maxEdgeId = std::max(nextEdgeId(), other.nextEdgeId());
    for(EdgeId edgeId(0); edgeId < maxEdgeId; ++edgeId)
    {
        if(edgeId < nextEdgeId() && edgeId < other.nextEdgeId())
        {
            if(containsEdgeId(edgeId) && !other.containsEdgeId(edgeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
                diff._edgesRemoved.push_back(edgeId);
            else if(!containsEdgeId(edgeId) && other.containsEdgeId(edgeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
                diff._edgesAdded.push_back(edgeId);
        }
        else if(edgeId < nextEdgeId() && containsEdgeId(edgeId)) // NOLINT clang-analyzer-optin.cplusplus.VirtualCall
            diff._edgesRemoved.push_back(edgeId);
        else if(edgeId < other.nextEdgeId() && other.containsEdgeId(edgeId))
            diff._edgesAdded.push_back(edgeId);
    }

    return diff;
}

void MutableGraph::beginTransaction()
{
    if(_graphChangeDepth++ <= 0)
    {
        emit transactionWillBegin(this);
        _mutex.lock();
        _graphChangeOccurred = false;
        emit graphWillChange(this);
    }
}

void MutableGraph::endTransaction(bool graphChangeOccurred)
{
    _graphChangeOccurred = _graphChangeOccurred || graphChangeOccurred;

    Q_ASSERT(_graphChangeDepth > 0);
    if(--_graphChangeDepth <= 0)
    {
        update();
        emit graphChanged(this, _graphChangeOccurred);
        _mutex.unlock();
        clearPhase();
        emit transactionEnded(this);
    }
}

bool MutableGraph::update()
{
    if(!_updateRequired)
        return false;

    _updateRequired = false;

    _nodeIds.clear();
    _unusedNodeIds.clear();
    std::fill(_n._multiplicities.begin(), _n._multiplicities.end(), 0);
    for(NodeId nodeId(0); nodeId < nextNodeId(); ++nodeId)
    {
        if(containsNodeId(nodeId))
        {
            _nodeIds.emplace_back(nodeId);

            if(typeOf(nodeId) == MultiElementType::Head)
            {
                const auto& mergedNodeIds = mergedNodeIdsForNodeId(nodeId);
                auto multiplicity = mergedNodeIds.size();
                for(auto mergedNodeId : mergedNodeIds)
                    _n._multiplicities[static_cast<int>(mergedNodeId)] = multiplicity;
            }
            else if(typeOf(nodeId) == MultiElementType::Not)
                _n._multiplicities[static_cast<int>(nodeId)] = 1;
        }
        else
            _unusedNodeIds.emplace_back(nodeId);
    }

    _edgeIds.clear();
    _unusedEdgeIds.clear();
    std::fill(_e._multiplicities.begin(), _e._multiplicities.end(), 0);
    for(EdgeId edgeId(0); edgeId < nextEdgeId(); ++edgeId)
    {
        if(containsEdgeId(edgeId))
        {
            _edgeIds.emplace_back(edgeId);

            if(typeOf(edgeId) == MultiElementType::Head)
            {
                const auto& mergedEdgeIds = mergedEdgeIdsForEdgeId(edgeId);
                auto multiplicity = mergedEdgeIds.size();
                for(auto mergedEdgeId : mergedEdgeIds)
                    _e._multiplicities[static_cast<int>(mergedEdgeId)] = multiplicity;
            }
            else if(typeOf(edgeId) == MultiElementType::Not)
                _e._multiplicities[static_cast<int>(edgeId)] = 1;
        }
        else
            _unusedEdgeIds.emplace_back(edgeId);
    }

    return true;
}

std::unique_lock<std::mutex> MutableGraph::tryLock()
{
    return {_mutex, std::try_to_lock};
}
