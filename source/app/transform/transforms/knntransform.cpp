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

#include "knntransform.h"

#include "app/transform/transformedgraph.h"
#include "app/graph/graphmodel.h"
#include "shared/utils/container.h"

#include <algorithm>
#include <functional>
#include <memory>

#include <QObject>

using namespace Qt::Literals::StringLiterals;

void KNNTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("k-NN"));

    if(config().attributeNames().empty())
    {
        addAlert(AlertType::Error, QObject::tr("Invalid parameter"));
        return;
    }

    auto attribute = _graphModel->attributeValueByName(config().attributeNames().front());

    auto k = static_cast<size_t>(std::get<int>(config().parameterByName(u"k"_s)->_value));
    const bool ascending = config().parameterHasValue(u"Rank Order"_s, u"Ascending"_s);

    struct KnnRank
    {
        size_t _source = 0;
        size_t _target = 0;
        double _mean = 0.0;
    };

    EdgeArray<KnnRank> ranks(target);
    EdgeArray<bool> removees(target, true);

    uint64_t progress = 0;
    for(auto nodeId : target.nodeIds())
    {
        auto edgeIds = target.nodeById(nodeId).edgeIds();
        auto kthPlus1 = edgeIds.begin() + static_cast<std::ptrdiff_t>(std::min(k, edgeIds.size()));

        if(ascending)
        {
            std::partial_sort(edgeIds.begin(), kthPlus1, edgeIds.end(),
                [&attribute](auto a, auto b) { return attribute.numericValueOf(a) < attribute.numericValueOf(b); });
        }
        else
        {
            std::partial_sort(edgeIds.begin(), kthPlus1, edgeIds.end(),
                [&attribute](auto a, auto b) { return attribute.numericValueOf(a) > attribute.numericValueOf(b); });
        }

        for(auto it = edgeIds.begin(); it != kthPlus1; ++it)
        {
            auto position = static_cast<size_t>(std::distance(edgeIds.begin(), it) + 1);

            if(target.edgeById(*it).sourceId() == nodeId)
                ranks[*it]._source = position;
            else
                ranks[*it]._target = position;

            removees.set(*it, false);
        }

        setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numNodes())));
    }

    progress = 0;

    for(const auto& edgeId : target.edgeIds())
    {
        if(removees.get(edgeId))
        {
            target.mutableGraph().removeEdge(edgeId);
        }
        else
        {
            auto& rank = ranks[edgeId];

            if(rank._source == 0)
                rank._mean = static_cast<double>(rank._target);
            else if(rank._target == 0)
                rank._mean = static_cast<double>(rank._source);
            else
                rank._mean = static_cast<double>(rank._source + rank._target) * 0.5;
        }

        setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numEdges())));
    }

    setProgress(-1);

    _graphModel->createAttribute(QObject::tr("k-NN Source Rank"))
        .setDescription(QObject::tr("The ranking given by k-NN, relative to its source node."))
        .setIntValueFn([ranks](EdgeId edgeId) { return static_cast<int>(ranks[edgeId]._source); })
        .setFlag(AttributeFlag::AutoRange);

    _graphModel->createAttribute(QObject::tr("k-NN Target Rank"))
        .setDescription(QObject::tr("The ranking given by k-NN, relative to its target node."))
        .setIntValueFn([ranks](EdgeId edgeId) { return static_cast<int>(ranks[edgeId]._target); })
        .setFlag(AttributeFlag::AutoRange);

    _graphModel->createAttribute(QObject::tr("k-NN Mean Rank"))
        .setDescription(QObject::tr("The mean ranking given by k-NN."))
        .setFloatValueFn([ranks](EdgeId edgeId) { return ranks[edgeId]._mean; })
        .setFlag(AttributeFlag::AutoRange);
}

std::unique_ptr<GraphTransform> KNNTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<KNNTransform>(*graphModel());
}
