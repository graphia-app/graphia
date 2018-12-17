#ifndef ATTRIBUTESYNTHESISTRANSFORM_H
#define ATTRIBUTESYNTHESISTRANSFORM_H

#include "transform/graphtransform.h"

class AttributeSynthesisTransform : public GraphTransform
{
public:
    explicit AttributeSynthesisTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

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

    GraphTransformCategory category() const override { return GraphTransformCategory::Attribute; }

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
            {
                "Name",
                ValueType::String,
                QObject::tr("The name of the new attribute."),
                QObject::tr("New Attribute")
            },
            {
                "Regular Expression",
                ValueType::String,
                QObject::tr(R"(A <a href="https://perldoc.perl.org/perlre.html">regular expression</a> that )"
                    "is matched against the source attribute values."),
                "(^.*$)"
            },
            {
                "Attribute Value",
                ValueType::String,
                QObject::tr("The value to assign to the attribute. Capture groups are referenced using \\n "
                    "syntax, where n is the index of the regex capture group."),
                "\\1"
            }
        };
    }

    bool configIsValid(const GraphTransformConfig& graphTransformConfig) const override;
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // ATTRIBUTESYNTHESISTRANSFORM_H
