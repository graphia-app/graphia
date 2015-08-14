#ifndef EDGECONTRACTIONTRANSFORM_H
#define EDGECONTRACTIONTRANSFORM_H

#include "graphtransform.h"
#include "../graph/graph.h"

#include <vector>
#include <functional>

using EdgeContractedFn = std::function<bool(const EdgeId)>;

class EdgeContractionTransform : public GraphTransform
{
public:
    void apply(TransformedGraph &target) const;

    void addEdgeContractionFilter(const EdgeContractedFn& f) { _edgeFilters.emplace_back(f); }

private:
    std::vector<EdgeContractedFn> _edgeFilters;
};

#endif // FILTERTRANSFORM_H
