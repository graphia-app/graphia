#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "graph.h"

#include <QObject>

#include <vector>
#include <memory>

class GraphComponent : public QObject, public ReadOnlyGraph
{
    friend class ComponentManager;

    Q_OBJECT
public:
    GraphComponent(const ReadOnlyGraph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
        QObject(other.parent()),
        _graph(other._graph),
        _nodeIdsList(other._nodeIdsList),
        _edgeIdsList(other._edgeIdsList)
    {}

private:
    const ReadOnlyGraph* _graph;
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

class ComponentManager : public QObject
{
    Q_OBJECT
    friend class Graph;

public:
    ComponentManager(const Graph& graph)
    {
        connect(&graph, &Graph::nodeAdded, this, &ComponentManager::onNodeAdded, Qt::DirectConnection);
        connect(&graph, &Graph::nodeWillBeRemoved, this, &ComponentManager::onNodeWillBeRemoved, Qt::DirectConnection);
        connect(&graph, &Graph::edgeAdded, this, &ComponentManager::onEdgeAdded, Qt::DirectConnection);
        connect(&graph, &Graph::edgeWillBeRemoved, this, &ComponentManager::onEdgeWillBeRemoved, Qt::DirectConnection);
        connect(&graph, &Graph::graphChanged, this, &ComponentManager::onGraphChanged, Qt::DirectConnection);

        connect(this, &ComponentManager::componentAdded, &graph, &Graph::componentAdded, Qt::DirectConnection);
        connect(this, &ComponentManager::componentWillBeRemoved, &graph, &Graph::componentWillBeRemoved, Qt::DirectConnection);
        connect(this, &ComponentManager::componentSplit, &graph, &Graph::componentSplit, Qt::DirectConnection);
        connect(this, &ComponentManager::componentsWillMerge, &graph, &Graph::componentsWillMerge, Qt::DirectConnection);
    }

protected slots:
    virtual void onNodeAdded(const Graph*, NodeId nodeId) = 0;
    virtual void onNodeWillBeRemoved(const Graph*, NodeId nodeId) = 0;

    virtual void onEdgeAdded(const Graph*, EdgeId edgeId) = 0;
    virtual void onEdgeWillBeRemoved(const Graph*, EdgeId edgeId) = 0;

    virtual void onGraphChanged(const Graph*) = 0;

protected:
    template<typename> friend class ComponentArray;
    std::unordered_set<ResizableGraphArray*> _componentArrayList;
    virtual int componentArrayCapacity() const = 0;

    std::vector<NodeId>& graphComponentNodeIdsList(std::shared_ptr<GraphComponent> graphComponent)
    {
        Q_ASSERT(graphComponent.get() != nullptr);
        return graphComponent->_nodeIdsList;
    }

    std::vector<EdgeId>& graphComponentEdgeIdsList(std::shared_ptr<GraphComponent> graphComponent)
    {
        Q_ASSERT(graphComponent.get() != nullptr);
        return graphComponent->_edgeIdsList;
    }

public:
    virtual const std::vector<ComponentId>& componentIds() const = 0;
    int numComponents() const { return static_cast<int>(componentIds().size()); }
    virtual std::shared_ptr<const GraphComponent> componentById(ComponentId componentId) = 0;
    virtual ComponentId componentIdOfNode(NodeId nodeId) const = 0;
    virtual ComponentId componentIdOfEdge(EdgeId edgeId) const = 0;

signals:
    void componentAdded(const Graph*, ComponentId) const;
    void componentWillBeRemoved(const Graph*, ComponentId) const;
    void componentSplit(const Graph*, ComponentId, const ElementIdSet<ComponentId>&) const;
    void componentsWillMerge(const Graph*, const ElementIdSet<ComponentId>&, ComponentId) const;
};

#endif // COMPONENTMANAGER_H
