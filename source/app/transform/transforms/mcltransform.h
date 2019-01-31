#ifndef MCLTRANSFORM_H
#define MCLTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class MCLTransform : public GraphTransform
{
public:
    explicit MCLTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

private:
    void enableDebugIteration(){ _debugIteration = true; }
    void enableDebugMatrices(){ _debugMatrices = true; }
    void disableDebugIteration(){ _debugIteration = false; }
    void disableDebugMatrices(){ _debugMatrices = false; }

private:
    const float MCL_PRUNE_LIMIT = 1e-4f;
    const float MCL_CONVERGENCE_LIMIT = 1e-3f;

    bool _debugIteration = false;
    bool _debugMatrices = false;

    void calculateMCL(float inflation, TransformedGraph& target) const;

private:
    GraphModel* _graphModel = nullptr;
};

class MCLTransformFactory : public GraphTransformFactory
{
public:
    explicit MCLTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(R"(<a href="https://kajeka.com/graphia/mcl">MCL - Markov Clustering</a> )"
                "finds discrete groups (clusters) of nodes based on a flow simulation model.");
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Granularity", ValueType::Float,
                QObject::tr("The size of the resultant clusters. "
                    "A larger granularity value results in smaller clusters."),
                2.0, 1.1, 3.5
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"MCL Cluster", ValueType::String, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // MCLTRANSFORM_H
