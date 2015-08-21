#include "componentmanager.h"

#include <queue>
#include <map>

ComponentIdSet ComponentManager::assignConnectedElementsComponentId(const Graph* graph,
        NodeId rootId, ComponentId componentId,
        NodeArray<ComponentId>& nodesComponentId,
        EdgeArray<ComponentId>& edgesComponentId)
{
    std::queue<NodeId> nodeIdSearchList;
    ComponentIdSet oldComponentIdsAffected;

    nodeIdSearchList.push(rootId);

    while(!nodeIdSearchList.empty())
    {
        auto nodeId = nodeIdSearchList.front();
        nodeIdSearchList.pop();
        oldComponentIdsAffected.insert(_nodesComponentId[nodeId]);
        nodesComponentId[nodeId] = componentId;

        auto edgeIds = graph->nodeById(nodeId).edgeIds();

        for(auto edgeId : edgeIds)
        {
            edgesComponentId[edgeId] = componentId;
            auto oppositeNodeId = graph->edgeById(edgeId).oppositeId(nodeId);

            if(nodesComponentId[oppositeNodeId] != componentId)
            {
                nodeIdSearchList.push(oppositeNodeId);
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

    NodeArray<ComponentId> newNodesComponentId(*const_cast<Graph*>(graph));
    EdgeArray<ComponentId> newEdgesComponentId(*const_cast<Graph*>(graph));
    ComponentIdSet newComponentIdsList;

    const std::vector<NodeId>& nodeIdsList = graph->nodeIds();

    // Search for mergers and splitters
    for(auto nodeId : nodeIdsList)
    {
        auto oldComponentId = _nodesComponentId[nodeId];

        if(newNodesComponentId[nodeId].isNull() && !oldComponentId.isNull())
        {
            if(newComponentIdsList.find(oldComponentId) != newComponentIdsList.end())
            {
                // We have already used this ID so this is a component that has split
                auto newComponentId = generateComponentId();
                newComponentIdsList.insert(newComponentId);
                assignConnectedElementsComponentId(graph, nodeId, newComponentId,
                                                   newNodesComponentId, newEdgesComponentId);

                queueGraphComponentUpdate(graph, oldComponentId);
                queueGraphComponentUpdate(graph, newComponentId);

                splitComponents[oldComponentId].insert(oldComponentId);
                splitComponents[oldComponentId].insert(newComponentId);
            }
            else
            {
                newComponentIdsList.insert(oldComponentId);
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
    for(auto nodeId : nodeIdsList)
    {
        if(newNodesComponentId[nodeId].isNull() && _nodesComponentId[nodeId].isNull())
        {
            auto newComponentId = generateComponentId();
            newComponentIdsList.insert(newComponentId);
            assignConnectedElementsComponentId(graph, nodeId, newComponentId, newNodesComponentId, newEdgesComponentId);
            queueGraphComponentUpdate(graph, newComponentId);

            newComponentIds.insert(newComponentId);
        }
    }

    // Search for removed components
    ComponentIdSet componentIdsToBeRemoved;
    for(auto componentId : _componentIdsList)
    {
        if(newComponentIdsList.find(componentId) != newComponentIdsList.end())
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

    _componentIdsList.push_back(newComponentId);

    for(auto* componentArray : _componentArrayList)
        componentArray->resize(componentArrayCapacity());

    return newComponentId;
}

void ComponentManager::releaseComponentId(ComponentId componentId)
{
    _componentIdsList.erase(std::remove(_componentIdsList.begin(), _componentIdsList.end(), componentId), _componentIdsList.end());
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

    std::vector<NodeId>& nodeIdsList = graphComponentNodeIdsList(*graphComponent);
    std::vector<EdgeId>& edgeIdsList = graphComponentEdgeIdsList(*graphComponent);

    nodeIdsList.clear();
    auto& nodeIds = graph->nodeIds();
    for(auto nodeId : nodeIds)
    {
        if(_nodesComponentId[nodeId] == componentId)
            nodeIdsList.push_back(nodeId);
    }

    edgeIdsList.clear();
    auto& edgeIds = graph->edgeIds();
    for(auto edgeId : edgeIds)
    {
        if(_edgesComponentId[edgeId] == componentId)
            edgeIdsList.push_back(edgeId);
    }
}

void ComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(_componentsMap.find(componentId) != _componentsMap.end())
    {
        _componentsMap.erase(componentId);
        _componentIdsList.erase(std::remove(_componentIdsList.begin(), _componentIdsList.end(), componentId), _componentIdsList.end());
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

    return _componentIdsList;
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
    auto i = std::find(_componentIdsList.begin(), _componentIdsList.end(), componentId);
    return i != _componentIdsList.end() ? *i : ComponentId();
}

ComponentId ComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.isNull())
        return ComponentId();

    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex, _debugPaused);

    auto componentId = _edgesComponentId.at(edgeId);
    auto i = std::find(_componentIdsList.begin(), _componentIdsList.end(), componentId);
    return i != _componentIdsList.end() ? *i : ComponentId();
}

