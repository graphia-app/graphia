/* Copyright © 2013-2021 Graphia Technologies Ltd.
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
            .setFlag(AttributeFlag::Searchable);
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
