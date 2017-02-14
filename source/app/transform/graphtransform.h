#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "shared/graph/elementid.h"
#include "graphtransformconfig.h"

#include <memory>

class Graph;
class GraphComponent;
class TransformedGraph;
class DataField;

class GraphTransform
{
public:
    virtual ~GraphTransform() {}

    // In some circumstances it may be a performance win to reimplement this instead of going
    // for the inplace transform version
    virtual bool applyFromSource(const Graph& source, TransformedGraph& target) const;

    virtual bool apply(TransformedGraph&) const { return false; }
    bool applyAndUpdate(TransformedGraph& target) const;

    bool repeating() const { return _repeating; }
    void setRepeating(bool repeating) { _repeating = repeating; }

private:
    bool _repeating = false;
};

class GraphTransformFactory
{
public:
    virtual ~GraphTransformFactory() {}

    virtual ElementType elementType() const { return ElementType::None; }
    virtual std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig,
                                                   const std::map<QString, DataField>& dataFields) const = 0;
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
