#include "simplecomponentmanager.h"

void SimpleComponentManager::nodeAdded(NodeId nodeId)
{
}

void SimpleComponentManager::nodeWillBeRemoved(NodeId nodeId)
{
}

void SimpleComponentManager::edgeAdded(EdgeId edgeId)
{
}

void SimpleComponentManager::edgeWillBeRemoved(EdgeId edgeId)
{
}

const QList<ComponentId>& SimpleComponentManager::componentIds() const
{
    return componentIdList;
}

const ReadOnlyGraph& SimpleComponentManager::componentById(ComponentId componentId) const
{
    return dummyToCompile;
}

