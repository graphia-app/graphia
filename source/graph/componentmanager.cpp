#include "componentmanager.h"

#include <queue>
#include <map>

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
        oldComponentIdsAffected.insert(_nodesComponentId[nodeId]);
        nodesComponentId[nodeId] = componentId;

        auto edgeIds = graph->nodeById(nodeId).edgeIds();

        for(auto edgeId : edgeIds)
        {
            edgesComponentId[edgeId] = componentId;
            auto oppositeNodeId = graph->edgeById(edgeId).oppositeId(nodeId);

            if(nodesComponentId[oppositeNodeId] != componentId)
            {
                nodeIds.push(oppositeNodeId);
                nodesComponentId[oppositeNodeId] = componentId;
            }
        }
    }

    // We don't count nodes that haven't yet been assigned a component
    oldComponentIdsAffected.erase(ComponentId());

    return oldComponentIdsAffected;
}

void ComponentManager::updateComponents(const Graph* graph)
{
    std::map<ComponentId, ComponentIdSet> splitComponents;
    ComponentIdSet newComponentIds;

    NodeArray<ComponentId> newNodesComponentId(*graph);
    EdgeArray<ComponentId> newEdgesComponentId(*graph);

    // Search for mergers and splitters
    for(auto nodeId : graph->nodeIds())
    {
        auto oldComponentId = _nodesComponentId[nodeId];

        if(newNodesComponentId[nodeId].isNull() && !oldComponentId.isNull())
        {
            if(newComponentIds.find(oldComponentId) != newComponentIds.end())
            {
                // We have already used this ID so this is a component that has split
                auto newComponentId = generateComponentId();
                newComponentIds.insert(newComponentId);
                assignConnectedElementsComponentId(graph, nodeId, newComponentId,
                                                   newNodesComponentId, newEdgesComponentId);

                queueGraphComponentUpdate(graph, oldComponentId);
                queueGraphComponentUpdate(graph, newComponentId);

                splitComponents[oldComponentId].insert(oldComponentId);
                splitComponents[oldComponentId].insert(newComponentId);
            }
            else
            {
                newComponentIds.insert(oldComponentId);
                auto componentIdsAffected = assignConnectedElementsComponentId(graph, nodeId, oldComponentId,
                                                                               newNodesComponentId, newEdgesComponentId);
                queueGraphComponentUpdate(graph, oldComponentId);

                if(componentIdsAffected.size() > 1)
                {
                    // More than one old component IDs were observed so components have merged
                    if(_debug) qDebug() << "componentsWillMerge" << componentIdsAffected << "->" << oldComponentId;
                    emit componentsWillMerge(graph, ComponentMergeSet(graph, std::move(componentIdsAffected), oldComponentId));
                    componentIdsAffected.erase(oldComponentId);

                    for(auto removedComponentId : componentIdsAffected)
                    {
                        if(_debug) qDebug() << "componentWillBeRemoved" << removedComponentId;
                        emit componentWillBeRemoved(graph, removedComponentId, true);
                        removeGraphComponent(removedComponentId);
                    }
                }
            }
        }
    }

    // Search for entirely new components
    for(auto nodeId : graph->nodeIds())
    {
        if(newNodesComponentId[nodeId].isNull() && _nodesComponentId[nodeId].isNull())
        {
            auto newComponentId = generateComponentId();
            newComponentIds.insert(newComponentId);
            assignConnectedElementsComponentId(graph, nodeId, newComponentId, newNodesComponentId, newEdgesComponentId);
            queueGraphComponentUpdate(graph, newComponentId);

            newComponentIds.insert(newComponentId);
        }
    }

    // Search for removed components
    ComponentIdSet componentIdsToBeRemoved;
    for(auto componentId : _componentIds)
    {
        if(newComponentIds.find(componentId) != newComponentIds.end())
            continue;

        componentIdsToBeRemoved.insert(componentId);
    }

    for(auto componentId : componentIdsToBeRemoved)
    {
        // Component removed
        if(_debug) qDebug() << "componentWillBeRemoved" << componentId;
        emit componentWillBeRemoved(graph, componentId, false);

        removeGraphComponent(componentId);
    }

    _nodesComponentId = std::move(newNodesComponentId);
    _edgesComponentId = std::move(newEdgesComponentId);

    for(auto componentId : _updatesRequired)
        updateGraphComponent(graph, componentId);

    _updatesRequired.clear();

    // Notify all the new components
    for(auto newComponentId : newComponentIds)
    {
        if(_debug) qDebug() << "componentAdded" << newComponentId;
        emit componentAdded(graph, newComponentId, false);
    }

    // Notify all the splits
    for(auto splitee : splitComponents)
    {
        auto& splitters = splitee.second;

        for(auto splitter : splitters)
        {
            if(splitter != splitee.first)
            {
                if(_debug) qDebug() << "componentAdded" << splitter;
                emit componentAdded(graph, splitter, true);
            }
        }

        if(_debug) qDebug() << "componentSplit" << splitee.first << "->" << splitters;
        emit componentSplit(graph, ComponentSplitSet(graph, splitee.first, std::move(splitters)));
    }
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

    _componentIds.push_back(newComponentId);

    for(auto* componentArray : _componentArrays)
        componentArray->resize(componentArrayCapacity());

    return newComponentId;
}

