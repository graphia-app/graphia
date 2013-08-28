#ifndef SIMPLECOMPONENTMANAGER_H
#define SIMPLECOMPONENTMANAGER_H

#include "componentmanager.h"
#include "grapharray.h"

#include <QMap>
#include <QQueue>
#include <QList>
#include <QSet>

class SimpleComponentManager : public ComponentManager
{
    Q_OBJECT
private:
    QList<ComponentId> componentIdsList;
    ComponentId nextComponentId;
    QQueue<NodeId> vacatedComponentIdQueue;
    QMap<ComponentId, GraphComponent*> componentsMap;
    QSet<ComponentId> updatesRequired;
    NodeArray<ComponentId> nodesComponentId;
    EdgeArray<ComponentId> edgesComponentId;

    void assignConnectedElementsComponentId(NodeId rootId, ComponentId componentId, EdgeId skipEdgeId);
    ComponentId generateComponentId();
    void releaseComponentId(ComponentId componentId);

    void updateGraphComponent(ComponentId componentId);
    void removeGraphComponent(ComponentId componentId);

public:
    SimpleComponentManager(Graph& graph) :
        ComponentManager(graph),
        nextComponentId(0),
        nodesComponentId(graph),
        edgesComponentId(graph)
    {}

private:
    void nodeAdded(NodeId nodeId);
    void nodeWillBeRemoved(NodeId nodeId);

    void edgeAdded(EdgeId edgeId);
    void edgeWillBeRemoved(EdgeId edgeId);

public:
    const QList<ComponentId>& componentIds() const;
    const ReadOnlyGraph& componentById(ComponentId componentId);
};

#endif // SIMPLECOMPONENTMANAGER_H
