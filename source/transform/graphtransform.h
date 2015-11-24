#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "datafield.h"

#include "../utils/cpp1x_hacks.h"
#include <memory>

class Graph;
class GraphComponent;
class TransformedGraph;

class GraphTransform
{
public:
    // In some circumstances it may be a performance win to reimplement this instead of going
    // for the inplace transform version
    virtual void apply(const Graph& source, TransformedGraph& target) const;

    virtual void apply(TransformedGraph&) const {}
};

class GraphTransformFactory
{
public:
    virtual std::unique_ptr<GraphTransform> create(const NodeConditionFn& conditionFn) const = 0;
    virtual std::unique_ptr<GraphTransform> create(const EdgeConditionFn& conditionFn) const = 0;
    virtual std::unique_ptr<GraphTransform> create(const ComponentConditionFn& conditionFn) const = 0;
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
