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

#ifndef FORWARDMULTIELEMENTATTRIBUTETRANSFORM_H
#define FORWARDMULTIELEMENTATTRIBUTETRANSFORM_H

#include "app/transform/graphtransform.h"
#include "app/attributes/attribute.h"

class ForwardMultiElementAttributeTransform : public GraphTransform
{
public:
    explicit ForwardMultiElementAttributeTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
};

class ForwardMultiElementAttributeTransformFactory : public GraphTransformFactory
{
public:
    explicit ForwardMultiElementAttributeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("For graphs with contracted edges, forward a selected "
            "attribute's values to all <i>child</i> multi-elements. Note that if "
            "there are no multi-elements, this transform will have no effect.");
    }

    QString category() const override { return QObject::tr("Attributes"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Attribute",
                ElementType::NodeAndEdge, ValueType::All,
                QObject::tr("The attribute whose values are forwarded.")
            }
        };
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // FORWARDMULTIELEMENTATTRIBUTETRANSFORM_H
