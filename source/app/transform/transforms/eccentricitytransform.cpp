#include "eccentricitytransform.h"
#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

#include <map>
#include <queue>

void EccentricityTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QStringLiteral("Eccentricity"));
    calculateDistances(target);
}

void EccentricityTransform::calculateDistances(TransformedGraph& target) const
{
    // Setup the contiguous ids
    std::map<NodeId, int> nodeIdToMatrixIndex;
    std::map<int, NodeId> matrixIndexToNodeId;
    for(auto nodeId : target.nodeIds())
    {
        auto size = static_cast<int>(nodeIdToMatrixIndex.size());
        nodeIdToMatrixIndex[nodeId] = size;
        matrixIndexToNodeId[size - 1] = nodeId;
    }

    NodeArray<int> maxDistances(target);

    target.setProgress(0);

    const auto& nodeIds = target.nodeIds();
    std::atomic_int progress(0);
    concurrent_for(nodeIds.begin(), nodeIds.end(),
    [this, &maxDistances, &progress, &target](const NodeId source)
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
            for(EdgeId edgeId : target.edgeIdsForNodeId(nodeId))
            {
                NodeId adjacentNodeId = target.edgeById(edgeId).oppositeId(nodeId);
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

