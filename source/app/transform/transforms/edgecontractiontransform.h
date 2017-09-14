#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "transform/graphtransform.h"
#include "graph/graph.h"
#include "attributes/attribute.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    EdgeContractionTransform(const GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    bool apply(TransformedGraph &target) const;

private:
    const GraphModel* _graphModel;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeContractionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const
    {
        return QObject::tr(R"(<a href="https://en.wikipedia.org/wiki/Edge_contraction">Remove edges</a> )"
                           "which match the specified condition while simultaneously "
                           "merging the pairs of nodes that they previously joined.");
    }
    ElementType elementType() const { return ElementType::Edge; }
    bool requiresCondition() const { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // EDGECONTRACTIONTRANSFORM_H
