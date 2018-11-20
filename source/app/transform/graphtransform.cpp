#include "graphtransform.h"
#include "transformedgraph.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "shared/utils/container.h"

static bool hasUnknownAttributes(const std::vector<QString>& attributeNames,
    const GraphModel& graphModel, const GraphTransform& transform)
{
    bool unknownAttributes = false;

    for(const auto& attributeName : attributeNames)
    {
        if(!graphModel.attributeExists(attributeName))
        {
            transform.addAlert(AlertType::Error, QObject::tr(R"(Unknown Attribute: "%1")").arg(attributeName));
            unknownAttributes = true;
        }
    }

    return unknownAttributes;
}

static bool hasInvalidAttributes(const std::vector<QString>& attributeNames,
    const GraphModel& graphModel, const GraphTransform& transform)
{
    bool invalidAttributes =
    std::any_of(attributeNames.begin(), attributeNames.end(),
    [&](const auto& attributeName)
    {
        return !graphModel.attributeValueByName(attributeName).isValid();
    });

    if(invalidAttributes)
    {
        transform.addAlert(AlertType::Error, QObject::tr("One more more invalid attributes"));
        return true;
    }

    return false;
}

bool GraphTransform::applyAndUpdate(TransformedGraph& target, const GraphModel& graphModel) const
{
    bool anyChange = false;
    bool change = false;

    do
    {
        target.resetChangeOccurred({});
        target.clearPhase();

        auto attributeNames = config().referencedAttributeNames();

        if(hasUnknownAttributes(attributeNames, graphModel, *this))
            continue;

        if(hasInvalidAttributes(attributeNames, graphModel, *this))
            continue;

        apply(target);
        target.update();
        change = target.changeOccurred({});
        anyChange = anyChange || change;
    } while(repeating() && change);

    return anyChange;
}

GraphTransformAttributeParameter GraphTransformFactory::attributeParameter(const QString& parameterName) const
{
    for(const auto& attributeParameter : attributeParameters())
    {
        if(attributeParameter.name() == parameterName)
            return attributeParameter;
    }

    return {};
}

GraphTransformParameter GraphTransformFactory::parameter(const QString& parameterName) const
{
    for(const auto& parameter : parameters())
    {
        if(parameter.name() == parameterName)
            return parameter;
    }

    return {};
}
