#include "eccentricitytransform.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

#include <map>
#include <queue>

bool EccentricityTransform::apply(TransformedGraph& target) const
{
    target.setPhase("Eccentricity");
    calculateDistances();
    return false;
}

void EccentricityTransform::calculateDistances() const
{
    // Setup the contiguous ids
    std::map<NodeId, int> nodeIdToMatrixIndex;
    std::map<int, NodeId> matrixIndexToNodeId;
    for(auto& nodeId : _graphModel->graph().nodeIds())
    {
        int size = static_cast<int>(nodeIdToMatrixIndex.size());
        nodeIdToMatrixIndex[nodeId] = size;
        matrixIndexToNodeId[size - 1] = nodeId;
    }

    NodeArray<float> maxDistances(_graphModel->graph());

    const auto& nodeIds = _graphModel->graph().nodeIds();
    concurrent_for(nodeIds.begin(), nodeIds.end(),
                   [this, &maxDistances](const NodeId source)
    {
        NodeArray<float> distance(_graphModel->graph());
        NodeArray<bool> visited(_graphModel->graph());

        auto comparator = [&distance](NodeId a, NodeId b){ return distance[a] > distance[b]; };
        std::priority_queue<NodeId, std::vector<NodeId>, decltype(comparator)> queue(comparator);

        for(auto nodeId : _graphModel->graph().nodeIds())
            distance[nodeId] = std::numeric_limits<float>::max();

        queue.push(source);
        distance[source] = 0.0f;

        while(!queue.empty())
        {
            auto nodeId = queue.top();
            queue.pop();

            if(visited.get(nodeId))
                continue;

            visited.set(nodeId, true);

            auto nodeWeight = distance[nodeId];
            for(EdgeId edgeId : _graphModel->graph().edgeIdsForNodeId(nodeId))
            {
                NodeId adjacentNodeId = _graphModel->graph().edgeById(edgeId).oppositeId(nodeId);
                const int adjacentNodeWeight = 1;
                if(!visited.get(adjacentNodeId) && (nodeWeight + adjacentNodeWeight < distance[adjacentNodeId]))
                {
                    distance[adjacentNodeId] = nodeWeight + adjacentNodeWeight;
                    queue.push(adjacentNodeId);
                }
            }
        }

        float maxDistance = 0.0f;
        for(auto& nodeId : _graphModel->graph().nodeIds())
        {
            if(distance[nodeId] != std::numeric_limits<float>::max())
                maxDistance = std::max(distance[nodeId], maxDistance);
        }

        maxDistances[source] = maxDistance;

    }, true);

    _graphModel->createAttribute(QObject::tr("Node Eccentricity"))
        .setDescription("A node's eccentricity is the length of the shortest path to the furthest node.")
        .setFloatValueFn([maxDistances](NodeId nodeId) { return maxDistances[nodeId]; });
}

std::unique_ptr<GraphTransform> EccentricityTransformFactory::create(const GraphTransformConfig&) const
{
    auto testTransform = std::make_unique<EccentricityTransform>(graphModel());

    return std::move(testTransform); //FIXME std::move required because of clang bug
}

