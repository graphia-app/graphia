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

    // The elements to be filtered are calculated first and then removed, because
    // removing elements during the filtering could affect the result of filter functions

    if(hasEdgeFilters())
    {
        std::vector<EdgeId> removees;

        for(auto edgeId : target.edgeIds())
        {
            if(edgeIdFiltered(edgeId))
                removees.push_back(edgeId);
        }

        for(auto edgeId : removees)
            target.removeEdge(edgeId);
    }

    if(hasNodeFilters())
    {
        std::vector<NodeId> removees;

        for(auto nodeId : target.nodeIds())
        {
            if(nodeIdFiltered(nodeId))
                removees.push_back(nodeId);
        }

        for(auto nodeId : removees)
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
