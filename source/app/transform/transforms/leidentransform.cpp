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

#include "app/transform/transformedgraph.h"

#include "shared/graph/grapharray.h"

#include "app/graph/graphmodel.h"

#include <QtGlobal>

#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <numeric>
#include <span>
#include <utility>

using namespace Qt::Literals::StringLiterals;

// clazy takes calls to capturing lambdas, and NodeArray's uncompared mutex, for comparisons that skip members
// clazy:excludeall=compare-member-check

// https://www.nature.com/articles/s41598-019-41695-z

void LeidenTransform::apply(TransformedGraph& target)
{
    auto granularity = config().parameterByName(u"Granularity"_s)->valueAsNumber();

    if(std::isnan(granularity))
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto resolution = 1.0 - granularity;

    const auto minResolution = 0.5;
    const auto maxResolution = 30.0;

    const auto logMin = std::log10(minResolution);
    const auto logMax = std::log10(maxResolution);
    const auto logRange = logMax - logMin;

    resolution = std::pow(10.0, logMin + (resolution * logRange));

    using CommunityId = NodeId; // Slight Hack, be careful with this

    const auto& edgeIds = target.edgeIds();
    EdgeArray<double> edgeWeights(target, 1.0);

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
            edgeWeights[edgeId] = attribute.numericValueOf(edgeId);
    }

    double totalWeight = std::accumulate(edgeIds.begin(), edgeIds.end(), 0.0,
    [&edgeWeights](double d, EdgeId edgeId)
    {
        return d + edgeWeights[edgeId];
    });

    std::vector<NodeArray<CommunityId>> iterations;
    size_t progressIteration = 1;
    setPhase(QObject::tr("Leiden Initialising"));

    NodeArray<CommunityId> communities(target);
    NodeArray<double> weightedDegrees(target);

    // Indexed by CommunityId, which can exceed the node count when nodes move to empty communities
    std::vector<double> communityDegrees;
    std::vector<size_t> communitySizes;

    // The weight from one node to each community it neighbours
    std::vector<double> neighbourCommunityWeights;
    std::vector<bool> isNeighbourCommunity;
    std::vector<CommunityId> neighbourCommunityIds;

    struct Neighbour
    {
        NodeId _nodeId;
        double _weight;
    };

    // A graph reduced to its nodes and their neighbours, each node's neighbours being contiguous
    struct Level
    {
        std::vector<NodeId> _nodeIds;
        std::vector<Neighbour> _neighbours;
        NodeArray<size_t> _neighbourStarts;
        NodeArray<size_t> _neighbourEnds;
    };

    auto emptyLevel = [&]
    {
        return Level{._nodeIds = {}, ._neighbours = {},
            ._neighbourStarts = NodeArray<size_t>(target), ._neighbourEnds = NodeArray<size_t>(target)};
    };

    auto targetLevel = [&]
    {
        auto level = emptyLevel();

        for(auto nodeId : target.nodeIds())
        {
            if(target.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            level._nodeIds.push_back(nodeId);
            level._neighbourStarts[nodeId] = level._neighbours.size();

            for(auto edgeId : target.nodeById(nodeId).edgeIds())
                level._neighbours.push_back({target.edgeById(edgeId).oppositeId(nodeId), edgeWeights[edgeId]});

            level._neighbourEnds[nodeId] = level._neighbours.size();
        }

        return level;
    };

    const auto baseLevel = targetLevel();
    auto level = emptyLevel();

    auto degreeOf = [&](CommunityId community) { return communityDegrees.at(static_cast<size_t>(community)); };
    auto sizeOf = [&](CommunityId community) { return communitySizes.at(static_cast<size_t>(community)); };

    auto add = [&](CommunityId community, NodeId nodeId)
    {
        auto index = static_cast<size_t>(community);
        if(index >= communitySizes.size())
        {
            communityDegrees.resize(index + 1, 0.0);
            communitySizes.resize(index + 1, 0);
        }

        communities[nodeId] = community;
        communityDegrees[index] += weightedDegrees[nodeId];
        communitySizes[index]++;
    };

    auto remove = [&](CommunityId community, NodeId nodeId)
    {
        auto index = static_cast<size_t>(community);
        communities[nodeId].setToNull();
        communityDegrees[index] -= weightedDegrees[nodeId];
        communitySizes[index]--;
    };

    auto addNeighbourWeight = [&](CommunityId community, double weight)
    {
        auto index = static_cast<size_t>(community);
        if(index >= isNeighbourCommunity.size())
        {
            neighbourCommunityWeights.resize(index + 1, 0.0);
            isNeighbourCommunity.resize(index + 1, false);
        }

        if(!isNeighbourCommunity[index])
        {
            isNeighbourCommunity[index] = true;
            neighbourCommunityIds.push_back(community);
        }

        neighbourCommunityWeights[index] += weight;
    };

    auto neighbourWeightTo = [&](CommunityId community)
    {
        auto index = static_cast<size_t>(community);
        return index < neighbourCommunityWeights.size() ? neighbourCommunityWeights[index] : 0.0;
    };

    auto clearNeighbourWeights = [&]
    {
        for(auto community : neighbourCommunityIds)
        {
            auto index = static_cast<size_t>(community);
            neighbourCommunityWeights[index] = 0.0;
            isNeighbourCommunity[index] = false;
        }

        neighbourCommunityIds.clear();
    };

    // The modularity gained by joining two groups of nodes, scaled by the total weight
    auto deltaQ = [&](double weightBetween, double degreeA, double degreeB)
    {
        return (resolution * weightBetween) - ((degreeA * degreeB) / totalWeight);
    };

    auto wellConnected = [&](double weightToRest, double degree, double communityDegree)
    {
        return deltaQ(weightToRest, degree, communityDegree - degree) >= 0.0;
    };

    auto neighboursOf = [&](NodeId nodeId)
    {
        auto start = level._neighbourStarts[nodeId];
        return std::span(level._neighbours).subspan(start, level._neighbourEnds[nodeId] - start);
    };

    auto relabel = [&](const std::vector<NodeId>& nodeIds, NodeArray<CommunityId>& communityIds)
    {
        std::vector<CommunityId> idMap;
        CommunityId nextRelabelledId = 0;

        for(auto nodeId : nodeIds)
        {
            auto index = static_cast<size_t>(communityIds[nodeId]);
            if(index >= idMap.size())
                idMap.resize(index + 1);

            if(idMap[index].isNull())
                idMap[index] = nextRelabelledId++;

            communityIds[nodeId] = idMap[index];
        }
    };

    // Each refined community becomes a node, whose neighbours are the refined communities its members neighbour
    auto coarsen = [&](const NodeArray<CommunityId>& refinedCommunities)
    {
        std::vector<std::vector<NodeId>> communityMembers;
        for(auto nodeId : level._nodeIds)
        {
            auto index = static_cast<size_t>(refinedCommunities[nodeId]);
            if(index >= communityMembers.size())
                communityMembers.resize(index + 1);

            communityMembers[index].push_back(nodeId); // clazy:exclude=reserve-candidates
        }

        auto coarseLevel = emptyLevel();
        uint64_t communityIndex = 0;

        for(const auto& members : communityMembers)
        {
            setProgress(static_cast<int>((communityIndex++ * 100) /
                static_cast<uint64_t>(communityMembers.size())));

            if(cancelled())
                break;

            clearNeighbourWeights();
            for(auto nodeId : members)
            {
                for(auto [neighbourNodeId, weight] : neighboursOf(nodeId))
                    addNeighbourWeight(refinedCommunities[neighbourNodeId], weight);
            }

            auto communityId = refinedCommunities[members.front()];
            coarseLevel._nodeIds.push_back(communityId);
            coarseLevel._neighbourStarts[communityId] = coarseLevel._neighbours.size();

            for(auto neighbourCommunityId : neighbourCommunityIds)
                coarseLevel._neighbours.push_back({neighbourCommunityId, neighbourWeightTo(neighbourCommunityId)});

            coarseLevel._neighbourEnds[communityId] = coarseLevel._neighbours.size();
        }

        return coarseLevel;
    };

    // Moves nodes between communities, revisiting the neighbours of those that move, until none can improve
    auto moveNodes = [&]
    {
        weightedDegrees.resetElements();
        communityDegrees.clear();
        communitySizes.clear();
        CommunityId nextCommunityId = 0;

        std::deque<NodeId> queue;
        NodeArray<bool> queued(target, false);

        for(auto nodeId : level._nodeIds)
        {
            for(const auto& neighbour : neighboursOf(nodeId))
                weightedDegrees[nodeId] += neighbour._weight;

            add(communities[nodeId], nodeId);
            queue.push_back(nodeId);
            queued.set(nodeId, true);

            // Communities are numbered below the node count, so new ones are numbered from it
            ++nextCommunityId;
        }

        auto numNodes = queue.size();
        int highestProgress = 0;

        while(!queue.empty() && !cancelled())
        {
            auto progress = static_cast<int>((static_cast<uint64_t>(numNodes - queue.size()) * 100) /
                static_cast<uint64_t>(numNodes));
            highestProgress = std::max(highestProgress, progress);
            setProgress(highestProgress);

            auto nodeId = queue.front();
            queue.pop_front();
            queued.set(nodeId, false);

            // Find total weights of neighbour communities
            clearNeighbourWeights();
            for(auto [neighbourNodeId, weight] : neighboursOf(nodeId))
            {
                // Skip loop edges
                if(neighbourNodeId == nodeId)
                    continue;

                addNeighbourWeight(communities.at(neighbourNodeId), weight);
            }

            auto communityId = communities[nodeId];
            auto nodeWeight = weightedDegrees[nodeId];

            double maxDeltaQ = deltaQ(neighbourWeightTo(communityId), nodeWeight,
                degreeOf(communityId) - nodeWeight);
            auto newCommunityId = communityId;

            // Find the neighbouring community with the greatest delta Q, the lowest id among equals
            for(auto neighbourCommunityId : neighbourCommunityIds)
            {
                if(neighbourCommunityId == communityId)
                    continue;

                auto neighbourDeltaQ = deltaQ(neighbourWeightTo(neighbourCommunityId), nodeWeight,
                    degreeOf(neighbourCommunityId));

                if(neighbourDeltaQ > maxDeltaQ || (neighbourDeltaQ == maxDeltaQ &&
                    newCommunityId != communityId && neighbourCommunityId < newCommunityId))
                {
                    maxDeltaQ = neighbourDeltaQ;
                    newCommunityId = neighbourCommunityId;
                }
            }

            // A node in a community of its own gains nothing
            if(maxDeltaQ < 0.0 && sizeOf(communityId) > 1)
                newCommunityId = nextCommunityId++;

            if(newCommunityId == communityId)
                continue;

            remove(communityId, nodeId);
            add(newCommunityId, nodeId);

            for(const auto& neighbour : neighboursOf(nodeId))
            {
                auto neighbourNodeId = neighbour._nodeId;

                if(!queued.get(neighbourNodeId) && communities.at(neighbourNodeId) != newCommunityId)
                {
                    queue.push_back(neighbourNodeId);
                    queued.set(neighbourNodeId, true);
                }
            }
        }

        setProgress(-1);

        // Coarsening by communities that are all single nodes would leave the graph as it is
        return std::ranges::any_of(communitySizes, [](auto size) { return size > 1; });
    };

    // Splits each community into well connected refined communities, by merging nodes that are themselves well connected
    auto refine = [&]
    {
        NodeArray<CommunityId> refinedCommunities(target);
        NodeArray<double> refinedDegrees(weightedDegrees);
        NodeArray<size_t> refinedSizes(target, 1);

        // The weight between each refined community and the rest of the community that contains it
        NodeArray<double> weightsToRest(target, 0.0);

        for(auto nodeId : level._nodeIds)
        {
            refinedCommunities[nodeId] = nodeId;

            for(auto [neighbourNodeId, weight] : neighboursOf(nodeId))
            {
                if(neighbourNodeId != nodeId && communities.at(neighbourNodeId) == communities[nodeId])
                    weightsToRest[nodeId] += weight;
            }
        }

        bool merged = false;
        uint64_t nodeIndex = 0;

        for(auto nodeId : level._nodeIds)
        {
            setProgress(static_cast<int>((nodeIndex++ * 100) /
                static_cast<uint64_t>(level._nodeIds.size())));

            if(cancelled())
                break;

            auto communityId = communities[nodeId];
            auto communityDegree = degreeOf(communityId);
            auto nodeWeight = weightedDegrees[nodeId];

            if(refinedSizes[nodeId] > 1 || !wellConnected(weightsToRest[nodeId], nodeWeight, communityDegree))
                continue;

            // Find total weights of neighbour refined communities, within the node's community
            clearNeighbourWeights();
            for(auto [neighbourNodeId, weight] : neighboursOf(nodeId))
            {
                // Skip loop edges
                if(neighbourNodeId == nodeId)
                    continue;

                if(communities.at(neighbourNodeId) == communityId)
                    addNeighbourWeight(refinedCommunities.at(neighbourNodeId), weight);
            }

            double maxDeltaQ = 0.0;
            auto newCommunityId = refinedCommunities[nodeId];

            // Find the well connected neighbouring refined community with the greatest delta Q, the lowest id among equals
            for(auto refinedCommunityId : neighbourCommunityIds)
            {
                auto refinedDegree = refinedDegrees[refinedCommunityId];

                if(!wellConnected(weightsToRest[refinedCommunityId], refinedDegree, communityDegree))
                    continue;

                auto refinedDeltaQ = deltaQ(neighbourWeightTo(refinedCommunityId), nodeWeight, refinedDegree);

                if(refinedDeltaQ > maxDeltaQ || (refinedDeltaQ == maxDeltaQ &&
                    newCommunityId != nodeId && refinedCommunityId < newCommunityId))
                {
                    maxDeltaQ = refinedDeltaQ;
                    newCommunityId = refinedCommunityId;
                }
            }

            if(newCommunityId == nodeId)
                continue;

            weightsToRest[newCommunityId] += weightsToRest[nodeId] - (2.0 * neighbourWeightTo(newCommunityId));
            refinedDegrees[newCommunityId] += nodeWeight;
            refinedSizes[newCommunityId]++;
            refinedCommunities[nodeId] = newCommunityId;
            merged = true;
        }

        setProgress(-1);

        // Coarsening by a refinement that merged nothing would leave the graph as it is
        return merged ? refinedCommunities : communities;
    };

    // Each node starts in a community of its own
    NodeArray<CommunityId> finalCommunities(target);
    for(auto nodeId : target.nodeIds())
    {
        if(target.typeOf(nodeId) != MultiElementType::Tail)
            finalCommunities[nodeId] = nodeId;
    }

    relabel(baseLevel._nodeIds, finalCommunities);

    // Each iteration starts from the communities of the last, and can only improve on them
    bool finished = totalWeight <= 0.0;
    while(!finished)
    {
        level = baseLevel;
        communities = finalCommunities;
        iterations.clear();

        size_t subProgressIteration = 1;
        bool coarsenable = false;
        do
        {
            setPhase(QObject::tr("Leiden Iteration %1.%2")
                .arg(QString::number(progressIteration), QString::number(subProgressIteration)));
            coarsenable = moveNodes();

            if(coarsenable && !cancelled())
            {
                setPhase(QObject::tr("Leiden Iteration %1.%2 Refining")
                    .arg(QString::number(progressIteration), QString::number(subProgressIteration)));
                auto refinedCommunities = refine();

                setPhase(QObject::tr("Leiden Iteration %1.%2 Coarsening")
                    .arg(QString::number(progressIteration), QString::number(subProgressIteration)));
                relabel(level._nodeIds, refinedCommunities);
                relabel(level._nodeIds, communities);

                // Each refined community starts in the community that contains it
                NodeArray<CommunityId> coarseCommunities(target);
                for(auto nodeId : level._nodeIds)
                    coarseCommunities[refinedCommunities[nodeId]] = communities[nodeId];

                iterations.emplace_back(refinedCommunities);
                level = coarsen(refinedCommunities);
                communities = coarseCommunities;
            }

            subProgressIteration++;
        }
        while(coarsenable && !cancelled());

        if(cancelled())
            return;

        // Walk back over our iterations to find each node's community
        NodeArray<CommunityId> iterationCommunities(target);
        for(auto nodeId : target.nodeIds())
        {
            if(target.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto coarseNodeId = std::accumulate(iterations.begin(), iterations.end(), nodeId,
                [](NodeId id, const auto& iteration) { return iteration.at(id); });

            iterationCommunities[nodeId] = communities.at(coarseNodeId);
        }

        relabel(baseLevel._nodeIds, iterationCommunities);
        finished = iterationCommunities == finalCommunities;
        finalCommunities = iterationCommunities;

        progressIteration++;
    }

    if(cancelled())
        return;

    setPhase(QObject::tr("Leiden Finalising"));

    // Sort communities by size
    std::map<CommunityId, size_t> communityHistogram;
    for(auto nodeId : target.nodeIds())
        communityHistogram[finalCommunities[nodeId]]++;
    std::vector<std::pair<CommunityId, size_t>> sortedCommunityHistogram;
    std::ranges::copy(communityHistogram, std::back_inserter(sortedCommunityHistogram));
    std::ranges::sort(sortedCommunityHistogram,
    [](const auto& a, const auto& b)
    {
        if(a.second == b.second)
            return a.first > b.first;

        return a.second > b.second;
    });

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
        auto communityId = finalCommunities[nodeId];
        if(communityId.isNull())
            continue;

        clusterNumber = clusterNumbers[communityId];
        clusterNames[nodeId] = u"Cluster %1"_s.arg(clusterNumber);
        clusterSizes[nodeId] = static_cast<int>(communityHistogram.at(communityId));
    }

    _graphModel->createAttribute(LeidenTransformFactory::clusterAttribute(_weighted))
        .setDescription(QObject::tr("The Leiden cluster in which the node resides."))
        .setStringValueFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId]; })
        .setDisplayValueFns(Attribute::numberedDisplayValueFns(u"Cluster "_s, QObject::tr("Cluster %1")))
        .setValueMissingFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId].isEmpty(); })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable);

    _graphModel->createAttribute(LeidenTransformFactory::clusterSizeAttribute(_weighted))
        .setDescription(QObject::tr("The size of the Leiden cluster in which the node resides."))
        .setIntValueFn([clusterSizes](NodeId nodeId) { return clusterSizes[nodeId]; })
        .setFlag(AttributeFlag::AutoRange);
}
