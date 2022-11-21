/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "graphsizeestimate.h"

#include "shared/graph/elementid_containers.h"
#include "shared/graph/undirectededge.h"

#include <QVector>

#include <algorithm>
#include <set>
#include <cstdlib>
#include <cmath>

QVariantMap graphSizeEstimateThreshold(EdgeList edgeList,
    size_t numSampleNodes, size_t maxNodes)
{
    if(edgeList.empty())
        return {};

    auto maxEdges = maxNodes * maxNodes;
    auto nodesScale = static_cast<double>(maxNodes) / static_cast<double>(numSampleNodes);
    auto edgesScale = nodesScale * nodesScale;

    std::sort(edgeList.begin(), edgeList.end(),
        [](const auto& a, const auto& b) { return std::abs(a._weight) > std::abs(b._weight); });

    const auto smallestWeight = std::abs(edgeList.back()._weight);
    const auto largestWeight = std::abs(edgeList.front()._weight);
    const auto numEstimateSamples = 100;
    const auto sampleQuantum = (largestWeight - smallestWeight) / (numEstimateSamples - 1);
    auto sampleCutoff = std::abs(largestWeight) - sampleQuantum;

    QVector<double> keys;
    QVector<double> estimatedNumNodes;
    QVector<double> estimatedNumEdges;
    QVector<double> estimatedNumUniqueEdges;

    keys.reserve(static_cast<qsizetype>(numEstimateSamples));
    estimatedNumNodes.reserve(static_cast<qsizetype>(numEstimateSamples));
    estimatedNumEdges.reserve(static_cast<qsizetype>(numEstimateSamples));
    estimatedNumUniqueEdges.reserve(static_cast<qsizetype>(numEstimateSamples));

    size_t numEdges = 0;
    size_t numUniqueEdges = 0;
    NodeIdSet nonSingletonNodes;
    std::set<UndirectedEdge> uniqueEdges;
    auto weight = largestWeight;

    auto append = [&](double w, size_t n, size_t e, size_t ue)
    {
        keys.append(w);
        estimatedNumNodes.append(std::ceil(std::min(static_cast<double>(n) * nodesScale, static_cast<double>(maxNodes))));
        estimatedNumEdges.append(std::ceil(std::min(static_cast<double>(e) * edgesScale, static_cast<double>(maxEdges))));
        estimatedNumUniqueEdges.append(std::ceil(std::min(static_cast<double>(ue) * edgesScale, static_cast<double>(maxEdges))));
    };

    for(const auto& edge : edgeList)
    {
        if(std::abs(edge._weight) < sampleCutoff)
        {
            append(weight, nonSingletonNodes.size(), numEdges, numUniqueEdges);
            sampleCutoff -= sampleQuantum;
            weight = std::abs(edge._weight);
        }

        nonSingletonNodes.insert(edge._source);
        nonSingletonNodes.insert(edge._target);
        numEdges++;

        if(uniqueEdges.emplace(edge._source, edge._target).second)
            numUniqueEdges++;
    }

    append(std::min(weight, smallestWeight), nonSingletonNodes.size(), numEdges, numUniqueEdges);

    std::reverse(keys.begin(), keys.end());
    std::reverse(estimatedNumNodes.begin(), estimatedNumNodes.end());
    std::reverse(estimatedNumEdges.begin(), estimatedNumEdges.end());
    std::reverse(estimatedNumUniqueEdges.begin(), estimatedNumUniqueEdges.end());

    keys.shrink_to_fit();
    estimatedNumNodes.shrink_to_fit();
    estimatedNumEdges.shrink_to_fit();
    estimatedNumUniqueEdges.shrink_to_fit();

    QVariantMap map;
    map.insert(QStringLiteral("keys"), QVariant::fromValue(keys));
    map.insert(QStringLiteral("numNodes"), QVariant::fromValue(estimatedNumNodes));
    map.insert(QStringLiteral("numEdges"), QVariant::fromValue(estimatedNumEdges));
    map.insert(QStringLiteral("numUniqueEdges"), QVariant::fromValue(estimatedNumUniqueEdges));
    return map;
}
