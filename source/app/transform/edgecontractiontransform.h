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
    EdgeContractionTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    ElementType elementType() const { return ElementType::Edge; }
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // EDGECONTRACTIONTRANSFORM_H
