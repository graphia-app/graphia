/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "edgereductiontransform.h"

#include "app/transform/transformedgraph.h"

#include <memory>
#include <random>

#include <QObject>

using namespace Qt::Literals::StringLiterals;

void EdgeReductionTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Edge Reduction"));

    auto percentage = static_cast<size_t>(std::get<int>(config().parameterByName(u"Percentage"_s)->_value));
    auto minimum = static_cast<size_t>(std::get<int>(config().parameterByName(u"Minimum"_s)->_value));

    EdgeArray<bool> removees(target, true);

    uint64_t progress = 0;
    for(auto nodeId : target.nodeIds())
    {
        auto edgeIds = target.nodeById(nodeId).edgeIds();

        if(edgeIds.empty())
            continue;

        std::mt19937 generator(static_cast<std::mt19937::result_type>(static_cast<size_t>(nodeId)));
        std::uniform_int_distribution<size_t> distribution(0, edgeIds.size() - 1);

        const size_t numEdgesToRetain = std::max(minimum, (edgeIds.size() * percentage) / 100);

        for(size_t i = 0u; i < numEdgesToRetain; i++)
        {
            auto index = distribution(generator);
            removees.set(edgeIds[index], false);
        }

        setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numNodes())));
    }

    progress = 0;

    for(const auto& edgeId : target.edgeIds())
    {
        if(removees.get(edgeId))
            target.mutableGraph().removeEdge(edgeId);

        setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numEdges())));
    }

    setProgress(-1);
}

std::unique_ptr<GraphTransform> EdgeReductionTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<EdgeReductionTransform>();
}
