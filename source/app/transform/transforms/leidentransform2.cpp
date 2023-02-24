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

#include "leidentransform2.h"

#include "transform/transformedgraph.h"

#include "shared/graph/grapharray.h"
#include "shared/utils/container.h"

#include "graph/graphmodel.h"

#include <map>
#include <vector>
#include <deque>
#include <stack>
#include <algorithm>
#include <cmath>

// https://www.nature.com/articles/s41598-019-41695-z

class LeidenPartition
{
public:
    using CommunityId = NodeId;

private:
    using EdgeWeights = EdgeArray<double>;
    using NodeDegreeWeights = NodeArray<double>;
    using NodeCardinality = NodeArray<size_t>;
    using CommunityDegreeWeights = std::map<CommunityId, double>;
    using Communities = NodeArray<CommunityId>;

    const Graph* _graph = nullptr;
    EdgeWeights _edgeWeights;
    NodeDegreeWeights _nodeDegreeWeights;
    NodeCardinality _nodeCardinality;
    CommunityDegreeWeights _communityDegreeWeights;
    Communities _communities;

    void addNodeTo(CommunityId community, NodeId nodeId)
    {
        _communities[nodeId] = community;
        _communityDegreeWeights[community] += _nodeDegreeWeights[nodeId];
    }

    void removeNodeFrom(CommunityId community, NodeId nodeId)
    {
        _communities[nodeId].setToNull();
        _communityDegreeWeights[community] -= _nodeDegreeWeights[nodeId];
    }

    void reset()
    {
        _nodeDegreeWeights.resetElements();
        _communityDegreeWeights.clear();
        _communities.resetElements();
    }

    void makeSingletonClusters()
    {
        reset();

        CommunityId nextCommunityId = 0;
        for(auto nodeId : _graph->nodeIds())
        {
            if(_graph->typeOf(nodeId) == MultiElementType::Tail)
                continue;

            for(auto edgeId : _graph->nodeById(nodeId).edgeIds())
                _nodeDegreeWeights[nodeId] += _edgeWeights[edgeId];

            addNodeTo(nextCommunityId++, nodeId);
        }
    }

public:
    explicit LeidenPartition(const Graph& graph) :
        _graph(&graph), _edgeWeights(graph, 1.0),
        _nodeDegreeWeights(graph), _nodeCardinality(graph, 1),
        _communities(graph)
    {
        makeSingletonClusters();
    }

    const NodeArray<CommunityId>& communities() const { return _communities; }

