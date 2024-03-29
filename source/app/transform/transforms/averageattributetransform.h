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

#ifndef AVERAGEATTRIBUTETRANSFORM_H
#define AVERAGEATTRIBUTETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

class AverageAttributeTransform : public GraphTransform
{
public:
    explicit AverageAttributeTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
};

class AverageAttributeTransformFactory : public GraphTransformFactory
{
public:
    explicit AverageAttributeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Create a new attribute whose values are determined by averaging "
            "the values of a numerical attribute, using a categorical attribute.");
    }

    QString category() const override { return QObject::tr("Attributes"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Categorical Attribute",
                ElementType::NodeAndEdge, ValueType::String,
                QObject::tr("The attribute whose shared values are used to guide the averaging.")
            },
            {
                "Numerical Attribute",
                ElementType::NodeAndEdge, ValueType::Numerical,
                QObject::tr("The attribute whose values are averaged.")
            }
        };
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // AVERAGEATTRIBUTETRANSFORM_H
