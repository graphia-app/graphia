#include "filtertransform.h"
#include "transformedgraph.h"

#include "../graph/componentmanager.h"

#include <algorithm>

static bool componentFiltered(std::vector<ComponentFilterFn> filters, const Graph& component)
{
    return std::any_of(filters.begin(), filters.end(), [&component](ComponentFilterFn f) { return f(component); });
}

void FilterTransform::apply(TransformedGraph& target) const
{
    for(auto edgeId : target.edgeIds())
    {
        if(edgeIdFiltered(edgeId))
            target.removeEdge(edgeId);
    }

    for(auto nodeId : target.nodeIds())
    {
        if(nodeIdFiltered(nodeId))
            target.removeNode(nodeId);
    }

    if(!_componentFilters.empty())
    {
        ComponentManager componentManager(target);

        for(auto componentId : componentManager.componentIds())
        {
            auto component = componentManager.componentById(componentId);
            if(componentFiltered(_componentFilters, *component))
                target.removeNodes(component->nodeIds());
        }
    }
}
