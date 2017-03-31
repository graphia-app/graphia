#include "attribute.h"

void Attribute::clearFunctions()
{
    _intNodeIdFn = nullptr;
    _intEdgeIdFn = nullptr;
    _intComponentFn = nullptr;

    _floatNodeIdFn = nullptr;
    _floatEdgeIdFn = nullptr;
    _floatComponentFn = nullptr;

    _stringNodeIdFn = nullptr;
    _stringEdgeIdFn = nullptr;
    _stringComponentFn = nullptr;
}

int Attribute::valueOf(Helper<int>, NodeId nodeId) const { Q_ASSERT(_intNodeIdFn != nullptr); return _intNodeIdFn(nodeId); }
int Attribute::valueOf(Helper<int>, EdgeId edgeId) const { Q_ASSERT(_intEdgeIdFn != nullptr); return _intEdgeIdFn(edgeId); }
int Attribute::valueOf(Helper<int>, const IGraphComponent& component) const { Q_ASSERT(_intComponentFn != nullptr); return _intComponentFn(component); }

double Attribute::valueOf(Helper<double>, NodeId nodeId) const { Q_ASSERT(_floatNodeIdFn != nullptr); return _floatNodeIdFn(nodeId); }
double Attribute::valueOf(Helper<double>, EdgeId edgeId) const { Q_ASSERT(_floatEdgeIdFn != nullptr); return _floatEdgeIdFn(edgeId); }
double Attribute::valueOf(Helper<double>, const IGraphComponent& component) const { Q_ASSERT(_floatComponentFn != nullptr); return _floatComponentFn(component); }

QString Attribute::valueOf(Helper<QString>, NodeId nodeId) const { Q_ASSERT(_stringNodeIdFn != nullptr); return _stringNodeIdFn(nodeId); }
QString Attribute::valueOf(Helper<QString>, EdgeId edgeId) const { Q_ASSERT(_stringEdgeIdFn != nullptr); return _stringEdgeIdFn(edgeId); }
QString Attribute::valueOf(Helper<QString>, const IGraphComponent& component) const { Q_ASSERT(_stringComponentFn != nullptr); return _stringComponentFn(component); }

Attribute& Attribute::setIntValueFn(ValueFn<int, NodeId> valueFn) { clearFunctions(); _intNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, EdgeId> valueFn) { clearFunctions(); _intEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) { clearFunctions(); _intComponentFn = valueFn; return *this; }

