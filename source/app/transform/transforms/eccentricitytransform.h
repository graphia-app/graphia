#ifndef ECCENTRICITYTRANSFORM_H
#define ECCENTRICITYTRANSFORM_H

#include "transform/graphtransform.h"
#include "shared/utils/flags.h"

class EccentricityTransform : public GraphTransform
{
public:
    explicit EccentricityTransform(GraphModel* graphModel) : _graphModel(graphModel) {}
    bool apply(TransformedGraph &target) const override;

private:
    GraphModel* _graphModel = nullptr;
    void calculateDistances(TransformedGraph& target) const;
};

class EccentricityTransformFactory : public GraphTransformFactory
{
public:
    explicit EccentricityTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(
            R"-(<a href="https://en.wikipedia.org/wiki/Eccentricity_(graph_theory)">Eccentricity</a> )-"
            "calculates the shortest path between every node and assigns the longest path length found for that node. "
            "This is a measure of a node's position within the overall graph structure.");
    }
    ElementType elementType() const override { return ElementType::None; }
    GraphTransformParameters parameters() const override
    {
        return {};
    }
    DeclaredAttributes declaredAttributes() const override
    {
        return {{"Node Eccentricity", {ValueType::Float, QObject::tr("Colour")}}};
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // ECCENTRICITYTRANSFORM_H
