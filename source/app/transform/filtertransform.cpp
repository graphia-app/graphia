#include "filtertransform.h"
#include "transformedgraph.h"
#include "conditionfncreator.h"

#include "graph/componentmanager.h"

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

bool FilterTransform::apply(TransformedGraph& target) const
{
    bool changed = false;

    target.setPhase(QObject::tr("Filtering"));

    // The elements to be filtered are calculated first and then removed, because
    // removing elements during the filtering could affect the result of filter functions

    if(hasEdgeFilters())
    {
        std::vector<EdgeId> removees;

        for(auto edgeId : target.edgeIds())
        {
            if(!edgeIdFiltered(edgeId) != !_invert)
                removees.push_back(edgeId);
        }

        changed = !removees.empty();
        for(auto edgeId : removees)
            target.mutableGraph().removeEdge(edgeId);
    }

    if(hasNodeFilters())
    {
        std::vector<NodeId> removees;

        for(auto nodeId : target.nodeIds())
        {
            if(!nodeIdFiltered(nodeId) != !_invert)
                removees.push_back(nodeId);
        }

        changed = !removees.empty();
        for(auto nodeId : removees)
            target.mutableGraph().removeNode(nodeId);
    }

    if(!_componentFilters.empty())
    {
        ComponentManager componentManager(target);

        for(auto componentId : componentManager.componentIds())
        {
            auto component = componentManager.componentById(componentId);
            if(!componentFiltered(_componentFilters, *component) != !_invert)
            {
                changed = true;
                target.mutableGraph().removeNodes(target.mutableGraph().mergedNodeIdsForNodeIds(component->nodeIds()));
            }
        }
    }

    return changed;
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const GraphTransformConfig& graphTransformConfig,
                                                               const std::map<QString, DataField>& dataFields) const
{
    auto filterTransform = std::make_unique<FilterTransform>(_invert);

    switch(elementType())
    {
    case ElementType::Node:
        filterTransform->addNodeFilter(CreateConditionFnFor::node(dataFields, graphTransformConfig._condition));
        break;

    case ElementType::Edge:
        filterTransform->addEdgeFilter(CreateConditionFnFor::edge(dataFields, graphTransformConfig._condition));
        break;

    case ElementType::Component:
        filterTransform->addComponentFilter(CreateConditionFnFor::component(dataFields, graphTransformConfig._condition));
        break;

    default:
        return nullptr;
    }

    if(!filterTransform->hasNodeFilters() && !filterTransform->hasEdgeFilters() && !filterTransform->hasComponentFilters())
        return nullptr; // No filters defined

    return std::move(filterTransform); //FIXME std::move required because of clang bug
}
