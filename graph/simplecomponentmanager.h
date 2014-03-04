#ifndef SIMPLECOMPONENTMANAGER_H
#define SIMPLECOMPONENTMANAGER_H

#include "componentmanager.h"
#include "grapharray.h"

#include <QMap>
#include <QQueue>
#include <QList>
#include <QSet>

/*
This class is somewhat sub-optimal in that it simply does a depth first
search on the graph resulting in O(n) performance. In the static case this
is perfectly acceptable as it only needs to be performed once, but in the
dynamic case we potentially have many graph changes in a short period of
time meaning that a graph's component configuration may change frequently.

O(log^2 n) performance is achievable using the Holm, de Lichtenberg and
Thorup algorithm explained in the paper "Poly-Logarithmic Deterministic
Fully-Dynamic Algorithms for Connectivity, Minimum Spanning Tree, 2-Edge,
and Biconnectivity". So the plan is to implement this as a replacement
for SimpleComponentManager in future.

Some links:
http://www.mpi-inf.mpg.de/departments/d1/teaching/ss12/AdvancedGraphAlgorithms/Slides08.pdf
http://courses.csail.mit.edu/6.851/spring07/scribe/lec05.pdf
http://www.youtube.com/watch?v=5NEzZPYs04c#t=3076
*/

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

    ComponentId generateComponentId();
    void releaseComponentId(ComponentId componentId);
    void queueGraphComponentUpdate(ComponentId componentId);
    void updateGraphComponent(ComponentId componentId);
    void removeGraphComponent(ComponentId componentId);

public:
    SimpleComponentManager(Graph& graph) :
        ComponentManager(graph),
        nextComponentId(0),
        nodesComponentId(graph, NullNodeId),
        edgesComponentId(graph, NullEdgeId)
    {}

private:
    // A more sophisticated implementation may make use of these
    void nodeAdded(NodeId) {}
    void nodeWillBeRemoved(NodeId) {}

    void edgeAdded(EdgeId) {}
    void edgeWillBeRemoved(EdgeId) {}

    void graphChanged(const Graph*);

    void updateComponents();

    int componentArrayCapacity() const { return nextComponentId; }

public:
    const QList<ComponentId>& componentIds() const;
    const GraphComponent* componentById(ComponentId componentId);
    ComponentId componentIdOfNode(NodeId nodeId) const;
    ComponentId componentIdOfEdge(EdgeId edgeId) const;
};

#endif // SIMPLECOMPONENTMANAGER_H
