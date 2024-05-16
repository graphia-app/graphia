/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "attributesynthesistransform.h"

#include "app/transform/transformedgraph.h"
#include "app/graph/graphmodel.h"

#include "shared/utils/typeidentity.h"

#include <memory>

#include <QObject>
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

static Alert attributeSynthesisTransformConfigIsValid(const GraphTransformConfig& config)
{
    if(config.attributeNames().empty())
        return {AlertType::Error, QObject::tr("Invalid parameter")};

    auto newAttributeName = config.parameterByName(u"Name"_s)->valueAsString();
    if(!GraphModel::attributeNameIsValid(newAttributeName))
        return {AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName)};

    const QRegularExpression regex(config.parameterByName(u"Regular Expression"_s)->valueAsString());
    if(!regex.isValid())
        return {AlertType::Error, QObject::tr("Invalid Regular Expression: %1").arg(regex.errorString())};

    return {AlertType::None, {}};
}

void AttributeSynthesisTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Attribute Synthesis"));

    auto alert = attributeSynthesisTransformConfigIsValid(config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    auto sourceAttribute = _graphModel->attributeValueByName(config().attributeNames().front());

    auto newAttributeName = config().parameterByName(u"Name"_s)->valueAsString();
    QRegularExpression regex(config().parameterByName(u"Regular Expression"_s)->valueAsString());
    auto attributeValue = config().parameterByName(u"Attribute Value"_s)->valueAsString();

    auto synthesise =
    [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        ElementIdArray<E, QString> newValues(target);
        TypeIdentity typeIdentity;

        for(auto elementId : elementIds)
        {
            QString value = sourceAttribute.stringValueOf(elementId);

            auto match = regex.match(value); // clazy:exclude=use-static-qregularexpression
            if(match.hasMatch())
            {
                newValues[elementId] = value.replace(regex, attributeValue); // clazy:exclude=use-static-qregularexpression
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
                .setFlag(AttributeFlag::Searchable);
            break;

        case TypeIdentity::Type::Int:
        {
            ElementIdArray<E, int> newIntValues(target);
            for(auto elementId : elementIds)
                newIntValues[elementId] = newValues[elementId].toInt();

            attribute.setIntValueFn([newIntValues](E elementId) { return newIntValues[elementId]; })
                .setFlag(AttributeFlag::AutoRange);
            break;
        }

        case TypeIdentity::Type::Float:
        {
            ElementIdArray<E, double> newFloatValues(target);
            for(auto elementId : elementIds)
                newFloatValues[elementId] = newValues[elementId].toDouble();

            attribute.setFloatValueFn([newFloatValues](E elementId) { return newFloatValues[elementId]; })
                .setFlag(AttributeFlag::AutoRange);
            break;
        }
        }
    };

    if(sourceAttribute.elementType() == ElementType::Node)
        synthesise(target.nodeIds());
    else if(sourceAttribute.elementType() == ElementType::Edge)
        synthesise(target.edgeIds());
}

bool AttributeSynthesisTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return attributeSynthesisTransformConfigIsValid(graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> AttributeSynthesisTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<AttributeSynthesisTransform>(*graphModel());
}
