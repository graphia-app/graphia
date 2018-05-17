#include "edgecontractiontransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

bool EdgeContractionTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    auto attributeNames = config().referencedAttributeNames();

    if(hasUnknownAttributes(attributeNames, *_graphModel))
        return false;

    bool ignoreTails =
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return _graphModel->attributeValueByName(attributeName).testFlag(AttributeFlag::IgnoreTails);
        });

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, config()._condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return false;
    }

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

std::unique_ptr<GraphTransform> EdgeContractionTransformFactory::create(const GraphTransformConfig&) const
{
    auto edgeContractionTransform = std::make_unique<EdgeContractionTransform>(*graphModel());

    return std::move(edgeContractionTransform); //FIXME std::move required because of clang bug
}
