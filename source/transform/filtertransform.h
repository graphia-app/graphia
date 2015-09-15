#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "graphtransform.h"
#include "../graph/graph.h"
#include "../graph/filter.h"

#include <vector>
#include <functional>

using ComponentFilterFn = std::function<bool(const Graph&)>;

class FilterTransform : public GraphTransform, public Filter
{
private:
    void filterComponents(TransformedGraph& target) const;

public:
    void apply(const Graph &source, TransformedGraph &target) const;
    void apply(TransformedGraph &target) const;

    void addComponentFilter(const ComponentFilterFn& f) { _componentFilters.emplace_back(f); }

private:
    std::vector<ComponentFilterFn> _componentFilters;
};

#endif // FILTERTRANSFORM_H
