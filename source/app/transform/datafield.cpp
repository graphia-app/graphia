#include "datafield.h"

void DataField::clearFunctions()
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

int DataField::valueOf(Helper<int>, NodeId nodeId) const { Q_ASSERT(_intNodeIdFn != nullptr); return _intNodeIdFn(nodeId); }
int DataField::valueOf(Helper<int>, EdgeId edgeId) const { Q_ASSERT(_intEdgeIdFn != nullptr); return _intEdgeIdFn(edgeId); }
int DataField::valueOf(Helper<int>, const IGraphComponent& component) const { Q_ASSERT(_intComponentFn != nullptr); return _intComponentFn(component); }

double DataField::valueOf(Helper<double>, NodeId nodeId) const { Q_ASSERT(_floatNodeIdFn != nullptr); return _floatNodeIdFn(nodeId); }
double DataField::valueOf(Helper<double>, EdgeId edgeId) const { Q_ASSERT(_floatEdgeIdFn != nullptr); return _floatEdgeIdFn(edgeId); }
double DataField::valueOf(Helper<double>, const IGraphComponent& component) const { Q_ASSERT(_floatComponentFn != nullptr); return _floatComponentFn(component); }

QString DataField::valueOf(Helper<QString>, NodeId nodeId) const { Q_ASSERT(_stringNodeIdFn != nullptr); return _stringNodeIdFn(nodeId); }
QString DataField::valueOf(Helper<QString>, EdgeId edgeId) const { Q_ASSERT(_stringEdgeIdFn != nullptr); return _stringEdgeIdFn(edgeId); }
QString DataField::valueOf(Helper<QString>, const IGraphComponent& component) const { Q_ASSERT(_stringComponentFn != nullptr); return _stringComponentFn(component); }

DataField& DataField::setIntValueFn(ValueFn<int, NodeId> valueFn) { clearFunctions(); _intNodeIdFn = valueFn; return *this; }
DataField& DataField::setIntValueFn(ValueFn<int, EdgeId> valueFn) { clearFunctions(); _intEdgeIdFn = valueFn; return *this; }
DataField& DataField::setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) { clearFunctions(); _intComponentFn = valueFn; return *this; }

DataField& DataField::setFloatValueFn(ValueFn<double, NodeId> valueFn) { clearFunctions(); _floatNodeIdFn = valueFn; return *this; }
DataField& DataField::setFloatValueFn(ValueFn<double, EdgeId> valueFn) { clearFunctions(); _floatEdgeIdFn = valueFn; return *this; }
DataField& DataField::setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) { clearFunctions(); _floatComponentFn = valueFn; return *this; }

DataField& DataField::setStringValueFn(ValueFn<QString, NodeId> valueFn) { clearFunctions(); _stringNodeIdFn = valueFn; return *this; }
DataField& DataField::setStringValueFn(ValueFn<QString, EdgeId> valueFn) { clearFunctions(); _stringEdgeIdFn = valueFn; return *this; }
DataField& DataField::setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) { clearFunctions(); _stringComponentFn = valueFn; return *this; }

DataField::Type DataField::type() const
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

FieldType DataField::valueType() const
{
    switch(type())
    {
    case Type::IntNode:             return FieldType::Int;
    case Type::IntEdge:             return FieldType::Int;
    case Type::IntComponent:        return FieldType::Int;

    case Type::FloatNode:           return FieldType::Float;
    case Type::FloatEdge:           return FieldType::Float;
    case Type::FloatComponent:      return FieldType::Float;

    case Type::StringNode:          return FieldType::String;
    case Type::StringEdge:          return FieldType::String;
    case Type::StringComponent:     return FieldType::String;

    default:                        return FieldType::Unknown;
    }
}

ElementType DataField::elementType() const
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

bool DataField::hasIntMin() const { return _intMin != std::numeric_limits<int>::max(); }
bool DataField::hasIntMax() const { return _intMax != std::numeric_limits<int>::min(); }
bool DataField::hasIntRange() const { return hasIntMin() && hasIntMax(); }

int DataField::intMin() const { return hasIntMin() ? _intMin : std::numeric_limits<int>::min(); }
int DataField::intMax() const { return hasIntMax() ? _intMax : std::numeric_limits<int>::max(); }
DataField& DataField::setIntMin(int intMin) { _intMin = intMin; return *this; }
DataField& DataField::setIntMax(int intMax) { _intMax = intMax; return *this; }

bool DataField::intValueInRange(int value) const
{
    if(hasIntMin() && value < intMin())
        return false;

    if(hasIntMax() && value > intMax())
        return false;

    return true;
}

bool DataField::hasFloatMin() const { return _floatMin != std::numeric_limits<double>::max(); }
bool DataField::hasFloatMax() const { return _floatMax != std::numeric_limits<double>::min(); }
bool DataField::hasFloatRange() const { return hasFloatMin() && hasFloatMax(); }

double DataField::floatMin() const { return hasFloatMin() ? _floatMin : std::numeric_limits<double>::min(); }
double DataField::floatMax() const { return hasFloatMax() ? _floatMax : std::numeric_limits<double>::max(); }
DataField& DataField::setFloatMin(double floatMin) { _floatMin = floatMin; return *this; }
DataField& DataField::setFloatMax(double floatMax) { _floatMax = floatMax; return *this; }

bool DataField::floatValueInRange(double value) const
{
    if(hasFloatMin() && value < floatMin())
        return false;

    if(hasFloatMax() && value > floatMax())
        return false;

    return true;
}

bool DataField::hasNumericRange() const
{
    switch(valueType())
    {
    case FieldType::Int: return hasIntRange();
    case FieldType::Float: return hasFloatRange();
    default: return false;
    }
}

double DataField::numericMin() const
{
    switch(valueType())
    {
    case FieldType::Int: return static_cast<double>(intMin());
    case FieldType::Float: return floatMin();
    default: return false;
    }
}

double DataField::numericMax() const
{
    switch(valueType())
    {
    case FieldType::Int: return static_cast<double>(intMax());
    case FieldType::Float: return floatMax();
    default: return false;
    }
}
