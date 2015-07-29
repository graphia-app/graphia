#ifndef ABSTRACTCOMPONENTMANAGER_H
#define ABSTRACTCOMPONENTMANAGER_H

#include "graph.h"

#include <QObject>

#include <vector>
#include <memory>

class ComponentSplitSet
{
private:
    const MutableGraph* _graph;
    ComponentId _oldComponentId;
    ElementIdSet<ComponentId> _splitters;

public:
    ComponentSplitSet(const MutableGraph* graph, ComponentId oldComponentId, ElementIdSet<ComponentId>&& splitters) :
        _graph(graph), _oldComponentId(oldComponentId), _splitters(splitters)
    {
    }

    ComponentId oldComponentId() const { return _oldComponentId; }
    const ElementIdSet<ComponentId>& splitters() const { return _splitters; }
    ComponentId largestComponentId() const;

    std::vector<NodeId> nodeIds() const;
};

class ComponentMergeSet
{
private:
    const MutableGraph* _graph;
    ElementIdSet<ComponentId> _mergers;
    ComponentId _newComponentId;

public:
    ComponentMergeSet(const MutableGraph* graph, ElementIdSet<ComponentId>&& mergers, ComponentId newComponentId) :
        _graph(graph), _mergers(mergers), _newComponentId(newComponentId)
    {
    }

    const ElementIdSet<ComponentId>& mergers() const { return _mergers; }
    ComponentId newComponentId() const { return _newComponentId; }

    std::vector<NodeId> nodeIds() const;
};

class GraphComponent : public Graph
{
    friend class AbstractComponentManager;

    Q_OBJECT
public:
    GraphComponent(const Graph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
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

    const std::vector<EdgeId>& edgeIds() const { return _edgeIdsList; }
    int numEdges() const { return static_cast<int>(_edgeIdsList.size()); }
    const Edge& edgeById(EdgeId edgeId) const { return _graph->edgeById(edgeId); }
};

class AbstractComponentManager : public QObject
{
    Q_OBJECT
    friend class MutableGraph;

public:
    AbstractComponentManager(MutableGraph& graph);
    virtual ~AbstractComponentManager();

protected slots:
    virtual void onGraphChanged(const MutableGraph*) = 0;

protected:
    template<typename> friend class ComponentArray;
    std::unordered_set<ResizableGraphArray*> _componentArrayList;
    virtual int componentArrayCapacity() const = 0;

    bool _debug;

    std::vector<NodeId>& graphComponentNodeIdsList(GraphComponent& graphComponent)
    {
        return graphComponent._nodeIdsList;
    }

    std::vector<EdgeId>& graphComponentEdgeIdsList(GraphComponent& graphComponent)
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
