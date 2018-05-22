#ifndef SPANNINGTREETRANSFORM_H
#define SPANNINGTREETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class SpanningTreeTransform : public GraphTransform
{
public:
    explicit SpanningTreeTransform() {}

    bool apply(TransformedGraph& target) const override;
};

class SpanningTreeTransformFactory : public GraphTransformFactory
{
public:
    explicit SpanningTreeTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Find a "
            R"(<a href="https://en.wikipedia.org/wiki/Spanning_tree">)"
            R"(spanning tree</a> for each component.)");
    }
    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // SPANNINGTREETRANSFORM_H
