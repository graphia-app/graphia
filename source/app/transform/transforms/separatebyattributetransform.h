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

#ifndef SEPARATEBYATTRIBUTETRANSFORM_H
#define SEPARATEBYATTRIBUTETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class SeparateByAttributeTransform : public GraphTransform
{
public:
    explicit SeparateByAttributeTransform(const GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) override;

private:
    const GraphModel* _graphModel;
};

class SeparateByAttributeTransformFactory : public GraphTransformFactory
{
public:
    explicit SeparateByAttributeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Remove edges whose node's have different attribute values.");
    }

    QString category() const override { return QObject::tr("Structural"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Node Attribute", ElementType::Node, ValueType::String,
                QObject::tr("Each edge's source and target nodes have the selected attribute's value compared. "
                    "If they differ, the edge is removed. This has the effect of grouping similar nodes into components.")
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // SEPARATEBYATTRIBUTETRANSFORM_H
