#ifndef GRAPHTRANSFORMATTRIBUTEPARAMETER_H
#define GRAPHTRANSFORMATTRIBUTEPARAMETER_H

#include "shared/graph/elementtype.h"
#include "shared/attributes/valuetype.h"

#include <QString>

#include <map>

class GraphTransformAttributeParameter
{
public:
    GraphTransformAttributeParameter(ElementType elementType, ValueType valueType, QString description) :
        _elementType(elementType), _valueType(valueType), _description(std::move(description))
    {}

    ElementType elementType() const { return _elementType; }
    ValueType valueType() const { return _valueType; }
    QString description() const { return _description; }

private:
    ElementType _elementType;
    ValueType _valueType;
    QString _description;
};

using GraphTransformAttributeParameters = std::map<QString, GraphTransformAttributeParameter>;

#endif // GRAPHTRANSFORMATTRIBUTEPARAMETER_H
