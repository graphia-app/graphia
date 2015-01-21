#include "simplecomponentmanager.h"

#include <queue>
#include <map>

ElementIdSet<ComponentId> SimpleComponentManager::assignConnectedElementsComponentId(const Graph* graph,
        NodeId rootId, ComponentId componentId,
        NodeArray<ComponentId>& nodesComponentId,
        EdgeArray<ComponentId>& edgesComponentId)
{
    std::queue<NodeId> nodeIdSearchList;
    ElementIdSet<ComponentId> oldComponentIdsAffected;

    nodeIdSearchList.push(rootId);

    while(!nodeIdSearchList.empty())
    {
        NodeId nodeId = nodeIdSearchList.front();
        nodeIdSearchList.pop();
        oldComponentIdsAffected.insert(_nodesComponentId[nodeId]);
        nodesComponentId[nodeId] = componentId;

        const ElementIdSet<EdgeId> edgeIds = graph->nodeById(nodeId).edges();

        for(EdgeId edgeId : edgeIds)
        {
            edgesComponentId[edgeId] = componentId;
            NodeId oppositeNodeId = graph->edgeById(edgeId).oppositeId(nodeId);

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

void SimpleComponentManager::updateComponents(const Graph* graph)
{
    std::map<ComponentId, ElementIdSet<ComponentId>> splitComponents;
    ElementIdSet<ComponentId> newComponentIds;

    NodeArray<ComponentId> newNodesComponentId(*const_cast<Graph*>(graph));
    EdgeArray<ComponentId> newEdgesComponentId(*const_cast<Graph*>(graph));
    ElementIdSet<ComponentId> newComponentIdsList;

    const std::vector<NodeId>& nodeIdsList = graph->nodeIds();

    // Search for mergers and splitters
    for(NodeId nodeId : nodeIdsList)
    {
        ComponentId oldComponentId = _nodesComponentId[nodeId];

        if(newNodesComponentId[nodeId].isNull() && !oldComponentId.isNull())
        {
            if(newComponentIdsList.find(oldComponentId) != newComponentIdsList.end())
            {
                // We have already used this ID so this is a component that has split
                ComponentId newComponentId = generateComponentId();
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
                ElementIdSet<ComponentId> componentIdsAffected =
                        assignConnectedElementsComponentId(graph, nodeId, oldComponentId,
                                                           newNodesComponentId, newEdgesComponentId);
                queueGraphComponentUpdate(graph, oldComponentId);

                if(componentIdsAffected.size() > 1)
                {
                    // More than one old component IDs were observed so components have merged
                    if(_debug) qDebug() << "componentsWillMerge" << componentIdsAffected << "->" << oldComponentId;
                    emit componentsWillMerge(graph, componentIdsAffected, oldComponentId);
                    componentIdsAffected.erase(oldComponentId);

                    for(ComponentId removedComponentId : componentIdsAffected)
                    {
                        if(_debug) qDebug() << "componentWillBeRemoved" << removedComponentId;
                        emit componentWillBeRemoved(graph, removedComponentId);
                        removeGraphComponent(removedComponentId);
                    }
                }
            }
        }
    }

    // Search for entirely new components
    for(NodeId nodeId : nodeIdsList)
    {
        if(newNodesComponentId[nodeId].isNull() && _nodesComponentId[nodeId].isNull())
        {
            ComponentId newComponentId = generateComponentId();
            newComponentIdsList.insert(newComponentId);
            assignConnectedElementsComponentId(graph, nodeId, newComponentId, newNodesComponentId, newEdgesComponentId);
            queueGraphComponentUpdate(graph, newComponentId);

            newComponentIds.insert(newComponentId);
        }
    }

    // Search for removed components
    ElementIdSet<ComponentId> componentIdsToBeRemoved;
    for(ComponentId componentId : _componentIdsList)
    {
        if(newComponentIdsList.find(componentId) != newComponentIdsList.end())
            continue;

        componentIdsToBeRemoved.insert(componentId);
    }

    for(ComponentId componentId : componentIdsToBeRemoved)
    {
        // Component removed
        if(_debug) qDebug() << "componentWillBeRemoved" << componentId;
        emit componentWillBeRemoved(graph, componentId);

        removeGraphComponent(componentId);
    }

    _nodesComponentId = std::move(newNodesComponentId);
    _edgesComponentId = std::move(newEdgesComponentId);

    for(ComponentId componentId : _updatesRequired)
        updateGraphComponent(graph, componentId);

    _updatesRequired.clear();

    // Notify all the splits
    for(auto splitee : splitComponents)
    {
        ElementIdSet<ComponentId>& splitters = splitee.second;

        for(ComponentId splitter : splitters)
        {
            if(splitter != splitee.first)
            {
                if(_debug) qDebug() << "componentAdded" << splitter;
                emit componentAdded(graph, splitter);
            }
        }

        if(_debug) qDebug() << "componentSplit" << splitee.first << "->" << splitters;
        emit componentSplit(graph, splitee.first, splitters);
    }

    // Notify all the new components
    for(ComponentId newComponentId : newComponentIds)
    {
        if(_debug) qDebug() << "componentAdded" << newComponentId;
        emit componentAdded(graph, newComponentId);
    }
}

ComponentId SimpleComponentManager::generateComponentId()
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

    for(ResizableGraphArray* componentArray : _componentArrayList)
        componentArray->resize(componentArrayCapacity());

    return newComponentId;
}

void SimpleComponentManager::releaseComponentId(ComponentId componentId)
{
    _componentIdsList.erase(std::remove(_componentIdsList.begin(), _componentIdsList.end(), componentId), _componentIdsList.end());
    _vacatedComponentIdQueue.push(componentId);
}

void SimpleComponentManager::queueGraphComponentUpdate(const Graph* graph, ComponentId componentId)
{
    _updatesRequired.insert(componentId);

    if(_componentsMap.find(componentId) == _componentsMap.end())
    {
        std::shared_ptr<GraphComponent> graphComponent = std::make_shared<GraphComponent>(graph);
        _componentsMap.emplace(componentId, graphComponent);
    }
}

void SimpleComponentManager::updateGraphComponent(const Graph* graph, ComponentId componentId)
{
    std::shared_ptr<GraphComponent> graphComponent = _componentsMap[componentId];

    std::vector<NodeId>& nodeIdsList = graphComponentNodeIdsList(*graphComponent);
    std::vector<EdgeId>& edgeIdsList = graphComponentEdgeIdsList(*graphComponent);

    nodeIdsList.clear();
    const std::vector<NodeId>& nodeIds = graph->nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        if(_nodesComponentId[nodeId] == componentId)
            nodeIdsList.push_back(nodeId);
    }

    edgeIdsList.clear();
    const std::vector<EdgeId>& edgeIds = graph->edgeIds();
    for(EdgeId edgeId : edgeIds)
    {
        if(_edgesComponentId[edgeId] == componentId)
            edgeIdsList.push_back(edgeId);
    }
}

void SimpleComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(_componentsMap.find(componentId) != _componentsMap.end())
    {
        _componentsMap.erase(componentId);
        _componentIdsList.erase(std::remove(_componentIdsList.begin(), _componentIdsList.end(), componentId), _componentIdsList.end());
        releaseComponentId(componentId);
        _updatesRequired.erase(componentId);
    }
}

void SimpleComponentManager::onGraphChanged(const Graph* graph)
{
    updateComponents(graph);
}

const std::vector<ComponentId>& SimpleComponentManager::componentIds() const
{
    return _componentIdsList;
}

const GraphComponent* SimpleComponentManager::componentById(ComponentId componentId)
{
    if(_componentsMap.find(componentId) != _componentsMap.end())
        return _componentsMap[componentId].get();

    Q_ASSERT(nullptr);
    return nullptr;
}

ComponentId SimpleComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId.isNull())
        return ComponentId();

    ComponentId componentId = _nodesComponentId.at(nodeId);
    auto i = std::find(_componentIdsList.begin(), _componentIdsList.end(), componentId);
    return i != _componentIdsList.end() ? *i : ComponentId();
}

ComponentId SimpleComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.isNull())
        return ComponentId();

    ComponentId componentId = _edgesComponentId.at(edgeId);
    auto i = std::find(_componentIdsList.begin(), _componentIdsList.end(), componentId);
    return i != _componentIdsList.end() ? *i : ComponentId();
}

