/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "betweennesstransform.h"

#include "app/transform/transformedgraph.h"
#include "app/graph/graphmodel.h"

#include "shared/graph/grapharray.h"
#include "shared/utils/threadpool.h"

#include <cstdint>
#include <thread>
#include <atomic>
#include <cstddef>
#include <vector>

using namespace Qt::Literals::StringLiterals;

void BetweennessTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Betweenness"));
    setProgress(0);

    const auto& nodeIds = target.nodeIds();
    const auto& edgeIds = target.edgeIds();
    std::atomic_int progress(0);

    struct BetweennessArrays
    {
        explicit BetweennessArrays(TransformedGraph& graph) :
            nodeBetweenness(graph, 0.0),
            edgeBetweenness(graph, 0.0)
        {}

        NodeArray<double> nodeBetweenness;
        EdgeArray<double> edgeBetweenness;
    };

    std::vector<BetweennessArrays> betweennessArrays(
        std::thread::hardware_concurrency(),
        BetweennessArrays{target});

    parallel_for(nodeIds.begin(), nodeIds.end(),
    [&](NodeId nodeId, size_t threadIndex)
    {
        if(cancelled())
            return;

        auto& arrays = betweennessArrays.at(threadIndex);
        auto& _nodeBetweenness = arrays.nodeBetweenness;
        auto& _edgeBetweenness = arrays.edgeBetweenness;

        // Brandes algorithm
        NodeArray<std::vector<NodeId>> predecessors(target);
        NodeArray<int64_t> sigma(target, 0);
        NodeArray<int64_t> distance(target, -1);
        NodeArray<double> delta(target, 0.0);

        // The BFS visits nodes in order of distance, so reversing it gives Brandes' stack
        std::vector<NodeId> visitOrder;

        sigma[nodeId] = 1.0;
        distance[nodeId] = 0;
        visitOrder.push_back(nodeId);

        for(size_t head = 0; head < visitOrder.size() && !cancelled(); head++)
        {
            auto other = visitOrder[head];

            for(auto neighbour : target.neighboursOf(other))
            {
                if(distance[neighbour] < 0)
                {
                    visitOrder.push_back(neighbour);
                    distance[neighbour] = distance[other] + 1;
                }

                if(distance[neighbour] == distance[other] + 1)
                {
                    sigma[neighbour] += sigma[other];
                    predecessors[neighbour].push_back(other);
                }
            }
        }

        for(auto it = visitOrder.rbegin(); it != visitOrder.rend() && !cancelled(); ++it)
        {
            auto other = *it;

            for(auto predecessor : predecessors[other])
            {
                auto d = (static_cast<double>(sigma[predecessor]) /
                    static_cast<double>(sigma[other])) * (1.0 + delta[other]);

                // When there are multiple edges between the nodes, the predecessor appears
                // once for each of them, so share each appearance across the edges
                auto edgeIdsBetween = target.edgeIdsBetween(predecessor, other);
                for(auto edgeId : edgeIdsBetween)
                    _edgeBetweenness[edgeId] += d / static_cast<double>(edgeIdsBetween.size());

                delta[predecessor] += d;
            }

            if(other != nodeId)
                _nodeBetweenness[other] += delta[other];
        }

        progress++;
        setProgress(progress.load() * 100 / static_cast<int>(target.numNodes()));
    });

    setProgress(-1);

    if(cancelled())
        return;

    NodeArray<double> nodeBetweenness(target, 0.0);
    EdgeArray<double> edgeBetweenness(target, 0.0);

    // Each shortest path is found twice from each of its ends, so the totals are halved
    for(const auto& arrays : betweennessArrays)
    {
        for(auto nodeId : nodeIds)
            nodeBetweenness[nodeId] += arrays.nodeBetweenness[nodeId] * 0.5;

        for(auto edgeId : edgeIds)
            edgeBetweenness[edgeId] += arrays.edgeBetweenness[edgeId] * 0.5;
    }

    _graphModel->createAttribute(BetweennessTransformFactory::nodeBetweennessAttribute())
        .setDescription(QObject::tr("A node's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([nodeBetweenness](NodeId nodeId) { return nodeBetweenness[nodeId]; })
        .setFlag(AttributeFlag::AutoRange)
        .setFlag(AttributeFlag::VisualiseByComponent);

    _graphModel->createAttribute(BetweennessTransformFactory::edgeBetweennessAttribute())
        .setDescription(QObject::tr("An edge's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([edgeBetweenness](EdgeId edgeId) { return edgeBetweenness[edgeId]; })
        .setFlag(AttributeFlag::AutoRange)
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> BetweennessTransformFactory::create(GraphModel& graphModel, const GraphTransformConfig&) const
{
    return std::make_unique<BetweennessTransform>(&graphModel);
}

