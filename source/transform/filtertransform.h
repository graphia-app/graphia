#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "graphtransform.h"
#include "../graph/graph.h"
#include "../graph/filter.h"

#include <vector>

class FilterTransform : public GraphTransform, public Filter
{
public:
    void apply(TransformedGraph &target) const;

    void addComponentFilter(const ComponentConditionFn& f) { _componentFilters.emplace_back(f); }

private:
    std::vector<ComponentConditionFn> _componentFilters;
};

class FilterTransformFactory : public GraphTransformFactory
{
public:
    std::unique_ptr<GraphTransform> create(const NodeConditionFn& conditionFn) const;
    std::unique_ptr<GraphTransform> create(const EdgeConditionFn& conditionFn) const;
    std::unique_ptr<GraphTransform> create(const ComponentConditionFn& conditionFn) const;
};

#endif // FILTERTRANSFORM_H
