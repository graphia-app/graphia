#include "simplecomponentmanager.h"

#include <QQueue>

static QSet<ComponentId> assignConnectedElementsComponentId(NodeId rootId, ComponentId componentId,
                                                            const Graph& graph,
                                                            NodeArray<ComponentId>& nodesComponentId,
                                                            EdgeArray<ComponentId>& edgesComponentId,
                                                            EdgeId skipEdgeId = EdgeId::Null())
{
    QQueue<NodeId> nodeIdSearchList;
    QSet<ComponentId> oldComponentIdsAffected;

    nodeIdSearchList.enqueue(rootId);

    while(!nodeIdSearchList.isEmpty())
    {
        NodeId nodeId = nodeIdSearchList.dequeue();
        oldComponentIdsAffected.insert(nodesComponentId[nodeId]);
        nodesComponentId[nodeId] = componentId;

        const QSet<EdgeId> edgeIds = graph.nodeById(nodeId).edges();

        for(EdgeId edgeId : edgeIds)
        {
            if(edgeId == skipEdgeId)
                continue;

            edgesComponentId[edgeId] = componentId;
            NodeId oppositeNodeId = graph.edgeById(edgeId).oppositeId(nodeId);

            if(nodesComponentId[oppositeNodeId] != componentId)
            {
                nodeIdSearchList.enqueue(oppositeNodeId);
                nodesComponentId[oppositeNodeId] = componentId;
            }
        }
    }

    oldComponentIdsAffected.remove(ComponentId::Null());

    return oldComponentIdsAffected;
}

void SimpleComponentManager::updateComponents()
{
    QMap<ComponentId, QSet<ComponentId>> splitComponents;
    QList<ComponentId> newComponentIds;

    NodeArray<ComponentId> newNodesComponentId(graph());
    EdgeArray<ComponentId> newEdgesComponentId(graph());
    QList<ComponentId> newComponentIdsList;

    newNodesComponentId.fill(ComponentId::Null());
    newEdgesComponentId.fill(ComponentId::Null());

    const QVector<NodeId>& nodeIdsList = graph().nodeIds();

    // Search for mergers and splitters
    for(NodeId nodeId : nodeIdsList)
    {
        ComponentId oldComponentId = nodesComponentId[nodeId];

        if(newNodesComponentId[nodeId].IsNull() && !oldComponentId.IsNull())
        {
            if(newComponentIdsList.contains(oldComponentId))
            {
                // We have already used this ID so this is a component that has split
                ComponentId newComponentId = generateComponentId();
                newComponentIdsList.append(newComponentId);
                assignConnectedElementsComponentId(nodeId, newComponentId, graph(),
                                                   newNodesComponentId, newEdgesComponentId);

                queueGraphComponentUpdate(oldComponentId);
                queueGraphComponentUpdate(newComponentId);

                splitComponents[oldComponentId].unite({oldComponentId, newComponentId});
            }
            else
            {
                newComponentIdsList.append(oldComponentId);
                QSet<ComponentId> componentIdsAffected =
                        assignConnectedElementsComponentId(nodeId, oldComponentId, graph(),
                                                           newNodesComponentId, newEdgesComponentId);
                queueGraphComponentUpdate(oldComponentId);

                if(componentIdsAffected.size() > 1)
                {
                    // More than one old component IDs were observed so components have merged
                    emit componentsWillMerge(&graph(), componentIdsAffected, oldComponentId);
                    componentIdsAffected.remove(oldComponentId);

                    for(ComponentId removedComponentId : componentIdsAffected)
                    {
                        emit componentWillBeRemoved(&graph(), removedComponentId);
                        removeGraphComponent(removedComponentId);
                    }
                }
            }
        }
    }

    // Search for entirely new components
    for(NodeId nodeId : nodeIdsList)
    {
        if(newNodesComponentId[nodeId].IsNull() && nodesComponentId[nodeId].IsNull())
        {
            ComponentId newComponentId = generateComponentId();
            newComponentIdsList.append(newComponentId);
            assignConnectedElementsComponentId(nodeId, newComponentId, graph(), newNodesComponentId, newEdgesComponentId);
            queueGraphComponentUpdate(newComponentId);

            newComponentIds.append(newComponentId);
        }
    }

    // Search for removed components
    QSet<ComponentId> removedComponentIds = componentIdsList.toSet().subtract(newComponentIdsList.toSet());
    for(ComponentId removedComponentId : removedComponentIds)
    {
        // Component removed
        emit componentWillBeRemoved(&graph(), removedComponentId);

        removeGraphComponent(removedComponentId);
    }

    nodesComponentId = newNodesComponentId;
    edgesComponentId = newEdgesComponentId;

    // Notify all the splits
    for(ComponentId splitee : splitComponents.keys())
    {
        QSet<ComponentId>& splitters = splitComponents[splitee];
        emit componentSplit(&graph(), splitee, splitters);

        for(ComponentId splitter : splitters)
        {
            if(splitter != splitee)
                emit componentAdded(&graph(), splitter);
        }
    }

    // Notify all the new components
    for(ComponentId newComponentId : newComponentIds)
        emit componentAdded(&graph(), newComponentId);
}