    bool moveNodes(double gamma)
    {
        bool modified = false;
        std::deque<NodeId> nodeIdQueue;

        for(auto nodeId : _graph->nodeIds())
            nodeIdQueue.push_back(nodeId);

        NodeArray<bool> visited(*_graph);

        double totalEdgeWeight = std::accumulate(_graph->edgeIds().begin(), _graph->edgeIds().end(), 0.0,
            [this](double d, EdgeId edgeId) { return d + _edgeWeights[edgeId]; });

        do
        {
            auto nodeId = nodeIdQueue.front();
            nodeIdQueue.pop_front();

            if(_graph->typeOf(nodeId) == MultiElementType::Tail)
                continue;

            // Find total weights of neighbour communities
            CommunityDegreeWeights neighbourCommunityWeights;
            for(auto edgeId : _graph->nodeById(nodeId).edgeIds())
            {
                auto neighbourNodeId = _graph->edgeById(edgeId).oppositeId(nodeId);

                // Skip loop edges
                if(neighbourNodeId == nodeId)
                    continue;

                if(_graph->typeOf(neighbourNodeId) == MultiElementType::Tail)
                    continue;

                auto neighbourCommunityId = _communities.at(neighbourNodeId);

                neighbourCommunityWeights[neighbourCommunityId] += _edgeWeights[edgeId];
            }

            auto communityId = _communities[nodeId];
            removeNodeFrom(communityId, nodeId);

            visited.set(nodeId, true);

            double maxDeltaQ = 0.0;
            auto newCommunityId = communityId;

            // Find the neighbouring community with the greatest delta Q
            for(auto [neighbourCommunityId, weight] : neighbourCommunityWeights)
            {
                auto communityWeight = _communityDegreeWeights.at(neighbourCommunityId);
                auto nodeWeight = _nodeDegreeWeights[nodeId];

                auto deltaQ = (gamma * weight) -
                    ((communityWeight * nodeWeight) / totalEdgeWeight);

                if(deltaQ > maxDeltaQ)
                {
                    maxDeltaQ = deltaQ;
                    newCommunityId = neighbourCommunityId;
                }
            }

            addNodeTo(newCommunityId, nodeId);

            if(newCommunityId != communityId)
            {
                modified = true;

                for(auto edgeId : _graph->nodeById(nodeId).edgeIds())
                {
                    auto neighbourNodeId = _graph->edgeById(edgeId).oppositeId(nodeId);

                    // Skip loop edges
                    if(neighbourNodeId == nodeId)
                        continue;

                    if(_graph->typeOf(neighbourNodeId) == MultiElementType::Tail)
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
        while(!nodeIdQueue.empty());

        return modified;
    }

    LeidenPartition refine(double gamma)
    {
        LeidenPartition refinedPartition = *this;
        refinedPartition.reset();
        refinedPartition.makeSingletonClusters();

        std::map<CommunityId, std::vector<NodeId>> subsets;

        for(auto nodeId : _graph->nodeIds())
        {
            auto communityId = _communities.at(nodeId);
            subsets[communityId].push_back(nodeId);
        }

        NodeArray<bool> merged(*_graph, false);
        NodeArray<double> cutWeights(*_graph);
        NodeArray<double> cToSMinusC(*_graph);
        NodeArray<double> refinedCommunityDegreeWeights(*_graph);

        for(auto communityId : _communities)
            cutWeights[communityId] = 0.0;

        std::vector<CommunityId> neighbourCommunities;

        double totalEdgeWeight = std::accumulate(_graph->edgeIds().begin(), _graph->edgeIds().end(), 0.0,
            [this](double d, EdgeId edgeId) { return d + _edgeWeights[edgeId]; }); //FIXME maybe store in member variable when _graph changes

        for(auto nodeId : _graph->nodeIds())
        {
            for(auto edgeId : _graph->nodeById(nodeId).edgeIds())
            {
                auto neighbourId = _graph->edgeById(edgeId).oppositeId(nodeId);

                if(_communities.at(nodeId) == _communities.at(neighbourId))
                    cToSMinusC[nodeId] += _edgeWeights[edgeId];
                else
                    refinedCommunityDegreeWeights[nodeId] += _edgeWeights[edgeId];
            }
        }

        auto mergeNodesSubset = [&](const std::vector<NodeId>& subset)
        {
            for(auto nodeId : subset)
            {
                auto previousCommunityId = _communities.at(nodeId);

                if(merged.get(nodeId))
                    continue;

                double weight = 0.0;
                for(auto edgeId : _graph->nodeById(nodeId).edgeIds())
                {
                    auto edgeWeight = _edgeWeights.at(edgeId);
                    auto neighbourId = _graph->edgeById(edgeId).oppositeId(nodeId);
                    auto neighbourCommunityId = _communities.at(neighbourId);
                    auto refinedNeighbourCommunityId = refinedPartition._communities.at(neighbourId);

                    if(previousCommunityId == neighbourCommunityId)
                    {
                        if(cutWeights[refinedNeighbourCommunityId] == 0.0)
                            neighbourCommunities.push_back(refinedNeighbourCommunityId);

                        cutWeights[refinedNeighbourCommunityId] += edgeWeight;
                    }

                    weight += edgeWeight;
                }

                auto v = (gamma * weight * (_communityDegreeWeights.at(previousCommunityId) - weight)) / totalEdgeWeight;

                if(cToSMinusC.at(nodeId) < v)
                    continue;

                auto refinedCommunityId = refinedPartition._communities.at(nodeId);
                auto threshold = cutWeights.at(refinedCommunityId) -
                    ((gamma * weight * (refinedCommunityDegreeWeights.at(refinedCommunityId) - weight)) / totalEdgeWeight);
                double maxDelta = 0.0;
                CommunityId newCommunityId;

                for(auto communityId : neighbourCommunities)
                {
                    auto refinedCommunityDegreeWeight = refinedCommunityDegreeWeights.at(communityId);

                    auto delta = cutWeights.at(communityId) -
                        ((gamma * weight * refinedCommunityDegreeWeight) / totalEdgeWeight);

                    if(delta < threshold)
                        continue;

                    auto v2 = (gamma * refinedCommunityDegreeWeight * (_communityDegreeWeights.at(previousCommunityId) -
                        refinedCommunityDegreeWeight)) / totalEdgeWeight;

                    if(cToSMinusC.at(communityId/*wtf?*/) >= v2 && delta > maxDelta)
                    {
                        maxDelta = delta;
                        newCommunityId = communityId;
                    }
                }

                if(newCommunityId.isNull())
                    continue;

                merged.set(newCommunityId, true);
                refinedPartition.removeNodeFrom(previousCommunityId, nodeId);

                refinedCommunityDegreeWeights[newCommunityId] += weight;
                cToSMinusC[newCommunityId] += cToSMinusC.at(refinedCommunityId) - (2.0 * cutWeights.at(newCommunityId));

                refinedPartition.addNodeTo(newCommunityId, nodeId);
            }
        };

        for(const auto& [communityId, nodeIds] : subsets)
            mergeNodesSubset(nodeIds);

        return refinedPartition;
    }
};

static MutableGraph coarsenGraph(const Graph& graph, const LeidenPartition& p)
{
    MutableGraph coarseGraph;
    /*EdgeArray<double> newWeights(_weights);
    newWeights.fill(0.0);*/

    coarseGraph.performTransaction([&](IMutableGraph&)
    {
        // Create a node for each community
        for(auto nodeId : graph.nodeIds())
        {
            if(graph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto newNodeId = p.communities().at(nodeId);

            if(coarseGraph.containsNodeId(newNodeId))
                continue;

            coarseGraph.reserveNodeId(newNodeId);
            auto assignedNodeId = coarseGraph.addNode(newNodeId);
            Q_ASSERT(assignedNodeId == newNodeId);
        }

        //uint64_t edgeIndex = 0;

        // Create an edge between community nodes for each
        // pair of connected communities in the base graph
        for(auto edgeId : graph.edgeIds())
        {
            /*_transform->setProgress(static_cast<int>((edgeIndex++ * 100) /
                static_cast<uint64_t>(graph.numEdges())));

            if(_transform->cancelled())
                break;*/

            const auto& edge = graph.edgeById(edgeId);
            auto sourceId = p.communities().at(edge.sourceId());
            auto targetId = p.communities().at(edge.targetId());
            //const double newWeight = _weights[edgeId];

            auto newEdgeId = coarseGraph.firstEdgeIdBetween(sourceId, targetId);
            if(newEdgeId.isNull())
                newEdgeId = coarseGraph.addEdge(sourceId, targetId);

            //newWeights[newEdgeId] += newWeight;
        }
    });

    return coarseGraph;
}

void LeidenTransform2::apply(TransformedGraph& target)
{
    auto resolution = 1.0 - std::get<double>(
        config().parameterByName(QStringLiteral("Granularity"))->_value);

    const auto minResolution = 0.5;
    const auto maxResolution = 30.0;

    const auto logMin = std::log10(minResolution);
    const auto logMax = std::log10(maxResolution);
    const auto logRange = logMax - logMin;

    resolution = std::pow(10.0f, logMin + (resolution * logRange));

    setPhase(QStringLiteral("Leiden"));

    std::stack<LeidenPartition> stack;

    const Graph* graph = &target;
    LeidenPartition partition(*graph);
    MutableGraph coarseGraph;

    do
    {
        auto modified = partition.moveNodes(resolution);

        if(!modified)
            break;

        auto refinedPartition = partition.refine(resolution);
        stack.push(refinedPartition);

        coarseGraph = coarsenGraph(*graph, refinedPartition);
        partition = refinedPartition;

    } while(true);

    /*_graphModel->createAttribute(QObject::tr(_weighted ? "Weighted Leiden Cluster" : "Leiden Cluster")) // clazy:exclude=tr-non-literal
        .setDescription(QObject::tr("The Leiden cluster in which the node resides."))
        .setStringValueFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId]; })
        .setValueMissingFn([clusterNames](NodeId nodeId) { return clusterNames[nodeId].isEmpty(); })
        .setFlag(AttributeFlag::FindShared)
        .setFlag(AttributeFlag::Searchable);

    _graphModel->createAttribute(QObject::tr(_weighted ? "Weighted Leiden Cluster Size" : "Leiden Cluster Size")) // clazy:exclude=tr-non-literal
        .setDescription(QObject::tr("The size of the Leiden cluster in which the node resides."))
        .setIntValueFn([clusterSizes](NodeId nodeId) { return clusterSizes[nodeId]; })
        .setFlag(AttributeFlag::AutoRange);*/
}
