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

class Partition
{
public:
    using CommunityId = NodeId; // Slight Hack, be careful with this

private:
    GraphTransform* _transform;
    const TransformedGraph* _graph = nullptr;
    MutableGraph _coarseGraph;
    EdgeArray<double> _weights;
    double _resolution = 1.0;

    NodeArray<CommunityId> _communities;
    NodeArray<double> _weightedDegrees;
    std::map<CommunityId, int> _communityDegrees;

    void add(CommunityId community, NodeId nodeId)
    {
        _communities[nodeId] = community;
        _communityDegrees[community] += static_cast<int>(_weightedDegrees[nodeId]);
    }

    void remove(CommunityId community, NodeId nodeId)
    {
        _communities[nodeId].setToNull();
        _communityDegrees[community] -= static_cast<int>(_weightedDegrees[nodeId]);
    }

    void reset()
    {
        _communities.resetElements();
        _weightedDegrees.resetElements();
        _communityDegrees.clear();
    }

public:
    Partition(GraphTransform* transform, const TransformedGraph* graph, double resolution) :
        _transform(transform),
        _graph(graph),
        _coarseGraph(graph->mutableGraph()),
        _weights(*graph, 1.0), _resolution(resolution),
        _communities(*graph), _weightedDegrees(*graph)
    {}

    const NodeArray<CommunityId>& communities() const { return _communities; }

    void setWeight(EdgeId edgeId, double weight)
    {
        Q_ASSERT(_coarseGraph.containsEdgeId(edgeId));
        _weights[edgeId] = weight;
    }

