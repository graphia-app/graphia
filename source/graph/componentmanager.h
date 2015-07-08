#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "componentmanager.h"
#include "grapharray.h"

#include <map>
#include <queue>
#include <mutex>

class ComponentManager : public AbstractComponentManager
{
    Q_OBJECT

public:
    ComponentManager(Graph& graph) :
        AbstractComponentManager(graph),
        _nextComponentId(0),
        _nodesComponentId(graph),
        _edgesComponentId(graph),
        _debugPaused(false)
    {}

private:
    std::vector<ComponentId> _componentIdsList;
    ComponentId _nextComponentId;
    std::queue<ComponentId> _vacatedComponentIdQueue;
    std::map<ComponentId, std::shared_ptr<GraphComponent>> _componentsMap;
    ElementIdSet<ComponentId> _updatesRequired;
    NodeArray<ComponentId> _nodesComponentId;
    EdgeArray<ComponentId> _edgesComponentId;

    mutable std::recursive_mutex _updateMutex;
    bool _debugPaused;

    ComponentId generateComponentId();
    void releaseComponentId(ComponentId componentId);
    void queueGraphComponentUpdate(const Graph* graph, ComponentId componentId);
    void updateGraphComponent(const Graph* graph, ComponentId componentId);
    void removeGraphComponent(ComponentId componentId);

    void updateComponents(const Graph* graph);
    int componentArrayCapacity() const { return _nextComponentId; }
    ElementIdSet<ComponentId> assignConnectedElementsComponentId(const Graph* graph, NodeId rootId, ComponentId componentId,
                                                                 NodeArray<ComponentId>& nodesComponentId,
                                                                 EdgeArray<ComponentId>& edgesComponentId);

private slots:
    void onGraphChanged(const Graph* graph);

public:
    const std::vector<ComponentId>& componentIds() const;
    const GraphComponent* componentById(ComponentId componentId) const;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
};

#endif // COMPONENTMANAGER_H
