#include "simplecomponentmanager.h"

#include <QQueue>

void SimpleComponentManager::assignConnectedElementsComponentId(NodeId rootId, ComponentId componentId, EdgeId skipEdgeId)
{
    QQueue<NodeId> nodeIdSearchList;

    nodeIdSearchList.enqueue(rootId);

    while(!nodeIdSearchList.isEmpty())
    {
        NodeId nodeId = nodeIdSearchList.dequeue();
        nodesComponentId[nodeId] = componentId;

        const QSet<EdgeId> edgeIds = graph().nodeById(nodeId).edges();

        for(EdgeId edgeId : edgeIds)
        {
            if(edgeId == skipEdgeId)
                continue;

            edgesComponentId[edgeId] = componentId;
            NodeId oppositeNodeId = graph().edgeById(edgeId).oppositeId(nodeId);

            if(nodesComponentId[oppositeNodeId] != componentId)
            {
                nodeIdSearchList.enqueue(oppositeNodeId);
                nodesComponentId[oppositeNodeId] = componentId;
            }
        }
    }
}

void SimpleComponentManager::findComponents()
{
    nodesComponentId.fill(NullComponentId);
    edgesComponentId.fill(NullComponentId);

    const QList<NodeId>& nodeIdsList = graph().nodeIds();
    for(NodeId nodeId : nodeIdsList)
    {
        if(nodesComponentId[nodeId] == NullComponentId)
        {
            ComponentId newComponentId = generateComponentId();
            assignConnectedElementsComponentId(nodeId, newComponentId, NullEdgeId);
            queueGraphComponentUpdate(newComponentId);

            // New component
            onComponentAdded(newComponentId);
        }
    }

    emit graphChanged(&graph());
}

ComponentId SimpleComponentManager::generateComponentId()
{
    ComponentId newComponentId;

    if(!vacatedComponentIdQueue.isEmpty())
        newComponentId = vacatedComponentIdQueue.dequeue();
    else
        newComponentId = nextComponentId++;

    componentIdsList.append(newComponentId);

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

    QList<NodeId>& nodeIdsList = graphComponentNodeIdsList(graphComponent);
    QList<NodeId>& edgeIdsList = graphComponentEdgeIdsList(graphComponent);

    nodeIdsList.clear();
    const QList<NodeId>& nodeIds = graph().nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        if(nodesComponentId[nodeId] == componentId)
            nodeIdsList.append(nodeId);
    }

    edgeIdsList.clear();
    const QList<EdgeId>& edgeIds = graph().edgeIds();
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
    }
}

void SimpleComponentManager::nodeAdded(NodeId nodeId)
{
    ComponentId newComponentId = generateComponentId();
    nodesComponentId[nodeId] = newComponentId;
    queueGraphComponentUpdate(newComponentId);

    // New component
    onComponentAdded(newComponentId);
}

void SimpleComponentManager::nodeWillBeRemoved(NodeId nodeId)
{
    ComponentId componentId = nodesComponentId[nodeId];

    if(graph().nodeById(nodeId).degree() == 0)
    {
        // Component removed
        emit componentWillBeRemoved(&graph(), componentId);

        removeGraphComponent(componentId);
    }
    else
        queueGraphComponentUpdate(componentId);
}

void SimpleComponentManager::edgeAdded(EdgeId edgeId)
{
    const Edge& edge = graph().edgeById(edgeId);

    if(nodesComponentId[edge.sourceId()] != nodesComponentId[edge.targetId()])
    {
        ComponentId firstComponentId = nodesComponentId[edge.sourceId()];
        ComponentId secondComponentId = nodesComponentId[edge.targetId()];

        // Components merged
        QSet<ComponentId> mergers;
        mergers.insert(firstComponentId);
        mergers.insert(secondComponentId);
        emit componentsWillMerge(&graph(), mergers, firstComponentId);

        // Component removed
        emit componentWillBeRemoved(&graph(), secondComponentId);

        // Assign every node in the second component to the first
        assignConnectedElementsComponentId(edge.targetId(), firstComponentId, edgeId);
        edgesComponentId[edgeId] = firstComponentId;
        queueGraphComponentUpdate(firstComponentId);

        removeGraphComponent(secondComponentId);
        releaseComponentId(secondComponentId);
    }
    else
    {
        ComponentId existingComponentId = nodesComponentId[edge.sourceId()];
        edgesComponentId[edgeId] = existingComponentId;
        queueGraphComponentUpdate(existingComponentId);
    }

}

void SimpleComponentManager::edgeWillBeRemoved(EdgeId edgeId)
{
    ComponentId newComponentId = generateComponentId();
    const Edge& edge = graph().edgeById(edgeId);
    ComponentId oldComponentId = nodesComponentId[edge.sourceId()];
    GraphComponent* oldGraphComponent = componentsMap[oldComponentId];

    // Assign every node connected to the target of the removed edge the new componentId
    assignConnectedElementsComponentId(edge.targetId(), newComponentId, edgeId);

    if(nodesComponentId[edge.sourceId()] == nodesComponentId[edge.targetId()])
    {
        // The edge removal didn't create a new component,
        // so go back to using the original ComponentId
        const QList<NodeId>& nodeIds = graphComponentNodeIdsList(oldGraphComponent);
        for(NodeId nodeId : nodeIds)
            nodesComponentId[nodeId] = oldComponentId;

        const QList<EdgeId>& edgeIds = graphComponentEdgeIdsList(oldGraphComponent);
        for(EdgeId edgeId : edgeIds)
            edgesComponentId[edgeId] = oldComponentId;

        queueGraphComponentUpdate(oldComponentId);
        releaseComponentId(newComponentId);
    }
    else
    {
        queueGraphComponentUpdate(oldComponentId);
        queueGraphComponentUpdate(newComponentId);

        // Components split
        QSet<ComponentId> splitters;
        splitters.insert(oldComponentId);
        splitters.insert(newComponentId);
        emit componentSplit(&graph(), oldComponentId, splitters);

        // New component
        onComponentAdded(newComponentId);
    }
}

void SimpleComponentManager::graphChanged(const Graph*)
{
    for(ComponentId componentId : updatesRequired)
        updateGraphComponent(componentId);

    updatesRequired.clear();
}

void SimpleComponentManager::onComponentAdded(ComponentId componentId)
{
    for(ResizableGraphArray* componentArray : componentArrayList)
        componentArray->resize(componentArrayCapacity());

    // New component
    emit componentAdded(&graph(), componentId);
}

const QList<ComponentId>& SimpleComponentManager::componentIds() const
{
    return componentIdsList;
}

const ReadOnlyGraph* SimpleComponentManager::componentById(ComponentId componentId)
{
    if(componentsMap.contains(componentId))
        return componentsMap[componentId];

    return nullptr;
}

ComponentId SimpleComponentManager::componentIdOfNode(NodeId nodeId) const
{
    if(nodeId == NullNodeId)
        return NullComponentId;

    return nodesComponentId[nodeId];
}

ComponentId SimpleComponentManager::componentIdOfEdge(EdgeId edgeId) const
{
    if(edgeId == NullEdgeId)
        return NullComponentId;

    return edgesComponentId[edgeId];
}

