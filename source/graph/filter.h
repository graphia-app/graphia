#ifndef FILTER_H
#define FILTER_H

#include "graph.h"

#include <vector>
#include <functional>
#include <algorithm>

class Filter
{
private:
    std::vector<NodeConditionFn> _nodeFilters;
    std::vector<EdgeConditionFn> _edgeFilters;

    template<typename ElementId>
    bool elementIdFiltered(const std::vector<ElementConditionFn<ElementId>>& filters, ElementId elementId) const
    {
        for(auto& filter : filters)
        {
            if(filter(elementId))
                return true;
        }

        return false;
    }

protected:
    bool hasNodeFilters() const { return !_nodeFilters.empty(); }
    bool hasEdgeFilters() const { return !_edgeFilters.empty(); }

public:
    void addNodeFilter(const NodeConditionFn& f) { _nodeFilters.emplace_back(f); }
    void addEdgeFilter(const EdgeConditionFn& f) { _edgeFilters.emplace_back(f); }

    bool nodeIdFiltered(NodeId nodeId) const { return elementIdFiltered(_nodeFilters, nodeId); }
    bool edgeIdFiltered(EdgeId edgeId) const { return elementIdFiltered(_edgeFilters, edgeId); }
};

#endif // FILTER_H

