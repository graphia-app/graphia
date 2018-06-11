#include "combineattributestransform.h"

#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"

#include "shared/utils/typeidentity.h"

#include <memory>

#include <QObject>
#include <QRegularExpression>

void CombineAttributesTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Combine Attributes"));

    const auto attributeNames = config().attributeNames();

    if(attributeNames.size() != 2)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameters"));
        return;
    }

    auto firstAttributeName = attributeNames.at(0);
    auto secondAttributeName = attributeNames.at(1);

    if(hasUnknownAttributes({firstAttributeName, secondAttributeName}, *_graphModel))
        return;

    auto firstAttribute = _graphModel->attributeValueByName(firstAttributeName);
    auto secondAttribute = _graphModel->attributeValueByName(secondAttributeName);

    if(!firstAttribute.isValid() || !secondAttribute.isValid())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid attribute"));
        return;
    }

    if(firstAttribute.elementType() != secondAttribute.elementType())
    {
        addAlert(AlertType::Error, QObject::tr("Attributes must both be node or edge attributes, not a mixture"));
        return;
    }

    auto newAttributeName = config().parameterByName(QStringLiteral("New Attribute Name"))->valueAsString();
    auto attributeValue = config().parameterByName(QStringLiteral("Attribute Value"))->valueAsString();

    auto attributeNameRegex = QRegularExpression(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_ ]*$"));
    if(newAttributeName.isEmpty() || !newAttributeName.contains(attributeNameRegex))
    {
        addAlert(AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName));
        return;
    }

    auto combine =
    [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        ElementIdArray<E, QString> newValues(target);
        TypeIdentity typeIdentity;

        for(auto elementId : elementIds)
        {
            QString firstValue = firstAttribute.stringValueOf(elementId);
            QString secondValue = secondAttribute.stringValueOf(elementId);

            QString replacement = attributeValue;
            replacement.replace(QStringLiteral("\\1"), firstValue);
            replacement.replace(QStringLiteral("\\2"), secondValue);

            newValues[elementId] = replacement;
            typeIdentity.updateType(newValues[elementId]);
        }

        auto& attribute = _graphModel->createAttribute(newAttributeName)
            .setDescription(QObject::tr("An attribute synthesised by the Combine Attributes transform."));

        switch(typeIdentity.type())
        {
        default:
        case TypeIdentity::Type::String:
        case TypeIdentity::Type::Unknown:
            attribute.setStringValueFn([newValues](E elementId) { return newValues[elementId]; })
                .setFlag(AttributeFlag::FindShared)
                .setSearchable(true);
            break;

        case TypeIdentity::Type::Int:
        {
            ElementIdArray<E, int> newIntValues(target);
            for(auto elementId : elementIds)
                newIntValues[elementId] = newValues[elementId].toInt();

            attribute.setIntValueFn([newIntValues](E elementId) { return newIntValues[elementId]; });
            break;
        }

        case TypeIdentity::Type::Float:
        {
            ElementIdArray<E, double> newFloatValues(target);
            for(auto elementId : elementIds)
                newFloatValues[elementId] = newValues[elementId].toDouble();

            attribute.setFloatValueFn([newFloatValues](E elementId) { return newFloatValues[elementId]; });
            break;
        }
        }
    };

    if(firstAttribute.elementType() == ElementType::Node)
        combine(target.nodeIds());
    else if(firstAttribute.elementType() == ElementType::Edge)
        combine(target.edgeIds());
}

std::unique_ptr<GraphTransform> CombineAttributesTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<CombineAttributesTransform>(*graphModel());
}
