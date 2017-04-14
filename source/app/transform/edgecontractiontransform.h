#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "graphtransform.h"
#include "graph/graph.h"
#include "attributes/attribute.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    EdgeContractionTransform(const NameAttributeMap& attributes,
                             const GraphTransformConfig& graphTransformConfig) :
        _attributes(&attributes), _graphTransformConfig(graphTransformConfig)
    {}

    bool apply(TransformedGraph &target) const;

private:
    const NameAttributeMap* _attributes;
    GraphTransformConfig _graphTransformConfig;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    explicit EdgeContractionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const
    {
        return QObject::tr(R"(<a href="https://en.wikipedia.org/wiki/Edge_contraction">Remove edges</a> )" //
                           "which match the specified condition while simultaneously "
                           "merging the pairs of nodes that they previously joined.");
    }
    ElementType elementType() const { return ElementType::Edge; }
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // EDGECONTRACTIONTRANSFORM_H
