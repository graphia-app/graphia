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

#ifndef EDGEREDUCTIONTRANSFORM_H
#define EDGEREDUCTIONTRANSFORM_H

#include "transform/graphtransform.h"

class EdgeReductionTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
};

class EdgeReductionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeReductionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the complexity of the graph by removing a pseudo-random "
            "percentage of edges, down to a specified minimum, per node.");
    }

    QString category() const override { return QObject::tr("Edge Reduction"); }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Percentage")
                .setType(ValueType::Int)
                .setDescription(QObject::tr("The percentage of edges to retain, per node."))
                .setInitialValue(10)
                .setRange(0, 100),

            GraphTransformParameter::create("Minimum")
                .setType(ValueType::Int)
                .setDescription(QObject::tr("The minimum number of edges to retain, per node."))
                .setInitialValue(5)
                .setMin(1)
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // EDGEREDUCTIONTRANSFORM_H
