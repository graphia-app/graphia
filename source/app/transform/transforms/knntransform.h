/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef KNNTRANSFORM_H
#define KNNTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include "shared/utils/redirects.h"

#include <vector>

class KNNTransform : public GraphTransform
{
public:
    explicit KNNTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class KNNTransformFactory : public GraphTransformFactory
{
public:
    explicit KNNTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the number of edges in the graph using the %1 algorithm.")
            .arg(u::redirectLink("knn", QObject::tr("k-nearest neighbours")));
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
            GraphTransformParameter::create("k")
                .setType(ValueType::Int)
                .setDescription(QObject::tr("The number of edges to rank and retain, per node."))
                .setInitialValue(5)
                .setMin(1),

            GraphTransformParameter::create("Rank Order")
                .setType(ValueType::StringList)
                .setDescription(QObject::tr("Whether or not larger or smaller values are ranked higher."))
                .setInitialValue(QStringList{"Descending", "Ascending"})
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // KNNTRANSFORM_H
