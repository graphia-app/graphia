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
    GraphTransformCategory category() const override { return GraphTransformCategory::Attribute; }
    ElementType elementType() const override { return _elementType; }
    bool requiresCondition() const override { return true; }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Name",
                ValueType::String,
                QObject::tr("The name of the new attribute."),
                QObject::tr("New Attribute")
            }
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
