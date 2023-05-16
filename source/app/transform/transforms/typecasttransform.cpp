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

#include "typecasttransform.h"

#include "graph/graphmodel.h"

#include <memory>

#include <QObject>

class TransformedGraph;

using namespace Qt::Literals::StringLiterals;

static Alert typecastTransformConfigIsValid(const GraphTransformConfig& config)
{
    if(config.attributeNames().empty())
        return {AlertType::Error, QObject::tr("Invalid parameter")};

    auto newAttributeName = config.parameterByName(u"Name"_s)->valueAsString();
    if(!GraphModel::attributeNameIsValid(newAttributeName))
        return {AlertType::Error, QObject::tr("Invalid Attribute Name: '%1'").arg(newAttributeName)};

    return {AlertType::None, {}};
}

void TypeCastTransform::apply(TransformedGraph&)
{
    setPhase(QObject::tr("Type Cast"));

    auto alert = typecastTransformConfigIsValid(config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto sourceAttributeName = config().attributeNames().front();
    const auto* sourceAttribute = _graphModel->attributeByName(sourceAttributeName);

    auto type = config().parameterByName(u"Type"_s)->valueAsString();

    auto newAttributeName = config().parameterByName(u"Name"_s)->valueAsString();

    auto& attribute = _graphModel->createAttribute(newAttributeName)
        .setDescription(QObject::tr("A copy of the attribute '%1' with its type converted to '%2'.")
        .arg(sourceAttributeName, type));

    auto cast = [&]<typename E>
    {
        if(type == u"Integer"_s)
            attribute.setIntValueFn([sourceAttribute](E elementId) { return sourceAttribute->intValueOf(elementId); });
        else if(type == u"Float"_s)
            attribute.setFloatValueFn([sourceAttribute](E elementId) { return sourceAttribute->floatValueOf(elementId); });
        else if(type == u"String"_s)
        {
            attribute.setStringValueFn([sourceAttribute](E elementId) { return sourceAttribute->stringValueOf(elementId); })
                .setFlag(AttributeFlag::FindShared)
                .setFlag(AttributeFlag::Searchable);
        }
    };

    if(sourceAttribute->elementType() == ElementType::Node)
        cast.operator()<NodeId>();
    else if(sourceAttribute->elementType() == ElementType::Edge)
        cast.operator()<EdgeId>();
}

bool TypeCastTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return typecastTransformConfigIsValid(graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> TypeCastTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<TypeCastTransform>(*graphModel());
}
