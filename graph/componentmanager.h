#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "graph.h"

#include <QObject>

typedef int ComponentId;
const static ComponentId NullComponentId = -1;

class GraphComponent : public QObject, public ReadOnlyGraph
{
    friend class Graph;

    Q_OBJECT
public:
    GraphComponent(ReadOnlyGraph& graph) : _graph(&graph) {}
    GraphComponent(const GraphComponent& other) :
        QObject(other.parent()),
        _graph(other._graph),
        nodeIdsList(other.nodeIdsList),
        edgeIdsList(other.edgeIdsList)
    {}

private:
    ReadOnlyGraph* _graph;
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

class ComponentManager
{
    friend class Graph;

protected:
    Graph* _graph;

public:
    ComponentManager(Graph& graph) :
        _graph(&graph)
    {}
    virtual ~ComponentManager() {}

private:
    virtual void nodeAdded(NodeId nodeId) = 0;
    virtual void nodeWillBeRemoved(NodeId nodeId) = 0;

    virtual void edgeAdded(EdgeId edgeId) = 0;
    virtual void edgeWillBeRemoved(EdgeId edgeId) = 0;

public:
    const Graph& graph() { return *_graph; }

    virtual const QList<ComponentId>& componentIds() const = 0;
    virtual const ReadOnlyGraph& componentById(ComponentId componentId) const = 0;
};

#endif // COMPONENTMANAGER_H
