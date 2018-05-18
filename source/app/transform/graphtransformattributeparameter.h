#ifndef GRAPHTRANSFORMATTRIBUTEPARAMETER_H
#define GRAPHTRANSFORMATTRIBUTEPARAMETER_H

#include "shared/graph/elementtype.h"
#include "shared/attributes/valuetype.h"

#include <QString>

#include <map>

class GraphTransformAttributeParameter
{
public:
    GraphTransformAttributeParameter() = default;

    GraphTransformAttributeParameter(QString name, ElementType elementType,
        ValueType valueType, QString description) :
        _name(std::move(name)), _elementType(elementType),
        _valueType(valueType), _description(std::move(description))
    {}

    QString name() const { return _name; }
    ElementType elementType() const { return _elementType; }
    ValueType valueType() const { return _valueType; }
    QString description() const { return _description; }

private:
    QString _name;
    ElementType _elementType;
    ValueType _valueType;
    QString _description;
};

using GraphTransformAttributeParameters = std::vector<GraphTransformAttributeParameter>;

#endif // GRAPHTRANSFORMATTRIBUTEPARAMETER_H
