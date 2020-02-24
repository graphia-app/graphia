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

#ifndef PERCENTNNTRANSFORM_H
#define PERCENTNNTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class PercentNNTransform : public GraphTransform
{
public:
    explicit PercentNNTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class PercentNNTransformFactory : public GraphTransformFactory
{
public:
    explicit PercentNNTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the number of edges in the graph using a variation of the "
            R"(<a href="https://graphia-app.github.io/redirects/knn">)"
            R"(k-nearest neighbours</a> algorithm, but instead of choosing the top k edges, )"
            "choose a percentage of the highest ranking edges.");
    }

    QString category() const override { return QObject::tr("Edge Reduction"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Ranked Attribute",
                ElementType::Edge, ValueType::Numerical,
                QObject::tr("The attribute whose value is used to rank edges.")
            }
        };
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Percent",
                ValueType::Int,
                QObject::tr("The percentage of edges to rank and retain, per node."),
                10, 0, 100
            },
            {
                "Minimum",
                ValueType::Int,
                QObject::tr("The minimum number of edges to retain, per node."),
                5, 1
            },
            {
                "Rank Order",
                ValueType::StringList,
                QObject::tr("Whether or not larger or smaller values are ranked higher."),
                QStringList{"Descending", "Ascending"}
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // PERCENTNNTRANSFORM_H
