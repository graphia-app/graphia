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

#include "componentmanager.h"

#include "shared/utils/thread.h"
#include "shared/utils/container.h"
#include "shared/graph/elementid_debug.h"

#include "graph.h"
#include "graphcomponent.h"

#include <map>
#include <queue>

ComponentManager::ComponentManager(Graph& graph,
                                   const NodeConditionFn& nodeFilter,
                                   const EdgeConditionFn& edgeFilter) :
    _nextComponentId(0),
    _nodesComponentId(graph),
    _edgesComponentId(graph),
    _debug(qEnvironmentVariableIntValue("COMPONENTS_DEBUG"))
{
    // Ignore all multi-elements
    addNodeFilter([&graph](NodeId nodeId) { return graph.typeOf(nodeId) == MultiElementType::Tail; });
    addEdgeFilter([&graph](EdgeId edgeId) { return graph.typeOf(edgeId) == MultiElementType::Tail; });

    if(nodeFilter)
        addNodeFilter(nodeFilter);

    if(edgeFilter)
        addEdgeFilter(edgeFilter);

    connect(&graph, &Graph::graphChanged, this, &ComponentManager::onGraphChanged, Qt::DirectConnection);

    graph.update();
    update(&graph);
}

ComponentManager::~ComponentManager() // NOLINT modernize-use-equals-default
{
    // Let the ComponentArrays know that we're going away
    for(auto* componentArray : _componentArrays)
        componentArray->invalidate();
}

ComponentIdSet ComponentManager::assignConnectedElementsComponentId(const Graph* graph,
        NodeId rootId, ComponentId componentId,
        NodeArray<ComponentId>& nodesComponentId,
        EdgeArray<ComponentId>& edgesComponentId)
{
    std::queue<NodeId> nodeIds;
    ComponentIdSet oldComponentIdsAffected;

    nodeIds.push(rootId);

    while(!nodeIds.empty())
    {
        auto nodeId = nodeIds.front();
        nodeIds.pop();
        oldComponentIdsAffected.insert(_nodesComponentId.at(nodeId));
        for(auto mergedNodeId : graph->mergedNodeIdsForNodeId(nodeId))
            nodesComponentId[mergedNodeId] = componentId;

        for(auto edgeId : graph->edgeIdsForNodeId(nodeId))
        {
            if(edgeIdFiltered(edgeId))
                continue;

            for(auto mergedEdgeId : graph->mergedEdgeIdsForEdgeId(edgeId))
                edgesComponentId[mergedEdgeId] = componentId;

            auto oppositeNodeId = graph->edgeById(edgeId).oppositeId(nodeId);

            if(nodesComponentId[oppositeNodeId] != componentId)
            {
                nodeIds.push(oppositeNodeId);
                for(auto mergedNodeId : graph->mergedNodeIdsForNodeId(oppositeNodeId))
                    nodesComponentId[mergedNodeId] = componentId;
            }
        }
    }

    // We don't count nodes that haven't yet been assigned a component
    oldComponentIdsAffected.erase(ComponentId());

    return oldComponentIdsAffected;
}

void ComponentManager::insertComponentArray(IGraphArray* componentArray)
{
    const std::unique_lock<std::mutex> lock(_componentArraysMutex);
    _componentArrays.insert(componentArray);
}

void ComponentManager::eraseComponentArray(IGraphArray* componentArray)
{
    const std::unique_lock<std::mutex> lock(_componentArraysMutex);
    _componentArrays.erase(componentArray);
}

