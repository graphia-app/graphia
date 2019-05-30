#ifndef GRAPHFILTER_H
#define GRAPHFILTER_H

#include "shared/graph/elementid_containers.h"

#include <vector>
#include <functional>
#include <algorithm>

class GraphFilter
{
private:
    std::vector<NodeConditionFn> _nodeFilters;
    std::vector<EdgeConditionFn> _edgeFilters;

    template<typename ElementId>
    static bool elementIdFiltered(const std::vector<ElementConditionFn<ElementId>>& filters, ElementId elementId)
    {
        return std::any_of(filters.begin(), filters.end(),
        [elementId](const auto& filter)
        {
            return filter(elementId);
        });
    }

public:
    void addNodeFilter(const NodeConditionFn& f) { if(f != nullptr) _nodeFilters.emplace_back(f); }
    void addEdgeFilter(const EdgeConditionFn& f) { if(f != nullptr) _edgeFilters.emplace_back(f); }

    bool hasNodeFilters() const { return !_nodeFilters.empty(); }
    bool hasEdgeFilters() const { return !_edgeFilters.empty(); }

    bool nodeIdFiltered(NodeId nodeId) const { return elementIdFiltered(_nodeFilters, nodeId); }
    bool edgeIdFiltered(EdgeId edgeId) const { return elementIdFiltered(_edgeFilters, edgeId); }
};

#endif // GRAPHFILTER_H

