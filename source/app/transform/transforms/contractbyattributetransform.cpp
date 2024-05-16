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

#include "contractbyattributetransform.h"
#include "app/transform/transformedgraph.h"
#include "app/attributes/conditionfncreator.h"
#include "app/graph/graphmodel.h"

#include "shared/utils/string.h"

#include <QObject>

using namespace Qt::Literals::StringLiterals;

void ContractByAttributeTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Contracting"));

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto attributeName = config().attributeNames().front();

    GraphTransformConfig::TerminalCondition condition
    {
        u"$source.%1"_s.arg(attributeName),
        ConditionFnOp::Equality::Equal,
        u"$target.%1"_s.arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(*_graphModel, condition);
    if(conditionFn == nullptr)
    {
        addAlert(AlertType::Error, QObject::tr("Invalid condition"));
        return;
    }

    EdgeIdSet edgeIdsToContract;

    for(auto edgeId : target.edgeIds())
    {
        if(conditionFn(edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.mutableGraph().contractEdges(edgeIdsToContract);
}

std::unique_ptr<GraphTransform> ContractByAttributeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<ContractByAttributeTransform>(*graphModel());
}
