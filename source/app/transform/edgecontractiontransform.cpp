#include "edgecontractiontransform.h"
#include "transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include <QObject>

bool EdgeContractionTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    auto attributeNames = _graphTransformConfig.attributeNames();

    bool unknownAttributes =
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return !u::contains(*_attributes, attributeName);
        });

    if(unknownAttributes)
        return false;

    bool ignoreTails =
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return _attributes->at(attributeName).testFlag(AttributeFlag::IgnoreTails);
        });

    auto conditionFn = CreateConditionFnFor::edge(*_attributes, _graphTransformConfig._condition);
    if(conditionFn == nullptr)
        return false;

    EdgeIdSet edgeIdsToContract;

    for(auto edgeId : target.edgeIds())
    {
        if(ignoreTails && target.typeOf(edgeId) == MultiElementType::Tail)
            continue;

        if(conditionFn(edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.mutableGraph().contractEdges(edgeIdsToContract);

    return !edgeIdsToContract.empty();
}

std::unique_ptr<GraphTransform> EdgeContractionTransformFactory::create(const GraphTransformConfig& graphTransformConfig) const
{
    auto edgeContractionTransform = std::make_unique<EdgeContractionTransform>(graphModel()->attributes(), graphTransformConfig);

    if(!conditionIsValid(elementType(), graphModel()->attributes(), graphTransformConfig._condition))
        return nullptr;

    return std::move(edgeContractionTransform); //FIXME std::move required because of clang bug
}
