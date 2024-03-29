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

#ifndef ATTRIBUTESYNTHESISTRANSFORM_H
#define ATTRIBUTESYNTHESISTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include "shared/utils/redirects.h"

class AttributeSynthesisTransform : public GraphTransform
{
public:
    explicit AttributeSynthesisTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) override;

private:
    GraphModel* _graphModel = nullptr;
};

class AttributeSynthesisTransformFactory : public GraphTransformFactory
{
public:
    explicit AttributeSynthesisTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Create a new attribute by permuting the values of "
            "an existing source attribute.");
    }

    QString category() const override { return QObject::tr("Attributes"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Source Attribute",
                ElementType::NodeAndEdge, ValueType::All,
                QObject::tr("The source attribute from which the new attribute is created.")
            }
        };
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            GraphTransformParameter::create("Name")
                .setType(ValueType::String)
                .setDescription(QObject::tr("The name of the new attribute."))
                .setInitialValue(QObject::tr("New Attribute"))
                .setValidatorRegex(IAttribute::ValidNameRegex),

            GraphTransformParameter::create("Regular Expression")
                .setType(ValueType::String)
                .setDescription(QObject::tr("A %1 that is matched against the source attribute values.")
                    .arg(u::redirectLink("regex", QObject::tr("regular expression"))))
                .setInitialValue("(^.*$)"),

            GraphTransformParameter::create("Attribute Value")
                .setType(ValueType::String)
                .setDescription(QObject::tr("The value to assign to the attribute. Capture groups are referenced using \\n "
                    "syntax, where n is the index of the regex capture group."))
                .setInitialValue(R"(\1)")
        };
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // ATTRIBUTESYNTHESISTRANSFORM_H
