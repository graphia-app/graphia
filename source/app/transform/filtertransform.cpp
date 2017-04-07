#include "filtertransform.h"
#include "transformedgraph.h"
#include "attributes/conditionfncreator.h"

#include "graph/graphmodel.h"
#include "graph/componentmanager.h"

#include <algorithm>

#include <QObject>

static bool componentFiltered(const std::vector<ComponentConditionFn>& filters, const IGraphComponent& component)
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
            if(_ignoreTails && target.typeOf(edgeId) == MultiElementType::Tail)
                continue;

            if(u::exclusiveOr(edgeIdFiltered(edgeId), _invert))
                removees.push_back(edgeId);
        }

        changed = !removees.empty() || changed;
        for(auto edgeId : removees)
            target.mutableGraph().removeEdge(edgeId);
    }

    if(hasNodeFilters())
    {
        std::vector<NodeId> removees;

        for(auto nodeId : target.nodeIds())
        {
            if(_ignoreTails && target.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            if(u::exclusiveOr(nodeIdFiltered(nodeId), _invert))
                removees.push_back(nodeId);
        }

        changed = !removees.empty() || changed;
        for(auto nodeId : removees)
            target.mutableGraph().removeNode(nodeId);
    }

    if(!_componentFilters.empty())
    {
        ComponentManager componentManager(target);

        for(auto componentId : componentManager.componentIds())
        {
            auto component = componentManager.componentById(componentId);
            if(u::exclusiveOr(componentFiltered(_componentFilters, *component), _invert))
            {
                changed = true;
                target.mutableGraph().removeNodes(target.mutableGraph().mergedNodeIdsForNodeIds(component->nodeIds()));
            }
        }
    }

    return changed;
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const GraphTransformConfig& graphTransformConfig) const
{
    auto filterTransform = std::make_unique<FilterTransform>(_invert);

    switch(elementType())
    {
    case ElementType::Node:
    {
        auto conditionFn = CreateConditionFnFor::node(graphModel()->attributes(), graphTransformConfig._condition);
        if(conditionFn == nullptr)
            return nullptr;

        filterTransform->addNodeFilter(conditionFn);
        break;
    }

    case ElementType::Edge:
    {
        auto conditionFn = CreateConditionFnFor::edge(graphModel()->attributes(), graphTransformConfig._condition);
        if(conditionFn == nullptr)
            return nullptr;

        filterTransform->addEdgeFilter(conditionFn);
        break;
    }

    case ElementType::Component:
    {
        auto conditionFn = CreateConditionFnFor::component(graphModel()->attributes(), graphTransformConfig._condition);
        if(conditionFn == nullptr)
            return nullptr;

        filterTransform->addComponentFilter(conditionFn);
        break;
    }

    default:
        return nullptr;
    }

    if(!filterTransform->hasNodeFilters() && !filterTransform->hasEdgeFilters() && !filterTransform->hasComponentFilters())
        return nullptr; // No filters defined

    auto attributeNames = graphTransformConfig.attributeNames();
    filterTransform->setIgnoreTails(
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return this->graphModel()->attribute(attributeName).testFlag(AttributeFlag::IgnoreTails);
        }));

    return std::move(filterTransform); //FIXME std::move required because of clang bug
}
