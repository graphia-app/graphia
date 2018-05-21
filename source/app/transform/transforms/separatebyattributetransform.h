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

    bool apply(TransformedGraph& target) const override;

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
