#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "shared/graph/grapharray.h"

#include "graphfilter.h"

#include <map>
#include <queue>
#include <mutex>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>

#include <QObject>
#include <QtGlobal>

class Graph;
class GraphComponent;

class ComponentSplitSet
{
private:
    ComponentId _oldComponentId;
    ComponentIdSet _splitters;

public:
    ComponentSplitSet(ComponentId oldComponentId, ComponentIdSet&& splitters) :
        _oldComponentId(oldComponentId), _splitters(splitters)
    {
        Q_ASSERT(!oldComponentId.isNull());
        Q_ASSERT(!std::any_of(_splitters.begin(), _splitters.end(),
                 [](const auto& splitter) { return splitter.isNull(); }));
    }

    ComponentId oldComponentId() const { return _oldComponentId; }
    const ComponentIdSet& splitters() const { return _splitters; }
};

class ComponentMergeSet
{
private:
    ComponentIdSet _mergers;
    ComponentId _newComponentId;

public:
    ComponentMergeSet(ComponentIdSet&& mergers, ComponentId newComponentId) :
        _mergers(mergers), _newComponentId(newComponentId)
    {
        Q_ASSERT(!newComponentId.isNull());
        Q_ASSERT(!std::any_of(_mergers.begin(), _mergers.end(),
                 [](const auto& merger) { return merger.isNull(); }));
    }

    const ComponentIdSet& mergers() const { return _mergers; }
    ComponentId newComponentId() const { return _newComponentId; }
};

class ComponentManager : public QObject, public GraphFilter
{
    friend class Graph;

    Q_OBJECT
public:
    explicit ComponentManager(Graph& graph,
                     const NodeConditionFn& nodeFilter = nullptr,
                     const EdgeConditionFn& edgeFilter = nullptr);
    ~ComponentManager() override;

private:
    std::vector<ComponentId> _componentIds;
    ComponentId _nextComponentId;
    std::queue<ComponentId> _vacatedComponentIdQueue;
    std::map<ComponentId, std::unique_ptr<GraphComponent>> _componentsMap;
    ComponentIdSet _updatesRequired;
    NodeArray<ComponentId> _nodesComponentId;
    EdgeArray<ComponentId> _edgesComponentId;

    mutable std::recursive_mutex _updateMutex;

    std::mutex _componentArraysMutex;
    std::unordered_set<IGraphArray*> _componentArrays;

    bool _debug = false;

    ComponentId generateComponentId();
    void queueGraphComponentUpdate(const Graph* graph, ComponentId componentId);
    void updateGraphComponents(const Graph* graph);
    void removeGraphComponent(ComponentId componentId);

    void update(const Graph* graph);
    int componentArrayCapacity() const { return _nextComponentId; }
    ComponentIdSet assignConnectedElementsComponentId(const Graph* graph, NodeId rootId, ComponentId componentId,
                                                      NodeArray<ComponentId>& nodesComponentId,
                                                      EdgeArray<ComponentId>& edgesComponentId);

    void insertComponentArray(IGraphArray* componentArray);
    void eraseComponentArray(IGraphArray* componentArray);

private slots:
    void onGraphChanged(const Graph* graph, bool changeOccurred);

public:
    const std::vector<ComponentId>& componentIds() const;
    int numComponents() const { return static_cast<int>(componentIds().size()); }
    bool containsComponentId(ComponentId componentId) const;
    const GraphComponent* componentById(ComponentId componentId) const;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;

    void enableDebug() { _debug = true; }
    void disbleDebug() { _debug = false; }

signals:
    void componentAdded(const Graph*, ComponentId, bool) const;
    void componentWillBeRemoved(const Graph*, ComponentId, bool) const;
    void componentSplit(const Graph*, const ComponentSplitSet&) const;
    void componentsWillMerge(const Graph*, const ComponentMergeSet&) const;

    void nodeRemovedFromComponent(const Graph*, NodeId, ComponentId) const;
    void edgeRemovedFromComponent(const Graph*, EdgeId, ComponentId) const;
    void nodeAddedToComponent(const Graph*, NodeId, ComponentId) const;
    void edgeAddedToComponent(const Graph*, EdgeId, ComponentId) const;
};

#endif // COMPONENTMANAGER_H
