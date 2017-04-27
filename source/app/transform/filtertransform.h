#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "graphtransform.h"
#include "graph/graph.h"
#include "attributes/attribute.h"

#include <vector>

class FilterTransform : public GraphTransform
{
public:
    explicit FilterTransform(ElementType elementType,
                             const NameAttributeMap& attributes,
                             bool invert) :
        _elementType(elementType),
        _attributes(&attributes),
        _invert(invert)
    {}

    bool apply(TransformedGraph &target) const;

private:
    ElementType _elementType;
    const NameAttributeMap* _attributes;
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

    QString description() const
    {
        return QObject::tr("%1 all the %2s which match the specified condition.")
                .arg(_invert ? QObject::tr("Keep") : QObject::tr("Remove"))
                .arg(elementTypeAsString(_elementType).toLower());
    }
    ElementType elementType() const { return _elementType; }
    bool requiresCondition() const { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const;
};

#endif // FILTERTRANSFORM_H
