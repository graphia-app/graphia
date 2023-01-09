/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "combineattributestransform.h"

#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"

#include "shared/utils/typeidentity.h"

#include <memory>

#include <QObject>
#include <QRegularExpression>

static Alert combineAttributesTransformConfigIsValid(const GraphModel& graphModel,
    const GraphTransformConfig& config, bool atApplication = false)
{
    const auto& attributeNames = config.attributeNames();
    if(attributeNames.size() != 2)
        return {AlertType::Error, QObject::tr("Invalid parameters")};

    if(atApplication)
    {
        auto firstAttribute = graphModel.attributeValueByName(attributeNames.at(0));
        auto secondAttribute = graphModel.attributeValueByName(attributeNames.at(1));

        if(firstAttribute.elementType() != secondAttribute.elementType())
            return {AlertType::Error, QObject::tr("Attributes must both be node or edge attributes, not a mixture")};
    }

    auto newAttributeName = config.parameterByName(QStringLiteral("Name"))->valueAsString();
    if(!GraphModel::attributeNameIsValid(newAttributeName))
        return {AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName)};

    return {AlertType::None, {}};
}

void CombineAttributesTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Combine Attributes"));

    auto alert = combineAttributesTransformConfigIsValid(*_graphModel, config(), true);
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    const auto attributeNames = config().attributeNames();

    auto firstAttribute = _graphModel->attributeValueByName(attributeNames.at(0));
    auto secondAttribute = _graphModel->attributeValueByName(attributeNames.at(1));

    auto newAttributeName = config().parameterByName(QStringLiteral("Name"))->valueAsString();
    auto attributeValue = config().parameterByName(QStringLiteral("Attribute Value"))->valueAsString();

    auto combine =
    [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        ElementIdArray<E, QString> newValues(target);
        TypeIdentity typeIdentity;

        for(auto elementId : elementIds)
        {
            const QString firstValue = firstAttribute.stringValueOf(elementId);
            const QString secondValue = secondAttribute.stringValueOf(elementId);

            QString replacement = attributeValue;
            replacement.replace(QStringLiteral(R"(\1)"), firstValue);
            replacement.replace(QStringLiteral(R"(\2)"), secondValue);

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
                .setFlag(AttributeFlag::Searchable);
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

bool CombineAttributesTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return combineAttributesTransformConfigIsValid(*graphModel(), graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> CombineAttributesTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<CombineAttributesTransform>(*graphModel());
}
