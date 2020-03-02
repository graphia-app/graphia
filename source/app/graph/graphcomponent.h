#ifndef GRAPHCOMPONENT_H
#define GRAPHCOMPONENT_H

#include "shared/graph/igraphcomponent.h"

class GraphComponent : public IGraphComponent
{
    friend class ComponentManager;

public:
    explicit GraphComponent(const Graph* graph) : _graph(graph) {}
    GraphComponent(const GraphComponent& other) :
        _graph(other._graph),
        _nodeIds(other._nodeIds),
        _edgeIds(other._edgeIds)
    {}

    GraphComponent& operator=(const GraphComponent& other)
    {
        if(&other == this)
            return *this;

        _graph = other._graph;
        _nodeIds = other._nodeIds;
        _edgeIds = other._edgeIds;

        return *this;
    }

private:
    const Graph* _graph;
    std::vector<NodeId> _nodeIds;
    std::vector<EdgeId> _edgeIds;

public:
    const std::vector<NodeId>& nodeIds() const override { return _nodeIds; }
    const std::vector<EdgeId>& edgeIds() const override { return _edgeIds; }
    const IGraph& graph() const override { return *_graph; }
};

#endif // GRAPHCOMPONENT_H
