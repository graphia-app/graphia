#include "filtertransform.h"
#include "transformedgraph.h"

#include "../graph/componentmanager.h"

#include <algorithm>

static bool componentFiltered(const std::vector<ComponentFilterFn>& filters, const Graph& component)
{
    for(auto& filter : filters)
    {
        if(filter(component))
            return true;
    }

    return false;
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
