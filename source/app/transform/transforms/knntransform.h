#ifndef KNNTRANSFORM_H
#define KNNTRANSFORM_H

#include "transform/graphtransform.h"
#include "attributes/attribute.h"

#include <vector>

class KNNTransform : public GraphTransform
{
public:
    explicit KNNTransform(GraphModel& graphModel) :
        _graphModel(&graphModel)
    {}

    void apply(TransformedGraph& target) const override;

private:
    GraphModel* _graphModel = nullptr;
};

class KNNTransformFactory : public GraphTransformFactory
{
public:
    explicit KNNTransformFactory(GraphModel* graphModel) :
        GraphTransformFactory(graphModel)
    {}

    QString description() const override
    {
        return QObject::tr("Reduce the number of edges in the graph using the "
            R"(<a href="https://en.wikipedia.org/wiki/K-nearest_neighbors_algorithm">)"
            R"(k-nearest neighbours</a> algorithm.)");
    }

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
                "k",
                ValueType::Int,
                QObject::tr("The number of edges to rank and retain, per node."),
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

#endif // KNNTRANSFORM_H
