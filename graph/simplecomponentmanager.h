#ifndef SIMPLECOMPONENTMANAGER_H
#define SIMPLECOMPONENTMANAGER_H

#include "componentmanager.h"
#include "grapharray.h"

class SimpleComponentManager : public ComponentManager
{
private:
    QList<ComponentId> componentIdList;
    GraphComponent dummyToCompile;

public:
    SimpleComponentManager(Graph& graph) :
        ComponentManager(graph),
        dummyToCompile(graph)
    {}

private:
    void nodeAdded(NodeId nodeId);
    void nodeWillBeRemoved(NodeId nodeId);

    void edgeAdded(EdgeId edgeId);
    void edgeWillBeRemoved(EdgeId edgeId);

public:
    const QList<ComponentId>& componentIds() const;
    const ReadOnlyGraph& componentById(ComponentId componentId) const;
};

#endif // SIMPLECOMPONENTMANAGER_H
