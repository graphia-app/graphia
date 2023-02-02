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

#include "filtertransform.h"
#include "transform/transformedgraph.h"
#include "attributes/conditionfncreator.h"

#include "graph/graphmodel.h"
#include "graph/graphcomponent.h"
#include "graph/componentmanager.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"

#include <algorithm>

#include <QObject>

void FilterTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Filtering"));

    // The elements to be filtered are calculated first and then removed, because
    // removing elements during the filtering could affect the result of filter functions

    switch(_elementType)
    {
    case ElementType::Node:
    {
        auto conditionFn = CreateConditionFnFor::node(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        std::vector<NodeId> removees;

        for(auto nodeId : target.nodeIds())
        {
            if(u::exclusiveOr(conditionFn(nodeId), _invert))
                removees.push_back(nodeId);
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            setProgress(static_cast<int>((progress++ * 100) / numRemovees));
        }
        break;
    }

    case ElementType::Edge:
    {
        auto conditionFn = CreateConditionFnFor::edge(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        std::vector<EdgeId> removees;

        for(auto edgeId : target.edgeIds())
        {
            if(u::exclusiveOr(conditionFn(edgeId), _invert))
                removees.push_back(edgeId);
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto edgeId : removees)
        {
            target.mutableGraph().removeEdge(edgeId);
            setProgress(static_cast<int>((progress++ * 100) / numRemovees));
        }
        break;
    }

    case ElementType::Component:
    {
        auto conditionFn = CreateConditionFnFor::component(*_graphModel, config()._condition);
        if(conditionFn == nullptr)
        {
            addAlert(AlertType::Error, QObject::tr("Invalid condition"));
            return;
        }

        const ComponentManager componentManager(target);
        std::vector<NodeId> removees;

        for(auto componentId : componentManager.componentIds())
        {
            const auto* component = componentManager.componentById(componentId);
            if(u::exclusiveOr(conditionFn(*component), _invert))
            {
                auto nodeIds = target.mutableGraph().mergedNodeIdsForNodeIds(component->nodeIds());
                removees.insert(removees.end(), nodeIds.begin(), nodeIds.end());
            }
        }

        auto numRemovees = static_cast<uint64_t>(removees.size());
        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            setProgress(static_cast<int>((progress++ * 100) / numRemovees));
        }
        break;
    }

    default:
        break;
    }
}

std::unique_ptr<GraphTransform> FilterTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<FilterTransform>(elementType(), *graphModel(), _invert);
}
