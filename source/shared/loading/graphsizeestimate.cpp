/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include <QVector>

#include <algorithm>
#include <cstdlib>

QVariantMap graphSizeEstimate(EdgeList edgeList,
    double nodesScale, double edgesScale,
    double nodesMax, double edgesMax)
{
    if(edgeList.empty())
        return QVariantMap();

    std::sort(edgeList.begin(), edgeList.end(),
        [](const auto& a, const auto& b) { return std::abs(a._weight) > std::abs(b._weight); });

    const auto smallestWeight = edgeList.back()._weight;
    const auto numEstimateSamples = 100;
    const auto sampleQuantum = (1.0 - smallestWeight) / (numEstimateSamples - 1);
    auto sampleCutoff = 1.0;

    QVector<double> keys;
    QVector<double> estimatedNumNodes;
    QVector<double> estimatedNumEdges;

    keys.reserve(static_cast<int>(edgeList.size()));
    estimatedNumNodes.reserve(static_cast<int>(edgeList.size()));
    estimatedNumEdges.reserve(static_cast<int>(edgeList.size()));

    size_t numSampledEdges = 0;
    NodeIdSet nonSingletonNodes;

    for(const auto& edge : edgeList)
    {
        nonSingletonNodes.insert(edge._source);
        nonSingletonNodes.insert(edge._target);
        numSampledEdges++;

        if(std::abs(edge._weight) <= sampleCutoff)
        {
            keys.append(std::abs(edge._weight));
            estimatedNumNodes.append(std::min(nonSingletonNodes.size() * nodesScale, nodesMax));
            estimatedNumEdges.append(std::min(numSampledEdges * edgesScale, edgesMax));

            sampleCutoff -= sampleQuantum;
        }
    }

    keys.append(std::abs(edgeList.back()._weight));
    estimatedNumNodes.append(std::min(nonSingletonNodes.size() * nodesScale, nodesMax));
    estimatedNumEdges.append(std::min(numSampledEdges * edgesScale, edgesMax));

    std::reverse(keys.begin(), keys.end());
    std::reverse(estimatedNumNodes.begin(), estimatedNumNodes.end());
    std::reverse(estimatedNumEdges.begin(), estimatedNumEdges.end());

    QVariantMap map;
    map.insert(QStringLiteral("keys"), QVariant::fromValue(keys));
    map.insert(QStringLiteral("numNodes"), QVariant::fromValue(estimatedNumNodes));
    map.insert(QStringLiteral("numEdges"), QVariant::fromValue(estimatedNumEdges));
    return map;
}
