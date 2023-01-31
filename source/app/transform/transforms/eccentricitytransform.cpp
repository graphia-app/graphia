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

#include "eccentricitytransform.h"
#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

#include <map>
#include <queue>

void EccentricityTransform::apply(TransformedGraph& target)
{
    target.setPhase(QStringLiteral("Eccentricity"));
    calculateDistances(target);
}

void EccentricityTransform::calculateDistances(TransformedGraph& target)
{
    // Setup the contiguous ids
    NodeIdMap<int> nodeIdToMatrixIndex;
    for(auto nodeId : target.nodeIds())
    {
        auto size = static_cast<int>(nodeIdToMatrixIndex.size());
        nodeIdToMatrixIndex[nodeId] = size;
    }

    NodeArray<int> maxDistances(target);

    target.setProgress(0);

    const auto& nodeIds = target.nodeIds();
    std::atomic_int progress(0);
    parallel_for(nodeIds.begin(), nodeIds.end(),
    [this, &maxDistances, &progress, &target](NodeId source)
    {
        if(cancelled())
            return;

        NodeArray<int> distance(target);
        NodeArray<bool> visited(target);

        auto comparator = [&distance](NodeId a, NodeId b){ return distance[a] > distance[b]; };
        std::priority_queue<NodeId, std::vector<NodeId>, decltype(comparator)> queue(comparator);

        for(auto nodeId : target.nodeIds())
            distance[nodeId] = std::numeric_limits<int>::max();

        queue.push(source);
        distance[source] = 0;

        while(!queue.empty())
        {
            if(cancelled())
                return;

            auto nodeId = queue.top();
            queue.pop();

            if(visited.get(nodeId))
                continue;

            visited.set(nodeId, true);

            auto nodeWeight = distance[nodeId];
            for(const EdgeId edgeId : target.edgeIdsForNodeId(nodeId))
            {
                const NodeId adjacentNodeId = target.edgeById(edgeId).oppositeId(nodeId);
                const int adjacentNodeWeight = 1;
                if(!visited.get(adjacentNodeId) && (nodeWeight + adjacentNodeWeight < distance[adjacentNodeId]))
                {
                    distance[adjacentNodeId] = nodeWeight + adjacentNodeWeight;
                    queue.push(adjacentNodeId);
                }
            }
        }

        int maxDistance = 0;
        for(auto nodeId : target.nodeIds())
        {
            if(distance[nodeId] != std::numeric_limits<int>::max())
                maxDistance = std::max(distance[nodeId], maxDistance);
        }

        maxDistances[source] = maxDistance;
        progress++;
        target.setProgress(progress.load() * 100 / static_cast<int>(target.numNodes()));
    });

    target.setProgress(-1);

    if(cancelled())
        return;

    _graphModel->createAttribute(QObject::tr("Node Eccentricity"))
        .setDescription(QObject::tr("A node's eccentricity is the length of the shortest path to the furthest node."))
        .setIntValueFn([maxDistances](NodeId nodeId) { return maxDistances[nodeId]; })
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> EccentricityTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<EccentricityTransform>(graphModel());
}

