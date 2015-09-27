#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "graph.h"
#include "grapharray.h"
#include "filter.h"

#include <map>
#include <queue>
#include <mutex>
#include <vector>
#include <functional>

class ComponentSplitSet
{
private:
    ComponentId _oldComponentId;
    ComponentIdSet _splitters;

public:
    ComponentSplitSet(ComponentId oldComponentId, ComponentIdSet&& splitters) :
        _oldComponentId(oldComponentId), _splitters(splitters)
    {}

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
    {}

    const ComponentIdSet& mergers() const { return _mergers; }
    ComponentId newComponentId() const { return _newComponentId; }
};

class GraphComponent : public Graph
{
    friend class ComponentManager;

    Q_OBJECT
public:
    GraphComponent(const Graph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
        Graph(),
        _graph(other._graph),
        _nodeIds(other._nodeIds),
        _edgeIds(other._edgeIds)
    {}

private:
    const Graph* _graph;
    std::vector<NodeId> _nodeIds;
    std::vector<EdgeId> _edgeIds;

public:
    const std::vector<NodeId>& nodeIds() const { return _nodeIds; }
    int numNodes() const { return static_cast<int>(_nodeIds.size()); }
    const Node& nodeById(NodeId nodeId) const { return _graph->nodeById(nodeId); }
    NodeIdSetCollection::Type typeOf(NodeId) const { return NodeIdSetCollection::Type::Not; }
    NodeIdSet multiNodesForNodeId(NodeId nodeId) const { return _graph->multiNodesForNodeId(nodeId); }

    const std::vector<EdgeId>& edgeIds() const { return _edgeIds; }
    int numEdges() const { return static_cast<int>(_edgeIds.size()); }
    const Edge& edgeById(EdgeId edgeId) const { return _graph->edgeById(edgeId); }
    EdgeIdSetCollection::Type typeOf(EdgeId) const { return EdgeIdSetCollection::Type::Not; }
    EdgeIdSet multiEdgesForEdgeId(EdgeId edgeId) const { return _graph->multiEdgesForEdgeId(edgeId); }

    void reserve(const Graph& other);
    void cloneFrom(const Graph& other);
};

class ComponentManager : public QObject, public Filter
{
    friend class Graph;

    Q_OBJECT
public:
    ComponentManager(Graph& graph,
                     NodeFilterFn nodeFilter = nullptr,
                     EdgeFilterFn edgeFilter = nullptr);
    virtual ~ComponentManager();

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

    std::unordered_set<GraphArray*> _componentArrays;

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

private slots:
    void onGraphChanged(const Graph* graph);

public:
    const std::vector<ComponentId>& componentIds() const;
    int numComponents() const { return static_cast<int>(componentIds().size()); }
    const GraphComponent* componentById(ComponentId componentId) const;
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;

signals:
    void componentAdded(const Graph*, ComponentId, bool) const;
    void componentWillBeRemoved(const Graph*, ComponentId, bool) const;
    void componentSplit(const Graph*, const ComponentSplitSet&) const;
    void componentsWillMerge(const Graph*, const ComponentMergeSet&) const;
};

#endif // COMPONENTMANAGER_H
