#ifndef SPANNINGTREETRANSFORM_H
#define SPANNINGTREETRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class SpanningTreeTransform : public GraphTransform
{
public:
    void apply(TransformedGraph& target) const override;
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
            R"(<a href="https://kajeka.com/graphia/spanning_tree">)"
            R"(spanning tree</a> for each component.)");
    }

    QString category() const override { return QObject::tr("Structural") ; }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Traversal Order",
                ValueType::StringList,
                QObject::tr("Whether to visit nodes level by level, or by maximising depth."),
                QStringList{"Breadth First", "Depth First"}
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // SPANNINGTREETRANSFORM_H