ComponentId SimpleComponentManager::generateComponentId()
{
    ComponentId newComponentId;

    if(!vacatedComponentIdQueue.isEmpty())
        newComponentId = vacatedComponentIdQueue.dequeue();
    else
        newComponentId = nextComponentId++;

    componentIdsList.append(newComponentId);

    for(ResizableGraphArray* componentArray : componentArrayList)
        componentArray->resize(componentArrayCapacity());

    return newComponentId;
}

void SimpleComponentManager::releaseComponentId(ComponentId componentId)
{
    componentIdsList.removeOne(componentId);
    vacatedComponentIdQueue.enqueue(componentId);
}

void SimpleComponentManager::queueGraphComponentUpdate(ComponentId componentId)
{
    updatesRequired.insert(componentId);

    if(!componentsMap.contains(componentId))
    {
        GraphComponent* graphComponent = new GraphComponent(this->graph());
        componentsMap.insert(componentId, graphComponent);
    }
}

void SimpleComponentManager::updateGraphComponent(ComponentId componentId)
{
    GraphComponent* graphComponent = componentsMap[componentId];

    QVector<NodeId>& nodeIdsList = graphComponentNodeIdsList(graphComponent);
    QVector<EdgeId>& edgeIdsList = graphComponentEdgeIdsList(graphComponent);

    nodeIdsList.clear();
    const QVector<NodeId>& nodeIds = graph().nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        if(nodesComponentId[nodeId] == componentId)
            nodeIdsList.append(nodeId);
    }

    edgeIdsList.clear();
    const QVector<EdgeId>& edgeIds = graph().edgeIds();
    for(EdgeId edgeId : edgeIds)
    {
        if(edgesComponentId[edgeId] == componentId)
            edgeIdsList.append(edgeId);
    }
}

void SimpleComponentManager::removeGraphComponent(ComponentId componentId)
{
    if(componentsMap.contains(componentId))
    {
        GraphComponent* graphComponent = componentsMap[componentId];
        delete graphComponent;

        componentsMap.remove(componentId);
        componentIdsList.removeOne(componentId);
        releaseComponentId(componentId);
        updatesRequired.remove(componentId);
    }
}

void SimpleComponentManager::graphChanged(const Graph*)
{
    updateComponents();

    for(ComponentId componentId : updatesRequired)
        updateGraphComponent(componentId);

    updatesRequired.clear();
}

const QList<ComponentId>& SimpleComponentManager::componentIds() const
{
    return componentIdsList;
}

const GraphComponent* SimpleComponentManager::componentById(ComponentId componentId)
{
    if(componentsMap.contains(componentId))
        return componentsMap[componentId];

    return nullptr;
}

ComponentId SimpleComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId.IsNull())
        return ComponentId::Null();

    ComponentId componentId = nodesComponentId[nodeId];
    return componentIdsList.contains(componentId) ? componentId : ComponentId::Null();
}

ComponentId SimpleComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId.IsNull())
        return ComponentId::Null();

    ComponentId componentId = edgesComponentId[edgeId];
    return componentIdsList.contains(componentId) ? componentId : ComponentId::Null();
}