void ComponentManager::update(const Graph* graph)
{
    if(_debug > 0) qDebug() << "ComponentManager::update begins" << this;

    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    ComponentIdMap<ComponentIdSet> splitComponents;
    ComponentIdSet splitComponentIds;
    ComponentIdMap<ComponentIdSet> mergedComponents;
    ComponentIdSet mergedComponentIds;
    ComponentIdSet componentIds;

    NodeArray<ComponentId> newNodesComponentId(*graph);
    EdgeArray<ComponentId> newEdgesComponentId(*graph);

    // Search for mergers and splitters
    for(auto nodeId : graph->nodeIds())
    {
        if(nodeIdFiltered(nodeId))
            continue;

        auto oldComponentId = _nodesComponentId[nodeId];

        if(newNodesComponentId[nodeId].isNull() && !oldComponentId.isNull())
        {
            if(componentIds.contains(oldComponentId))
            {
                // We have already used this ID so this is a component that has split
                auto newComponentId = generateComponentId();
                componentIds.insert(newComponentId);
                assignConnectedElementsComponentId(graph, nodeId, newComponentId,
                                                   newNodesComponentId, newEdgesComponentId);

                queueGraphComponentUpdate(graph, oldComponentId);
                queueGraphComponentUpdate(graph, newComponentId);

                splitComponents[oldComponentId].insert(oldComponentId);
                splitComponents[oldComponentId].insert(newComponentId);
                splitComponentIds.insert(newComponentId);
            }
            else
            {
                componentIds.insert(oldComponentId);
                auto componentIdsAffected = assignConnectedElementsComponentId(graph, nodeId, oldComponentId,
                                                                               newNodesComponentId, newEdgesComponentId);
                queueGraphComponentUpdate(graph, oldComponentId);

                if(componentIdsAffected.size() > 1)
                {
                    // More than one old component IDs were observed so components have merged
                    mergedComponents[oldComponentId].insert(componentIdsAffected.begin(), componentIdsAffected.end());
                    componentIdsAffected.erase(oldComponentId);
                    mergedComponentIds.insert(componentIdsAffected.begin(), componentIdsAffected.end());
                }
            }
        }
    }

    // Search for entirely new components
    for(auto nodeId : graph->nodeIds())
    {
        if(nodeIdFiltered(nodeId))
            continue;

        if(newNodesComponentId[nodeId].isNull() && _nodesComponentId[nodeId].isNull())
        {
            auto newComponentId = generateComponentId();
            componentIds.insert(newComponentId);
            assignConnectedElementsComponentId(graph, nodeId, newComponentId, newNodesComponentId, newEdgesComponentId);
            queueGraphComponentUpdate(graph, newComponentId);
        }
    }

    // Resize the component arrays
    for(auto* componentArray : _componentArrays)
        componentArray->resize(componentArrayCapacity());

    // Search for added or removed components
    ComponentIdSet componentIdsToBeAdded;
    ComponentIdSet componentIdsToBeRemoved;

    std::set_difference(componentIds.begin(), componentIds.end(),
        _componentIdsSet.begin(), _componentIdsSet.end(),
        std::inserter(componentIdsToBeAdded, componentIdsToBeAdded.begin()));
    std::set_difference(_componentIdsSet.begin(), _componentIdsSet.end(),
        componentIds.begin(), componentIds.end(),
        std::inserter(componentIdsToBeRemoved, componentIdsToBeRemoved.begin()));

    // Find nodes and edges that have been added or removed
    NodeIdMap<ComponentId> nodeIdAdds;
    EdgeIdMap<ComponentId> edgeIdAdds;
    NodeIdMap<ComponentId> nodeIdRemoves;
    EdgeIdMap<ComponentId> edgeIdRemoves;
    NodeIdMap<std::pair<ComponentId, ComponentId>> nodeIdMoves;
    EdgeIdMap<std::pair<ComponentId, ComponentId>> edgeIdMoves;

    auto maxNumNodes = static_cast<int>(std::max(_nodesComponentId.size(), newNodesComponentId.size()));
    for(NodeId nodeId(0); nodeId < maxNumNodes; ++nodeId)
    {
        auto oldComponentId = _nodesComponentId[nodeId];
        auto newComponentId = newNodesComponentId[nodeId];

        if(oldComponentId == newComponentId)
            continue;

        if(oldComponentId.isNull() && !newComponentId.isNull())
            nodeIdAdds[nodeId] = newComponentId;
        else if(!oldComponentId.isNull() && newComponentId.isNull())
            nodeIdRemoves[nodeId] = oldComponentId;
        else if(!oldComponentId.isNull() && !newComponentId.isNull())
        {
            if(!componentIdsToBeRemoved.contains(oldComponentId) && !componentIdsToBeAdded.contains(newComponentId))
                nodeIdMoves[nodeId] = {oldComponentId, newComponentId};
            else if(componentIdsToBeRemoved.contains(oldComponentId) && componentIdsToBeAdded.contains(newComponentId))
                nodeIdAdds[nodeId] = newComponentId;
        }
    }

    auto maxNumEdges = static_cast<int>(std::max(_edgesComponentId.size(), newEdgesComponentId.size()));
    for(EdgeId edgeId(0); edgeId < maxNumEdges; ++edgeId)
    {
        auto oldComponentId = _edgesComponentId[edgeId];
        auto newComponentId = newEdgesComponentId[edgeId];

        if(oldComponentId == newComponentId)
            continue;

        if(oldComponentId.isNull() && !newComponentId.isNull())
            edgeIdAdds[edgeId] = newComponentId;
        else if(!oldComponentId.isNull() && newComponentId.isNull())
            edgeIdRemoves[edgeId] = oldComponentId;
        else if(!oldComponentId.isNull() && !newComponentId.isNull())
        {
            if(!componentIdsToBeRemoved.contains(oldComponentId) && !componentIdsToBeAdded.contains(newComponentId))
                edgeIdMoves[edgeId] = {oldComponentId, newComponentId};
            else if(componentIdsToBeRemoved.contains(oldComponentId) && componentIdsToBeAdded.contains(newComponentId))
                edgeIdAdds[edgeId] = newComponentId;
        }
    }

    // In the case where nodes move from one component to another, a merge
    // is potentially falsely detected, so check that the merges have
    // corresponding removes, erasing them if they don't
    for(auto& [merger, mergees] : mergedComponents)
    {
        auto mergerCapture = merger; //FIXME work around for patchy clang C++20 support
        std::erase_if(mergees, [&](const auto& componentId)
        {
            return !componentIdsToBeRemoved.contains(componentId) && componentId != mergerCapture;
        });
    }

    std::erase_if(mergedComponents, [](const auto& m) { return m.second.size() <= 1; });
    std::erase_if(mergedComponentIds, [&](const auto& componentId)
    {
        return !componentIdsToBeRemoved.contains(componentId);
    });

    // Notify all the merges
    for(auto& mergee : mergedComponents)
    {
        if(_debug > 0) qDebug() << "componentsWillMerge" << mergee.second << "->" << mergee.first;
        emit componentsWillMerge(graph, ComponentMergeSet(std::move(mergee.second), mergee.first));
    }

    // Removed components
    for(auto componentId : componentIdsToBeRemoved)
    {
        Q_ASSERT(!componentId.isNull());
        if(_debug > 0) qDebug() << "componentWillBeRemoved" << componentId;
        const bool hasMerged = mergedComponentIds.contains(componentId);
        emit componentWillBeRemoved(graph, componentId, hasMerged);

        if(!hasMerged)
        {
            std::erase_if(nodeIdRemoves, [&](const auto& nodeIdRemove) { return nodeIdRemove.second == componentId; });
            std::erase_if(edgeIdRemoves, [&](const auto& edgeIdRemove) { return edgeIdRemove.second == componentId; });
        }

        _componentIdsSet.erase(componentId);
        removeGraphComponent(componentId);
    }

    _componentIds.clear();
    std::copy(_componentIdsSet.begin(), _componentIdsSet.end(),
        std::back_inserter(_componentIds));

    shrinkComponentsArrayToFit();

    _nodesComponentId = std::move(newNodesComponentId);
    _edgesComponentId = std::move(newEdgesComponentId);

    updateGraphComponents(graph);

    _updatesRequired.clear();

    std::copy(componentIdsToBeAdded.begin(), componentIdsToBeAdded.end(),
        std::back_inserter(_componentIds));
    std::copy(componentIdsToBeAdded.begin(), componentIdsToBeAdded.end(),
        std::inserter(_componentIdsSet, _componentIdsSet.begin()));

    std::stable_sort(_componentIds.begin(), _componentIds.end(),
    [this](auto a, auto b)
    {
        auto componentA = this->componentById(a);
        auto componentB = this->componentById(b);

        if(componentA->numNodes() == componentB->numNodes())
            return a < b;

        return componentA->numNodes() > componentB->numNodes();
    });

    lock.unlock();

    // Notify all the new components
    for(auto componentId : componentIdsToBeAdded)
    {
        Q_ASSERT(!componentId.isNull());
        if(_debug > 0) qDebug() << "componentAdded" << componentId;
        const bool hasSplit = splitComponentIds.contains(componentId);
        emit componentAdded(graph, componentId, hasSplit);

        if(!hasSplit)
        {
            std::erase_if(nodeIdAdds, [&](const auto& nodeIdAdd) { return nodeIdAdd.second == componentId; });
            std::erase_if(edgeIdAdds, [&](const auto& edgeIdAdd) { return edgeIdAdd.second == componentId; });
        }
    }

    // Notify all the splits
    for(auto& splitee : splitComponents)
    {
        if(_debug > 0) qDebug() << "componentSplit" << splitee.first << "->" << splitee.second;
        emit componentSplit(graph, ComponentSplitSet(splitee.first, std::move(splitee.second)));
    }

    // Notify node adds and removes
    for(const auto& [nodeId, componentId] : nodeIdAdds)
        emit nodeAddedToComponent(graph, nodeId, componentId);

    for(const auto& [edgeId, componentId] : edgeIdAdds)
        emit edgeAddedToComponent(graph, edgeId, componentId);

    for(const auto& [nodeId, componentId] : nodeIdRemoves)
        emit nodeRemovedFromComponent(graph, nodeId, componentId);

    for(const auto& [edgeId, componentId] : edgeIdRemoves)
        emit edgeRemovedFromComponent(graph, edgeId, componentId);

    for(const auto& [nodeId, moveComponentIds] : nodeIdMoves)
        emit nodeMovedBetweenComponents(graph, nodeId, moveComponentIds.first, moveComponentIds.second);

    for(const auto& [edgeId, moveComponentIds] : edgeIdMoves)
        emit edgeMovedBetweenComponents(graph, edgeId, moveComponentIds.first, moveComponentIds.second);

    if(_debug > 1)
    {
        qDebug() << "ComponentIds:" << _componentIdsSet;

        for(auto componentId : _componentIds)
            qDebug() << componentId << componentFor(componentId)->nodeIds();
    }

    if(_debug > 0) qDebug() << "ComponentManager::update ends" << this << _componentIdsSet;
}