Attribute& Attribute::setFloatValueFn(ValueFn<double, NodeId> valueFn) { clearFunctions(); _floatNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setFloatValueFn(ValueFn<double, EdgeId> valueFn) { clearFunctions(); _floatEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) { clearFunctions(); _floatComponentFn = valueFn; return *this; }

Attribute& Attribute::setStringValueFn(ValueFn<QString, NodeId> valueFn) { clearFunctions(); _stringNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setStringValueFn(ValueFn<QString, EdgeId> valueFn) { clearFunctions(); _stringEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) { clearFunctions(); _stringComponentFn = valueFn; return *this; }

Attribute::Type Attribute::type() const
{
    if(_intNodeIdFn != nullptr)         return Type::IntNode;
    if(_intEdgeIdFn != nullptr)         return Type::IntEdge;
    if(_intComponentFn != nullptr)      return Type::IntComponent;

    if(_floatNodeIdFn != nullptr)       return Type::FloatNode;
    if(_floatEdgeIdFn != nullptr)       return Type::FloatEdge;
    if(_floatComponentFn != nullptr)    return Type::FloatComponent;

    if(_stringNodeIdFn != nullptr)      return Type::StringNode;
    if(_stringEdgeIdFn != nullptr)      return Type::StringEdge;
    if(_stringComponentFn != nullptr)   return Type::StringComponent;

    return Type::Unknown;
}

void Attribute::disableAutoRange()
{
    _flags.reset(AttributeFlag::AutoRangeMutable, AttributeFlag::AutoRangeTransformed);
}

ValueType Attribute::valueType() const
{
    switch(type())
    {
    case Type::IntNode:             return ValueType::Int;
    case Type::IntEdge:             return ValueType::Int;
    case Type::IntComponent:        return ValueType::Int;

    case Type::FloatNode:           return ValueType::Float;
    case Type::FloatEdge:           return ValueType::Float;
    case Type::FloatComponent:      return ValueType::Float;

    case Type::StringNode:          return ValueType::String;
    case Type::StringEdge:          return ValueType::String;
    case Type::StringComponent:     return ValueType::String;

    default:                        return ValueType::Unknown;
    }
}

ElementType Attribute::elementType() const
{
    switch(type())
    {
    case Type::IntNode:             return ElementType::Node;
    case Type::FloatNode:           return ElementType::Node;
    case Type::StringNode:          return ElementType::Node;

    case Type::IntEdge:             return ElementType::Edge;
    case Type::FloatEdge:           return ElementType::Edge;
    case Type::StringEdge:          return ElementType::Edge;

    case Type::IntComponent:        return ElementType::Component;
    case Type::FloatComponent:      return ElementType::Component;
    case Type::StringComponent:     return ElementType::Component;

    default:                        return ElementType::None;
    }
}

bool AttributeRange<int>::hasMin() const { return _attribute._intMin != std::numeric_limits<int>::max(); }
bool AttributeRange<int>::hasMax() const { return _attribute._intMax != std::numeric_limits<int>::min(); }
bool AttributeRange<int>::hasRange() const { return hasMin() && hasMax(); }

int AttributeRange<int>::min() const { return hasMin() ? _attribute._intMin : std::numeric_limits<int>::min(); }
int AttributeRange<int>::max() const { return hasMax() ? _attribute._intMax : std::numeric_limits<int>::max(); }
IAttribute& AttributeRange<int>::setMin(int min) { _attribute._intMin = min; _attribute.disableAutoRange(); return _attribute; }
IAttribute& AttributeRange<int>::setMax(int max) { _attribute._intMax = max; _attribute.disableAutoRange(); return _attribute; }

bool AttributeRange<double>::hasMin() const { return _attribute._floatMin != std::numeric_limits<double>::max(); }
bool AttributeRange<double>::hasMax() const { return _attribute._floatMax != std::numeric_limits<double>::min(); }
bool AttributeRange<double>::hasRange() const { return hasMin() && hasMax(); }

double AttributeRange<double>::min() const { return hasMin() ? _attribute._floatMin : std::numeric_limits<double>::min(); }
double AttributeRange<double>::max() const { return hasMax() ? _attribute._floatMax : std::numeric_limits<double>::max(); }
IAttribute& AttributeRange<double>::setMin(double min) { _attribute._floatMin = min; _attribute.disableAutoRange(); return _attribute; }
IAttribute& AttributeRange<double>::setMax(double max) { _attribute._floatMax = max; _attribute.disableAutoRange(); return _attribute; }

bool AttributeNumericRange::hasMin() const
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: return _attribute._intRange.hasMin();
    case ValueType::Float: return _attribute._floatRange.hasMin();
    default: return false;
    }
}

bool AttributeNumericRange::hasMax() const
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: return _attribute._intRange.hasMax();
    case ValueType::Float: return _attribute._floatRange.hasMax();
    default: return false;
    }
}

bool AttributeNumericRange::hasRange() const
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: return _attribute._intRange.hasRange();
    case ValueType::Float: return _attribute._floatRange.hasRange();
    default: return false;
    }
}

double AttributeNumericRange::min() const
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: return static_cast<double>(_attribute._intRange.min());
    case ValueType::Float: return _attribute._floatRange.min();
    default: return std::numeric_limits<double>::max();
    }
}

double AttributeNumericRange::max() const
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: return static_cast<double>(_attribute._floatRange.max());
    case ValueType::Float: return _attribute._floatRange.max();
    default: return std::numeric_limits<double>::min();
    }
}

IAttribute& AttributeNumericRange::setMin(double min)
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: _attribute._intRange.setMin(static_cast<int>(min));
    case ValueType::Float: _attribute._intRange.setMin(min);
    default: break;
    }

    return _attribute;
}

IAttribute& AttributeNumericRange::setMax(double max)
{
    switch(_attribute.valueType())
    {
    case ValueType::Int: _attribute._intRange.setMax(static_cast<int>(max));
    case ValueType::Float: _attribute._intRange.setMax(max);
    default: break;
    }

    return _attribute;
}
