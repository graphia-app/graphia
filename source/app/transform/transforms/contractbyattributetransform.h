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

#ifndef CONTRACTBYATTRIBUTETRANSFORM_H
#define CONTRACTBYATTRIBUTETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include "shared/utils/redirects.h"

#include <vector>

class ContractByAttributeTransform : public GraphTransform
{
public:
    explicit ContractByAttributeTransform(const GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    const GraphModel* _graphModel;
};

class ContractByAttributeTransformFactory : public GraphTransformFactory
{
public:
    explicit ContractByAttributeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("%1 whose node's share the same attribute value.")
            .arg(u::redirectLink("contraction", QObject::tr("Contract edges")));
    }

    QString category() const override { return QObject::tr("Structural"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Node Attribute", ElementType::Node, ValueType::String,
                QObject::tr("Each edge's source and target nodes have the selected attribute's value compared. "
                    "If they match, the edge is contracted, meaning it is hidden and its nodes merged.")
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return
        {
            {"Node Multiplicity", ValueType::Int, {}, QObject::tr("Size")},
            {"Edge Multiplicity", ValueType::Int, {}, QObject::tr("Size")}
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // CONTRACTBYATTRIBUTETRANSFORM_H