ComponentId ComponentManager::generateComponentId()
{
    ComponentId newComponentId;

    if(!_vacatedComponentIdQueue.empty())
    {
        newComponentId = _vacatedComponentIdQueue.front();
        _vacatedComponentIdQueue.pop();
    }
    else
        newComponentId = _nextComponentId++;

    return newComponentId;
}

void ComponentManager::queueGraphComponentUpdate(const Graph* graph, ComponentId componentId)
{
    _updatesRequired.insert(componentId);

    if(componentFor(componentId) == nullptr)
    {
        auto graphComponent = std::make_unique<GraphComponent>(graph);
        setComponentFor(componentId, std::move(graphComponent));
    }
}

void ComponentManager::updateGraphComponents(const Graph* graph)
{
    for(auto componentId : _componentIds)
    {
        if(_updatesRequired.contains(componentId))
        {
            auto* graphComponent = componentFor(componentId);

            graphComponent->_nodeIds.clear();
            graphComponent->_edgeIds.clear();
        }
    }

    for(auto nodeId : graph->nodeIds())
    {
        if(nodeIdFiltered(nodeId))
            continue;

        auto componentId = _nodesComponentId[nodeId];

        if(_updatesRequired.contains(componentId))
            componentFor(componentId)->_nodeIds.push_back(nodeId);
    }

    for(auto edgeId : graph->edgeIds())
    {
        if(edgeIdFiltered(edgeId))
            continue;

        auto componentId = _edgesComponentId[edgeId];

        if(_updatesRequired.contains(componentId))
            componentFor(componentId)->_edgeIds.push_back(edgeId);
    }
}

void ComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(componentFor(componentId) != nullptr)
    {
        setComponentFor(componentId, nullptr);
        _vacatedComponentIdQueue.push(componentId);
        _updatesRequired.erase(componentId);
    }
}

GraphComponent* ComponentManager::componentFor(ComponentId componentId)
{
    Q_ASSERT(!componentId.isNull());

    auto index = static_cast<size_t>(componentId);

    if(index >= _components.size())
        return nullptr;

    return _components.at(index).get();
}

const GraphComponent* ComponentManager::componentFor(ComponentId componentId) const
{
    Q_ASSERT(!componentId.isNull());

    auto index = static_cast<size_t>(componentId);

    if(index >= _components.size())
        return nullptr;

    return _components.at(index).get();
}

void ComponentManager::setComponentFor(ComponentId componentId, std::unique_ptr<GraphComponent> graphComponent)
{
    Q_ASSERT(!componentId.isNull());

    auto index = static_cast<size_t>(componentId);
    if(graphComponent != nullptr && index >= _components.size())
    {
        const size_t newSize = index > 0 ? index * 2 : 1;
        _components.resize(newSize);
    }

    _components[index] = std::move(graphComponent);
}

void ComponentManager::shrinkComponentsArrayToFit()
{
    if(_components.empty())
        return;

    size_t newSize = _components.size();

    while(newSize > 0 && _components.at(newSize - 1) == nullptr)
        newSize--;

    _components.resize(newSize);
}

