#include "graphtransform.h"
#include "transformedgraph.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "shared/utils/container.h"

bool GraphTransform::applyAndUpdate(TransformedGraph& target) const
{
    bool anyChange = false;
    bool change = false;

    do
    {
        target.clearPhase();
        change = apply(target);
        anyChange = anyChange || change;
        target.update();
    } while(repeating() && change);

    return anyChange;
}

bool GraphTransform::hasUnknownAttributes(const std::vector<QString>& referencedAttributes,
                                          const GraphModel& graphModel) const
{
    bool unknownAttributes = false;

    for(const auto& referencedAttributeName : referencedAttributes)
    {
        if(!graphModel.attributeExists(referencedAttributeName))
        {
            addAlert(AlertType::Error, QObject::tr(R"(Unknown Attribute: "%1")").arg(referencedAttributeName));
            unknownAttributes = true;
        }
    }

    return unknownAttributes;
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
