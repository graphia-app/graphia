#ifndef BETWEENNESSTRANSFORM_H
#define BETWEENNESSTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class BetweennessTransform : public GraphTransform
{
public:
    explicit BetweennessTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class BetweennessTransformFactory : public GraphTransformFactory
{
public:
    explicit BetweennessTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(
            R"-(<a href="https://kajeka.com/graphia/betweenness">Betweenness Centrality</a> )-"
            " is a measure of centrality in a graph based on shortest paths between nodes. "
            "The betweenness centrality for each node is the number of these shortest paths "
            "that pass through the node.");
    }
    QString category() const override { return QObject::tr("Metrics"); }
    ElementType elementType() const override { return ElementType::None; }
    DefaultVisualisations defaultVisualisations() const override
    {
        return
        {
            {"Node Betweenness", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")},
            {"Edge Betweenness", ValueType::Float, {AttributeFlag::VisualiseByComponent}, QObject::tr("Colour")}
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // BETWEENNESSTRANSFORM_H
