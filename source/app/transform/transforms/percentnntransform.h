#ifndef PERCENTNNTRANSFORM_H
#define PERCENTNNTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class PercentNNTransform : public GraphTransform
{
public:
    explicit PercentNNTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class PercentNNTransformFactory : public GraphTransformFactory
{
public:
    explicit PercentNNTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the number of edges in the graph using a variation of the "
            R"(<a href="https://kajeka.com/graphia/knn">)"
            R"(k-nearest neighbours</a> algorithm, but instead of choosing the top k edges, )"
            "choose a percentage of the highest ranking edges.");
    }

    QString category() const override { return QObject::tr("Edge Reduction"); }

    GraphTransformAttributeParameters attributeParameters() const override
    {
        return
        {
            {
                "Ranked Attribute",
                ElementType::Edge, ValueType::Numerical,
                QObject::tr("The attribute whose value is used to rank edges.")
            }
        };
    }

    GraphTransformParameters parameters() const override
    {
        return
        {
            {
                "Percent",
                ValueType::Int,
                QObject::tr("The percentage of edges to rank and retain, per node."),
                10, 0, 100
            },
            {
                "Minimum",
                ValueType::Int,
                QObject::tr("The minimum number of edges to retain, per node."),
                5, 1
            },
            {
                "Rank Order",
                ValueType::StringList,
                QObject::tr("Whether or not larger or smaller values are ranked higher."),
                QStringList{"Descending", "Ascending"}
            }
        };
    }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // PERCENTNNTRANSFORM_H
