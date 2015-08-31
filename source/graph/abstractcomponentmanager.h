#ifndef ABSTRACTCOMPONENTMANAGER_H
#define ABSTRACTCOMPONENTMANAGER_H

#include "graph.h"

#include <QObject>

#include <vector>
#include <memory>

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
    friend class AbstractComponentManager;

    Q_OBJECT
public:
    GraphComponent(const Graph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
        Graph(),
        _graph(other._graph),
        _nodeIdsList(other._nodeIdsList),
        _edgeIdsList(other._edgeIdsList)
    {}

private:
    const Graph* _graph;
    std::vector<NodeId> _nodeIdsList;
    std::vector<EdgeId> _edgeIdsList;

public:
    const std::vector<NodeId>& nodeIds() const { return _nodeIdsList; }
    int numNodes() const { return static_cast<int>(_nodeIdsList.size()); }
    const Node& nodeById(NodeId nodeId) const { return _graph->nodeById(nodeId); }
    MultiNodeId::Type typeOf(NodeId) const { return MultiNodeId::Type::Not; }

    const std::vector<EdgeId>& edgeIds() const { return _edgeIdsList; }
    int numEdges() const { return static_cast<int>(_edgeIdsList.size()); }
    const Edge& edgeById(EdgeId edgeId) const { return _graph->edgeById(edgeId); }
    MultiEdgeId::Type typeOf(EdgeId) const { return MultiEdgeId::Type::Not; }

    void reserve(const Graph& other);
    void cloneFrom(const Graph& other);
};

class AbstractComponentManager : public QObject
{
    Q_OBJECT
    friend class Graph;

public:
    AbstractComponentManager(Graph& graph, bool ignoreMultiElements = true);
    virtual ~AbstractComponentManager();

protected slots:
    virtual void onGraphChanged(const Graph*) = 0;

protected:
    template<typename> friend class ComponentArray;
    std::unordered_set<GraphArray*> _componentArrays;
    virtual int componentArrayCapacity() const = 0;

    bool _debug = false;
    bool _ignoreMultiElements = false;

    std::vector<NodeId>& graphComponentNodeIds(GraphComponent& graphComponent)
    {
        return graphComponent._nodeIdsList;
    }

    std::vector<EdgeId>& graphComponentEdgeIds(GraphComponent& graphComponent)
    {
        return graphComponent._edgeIdsList;
    }

public:
    virtual const std::vector<ComponentId>& componentIds() const = 0;
    int numComponents() const { return static_cast<int>(componentIds().size()); }
    virtual const GraphComponent* componentById(ComponentId componentId) const = 0;
    virtual ComponentId componentIdOfNode(NodeId nodeId) const = 0;
    virtual ComponentId componentIdOfEdge(EdgeId edgeId) const = 0;

signals:
    void componentAdded(const Graph*, ComponentId, bool) const;
    void componentWillBeRemoved(const Graph*, ComponentId, bool) const;
    void componentSplit(const Graph*, const ComponentSplitSet&) const;
    void componentsWillMerge(const Graph*, const ComponentMergeSet&) const;
};

#endif // ABSTRACTCOMPONENTMANAGER_H
