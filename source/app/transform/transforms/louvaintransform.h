#ifndef LOUVAINTRANSFORM_H
#define LOUVAINTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class LouvainTransform : public GraphTransform
{
public:
    explicit LouvainTransform(GraphModel* graphModel, bool weighted) :
        _graphModel(graphModel), _weighted(weighted) {}
    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
    bool _weighted = false;
};

class LouvainTransformFactory : public GraphTransformFactory
{
public:
    explicit LouvainTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(R"(<a href="https://kajeka.com/graphia/louvain">)"
            "Louvain modularity</a> is a method for finding clusters by measuring edge "
            "density from within communities to neighbouring communities.");
    }

    QString category() const override { return QObject::tr("Clustering"); }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Granularity", ValueType::Float,
                QObject::tr("The size of the resultant clusters. "
                    "A larger granularity value results in smaller clusters."),
                0.5, 0.0, 1.0
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Louvain Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig&) const override
    {
        return std::make_unique<LouvainTransform>(graphModel(), false);
    }
};

class WeightedLouvainTransformFactory : public LouvainTransformFactory
{
public:
    using LouvainTransformFactory::LouvainTransformFactory;

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Weighting Attribute",
                ElementType::Edge, ValueType::Numerical,
                QObject::tr("The attribute whose value is used to weight edges.")
            }
        };
    }

    DefaultVisualisations defaultVisualisations() const override
    {
        return {{"Weighted Louvain Cluster", ValueType::String, {}, QObject::tr("Colour")}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig&) const override
    {
        return std::make_unique<LouvainTransform>(graphModel(), true);
    }
};

#endif // LOUVAINTRANSFORM_H
