#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "transform/graphtransform.h"
#include "graph/graph.h"
#include "attributes/attribute.h"

#include <vector>

class FilterTransform : public GraphTransform
{
public:
    explicit FilterTransform(ElementType elementType,
                             const GraphModel& graphModel,
                             bool invert) :
        _elementType(elementType),
        _graphModel(&graphModel),
        _invert(invert)
    {}

    bool apply(TransformedGraph &target) const override;

private:
    ElementType _elementType;
    const GraphModel* _graphModel;
    bool _invert = false;
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

    QString description() const override
    {
        return QObject::tr("%1 all the %2s which match the specified condition.")
                .arg(_invert ? QObject::tr("Keep") : QObject::tr("Remove"),
                     elementTypeAsString(_elementType).toLower());
    }
    ElementType elementType() const override { return _elementType; }
    bool requiresCondition() const override { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // FILTERTRANSFORM_H
