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

void BetweennessTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QStringLiteral("Betweenness"));
    target.setProgress(0);

    const auto& nodeIds = target.nodeIds();
    const auto& edegIds = target.edgeIds();
    std::atomic_int progress(0);

    // Use a map of node/edge arrays keyed per thread, and accumulate
    // the results when the algorithm completes, thus avoiding any locking
    std::map<std::thread::id, NodeArray<double>> nodeBetweennessMap;
    std::map<std::thread::id, EdgeArray<double>> edgeBetweennessMap;

    concurrent_for(nodeIds.begin(), nodeIds.end(),
    [&](const NodeId nodeId)
    {
        std::thread::id currentThreadId = std::this_thread::get_id();

        auto& _nodeBetweenness = nodeBetweennessMap.try_emplace(
            currentThreadId, NodeArray<double>{target, 0.0}).first->second;
        auto& _edgeBetweenness = edgeBetweennessMap.try_emplace(
            currentThreadId, EdgeArray<double>{target, 0.0}).first->second;

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

        while(!queue.empty())
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

        if(cancelled())
            return;

        while(!stack.empty())
        {
            auto other = stack.top();
            stack.pop();

            for(auto predecessor : predecessors[other])
            {
                auto d = (static_cast<double>(sigma[predecessor]) /
                    static_cast<double>(sigma[other])) * (1.0 + delta[other]);

                auto edgeIds = target.edgeIdsBetween(predecessor, other);
                for(auto edgeId : edgeIds)
                    _edgeBetweenness[edgeId] += d;

                delta[predecessor] += d;
            }

            if(other != nodeId)
                _nodeBetweenness[other] += delta[other];
        }

        progress++;
        target.setProgress(progress.load() * 100 / static_cast<int>(target.numNodes()));

        if(cancelled())
            return;
    }, true);

    target.setProgress(-1);

    if(cancelled())
        return;

    NodeArray<double> nodeBetweenness(target, 0.0);
    for(const auto& [threadId, _nodeBetweenness] : nodeBetweennessMap)
    {
        for(auto nodeId : nodeIds)
            nodeBetweenness[nodeId] += _nodeBetweenness[nodeId];
    }

    EdgeArray<double> edgeBetweenness(target, 0.0);
    for(const auto& [threadId, _edgeBetweenness] : edgeBetweennessMap)
    {
        for(auto edgeId : edegIds)
            edgeBetweenness[edgeId] += _edgeBetweenness[edgeId];
    }

    _graphModel->createAttribute(QObject::tr("Node Betweenness"))
        .setDescription(QObject::tr("A node's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([nodeBetweenness](NodeId nodeId) { return nodeBetweenness[nodeId]; })
        .setFlag(AttributeFlag::VisualiseByComponent);

    _graphModel->createAttribute(QObject::tr("Edge Betweenness"))
        .setDescription(QObject::tr("An edge's betweenness is the number of shortest paths that pass through it."))
        .setFloatValueFn([edgeBetweenness](EdgeId edgeId) { return edgeBetweenness[edgeId]; })
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> BetweennessTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<BetweennessTransform>(graphModel());
}

