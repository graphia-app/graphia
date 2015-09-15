#ifndef FILTER_H
#define FILTER_H

#include "graph.h"

#include <vector>
#include <functional>
#include <algorithm>

template<typename Element> using ElementFilterFn = std::function<bool(const Element)>;
using NodeFilterFn = ElementFilterFn<NodeId>;
using EdgeFilterFn = ElementFilterFn<EdgeId>;

class Filter
{
private:
    std::vector<NodeFilterFn> _nodeFilters;
    std::vector<EdgeFilterFn> _edgeFilters;

    template<typename ElementId>
    bool elementIdFiltered(const std::vector<ElementFilterFn<ElementId>>& filters, ElementId elementId) const
    {
        return std::any_of(filters.cbegin(), filters.cend(), [elementId](ElementFilterFn<ElementId> f) { return f(elementId); });
    }

public:
    void addNodeFilter(const NodeFilterFn& f) { _nodeFilters.emplace_back(f); }
    void addEdgeFilter(const EdgeFilterFn& f) { _edgeFilters.emplace_back(f); }

    bool nodeIdFiltered(NodeId nodeId) const { return elementIdFiltered(_nodeFilters, nodeId); }
    bool edgeIdFiltered(EdgeId edgeId) const { return elementIdFiltered(_edgeFilters, edgeId); }
};

#endif // FILTER_H

