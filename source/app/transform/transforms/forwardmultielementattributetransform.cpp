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

#include "forwardmultielementattributetransform.h"

#include "transform/transformedgraph.h"
#include "transform/transformattribute.h"

#include "graph/graphmodel.h"

#include "shared/utils/typeidentity.h"

#include <memory>

#include <QObject>

static Alert forwardMultiElementAttributeTransformConfigIsValid(const GraphTransformConfig& config)
{
    if(config.attributeNames().empty())
        return {AlertType::Error, QObject::tr("Invalid parameter")};

    return {AlertType::None, {}};
}

void ForwardMultiElementAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Forward Multi-Element Attribute"));

    auto alert = forwardMultiElementAttributeTransformConfigIsValid(config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    auto attributeName = config().attributeNames().front();

    transformAttribute(_graphModel, target, attributeName,
    [](const auto& attributeValueOfFn, auto headElementId, auto /*elementId*/)
    {
        return attributeValueOfFn(headElementId);
    },
    [](const auto& attributeValueMissingOfFn, auto headElementId, auto /*elementId*/)
    {
        return attributeValueMissingOfFn(headElementId);
    });
}

bool ForwardMultiElementAttributeTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return forwardMultiElementAttributeTransformConfigIsValid(graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> ForwardMultiElementAttributeTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<ForwardMultiElementAttributeTransform>(*graphModel());
}
