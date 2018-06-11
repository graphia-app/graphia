#include "contractbyattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

void ContractByAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    const auto attributeNames = config().attributeNames();

    if(attributeNames.empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto attributeName = attributeNames.front();

    if(hasUnknownAttributes({attributeName}, *_graphModel))
        return;

    auto attribute = _graphModel->attributeValueByName(attributeName);

    if(!attribute.isValid())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid attribute"));
        return;
    }

    bool ignoreTails = attribute.testFlag(AttributeFlag::IgnoreTails);

    GraphTransformConfig::TerminalCondition condition
    {
        QStringLiteral("$source.%1").arg(attributeName),
        ConditionFnOp::Equality::Equal,
        QStringLiteral("$target.%1").arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return;
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
}

std::unique_ptr<GraphTransform> ContractByAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<ContractByAttributeTransform>(*graphModel());
}
