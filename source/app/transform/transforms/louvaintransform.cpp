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

#include "louvaintransform.h"

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
#include <iterator>
#include <numeric>
#include <span>
#include <utility>

using namespace Qt::Literals::StringLiterals;

// https://arxiv.org/abs/0803.0476

void LouvainTransform::apply(TransformedGraph& target)
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
    setPhase(QObject::tr("Louvain Initialising"));

    NodeArray<CommunityId> communities(target);
    NodeArray<double> weightedDegrees(target);
    NodeArray<double> communityDegrees(target, 0.0);

    // The weight from one node to each community it neighbours
    NodeArray<double> neighbourCommunityWeights(target, 0.0);
    NodeArray<bool> isNeighbourCommunity(target, false);
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
                level._neighbours.push_back({target.edgeById(edgeId).oppositeId(nodeId), weights[edgeId]});

            level._neighbourEnds[nodeId] = level._neighbours.size();
        }

        return level;
    };

    auto level = targetLevel();

    auto add = [&](CommunityId community, NodeId nodeId)
    {
        communities[nodeId] = community;
        communityDegrees[community] += weightedDegrees[nodeId];
    };

    auto remove = [&](CommunityId community, NodeId nodeId)
    {
        communities[nodeId].setToNull();
        communityDegrees[community] -= weightedDegrees[nodeId];
    };

    auto addNeighbourWeight = [&](CommunityId community, double weight)
    {
        if(!isNeighbourCommunity.get(community))
        {
            isNeighbourCommunity.set(community, true);
            neighbourCommunityIds.push_back(community);
        }

        neighbourCommunityWeights[community] += weight;
    };

    auto clearNeighbourWeights = [&]
    {
        for(auto community : neighbourCommunityIds)
        {
            neighbourCommunityWeights[community] = 0.0;
            isNeighbourCommunity.set(community, false);
        }

        neighbourCommunityIds.clear();
    };

    auto neighboursOf = [&](NodeId nodeId)
    {
        auto start = level._neighbourStarts[nodeId];
        return std::span(level._neighbours).subspan(start, level._neighbourEnds[nodeId] - start);
    };

    auto relabel = [&]
    {
        NodeArray<CommunityId> idMap(target);
        CommunityId nextCommunityId = 0;

        for(auto nodeId : level._nodeIds)
        {
            auto oldCommunityId = communities[nodeId];

            if(idMap[oldCommunityId].isNull())
                idMap[oldCommunityId] = nextCommunityId++;

            communities[nodeId] = idMap[oldCommunityId];
        }
    };

    // Each community becomes a node, whose neighbours are the communities its members neighbour
    auto coarsen = [&]
    {
        std::vector<std::vector<NodeId>> communityMembers;
        for(auto nodeId : level._nodeIds)
        {
            auto index = static_cast<size_t>(communities[nodeId]);
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
                    addNeighbourWeight(communities[neighbourNodeId], weight);
            }

            auto communityId = communities[members.front()];
            coarseLevel._nodeIds.push_back(communityId);
            coarseLevel._neighbourStarts[communityId] = coarseLevel._neighbours.size();

            for(auto neighbourCommunityId : neighbourCommunityIds)
            {
                coarseLevel._neighbours.push_back({neighbourCommunityId,
                    neighbourCommunityWeights[neighbourCommunityId]});
            }

            coarseLevel._neighbourEnds[communityId] = coarseLevel._neighbours.size();
        }

        return coarseLevel;
    };

    auto moveNodes = [&]
    {
        if(cancelled())
            return false;

        CommunityId nextCommunityId = 0;
        for(auto nodeId : level._nodeIds)
        {
            for(const auto& neighbour : neighboursOf(nodeId))
                weightedDegrees[nodeId] += neighbour._weight;

            add(nextCommunityId++, nodeId);
        }

        size_t subProgressIteration = 1;
        bool modified = false;
        bool improved = false;
        do
        {
            improved = false;
            setProgress(0);
            uint64_t nodeIndex = 0;

            setPhase(QObject::tr("Louvain Iteration %1.%2")
                .arg(QString::number(progressIteration), QString::number(subProgressIteration++)));

            for(auto nodeId : level._nodeIds)
            {
                setProgress(static_cast<int>((nodeIndex++ * 100) /
                    static_cast<uint64_t>(level._nodeIds.size())));

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
                remove(communityId, nodeId);

                double maxDeltaQ = 0.0;
                CommunityId bestCommunityId;

                // Find the neighbouring community with the greatest delta Q, the lowest id among equals
                for(auto neighbourCommunityId : neighbourCommunityIds)
                {
                    auto weight = neighbourCommunityWeights[neighbourCommunityId];
                    auto communityWeight = communityDegrees.at(neighbourCommunityId);
                    auto nodeWeight = weightedDegrees[nodeId];

                    auto deltaQ = (resolution * weight) -
                        ((communityWeight * nodeWeight) / totalWeight);

                    if(deltaQ > maxDeltaQ || (deltaQ == maxDeltaQ && !bestCommunityId.isNull() &&
                        neighbourCommunityId < bestCommunityId))
                    {
                        maxDeltaQ = deltaQ;
                        bestCommunityId = neighbourCommunityId;
                    }
                }

                auto newCommunityId = bestCommunityId.isNull() ? communityId : bestCommunityId;
                add(newCommunityId, nodeId);

                if(newCommunityId != communityId)
                    improved = modified = true;

                if(cancelled())
                    break;
            }

            setProgress(-1);
        }
        while(improved && !cancelled());

        return modified;
    };

    bool finished = false;
    do
    {
        setProgress(-1);

        communities.resetElements();
        weightedDegrees.resetElements();
        communityDegrees.resetElements();

        finished = !moveNodes();

        if(!finished && !cancelled())
        {
            relabel();
            iterations.emplace_back(communities);

            setPhase(QObject::tr("Louvain Iteration %1 Coarsening")
                .arg(QString::number(progressIteration)));
            level = coarsen();
        }

        progressIteration++;
    }
    while(!finished && !cancelled());

    if(cancelled())
        return;

    setPhase(QObject::tr("Louvain Finalising"));

    // Set each CommunityId to be the same as the NodeId, initially
    for(auto nodeId : target.nodeIds())
    {
        if(target.typeOf(nodeId) == MultiElementType::Tail)
            communities[nodeId].setToNull();
        else
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
        auto communityId = communities[nodeId];
        if(communityId.isNull())
            continue;

        clusterNumber = clusterNumbers[communityId];
        clusterNames[nodeId] = u"Cluster %1"_s.arg(clusterNumber);
        clusterSizes[nodeId] = static_cast<int>(communityHistogram.at(communityId));
    }

    _graphModel->createAttribute(LouvainTransformFactory::clusterAttribute(_weighted))
        .setDescription(QObject::tr("The Louvain cluster in which the node resides."))
        .setStringValueFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId]; })
        .setDisplayValueFns(Attribute::numberedDisplayValueFns(u"Cluster "_s, QObject::tr("Cluster %1")))
        .setValueMissingFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId].isEmpty(); })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable);

    _graphModel->createAttribute(LouvainTransformFactory::clusterSizeAttribute(_weighted))
        .setDescription(QObject::tr("The size of the Louvain cluster in which the node resides."))
        .setIntValueFn([clusterSizes](NodeId nodeId) { return clusterSizes[nodeId]; })
        .setFlag(AttributeFlag::AutoRange);
}
