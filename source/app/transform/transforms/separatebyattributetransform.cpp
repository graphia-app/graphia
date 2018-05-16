#include "separatebyattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

bool SeparateByAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    const auto attributeNames = config().attributeNames();

    if(attributeNames.empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return false;
    }

    auto attributeName = attributeNames.front();

    if(hasUnknownAttributes({attributeName}, u::toQStringVector(_graphModel->availableAttributes())))
        return false;

    bool ignoreTails = _graphModel->attributeValueByName(attributeName).testFlag(AttributeFlag::IgnoreTails);

    GraphTransformConfig::TerminalCondition condition
    {
        QStringLiteral("$source.%1").arg(attributeName),
        ConditionFnOp::Equality::NotEqual,
        QStringLiteral("$target.%1").arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return false;
    }

    EdgeIdSet edgeIdsToRemove;

    for(auto edgeId : target.edgeIds())
    {
        if(ignoreTails && target.typeOf(edgeId) == MultiElementType::Tail)
            continue;

        if(conditionFn(edgeId))
            edgeIdsToRemove.insert(edgeId);
    }

    target.mutableGraph().removeEdges(edgeIdsToRemove);

    return !edgeIdsToRemove.empty();
}

std::unique_ptr<GraphTransform> SeparateByAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<SeparateByAttributeTransform>(*graphModel());
}
