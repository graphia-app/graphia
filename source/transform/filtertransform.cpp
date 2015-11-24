#include "filtertransform.h"
#include "transformedgraph.h"

#include "../graph/componentmanager.h"

#include <algorithm>

#include <QObject>

static bool componentFiltered(const std::vector<ComponentConditionFn>& filters, const GraphComponent& component)
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
    target.setSubPhase(QObject::tr("Filtering"));

    //FIXME probably need to create lists first, then do the removals in a second step
    if(hasEdgeFilters())
    {
        for(auto edgeId : target.edgeIds())
        {
            if(edgeIdFiltered(edgeId))
                target.removeEdge(edgeId);
        }
    }

    if(hasNodeFilters())
    {
        for(auto nodeId : target.nodeIds())
        {
            if(nodeIdFiltered(nodeId))
                target.removeNode(nodeId);
        }
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

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const NodeConditionFn& conditionFn) const
{
    auto filterTransform = std::make_unique<FilterTransform>();
    filterTransform->addNodeFilter(conditionFn);

    return std::move(filterTransform);
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const EdgeConditionFn& conditionFn) const
{
    auto filterTransform = std::make_unique<FilterTransform>();
    filterTransform->addEdgeFilter(conditionFn);

    return std::move(filterTransform);
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const ComponentConditionFn& conditionFn) const
{
    auto filterTransform = std::make_unique<FilterTransform>();
    filterTransform->addComponentFilter(conditionFn);

    return std::move(filterTransform);
}
