#ifndef EDGEREDUCTIONTRANSFORM_H
#define EDGEREDUCTIONTRANSFORM_H

#include "transform/graphtransform.h"

class EdgeReductionTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
};

class EdgeReductionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeReductionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the complexity of the graph by removing a pseudo-random "
            "percentage of edges, down to a specified minimum, per node.");
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Percentage",
                ValueType::Int,
                QObject::tr("The percentage of edges to retain, per node."),
                10, 0, 100
            },
            {
                "Minimum",
                ValueType::Int,
                QObject::tr("The minimum number of edges to retain, per node."),
                5, 1
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // EDGEREDUCTIONTRANSFORM_H
