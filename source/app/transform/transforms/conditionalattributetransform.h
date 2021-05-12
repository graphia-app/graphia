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

#ifndef CONDITIONALATTRIBUTETRANSFORM_H
#define CONDITIONALATTRIBUTETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class ConditionalAttributeTransform : public GraphTransform
{
public:
    explicit ConditionalAttributeTransform(ElementType elementType,
        GraphModel& graphModel) :
        _elementType(elementType),
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    ElementType _elementType;
    GraphModel* _graphModel;
};

class ConditionalAttributeTransformFactory : public GraphTransformFactory
{
private:
    ElementType _elementType = ElementType::None;

public:
    ConditionalAttributeTransformFactory(GraphModel* graphModel, ElementType elementType) :
        GraphTransformFactory(graphModel), _elementType(elementType)
    {}

    QString description() const override
    {
        return QObject::tr("Create an attribute whose value is the "
            "result of the specified condition.");
    }
    QString category() const override { return QObject::tr("Attributes"); }
    ElementType elementType() const override { return _elementType; }
    bool requiresCondition() const override { return true; }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Name")
                .setType(ValueType::String)
                .setDescription(QObject::tr("The name of the new attribute."))
                .setInitialValue(QObject::tr("New Attribute"))
                .setValidatorRegex(Attribute::ValidNameRegex),
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Name", ValueType::String, {}, QObject::tr("Colour")}};
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // CONDITIONALATTRIBUTETRANSFORM_H
