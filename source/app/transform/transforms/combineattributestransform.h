#ifndef COMBINEATTRIBUTESTRANSFORM_H
#define COMBINEATTRIBUTESTRANSFORM_H

#include "transform/graphtransform.h"

class CombineAttributesTransform : public GraphTransform
{
public:
    explicit CombineAttributesTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class CombineAttributesTransformFactory : public GraphTransformFactory
{
public:
    explicit CombineAttributesTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Create a new attribute by combining "
            "two other attributes.");
    }

    GraphTransformCategory category() const override { return GraphTransformCategory::Attribute; }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "First Attribute",
                ElementType::NodeAndEdge, ValueType::All,
                QObject::tr("The first attribute from which the new attribute is created.")
            },
            {
                "Second Attribute",
                ElementType::NodeAndEdge, ValueType::All,
                QObject::tr("The second attribute from which the new attribute is created.")
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
                "Attribute Value",
                ValueType::String,
                QObject::tr("The value to assign to the attribute. \\1 and \\2 will be substituted "
                    "by the first and second attributes, respectively."),
                "\\1 \\2"
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // COMBINEATTRIBUTESTRANSFORM_H
