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

#include "sharedtextvisualisationchannel.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"

#include "app/transform/transformedgraph.h"

#include "app/transform/graphtransformconfig.h"
#include "app/attributes/conditionfncreator.h"

#include <QObject>

TextVisuals SharedTextVisualisationChannel::textVisuals(const QString& attributeName,
    const GraphModel& graphModel, const TransformedGraph& graph)
{
    Q_ASSERT(graphModel.attributeExists(attributeName));
    if(!graphModel.attributeExists(attributeName))
        return {};

    TextVisuals textVisuals;

    const auto* attribute = graphModel.attributeByName(attributeName);
    auto contractedGraph = graph.mutableGraph();

    GraphTransformConfig::TerminalCondition condition
    {
        u"$source.%1"_s.arg(attributeName), //FIXME needs parsed name?
        ConditionFnOp::Equality::Equal,
        u"$target.%1"_s.arg(attributeName),
    };

    auto conditionFn = CreateConditionFnFor::edge(graphModel, condition);
    Q_ASSERT(conditionFn != nullptr);
    if(conditionFn == nullptr)
        return {};

    EdgeIdSet edgeIdsToContract;

    for(const EdgeId edgeId : contractedGraph.edgeIds())
    {
        if(conditionFn(edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    contractedGraph.contractEdges(edgeIdsToContract);

    for(const NodeId nodeId : contractedGraph.nodeIds())
    {
        TextVisual textVisual;
        textVisual._text = attribute->stringValueOf(nodeId);

        switch(contractedGraph.typeOf(nodeId))
        {
        case MultiElementType::Not:
            textVisual._nodeIds = {nodeId};
            break;

        case MultiElementType::Head:
        {
            auto mergedNodeIds = contractedGraph.mergedNodeIdsForNodeId(nodeId);

            textVisual._nodeIds.reserve(static_cast<size_t>(mergedNodeIds.size()));
            for(const NodeId mergedNodeId : mergedNodeIds)
                textVisual._nodeIds.push_back(mergedNodeId);

            break;
        }

        case MultiElementType::Tail:
            continue;
        }

        const ComponentId componentId = graphModel.graph().componentIdOfNode(nodeId);
        textVisuals[componentId].push_back(textVisual);
    }

    return textVisuals;
}

QString SharedTextVisualisationChannel::description(ElementType, ValueType) const
{
    return QObject::tr("Groups of nodes that share an attribute value will be labelled as such.");
}
