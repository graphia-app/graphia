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

#include "betweennesstransform.h"

#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"

#include "shared/graph/grapharray.h"
#include "shared/utils/threadpool.h"

#include <cstdint>
#include <stack>
#include <queue>
#include <map>
#include <thread>

using namespace Qt::Literals::StringLiterals;

void BetweennessTransform::apply(TransformedGraph& target)
{
    setPhase(u"Betweenness"_s);
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
        auto& arrays = betweennessArrays.at(threadIndex);
        auto& _nodeBetweenness = arrays.nodeBetweenness;
        auto& _edgeBetweenness = arrays.edgeBetweenness;

        // Brandes algorithm
        NodeArray<std::vector<NodeId>> predecessors(target);
        NodeArray<int64_t> sigma(target, 0);
        NodeArray<int64_t> distance(target, -1);
        NodeArray<double> delta(target, 0.0);

        std::stack<NodeId> stack;
        std::queue<NodeId> queue;

        sigma[nodeId] = 1.0;
        distance[nodeId] = 0;
        queue.push(nodeId);

        while(!queue.empty() && !cancelled())
        {
            auto other = queue.front();
            queue.pop();
            stack.push(other);

            for(auto neighbour : target.neighboursOf(other))
            {
                if(distance[neighbour] < 0)
                {
                    queue.push(neighbour);
                    distance[neighbour] = distance[other] + 1;
                }

                if(distance[neighbour] == distance[other] + 1)
                {
                    sigma[neighbour] += sigma[other];
                    predecessors[neighbour].push_back(other);
                }
            }
        }

        while(!stack.empty() && !cancelled())
        {
            auto other = stack.top();
            stack.pop();

            for(auto predecessor : predecessors[other])
            {
                auto d = (static_cast<double>(sigma[predecessor]) /
                    static_cast<double>(sigma[other])) * (1.0 + delta[other]);

                for(auto edgeId : target.edgeIdsBetween(predecessor, other))
                    _edgeBetweenness[edgeId] += d;

                delta[predecessor] += d;
            }

            if(other != nodeId)
                _nodeBetweenness[other] += delta[other];
        }

        progress++;
        setProgress(progress.load() * 100 / static_cast<int>(target.numNodes()));

        if(cancelled())
            return;
    });

    setProgress(-1);

    if(cancelled())
        return;

    NodeArray<double> nodeBetweenness(target, 0.0);
    EdgeArray<double> edgeBetweenness(target, 0.0);
    for(const auto& arrays : betweennessArrays)
    {
        for(auto nodeId : nodeIds)
            nodeBetweenness[nodeId] += arrays.nodeBetweenness[nodeId];

        for(auto edgeId : edgeIds)
            edgeBetweenness[edgeId] += arrays.edgeBetweenness[edgeId];
    }

    _graphModel->createAttribute(QObject::tr("Node Betweenness"))
        .setDescription(QObject::tr("A node's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([nodeBetweenness](NodeId nodeId) { return nodeBetweenness[nodeId]; })
        .setFlag(AttributeFlag::AutoRange)
        .setFlag(AttributeFlag::VisualiseByComponent);

    _graphModel->createAttribute(QObject::tr("Edge Betweenness"))
        .setDescription(QObject::tr("An edge's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([edgeBetweenness](EdgeId edgeId) { return edgeBetweenness[edgeId]; })
        .setFlag(AttributeFlag::AutoRange)
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> BetweennessTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<BetweennessTransform>(graphModel());
}

