#include "attributesynthesistransform.h"

#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"

#include "shared/utils/typeidentity.h"

#include <memory>

#include <QObject>
#include <QRegularExpression>

void AttributeSynthesisTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Attribute Synthesis"));

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto sourceAttribute = _graphModel->attributeValueByName(config().attributeNames().front());

    auto newAttributeName = config().parameterByName(QStringLiteral("New Attribute Name"))->valueAsString();
    auto regexString = config().parameterByName(QStringLiteral("Regular Expression"))->valueAsString();
    auto attributeValue = config().parameterByName(QStringLiteral("Attribute Value"))->valueAsString();

    QRegularExpression regex(regexString);

    auto attributeNameRegex = QRegularExpression(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_ ]*$"));
    if(newAttributeName.isEmpty() || !newAttributeName.contains(attributeNameRegex))
    {
        addAlert(AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName));
        return;
    }

    if(!regex.isValid())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid Regular Expression: %1").arg(regex.errorString()));
        return;
    }

    auto synthesise =
    [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        ElementIdArray<E, QString> newValues(target);
        TypeIdentity typeIdentity;

        for(auto elementId : elementIds)
        {
            QString value = sourceAttribute.stringValueOf(elementId);

            auto match = regex.match(value);
            if(match.hasMatch())
            {
                newValues[elementId] = value.replace(regex, attributeValue);
                typeIdentity.updateType(newValues[elementId]);
            }
        }

        auto& attribute = _graphModel->createAttribute(newAttributeName)
            .setDescription(QObject::tr("An attribute synthesised by the Attribute Synthesis transform."));

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

    if(sourceAttribute.elementType() == ElementType::Node)
        synthesise(target.nodeIds());
    else if(sourceAttribute.elementType() == ElementType::Edge)
        synthesise(target.edgeIds());
}

std::unique_ptr<GraphTransform> AttributeSynthesisTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<AttributeSynthesisTransform>(*graphModel());
}
