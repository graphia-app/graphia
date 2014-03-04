#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "graph.h"

#include <QObject>

class GraphComponent : public QObject, public ReadOnlyGraph
{
    friend class ComponentManager;

    Q_OBJECT
public:
    GraphComponent(const ReadOnlyGraph& graph) : _graph(&graph) {}
    GraphComponent(const GraphComponent& other) :
        QObject(other.parent()),
        _graph(other._graph),
        nodeIdsList(other.nodeIdsList),
        edgeIdsList(other.edgeIdsList)
    {}

private:
    const ReadOnlyGraph* _graph;
    QList<NodeId> nodeIdsList;
    QList<EdgeId> edgeIdsList;

public:
    const ReadOnlyGraph& graph() { return *_graph; }

    const QList<NodeId>& nodeIds() const { return nodeIdsList; }
    int numNodes() const { return nodeIdsList.size(); }
    const Node& nodeById(NodeId nodeId) const { return _graph->nodeById(nodeId); }

    const QList<EdgeId>& edgeIds() const { return edgeIdsList; }
    int numEdges() const { return edgeIdsList.size(); }
    const Edge& edgeById(EdgeId edgeId) const { return _graph->edgeById(edgeId); }
};

class ComponentManager : public QObject
{
    Q_OBJECT
    friend class Graph;

public:
    ComponentManager(Graph& graph) :
        _graph(&graph)
    {
        connect(this, &ComponentManager::componentAdded, &graph, &Graph::componentAdded, Qt::DirectConnection);
        connect(this, &ComponentManager::componentWillBeRemoved, &graph, &Graph::componentWillBeRemoved, Qt::DirectConnection);
        connect(this, &ComponentManager::componentSplit, &graph, &Graph::componentSplit, Qt::DirectConnection);
        connect(this, &ComponentManager::componentsWillMerge, &graph, &Graph::componentsWillMerge, Qt::DirectConnection);
    }
    virtual ~ComponentManager() {}

protected:
    Graph* _graph;

    virtual void nodeAdded(NodeId nodeId) = 0;
    virtual void nodeWillBeRemoved(NodeId nodeId) = 0;

    virtual void edgeAdded(EdgeId edgeId) = 0;
    virtual void edgeWillBeRemoved(EdgeId edgeId) = 0;

    virtual void graphChanged(const Graph*) = 0;

    template<typename> friend class ComponentArray;
    QList<ResizableGraphArray*> componentArrayList;
    virtual int componentArrayCapacity() const = 0;

    QList<NodeId>& graphComponentNodeIdsList(GraphComponent* graphComponent)
    {
        Q_ASSERT(graphComponent != nullptr);
        return graphComponent->nodeIdsList;
    }

    QList<EdgeId>& graphComponentEdgeIdsList(GraphComponent* graphComponent)
    {
        Q_ASSERT(graphComponent != nullptr);
        return graphComponent->edgeIdsList;
    }

public:
    Graph& graph() { return *_graph; }

    virtual const QList<ComponentId>& componentIds() const = 0;
    int numComponents() const { return componentIds().size(); }
    virtual const GraphComponent* componentById(ComponentId componentId) = 0;
    virtual ComponentId componentIdOfNode(NodeId nodeId) const = 0;
    virtual ComponentId componentIdOfEdge(EdgeId edgeId) const = 0;

signals:
    void componentAdded(const Graph*, ComponentId) const;
    void componentWillBeRemoved(const Graph*, ComponentId) const;
    void componentSplit(const Graph*, ComponentId, const QSet<ComponentId>&) const;
    void componentsWillMerge(const Graph*, const QSet<ComponentId>&, ComponentId) const;
};

#endif // COMPONENTMANAGER_H
