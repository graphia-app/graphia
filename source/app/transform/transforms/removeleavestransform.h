#ifndef REMOVELEAVESTRANSFORM_H
#define REMOVELEAVESTRANSFORM_H

#include "transform/graphtransform.h"

class RemoveLeavesTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
};

class RemoveLeavesTransformFactory : public GraphTransformFactory
{
public:
    explicit RemoveLeavesTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Remove leaf nodes from the graph.");
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Limit",
                ValueType::Int,
                QObject::tr("The number of leaves to remove from a branch before stopping. "
                    "Setting this to 0 will remove the entire branch, leaving only cycles."),
                0, 0
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // REMOVELEAVESTRANSFORM_H
