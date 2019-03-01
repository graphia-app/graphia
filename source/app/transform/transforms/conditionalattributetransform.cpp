#include "conditionalattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"

#include "graph/graphmodel.h"

#include <algorithm>

#include <QObject>

static Alert conditionalAttributeTransformConfigIsValid(const GraphTransformConfig& config)
{
    auto newAttributeName = config.parameterByName(QStringLiteral("Name"))->valueAsString();
    if(!GraphModel::attributeNameIsValid(newAttributeName))
        return {AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName)};

    return {AlertType::None, {}};
}

void ConditionalAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Boolean Attribute"));

    auto alert = conditionalAttributeTransformConfigIsValid(config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    auto newAttributeName = config().parameterByName(QStringLiteral("Name"))->valueAsString();

    auto synthesise =
    [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        ElementIdArray<E, QString> newValues(target);

        auto conditionFn = CreateConditionFnFor::elementType<E>(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        for(auto elementId : elementIds)
            newValues[elementId] = conditionFn(elementId) ? QObject::tr("True") : QObject::tr("False");

        auto& attribute = _graphModel->createAttribute(newAttributeName)
            .setDescription(QObject::tr("An attribute synthesised by the Boolean Attribute transform."));

        attribute.setStringValueFn([newValues](E elementId) { return newValues[elementId]; })
            .setFlag(AttributeFlag::FindShared)
            .setSearchable(true);
    };

    if(_elementType == ElementType::Node)
        synthesise(target.nodeIds());
    else if(_elementType == ElementType::Edge)
        synthesise(target.edgeIds());
}

bool ConditionalAttributeTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return conditionalAttributeTransformConfigIsValid(graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> ConditionalAttributeTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<ConditionalAttributeTransform>(elementType(), *graphModel());
}
