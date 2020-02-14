#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    explicit EdgeContractionTransform(const GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    const GraphModel* _graphModel;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeContractionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr(R"(<a href="https://kajeka.com/graphia/contraction">Remove edges</a> )"
                           "which match the specified condition while simultaneously "
                           "merging the pairs of nodes that they previously joined.");
    }
    QString category() const override { return QObject::tr("Structural"); }
    ElementType elementType() const override { return ElementType::Edge; }
    bool requiresCondition() const override { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // EDGECONTRACTIONTRANSFORM_H
