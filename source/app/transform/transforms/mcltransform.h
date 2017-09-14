#ifndef MCLTRANSFORM_H
#define MCLTRANSFORM_H

#include "transform/graphtransform.h"
#include "graph/graph.h"
#include "shared/utils/flags.h"

class MCLTransform : public GraphTransform
{
public:
    explicit MCLTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    bool apply(TransformedGraph &target) const;

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

    QString description() const
    {
        return R"(<a href="https://micans.org/mcl/">MCL - Markov Clustering</a> )"
                "finds discrete groups (clusters) of nodes based on a flow simulation model.";
    }
    ElementType elementType() const { return ElementType::None; }
    GraphTransformParameters parameters() const
    {
        return {{"Granularity", {ValueType::Float, "Controls the size of the resultant clusters. "
            "A larger granularity value results in smaller clusters.", 2.2, 1.1, 3.5}}};
    }
    DeclaredAttributes declaredAttributes() const
    {
        return {{"MCL Cluster", {ValueType::String, QObject::tr("Colour")}}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // MCLTRANSFORM_H
