#include "attribute.h"

void Attribute::clearFunctions(bool setAutoRange)
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

    if(setAutoRange && _autoRange == AutoRange::Unknown)
        _autoRange = AutoRange::Yes;
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

Attribute& Attribute::setIntValueFn(ValueFn<int, NodeId> valueFn) { clearFunctions(true); _intNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, EdgeId> valueFn) { clearFunctions(true); _intEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) { clearFunctions(); _intComponentFn = valueFn; return *this; }

Attribute& Attribute::setFloatValueFn(ValueFn<double, NodeId> valueFn) { clearFunctions(true); _floatNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setFloatValueFn(ValueFn<double, EdgeId> valueFn) { clearFunctions(true); _floatEdgeIdFn = valueFn; return *this; }
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

bool Attribute::hasIntMin() const { return _intMin != std::numeric_limits<int>::max(); }
bool Attribute::hasIntMax() const { return _intMax != std::numeric_limits<int>::min(); }
bool Attribute::hasIntRange() const { return hasIntMin() && hasIntMax(); }

int Attribute::intMin() const { return hasIntMin() ? _intMin : std::numeric_limits<int>::min(); }
int Attribute::intMax() const { return hasIntMax() ? _intMax : std::numeric_limits<int>::max(); }
Attribute& Attribute::setIntMin(int intMin) { _intMin = intMin; _autoRange = AutoRange::No; return *this; }
Attribute& Attribute::setIntMax(int intMax) { _intMax = intMax; _autoRange = AutoRange::No; return *this; }

bool Attribute::intValueInRange(int value) const
{
    if(hasIntMin() && value < intMin())
        return false;

    if(hasIntMax() && value > intMax())
        return false;

    return true;
}

bool Attribute::hasFloatMin() const { return _floatMin != std::numeric_limits<double>::max(); }
bool Attribute::hasFloatMax() const { return _floatMax != std::numeric_limits<double>::min(); }
bool Attribute::hasFloatRange() const { return hasFloatMin() && hasFloatMax(); }

double Attribute::floatMin() const { return hasFloatMin() ? _floatMin : std::numeric_limits<double>::min(); }
double Attribute::floatMax() const { return hasFloatMax() ? _floatMax : std::numeric_limits<double>::max(); }
Attribute& Attribute::setFloatMin(double floatMin) { _floatMin = floatMin; _autoRange = AutoRange::No; return *this; }
Attribute& Attribute::setFloatMax(double floatMax) { _floatMax = floatMax; _autoRange = AutoRange::No; return *this; }

bool Attribute::floatValueInRange(double value) const
{
    if(hasFloatMin() && value < floatMin())
        return false;

    if(hasFloatMax() && value > floatMax())
        return false;

    return true;
}

bool Attribute::hasNumericRange() const
{
    switch(valueType())
    {
    case ValueType::Int: return hasIntRange();
    case ValueType::Float: return hasFloatRange();
    default: return false;
    }
}

double Attribute::numericMin() const
{
    switch(valueType())
    {
    case ValueType::Int: return static_cast<double>(intMin());
    case ValueType::Float: return floatMin();
    default: return false;
    }
}

double Attribute::numericMax() const
{
    switch(valueType())
    {
    case ValueType::Int: return static_cast<double>(intMax());
    case ValueType::Float: return floatMax();
    default: return false;
    }
}
