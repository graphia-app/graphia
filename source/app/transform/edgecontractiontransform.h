#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "graphtransform.h"
#include "../graph/graph.h"

#include <vector>

class EdgeContractionTransform : public GraphTransform
{
public:
    void apply(TransformedGraph &target) const;

    void addEdgeContractionFilter(const EdgeConditionFn& f) { _edgeFilters.emplace_back(f); }

private:
    std::vector<EdgeConditionFn> _edgeFilters;
};

class EdgeContractionTransformFactory : public GraphTransformFactory
{
public:
    std::unique_ptr<GraphTransform> create(const NodeConditionFn&) const { return nullptr; }
    std::unique_ptr<GraphTransform> create(const EdgeConditionFn& conditionFn) const;
    std::unique_ptr<GraphTransform> create(const ComponentConditionFn&) const { return nullptr; }
};

#endif // FILTERTRANSFORM_H
