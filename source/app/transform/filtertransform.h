#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "graphtransform.h"
#include "graph/graph.h"
#include "graph/filter.h"

#include <vector>

class FilterTransform : public GraphTransform, public Filter
{
public:
    explicit FilterTransform(bool invert) : _invert(invert) {}

    bool apply(TransformedGraph &target) const;

    void addComponentFilter(const ComponentConditionFn& f) { if(f != nullptr) _componentFilters.emplace_back(f); }
    bool hasComponentFilters() const { return !_componentFilters.empty(); }

    void setIgnoreTails(bool ignoreTails) { _ignoreTails = ignoreTails; }

private:
    bool _invert = false;
    bool _ignoreTails = false;
    std::vector<ComponentConditionFn> _componentFilters;
};

class FilterTransformFactory : public GraphTransformFactory
{
private:
    ElementType _elementType = ElementType::None;
    bool _invert = false;

public:
    FilterTransformFactory(GraphModel* graphModel, ElementType elementType, bool invert) :
        GraphTransformFactory(graphModel),
        _elementType(elementType), _invert(invert)
    {}

    ElementType elementType() const { return _elementType; }
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // FILTERTRANSFORM_H
