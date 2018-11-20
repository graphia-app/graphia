#include "filtertransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"

#include "graph/graphmodel.h"
#include "graph/graphcomponent.h"
#include "graph/componentmanager.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"

#include <algorithm>

#include <QObject>

void FilterTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Filtering"));

    auto attributeNames = config().referencedAttributeNames();

    bool ignoreTails =
    std::any_of(attributeNames.begin(), attributeNames.end(),
    [this](const auto& attributeName)
    {
        return _graphModel->attributeValueByName(attributeName).testFlag(AttributeFlag::IgnoreTails);
    });

    // The elements to be filtered are calculated first and then removed, because
    // removing elements during the filtering could affect the result of filter functions

    switch(_elementType)
    {
    case ElementType::Node:
    {
        auto conditionFn = CreateConditionFnFor::node(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        std::vector<NodeId> removees;

        for(auto nodeId : target.nodeIds())
        {
            if(ignoreTails && target.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            if(u::exclusiveOr(conditionFn(nodeId), _invert))
                removees.push_back(nodeId);
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            target.setProgress((progress++ * 100) / numRemovees);
        }
        break;
    }

    case ElementType::Edge:
    {
        auto conditionFn = CreateConditionFnFor::edge(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        std::vector<EdgeId> removees;

        for(auto edgeId : target.edgeIds())
        {
            if(ignoreTails && target.typeOf(edgeId) == MultiElementType::Tail)
                continue;

            if(u::exclusiveOr(conditionFn(edgeId), _invert))
                removees.push_back(edgeId);
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto edgeId : removees)
        {
            target.mutableGraph().removeEdge(edgeId);
            target.setProgress((progress++ * 100) / numRemovees);
        }
        break;
    }

    case ElementType::Component:
    {
        auto conditionFn = CreateConditionFnFor::component(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        ComponentManager componentManager(target);
        std::vector<NodeId> removees;

        for(auto componentId : componentManager.componentIds())
        {
            auto component = componentManager.componentById(componentId);
            if(u::exclusiveOr(conditionFn(*component), _invert))
            {
                auto nodeIds = target.mutableGraph().mergedNodeIdsForNodeIds(component->nodeIds());
                removees.insert(removees.end(), nodeIds.begin(), nodeIds.end());
            }
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            target.setProgress((progress++ * 100) / numRemovees);
        }
        break;
    }

    default:
        break;
    }
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<FilterTransform>(elementType(), *graphModel(), _invert);
}
