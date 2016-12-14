#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "graphtransform.h"
#include "graph/graph.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    void apply(TransformedGraph &target) const;

    void addEdgeContractionFilter(const EdgeConditionFn& f) { _edgeFilters.emplace_back(f); }
    bool hasEdgeContractionFilters() const { return !_edgeFilters.empty(); }

private:
    std::vector<EdgeConditionFn> _edgeFilters;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    ElementType elementType() const { return ElementType::Edge; }
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig,
                                           const std::map<QString, DataField>& dataFields) const;
};

#endif // EDGECONTRACTIONTRANSFORM_H