void ComponentManager::onGraphChanged(const Graph* graph, bool changeOccurred)
{
    if(_enabled && changeOccurred)
    {
        graph->setPhase(tr("Componentising"));
        update(graph);
        graph->clearPhase();
    }
}

#include <chrono>

template<typename T> class unique_lock_with_warning
{
public:
    unique_lock_with_warning() = default;
    unique_lock_with_warning(unique_lock_with_warning&&) noexcept = default;
    unique_lock_with_warning(const unique_lock_with_warning&) = delete;
    unique_lock_with_warning& operator=(unique_lock_with_warning&&) noexcept = default;
    unique_lock_with_warning& operator=(const unique_lock_with_warning&) = delete;

    explicit unique_lock_with_warning(T& mutex) :
        _lock(mutex, std::defer_lock)
    {
        const int MIN_WARNING_MILLISECONDS = 100;
        const std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

        if(!_lock.try_lock())
        {
            _lock.lock();

            const std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
            auto timeToAcquireLock = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if(timeToAcquireLock > MIN_WARNING_MILLISECONDS)
            {
                qWarning() << "WARNING: thread" << u::currentThreadName() <<
                              "was blocked for" << timeToAcquireLock << "ms";
            }
        }
    }

    ~unique_lock_with_warning() // NOLINT modernize-use-equals-default
    {
        if(_lock.owns_lock())
            _lock.unlock();
    }

private:
    std::unique_lock<T> _lock;
};

const std::vector<ComponentId>& ComponentManager::componentIds() const
{
    const unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    return _componentIds;
}

bool ComponentManager::containsComponentId(ComponentId componentId) const
{
    const unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    return _componentIdsSet.contains(componentId);
}

const GraphComponent* ComponentManager::componentById(ComponentId componentId) const
{
    const unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    const auto* component = componentFor(componentId);
    Q_ASSERT(component != nullptr);

    return component;
}

ComponentId ComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId.isNull())
        return {};

    const unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    auto componentId = _nodesComponentId.at(nodeId);
    if(_componentIdsSet.contains(componentId))
        return componentId;

    if(_debug > 0) qDebug() << "Can't find componentId of nodeId" << nodeId;
    return {};
}

ComponentId ComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.isNull())
        return {};

    const unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    auto componentId = _edgesComponentId.at(edgeId);
    if(_componentIdsSet.contains(componentId))
        return componentId;

    if(_debug > 0) qDebug() << "Can't find componentId of edgeId" << edgeId;
    return {};
}