void ComponentManager::releaseComponentId(ComponentId componentId)
{
    _componentIds.erase(std::remove(_componentIds.begin(), _componentIds.end(), componentId), _componentIds.end());
    _vacatedComponentIdQueue.push(componentId);
}

void ComponentManager::queueGraphComponentUpdate(const Graph* graph, ComponentId componentId)
{
    _updatesRequired.insert(componentId);

    if(_componentsMap.find(componentId) == _componentsMap.end())
    {
        std::shared_ptr<GraphComponent> graphComponent = std::make_shared<GraphComponent>(graph);
        _componentsMap.emplace(componentId, graphComponent);
    }
}

void ComponentManager::updateGraphComponent(const Graph* graph, ComponentId componentId)
{
    std::shared_ptr<GraphComponent> graphComponent = _componentsMap[componentId];

    std::vector<NodeId>& nodeIds = graphComponentNodeIds(*graphComponent);
    std::vector<EdgeId>& edgeIds = graphComponentEdgeIds(*graphComponent);

    nodeIds.clear();
    for(auto nodeId : graph->nodeIds())
    {
        if(_nodesComponentId[nodeId] == componentId)
            nodeIds.push_back(nodeId);
    }

    edgeIds.clear();
    for(auto edgeId : graph->edgeIds())
    {
        if(_edgesComponentId[edgeId] == componentId)
            edgeIds.push_back(edgeId);
    }
}

void ComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(_componentsMap.find(componentId) != _componentsMap.end())
    {
        _componentsMap.erase(componentId);
        _componentIds.erase(std::remove(_componentIds.begin(), _componentIds.end(), componentId), _componentIds.end());
        releaseComponentId(componentId);
        _updatesRequired.erase(componentId);
    }
}

void ComponentManager::onGraphChanged(const Graph* graph)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);
    graph->debugPauser.pause("Call updateComponents from ComponentManager::onGraphChanged", &_debugPaused);
    updateComponents(graph);
    graph->debugPauser.pause("Signal Graph::onGraphChanged", &_debugPaused);
}

template<typename T> class unique_lock_with_warning
{
public:
    unique_lock_with_warning(T& mutex, bool alreadyBlocked) :
        _lock(mutex, std::try_to_lock)
    {
        if(!_lock.owns_lock())
        {
            qWarning() << "Needed to acquire lock when reading from ComponentManager; "
                          "this implies inappropriate concurrent access which is bad "
                          "because it means this thread is blocked until the update completes";

            // If the DebugPauser is in action, it is already effectively locking things
            // for us, so trying to lock again here causes deadlock
            if(!alreadyBlocked)
                _lock.lock();
        }
    }

    ~unique_lock_with_warning()
    {
        if(_lock.owns_lock())
            _lock.unlock();
    }

private:
    std::unique_lock<T> _lock;
};

const std::vector<ComponentId>& ComponentManager::componentIds() const
{
    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex, _debugPaused);

    return _componentIds;
}

const GraphComponent* ComponentManager::componentById(ComponentId componentId) const
{
    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex, _debugPaused);

    if(_componentsMap.find(componentId) != _componentsMap.end())
        return _componentsMap.at(componentId).get();

    Q_ASSERT(!"ComponentManager::componentById returning nullptr");
    return nullptr;
}

ComponentId ComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId.isNull())
        return ComponentId();

    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex, _debugPaused);

    auto componentId = _nodesComponentId.at(nodeId);
    auto i = std::find(_componentIds.begin(), _componentIds.end(), componentId);
    return i != _componentIds.end() ? *i : ComponentId();
}

ComponentId ComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.isNull())
        return ComponentId();

    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex, _debugPaused);

    auto componentId = _edgesComponentId.at(edgeId);
    auto i = std::find(_componentIds.begin(), _componentIds.end(), componentId);
    return i != _componentIds.end() ? *i : ComponentId();
}