    void relabel()
    {
        std::map<CommunityId, CommunityId> idMap;
        CommunityId nextCommunityId = 0;

        for(auto nodeId : _coarseGraph.nodeIds())
        {
            if(_coarseGraph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto oldCommunityId = _communities[nodeId];

            if(!u::contains(idMap, oldCommunityId))
                _communities[nodeId] = idMap[oldCommunityId] = nextCommunityId++;
            else
                _communities[nodeId] = idMap.at(oldCommunityId);
        }
    }

    void makeSingletonClusters()
    {
        reset();

        CommunityId nextCommunityId = 0;
        for(auto nodeId : _coarseGraph.nodeIds())
        {
            if(_coarseGraph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            for(auto edgeId : _coarseGraph.nodeById(nodeId).edgeIds())
                _weightedDegrees[nodeId] += _weights[edgeId];

            add(nextCommunityId++, nodeId);
        }
    }

    bool moveNodes()
    {
        if(_transform->cancelled())
            return false;

        bool modified = false;
        std::deque<NodeId> nodeIdQueue;

        for(auto nodeId : _coarseGraph.nodeIds())
            nodeIdQueue.push_back(nodeId);

        NodeArray<bool> visited(_coarseGraph);

        int highestProgress = 0;
        _transform->setProgress(0);

        double totalWeight = std::accumulate(_coarseGraph.edgeIds().begin(), _coarseGraph.edgeIds().end(), 0.0,
        [this](double d, EdgeId edgeId)
        {
            return d + _weights[edgeId];
        });

        do
        {
            auto nodeId = nodeIdQueue.front();
            nodeIdQueue.pop_front();

            auto progress = static_cast<int>(
                (static_cast<uint64_t>(_coarseGraph.numNodes() - nodeIdQueue.size()) * 100) /
                static_cast<uint64_t>(_coarseGraph.numNodes()));

            highestProgress = std::max(highestProgress, progress);
            _transform->setProgress(highestProgress);

            if(_coarseGraph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            // Find total weights of neighbour communities
            std::map<CommunityId, double> neighbourCommunityWeights;
            for(auto edgeId : _coarseGraph.nodeById(nodeId).edgeIds())
            {
                auto neighbourNodeId = _coarseGraph.edgeById(edgeId).oppositeId(nodeId);

                // Skip loop edges
                if(neighbourNodeId == nodeId)
                    continue;

                if(_coarseGraph.typeOf(neighbourNodeId) == MultiElementType::Tail)
                    continue;

                auto neighbourCommunityId = _communities.at(neighbourNodeId);

                neighbourCommunityWeights[neighbourCommunityId] += _weights[edgeId];
            }

            auto communityId = _communities[nodeId];
            remove(communityId, nodeId);

            visited.set(nodeId, true);

            double maxDeltaQ = 0.0;
            auto newCommunityId = communityId;

            // Find the neighbouring community with the greatest delta Q
            for(auto [neighbourCommunityId, weight] : neighbourCommunityWeights)
            {
                auto communityWeight = _communityDegrees.at(neighbourCommunityId);
                auto nodeWeight = _weightedDegrees[nodeId];

                auto deltaQ = (_resolution * weight) -
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

                for(auto edgeId : _coarseGraph.nodeById(nodeId).edgeIds())
                {
                    auto neighbourNodeId = _coarseGraph.edgeById(edgeId).oppositeId(nodeId);

                    // Skip loop edges
                    if(neighbourNodeId == nodeId)
                        continue;

                    if(_coarseGraph.typeOf(neighbourNodeId) == MultiElementType::Tail)
                        continue;

                    if(newCommunityId == _communities[neighbourNodeId])
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
        while(!nodeIdQueue.empty() && !_transform->cancelled());

        _transform->setProgress(-1);

        return modified;
    }

    Partition refine()
    {
        Partition pRefined(_transform, _graph, _resolution);
        pRefined.reset();
        pRefined.makeSingletonClusters();

        std::map<CommunityId, std::vector<NodeId>> c;

        for(auto nodeId : _coarseGraph.nodeIds())
        {
            auto communityId = _communities.at(nodeId);
            c[communityId].push_back(nodeId);
        }

        NodeArray<bool> merged(_coarseGraph, false);
        NodeArray<double> cToSMinusC(_coarseGraph);
        NodeArray<double> refinedVolumes(_coarseGraph);

        for(auto nodeId : _coarseGraph.nodeIds())
        {
            const auto& node = _coarseGraph.nodeById(nodeId);

            for(auto edgeId : node.edgeIds())
            {
                const auto& edge = _coarseGraph.edgeById(edgeId);
                auto neighbourId = edge.oppositeId(nodeId);

                if(_communities.at(nodeId) == _communities.at(neighbourId))
                    cToSMinusC[nodeId] += _weights[edgeId];
                else
                    refinedVolumes[nodeId] += _weights[edgeId];
            }
        }

        auto mergeNodesSubset = [&](const std::vector<NodeId>& subset)
        {
            for(auto nodeId : subset)
            {
                if(merged.get(nodeId))
                    continue;

                if(0/* node is not well connected*/)
                    continue;
            }
        };

        for(const auto& [communityId, nodeIds] : c)
            mergeNodesSubset(nodeIds);

        return pRefined;
    }

    void coarsen(const Partition& p)
    {
        MutableGraph coarseGraph;
        EdgeArray<double> newWeights(_weights);
        newWeights.fill(0.0);

        coarseGraph.performTransaction([&](IMutableGraph&)
        {
            // Create a node for each community
            for(auto nodeId : _coarseGraph.nodeIds())
            {
                if(_coarseGraph.typeOf(nodeId) == MultiElementType::Tail)
                    continue;

                auto newNodeId = p.communities().at(nodeId);

                if(coarseGraph.containsNodeId(newNodeId))
                    continue;

                coarseGraph.reserveNodeId(newNodeId);
                auto assignedNodeId = coarseGraph.addNode(newNodeId);
                Q_ASSERT(assignedNodeId == newNodeId);
            }

            uint64_t edgeIndex = 0;

            // Create an edge between community nodes for each
            // pair of connected communities in the base graph
            for(auto edgeId : _coarseGraph.edgeIds())
            {
                _transform->setProgress(static_cast<int>((edgeIndex++ * 100) /
                    static_cast<uint64_t>(_coarseGraph.numEdges())));

                if(_transform->cancelled())
                    break;

                const auto& edge = _coarseGraph.edgeById(edgeId);
                auto sourceId = p.communities().at(edge.sourceId());
                auto targetId = p.communities().at(edge.targetId());
                const double newWeight = _weights[edgeId];

                auto newEdgeId = coarseGraph.firstEdgeIdBetween(sourceId, targetId);
                if(newEdgeId.isNull())
                    newEdgeId = coarseGraph.addEdge(sourceId, targetId);

                newWeights[newEdgeId] += newWeight;
            }
        });

        _coarseGraph = coarseGraph;
        _weights = newWeights;
    }
};

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

    std::vector<NodeArray<Partition::CommunityId>> iterations;
    size_t progressIteration = 1;
    setPhase(QStringLiteral("Leiden Initialising"));

    MutableGraph graph(target.mutableGraph());
    Partition p(this, &target, resolution);

    if(_weighted)
    {
        if(config().attributeNames().empty())
        {
            addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
            return;
        }

        auto attribute = _graphModel->attributeValueByName(
            config().attributeNames().front());

        for(auto edgeId : target.edgeIds())
            p.setWeight(edgeId, attribute.numericValueOf(edgeId));
    }

    bool finished = false;
    do // NOLINT bugprone-infinite-loop
    {
        setProgress(-1);
        setPhase(QStringLiteral("Leiden Iteration %1")
            .arg(QString::number(progressIteration)));

        p.makeSingletonClusters();
        finished = !p.moveNodes();

        if(finished || cancelled())
            break;

        //p.relabel();
        iterations.emplace_back(p.communities());

        setPhase(QStringLiteral("Leiden Iteration %1 Refining")
            .arg(QString::number(progressIteration)));
        auto pRefined = p.refine();

        setPhase(QStringLiteral("Leiden Iteration %1 Coarsening")
            .arg(QString::number(progressIteration)));
        p.coarsen(pRefined);

        progressIteration++;
    }
    while(!cancelled());

    if(cancelled())
        return;

    setPhase(QStringLiteral("Leiden Finalising"));

    NodeArray<Partition::CommunityId> communities(target);

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
    std::map<Partition::CommunityId, size_t> communityHistogram;
    for(auto nodeId : target.nodeIds())
        communityHistogram[communities[nodeId]]++;
    std::vector<std::pair<Partition::CommunityId, size_t>> sortedCommunityHistogram;
    std::copy(communityHistogram.begin(), communityHistogram.end(),
        std::back_inserter(sortedCommunityHistogram));
    std::sort(sortedCommunityHistogram.begin(), sortedCommunityHistogram.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // Assign cluster numbers to each community
    std::map<Partition::CommunityId, size_t> clusterNumbers;
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
