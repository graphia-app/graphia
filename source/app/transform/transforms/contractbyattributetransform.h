#ifndef CONTRACTBYATTRIBUTETRANSFORM_H
#define CONTRACTBYATTRIBUTETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

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
        return QObject::tr(R"(<a href="https://en.wikipedia.org/wiki/Edge_contraction">Contract edges</a> )"
            "whose node's share the same attribute value.");
    }

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

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // CONTRACTBYATTRIBUTETRANSFORM_H
