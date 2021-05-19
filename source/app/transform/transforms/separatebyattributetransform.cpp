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

#include "separatebyattributetransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

void SeparateByAttributeTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto attributeName = config().attributeNames().front();

    GraphTransformConfig::TerminalCondition condition
    {
        QStringLiteral("$source.%1").arg(attributeName),
        ConditionFnOp::Equality::NotEqual,
        QStringLiteral("$target.%1").arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return;
    }

    EdgeIdSet edgeIdsToRemove;

    for(auto edgeId : target.edgeIds())
    {
        if(conditionFn(edgeId))
            edgeIdsToRemove.insert(edgeId);
    }

    target.mutableGraph().removeEdges(edgeIdsToRemove);
}

std::unique_ptr<GraphTransform> SeparateByAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<SeparateByAttributeTransform>(*graphModel());
}
