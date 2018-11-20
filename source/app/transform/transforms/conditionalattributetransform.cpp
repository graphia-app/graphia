#include "conditionalattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"

#include "graph/graphmodel.h"

#include <algorithm>

#include <QObject>

void ConditionalAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Boolean Attribute"));

    auto attributeNames = config().referencedAttributeNames();

    if(hasUnknownAttributes(attributeNames, *_graphModel))
        return;

    bool invalidAttributes =
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return !_graphModel->attributeValueByName(attributeName).isValid();
        });

    if(invalidAttributes)
    {
        addAlert(AlertType::Error, QObject::tr("One more more invalid attributes"));
        return;
    }

    auto newAttributeName = config().parameterByName(QStringLiteral("New Attribute Name"))->valueAsString();

    auto attributeNameRegex = QRegularExpression(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_ ]*$"));
    if(newAttributeName.isEmpty() || !newAttributeName.contains(attributeNameRegex))
    {
        addAlert(AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName));
        return;
    }

    bool ignoreTails =
    std::any_of(attributeNames.begin(), attributeNames.end(),
    [this](const auto& attributeName)
    {
        return _graphModel->attributeValueByName(attributeName).testFlag(AttributeFlag::IgnoreTails);
    });

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
        {
            if(ignoreTails && target.typeOf(elementId) == MultiElementType::Tail)
                continue;

            newValues[elementId] = conditionFn(elementId) ? QObject::tr("True") : QObject::tr("False");
        }

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

std::unique_ptr<GraphTransform> ConditionalAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<ConditionalAttributeTransform>(elementType(), *graphModel());
}
