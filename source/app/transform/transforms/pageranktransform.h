#ifndef PAGERANKTRANSFORM_H
#define PAGERANKTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class PageRankTransform : public GraphTransform
{
public:
    explicit PageRankTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

    void enableDebug() { _debug = true; }
    void disableDebug() { _debug = false; }

private:
    const float PAGERANK_DAMPING = 0.8f;
    const float PAGERANK_EPSILON = 1e-6f;
    const float PAGERANK_ACCELERATION_MINIMUM = 1e-10f;
    const int PAGERANK_ITERATION_LIMIT = 1000;
    const int AVG_COUNT = 10;

    bool _debug = false;

    void calculatePageRank(TransformedGraph& target) const;
    GraphModel* _graphModel = nullptr;
};

class PageRankTransformFactory : public GraphTransformFactory
{
public:
    explicit PageRankTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(R"(Calculates a <a href="https://kajeka.com/graphia/pagerank">PageRank</a> )"
            "centrality measurement for each node. This can be viewed as "
            "measure of a node's relative importance in the graph.");
    }
    ElementType elementType() const override { return ElementType::None; }
    GraphTransformParameters parameters() const override
    {
        return {};
    }
    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Node PageRank", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // PAGERANKTRANSFORM_H
