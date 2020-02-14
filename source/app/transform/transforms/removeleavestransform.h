#ifndef REMOVELEAVESTRANSFORM_H
#define REMOVELEAVESTRANSFORM_H

#include "transform/graphtransform.h"

class RemoveLeavesTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
};

class RemoveBranchesTransform : public GraphTransform
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

    QString category() const override { return QObject::tr("Structural"); }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Limit",
                ValueType::Int,
                QObject::tr("The number of leaves to remove from a branch before stopping."),
                1, 1
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

class RemoveBranchesTransformFactory : public GraphTransformFactory
{
public:
    explicit RemoveBranchesTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Remove branches from the graph, leaving only cycles.");
    }

    QString category() const override { return QObject::tr("Structural"); }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // REMOVELEAVESTRANSFORM_H
