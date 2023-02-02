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

#include "leidentransform.h"

#include "transform/transformedgraph.h"

#include "shared/graph/grapharray.h"
#include "shared/utils/container.h"

#include "graph/graphmodel.h"

#include <map>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>

// https://www.nature.com/articles/s41598-019-41695-z

void LeidenTransform::apply(TransformedGraph& target)
{
    auto resolution = 1.0 - std::get<double>(
        config().parameterByName(QStringLiteral("Granularity"))->_value);

    const auto minResolution = 0.5;
    const auto maxResolution = 30.0;

    const auto logMin = std::log10(minResolution);
    const auto logMax = std::log10(maxResolution);
    const auto logRange = logMax - logMin;

    resolution = std::pow(10.0f, logMin + (resolution * logRange));

    using CommunityId = NodeId; // Slight Hack, be careful with this

    const auto& edgeIds = target.edgeIds();
    EdgeArray<double> weights(target, 1.0);

    if(_weighted)
    {
        if(config().attributeNames().empty())
        {
            addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
            return;
        }

        auto attribute = _graphModel->attributeValueByName(
            config().attributeNames().front());

        for(auto edgeId : edgeIds)
            weights[edgeId] = attribute.numericValueOf(edgeId);
    }

    double totalWeight = std::accumulate(edgeIds.begin(), edgeIds.end(), 0.0,
    [&weights](double d, EdgeId edgeId)
    {
        return d + weights[edgeId];
    });

    std::vector<NodeArray<CommunityId>> iterations;
    size_t progressIteration = 1;
    setPhase(QStringLiteral("Leiden Initialising"));

    NodeArray<CommunityId> communities(target);
    NodeArray<double> weightedDegrees(target);
    std::map<CommunityId, int> communityDegrees;

    auto add = [&](CommunityId community, NodeId nodeId)
    {
        communities[nodeId] = community;
        communityDegrees[community] += static_cast<int>(weightedDegrees[nodeId]);
    };

    auto remove = [&](CommunityId community, NodeId nodeId)
    {
        communities[nodeId].setToNull();
        communityDegrees[community] -= static_cast<int>(weightedDegrees[nodeId]);
    };

    auto relabel = [&](MutableGraph& graph)
    {
        std::map<CommunityId, CommunityId> idMap;
        CommunityId nextCommunityId = 0;

        for(auto nodeId : graph.nodeIds())
        {
            if(graph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto oldCommunityId = communities[nodeId];

            if(!u::contains(idMap, oldCommunityId))
                communities[nodeId] = idMap[oldCommunityId] = nextCommunityId++;
            else
                communities[nodeId] = idMap.at(oldCommunityId);
        }
    };

    auto coarsen = [&](MutableGraph& graph)
    {
        MutableGraph coarseGraph;
        EdgeArray<double> newWeights(target, 0.0);

        coarseGraph.performTransaction([&](IMutableGraph&)
        {
            // Create a node for each community
            for(auto nodeId : graph.nodeIds())
            {
                if(graph.typeOf(nodeId) == MultiElementType::Tail)
                    continue;

                auto newNodeId = communities[nodeId];

                if(coarseGraph.containsNodeId(newNodeId))
                    continue;

                coarseGraph.reserveNodeId(newNodeId);
                auto assignedNodeId = coarseGraph.addNode(newNodeId);
                Q_ASSERT(assignedNodeId == newNodeId);
            }

            uint64_t edgeIndex = 0;

            // Create an edge between community nodes for each
            // pair of connected communities in the base graph
            for(auto edgeId : graph.edgeIds())
            {
                setProgress(static_cast<int>((edgeIndex++ * 100) /
                    static_cast<uint64_t>(graph.numEdges())));

                if(cancelled())
                    break;

                const auto& edge = graph.edgeById(edgeId);
                auto sourceId = communities.at(edge.sourceId());
                auto targetId = communities.at(edge.targetId());
                const double newWeight = weights[edgeId];

                auto newEdgeId = coarseGraph.firstEdgeIdBetween(sourceId, targetId);
                if(newEdgeId.isNull())
                    newEdgeId = coarseGraph.addEdge(sourceId, targetId);

                newWeights[newEdgeId] += newWeight;
            }
        });

        graph = coarseGraph;
        weights = newWeights;
    };

    auto moveNodes = [&](MutableGraph& graph)
    {
        if(cancelled())
            return false;

        CommunityId nextCommunityId = 0;
        for(auto nodeId : graph.nodeIds())
        {
            if(graph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            for(auto edgeId : graph.nodeById(nodeId).edgeIds())
                weightedDegrees[nodeId] += weights[edgeId];

            add(nextCommunityId++, nodeId);
        }

        bool modified = false;
        std::deque<NodeId> nodeIdQueue;

        for(auto nodeId : graph.nodeIds())
            nodeIdQueue.push_back(nodeId);

        NodeArray<bool> visited(graph);

        int highestProgress = 0;
        target.setProgress(0);

        do
        {
            setPhase(QStringLiteral("Leiden Iteration %1")
                .arg(QString::number(progressIteration)));

            auto nodeId = nodeIdQueue.front();
            nodeIdQueue.pop_front();

            auto progress = static_cast<int>(
                (static_cast<uint64_t>(graph.numNodes() - nodeIdQueue.size()) * 100) /
                static_cast<uint64_t>(graph.numNodes()));

            highestProgress = std::max(highestProgress, progress);
            target.setProgress(highestProgress);

            if(graph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            // Find total weights of neighbour communities
            std::map<CommunityId, double> neighbourCommunityWeights;
            for(auto edgeId : graph.nodeById(nodeId).edgeIds())
            {
                auto neighbourNodeId = graph.edgeById(edgeId).oppositeId(nodeId);

                // Skip loop edges
                if(neighbourNodeId == nodeId)
                    continue;

                if(graph.typeOf(neighbourNodeId) == MultiElementType::Tail)
                    continue;

                auto neighbourCommunityId = communities.at(neighbourNodeId);

                neighbourCommunityWeights[neighbourCommunityId] += weights[edgeId];
            }

            auto communityId = communities[nodeId];
            remove(communityId, nodeId);

            visited.set(nodeId, true);

            double maxDeltaQ = 0.0;
            auto newCommunityId = communityId;

            // Find the neighbouring community with the greatest delta Q
            for(auto [neighbourCommunityId, weight] : neighbourCommunityWeights)
            {
                auto communityWeight = communityDegrees.at(neighbourCommunityId);
                auto nodeWeight = weightedDegrees[nodeId];

                auto deltaQ = (resolution * weight) -
                    ((communityWeight * nodeWeight) / totalWeight);

                if(deltaQ > maxDeltaQ)
                {
                    maxDeltaQ = deltaQ;
                    newCommunityId = neighbourCommunityId;
                }
            }

            add(newCommunityId, nodeId);

            if(newCommunityId != communityId)
            {
                modified = true;

                for(auto edgeId : graph.nodeById(nodeId).edgeIds())
                {
                    auto neighbourNodeId = graph.edgeById(edgeId).oppositeId(nodeId);

                    // Skip loop edges
                    if(neighbourNodeId == nodeId)
                        continue;

                    if(graph.typeOf(neighbourNodeId) == MultiElementType::Tail)
                        continue;

                    if(newCommunityId == communities[neighbourNodeId])
                        continue;

                    // Add neighbourNodeId to nodeIdQueue (if not already in there)
                    if(visited.get(neighbourNodeId))
                    {
                        nodeIdQueue.push_back(neighbourNodeId);
                        visited.set(neighbourNodeId, false);
                    }
                }
            }
        }
        while(!nodeIdQueue.empty() && !cancelled());

        target.setProgress(-1);

        return modified;
    };

    MutableGraph graph(target.mutableGraph());

    bool finished = false;
    do // NOLINT bugprone-infinite-loop
    {
        setProgress(-1);

        communities.resetElements();
        weightedDegrees.resetElements();
        communityDegrees.clear();

        finished = !moveNodes(graph);

        if(!finished && !cancelled())
        {
            relabel(graph);
            iterations.emplace_back(communities);

            setPhase(QStringLiteral("Leiden Iteration %1 Coarsening")
                .arg(QString::number(progressIteration)));
            coarsen(graph);
        }

        progressIteration++;
    }
    while(!finished && !cancelled());

    if(cancelled())
        return;

    setPhase(QStringLiteral("Leiden Finalising"));

    // Set each CommunityId to be the same as the NodeId, initially
    for(auto nodeId : target.nodeIds())
    {
        if(graph.typeOf(nodeId) == MultiElementType::Tail)
            continue;

        communities[nodeId] = nodeId;
    }

    // Walk back over our iterations to build the final communities
    for(auto nodeId : target.nodeIds())
    {
        for(const auto& iteration : iterations)
        {
            auto communityId = communities[nodeId];

            if(!communityId.isNull())
                communities[nodeId] = iteration.at(communities[nodeId]);

            if(cancelled())
                return;
        }
    }

    // Sort communities by size
    std::map<CommunityId, size_t> communityHistogram;
    for(auto nodeId : target.nodeIds())
        communityHistogram[communities[nodeId]]++;
    std::vector<std::pair<CommunityId, size_t>> sortedCommunityHistogram;
    std::copy(communityHistogram.begin(), communityHistogram.end(),
        std::back_inserter(sortedCommunityHistogram));
    std::sort(sortedCommunityHistogram.begin(), sortedCommunityHistogram.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // Assign cluster numbers to each community
    std::map<CommunityId, size_t> clusterNumbers;
    size_t clusterNumber = 1;
    for(auto [communityId, size] : sortedCommunityHistogram)
    {
        if(!communityId.isNull())
            clusterNumbers[communityId] = clusterNumber++;
    }

    NodeArray<QString> clusterNames(target);
    NodeArray<int> clusterSizes(target);

    for(auto nodeId : target.nodeIds())
    {
        auto communityId = communities[nodeId];
        if(communityId.isNull())
            continue;

        clusterNumber = clusterNumbers[communityId];
        clusterNames[nodeId] = QObject::tr("Cluster %1").arg(clusterNumber);
        clusterSizes[nodeId] = static_cast<int>(communityHistogram.at(communityId));
    }

    _graphModel->createAttribute(QObject::tr(_weighted ? "Weighted Leiden Cluster" : "Leiden Cluster")) // clazy:exclude=tr-non-literal
        .setDescription(QObject::tr("The Leiden cluster in which the node resides."))
        .setStringValueFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId]; })
        .setValueMissingFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId].isEmpty(); })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable);

    _graphModel->createAttribute(QObject::tr(_weighted ? "Weighted Leiden Cluster Size" : "Leiden Cluster Size")) // clazy:exclude=tr-non-literal
        .setDescription(QObject::tr("The size of the Leiden cluster in which the node resides."))
        .setIntValueFn([clusterSizes](NodeId nodeId) { return clusterSizes[nodeId]; })
        .setFlag(AttributeFlag::AutoRange);
}
