#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "graphtransform.h"
#include "../graph/graph.h"

#include <vector>
#include <functional>

template<typename Element> using ElementFilterFn = std::function<bool(const Element)>;
using NodeFilterFn = ElementFilterFn<NodeId>;
using EdgeFilterFn = ElementFilterFn<EdgeId>;
using ComponentFilterFn = std::function<bool(const Graph&)>;

class FilterTransform : public GraphTransform
{
private:
    void filterComponents(TransformedGraph& target) const;

public:
    void apply(const Graph &source, TransformedGraph &target) const;
    void apply(TransformedGraph &target) const;

    void addNodeFilter(const NodeFilterFn& f) { _nodeFilters.emplace_back(f); }
    void addEdgeFilter(const EdgeFilterFn& f) { _edgeFilters.emplace_back(f); }
    void addComponentFilter(const ComponentFilterFn& f) { _componentFilters.emplace_back(f); }

private:
    std::vector<NodeFilterFn> _nodeFilters;
    std::vector<EdgeFilterFn> _edgeFilters;
    std::vector<ComponentFilterFn> _componentFilters;
};

#endif // FILTERTRANSFORM_H
