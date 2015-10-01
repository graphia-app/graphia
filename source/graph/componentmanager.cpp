#include "componentmanager.h"

#include "../utils/utils.h"

#include <queue>
#include <map>

void GraphComponent::reserve(const Graph& other)
{
    const GraphComponent* otherGraphComponent = dynamic_cast<const GraphComponent*>(&other);
    Q_ASSERT(otherGraphComponent != nullptr);

    Q_ASSERT(_graph == otherGraphComponent->_graph);
    _nodeIds.reserve(otherGraphComponent->_nodeIds.size());
    _edgeIds.reserve(otherGraphComponent->_edgeIds.size());
}

void GraphComponent::cloneFrom(const Graph& other)
{
    const GraphComponent* otherGraphComponent = dynamic_cast<const GraphComponent*>(&other);
    Q_ASSERT(otherGraphComponent != nullptr);

    Q_ASSERT(_graph == otherGraphComponent->_graph);
    _nodeIds = otherGraphComponent->_nodeIds;
    _edgeIds = otherGraphComponent->_edgeIds;
}

ComponentManager::ComponentManager(Graph& graph, NodeFilterFn nodeFilter, EdgeFilterFn edgeFilter) :
    _nextComponentId(0),
    _nodesComponentId(graph),
    _edgesComponentId(graph)
{
    if(qgetenv("COMPONENTS_DEBUG").toInt())
        _debug = true;

    // Ignore all multi-elements
    addNodeFilter([&graph](NodeId nodeId) { return graph.typeOf(nodeId) == NodeIdSetCollection::Type::Tail; });
    addEdgeFilter([&graph](EdgeId edgeId) { return graph.typeOf(edgeId) == EdgeIdSetCollection::Type::Tail; });

    if(nodeFilter)
        addNodeFilter(nodeFilter);

    if(edgeFilter)
        addEdgeFilter(edgeFilter);

    connect(&graph, &Graph::graphChanged, this, &ComponentManager::onGraphChanged, Qt::DirectConnection);

    graph.update();
    update(&graph);
}

ComponentManager::~ComponentManager()
{
    // Let the ComponentArrays know that we're going away
    for(auto componentArray : _componentArrays)
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
        oldComponentIdsAffected.insert(_nodesComponentId[nodeId]);
        nodesComponentId[nodeId] = componentId;

        for(auto edgeId : graph->edgeIdsForNodeId(nodeId))
        {
            if(edgeIdFiltered(edgeId))
                continue;

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

void ComponentManager::update(const Graph* graph)
{
    std::unique_lock<std::recursive_mutex> lock(_updateMutex);

    std::map<ComponentId, ComponentIdSet> splitComponents;
    ComponentIdSet splitComponentIds;
    std::map<ComponentId, ComponentIdSet> mergedComponents;
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
            if(u::contains(componentIds, oldComponentId))
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
    auto componentIdsToBeAdded = u::setDifference(componentIds, _componentIds);
    auto componentIdsToBeRemoved = u::setDifference(_componentIds, componentIds);

    // Notify all the merges
    for(auto& mergee : mergedComponents)
    {
        if(_debug) qDebug() << "componentsWillMerge" << mergee.second << "->" << mergee.first;
        emit componentsWillMerge(graph, ComponentMergeSet(std::move(mergee.second), mergee.first));
    }

    // Removed components
    for(auto componentId : componentIdsToBeRemoved)
    {
        if(_debug) qDebug() << "componentWillBeRemoved" << componentId;
        emit componentWillBeRemoved(graph, componentId, u::contains(mergedComponentIds, componentId));

        u::removeByValue(_componentIds, componentId);
        removeGraphComponent(componentId);
    }

    _nodesComponentId = std::move(newNodesComponentId);
    _edgesComponentId = std::move(newEdgesComponentId);

    updateGraphComponents(graph);

    _updatesRequired.clear();

    // Notify all the new components
    for(auto componentId : componentIdsToBeAdded)
    {
        if(_debug) qDebug() << "componentAdded" << componentId;
        _componentIds.push_back(componentId);
        emit componentAdded(graph, componentId, u::contains(splitComponentIds, componentId));
    }

    // Notify all the splits
    for(auto& splitee : splitComponents)
    {
        if(_debug) qDebug() << "componentSplit" << splitee.first << "->" << splitee.second;
        emit componentSplit(graph, ComponentSplitSet(splitee.first, std::move(splitee.second)));
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

    return newComponentId;
}

void ComponentManager::queueGraphComponentUpdate(const Graph* graph, ComponentId componentId)
{
    _updatesRequired.insert(componentId);

    if(!u::contains(_componentsMap, componentId))
    {
        std::shared_ptr<GraphComponent> graphComponent = std::make_shared<GraphComponent>(graph);
        _componentsMap.emplace(componentId, graphComponent);
    }
}

void ComponentManager::updateGraphComponents(const Graph* graph)
{
    for(auto& graphComponent : _componentsMap)
    {
        if(u::contains(_updatesRequired, graphComponent.first))
        {
            graphComponent.second->_nodeIds.clear();
            graphComponent.second->_edgeIds.clear();
        }
    }

    for(auto nodeId : graph->nodeIds())
    {
        if(nodeIdFiltered(nodeId))
            continue;

        auto componentId = _nodesComponentId[nodeId];

        if(u::contains(_updatesRequired, componentId))
            _componentsMap[componentId]->_nodeIds.push_back(nodeId);
    }

    for(auto edgeId : graph->edgeIds())
    {
        if(edgeIdFiltered(edgeId))
            continue;

        auto componentId = _edgesComponentId[edgeId];

        if(u::contains(_updatesRequired, componentId))
            _componentsMap[componentId]->_edgeIds.push_back(edgeId);
    }
}

void ComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(u::contains(_componentsMap, componentId))
    {
        _componentsMap.erase(componentId);
        _vacatedComponentIdQueue.push(componentId);
        _updatesRequired.erase(componentId);
    }
}

void ComponentManager::onGraphChanged(const Graph* graph)
{
    graph->debugPauser.pause("Call updateComponents from ComponentManager::onGraphChanged", &_debugPaused);
    graph->setPhase(tr("Componentising"));
    update(graph);
    graph->debugPauser.pause("Signal Graph::onGraphChanged", &_debugPaused);
}

template<typename T> class unique_lock_with_warning
{
public:
    unique_lock_with_warning(T& mutex) :
        _lock(mutex, std::try_to_lock)
    {
        if(!_lock.owns_lock())
        {
            qWarning() << "Needed to acquire lock when reading from ComponentManager; "
                          "this implies inappropriate concurrent access which is bad "
                          "because it means the thread" << u::currentThreadName() <<
                          "is blocked until the update completes";

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
    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    return _componentIds;
}

const GraphComponent* ComponentManager::componentById(ComponentId componentId) const
{
    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    if(u::contains(_componentsMap, componentId))
        return _componentsMap.at(componentId).get();

    Q_ASSERT(!"ComponentManager::componentById returning nullptr");
    return nullptr;
}

ComponentId ComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId.isNull())
        return ComponentId();

    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    auto componentId = _nodesComponentId.at(nodeId);
    auto i = std::find(_componentIds.begin(), _componentIds.end(), componentId);
    return i != _componentIds.end() ? *i : ComponentId();
}

ComponentId ComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.isNull())
        return ComponentId();

    unique_lock_with_warning<std::recursive_mutex> lock(_updateMutex);

    auto componentId = _edgesComponentId.at(edgeId);
    auto i = std::find(_componentIds.begin(), _componentIds.end(), componentId);
    return i != _componentIds.end() ? *i : ComponentId();
}

