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

#ifndef TYPECASTTRANSFORM_H
#define TYPECASTTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

class TypeCastTransform : public GraphTransform
{
public:
    explicit TypeCastTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
};

class TypeCastTransformFactory : public GraphTransformFactory
{
public:
    explicit TypeCastTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Make a copy an existing attribute and convert its type. "
            "Where there is no sensible conversion, a default value will be applied.");
    }

    QString category() const override { return QObject::tr("Attributes"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Attribute",
                ElementType::NodeAndEdge, ValueType::All,
                QObject::tr("The attribute whose type is to be changed.")
            }
        };
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Type")
                .setType(ValueType::StringList)
                .setDescription(QObject::tr("The type conversion to apply."))
                .setInitialValue(QStringList{"Integer", "Float", "String"}),

            GraphTransformParameter::create("Name")
                .setType(ValueType::String)
                .setDescription(QObject::tr("The name of the new attribute."))
                .setInitialValue(QObject::tr("New Attribute"))
                .setValidatorRegex(IAttribute::ValidNameRegex)
        };
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // TYPECASTTRANSFORM_H
