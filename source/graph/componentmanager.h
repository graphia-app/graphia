#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "abstractcomponentmanager.h"
#include "grapharray.h"

#include <map>
#include <queue>
#include <mutex>

class ComponentManager : public AbstractComponentManager
{
    Q_OBJECT

public:
    ComponentManager(Graph& graph, bool ignoreMultiElements = true);

private:
    std::vector<ComponentId> _componentIds;
    ComponentId _nextComponentId;
    std::queue<ComponentId> _vacatedComponentIdQueue;
    std::map<ComponentId, std::shared_ptr<GraphComponent>> _componentsMap;
    ComponentIdSet _updatesRequired;
    NodeArray<ComponentId> _nodesComponentId;
    EdgeArray<ComponentId> _edgesComponentId;

    mutable std::recursive_mutex _updateMutex;
    bool _debugPaused = false;

    ComponentId generateComponentId();
    void queueGraphComponentUpdate(const Graph* graph, ComponentId componentId);
    void updateGraphComponents(const Graph* graph);
    void removeGraphComponent(ComponentId componentId);

    void update(const Graph* graph);
    int componentArrayCapacity() const { return _nextComponentId; }
    ComponentIdSet assignConnectedElementsComponentId(const Graph* graph, NodeId rootId, ComponentId componentId,
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
