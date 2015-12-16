#include "datafield.h"

DEFINE_REFLECTED_ENUM(ConditionFnOp,
    "",
    "=",
    "≠",
    "<",
    ">",
    "≤",
    "≥",
    "Contains",
    "Starts With",
    "Ends With",
    "Matches Regex"
)

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

int DataField::valueOf(Helper<int>, NodeId nodeId) const { return _intNodeIdFn(nodeId); }
int DataField::valueOf(Helper<int>, EdgeId edgeId) const { return _intEdgeIdFn(edgeId); }
int DataField::valueOf(Helper<int>, const GraphComponent& component) const { return _intComponentFn(component); }

float DataField::valueOf(Helper<float>, NodeId nodeId) const { return _floatNodeIdFn(nodeId); }
float DataField::valueOf(Helper<float>, EdgeId edgeId) const { return _floatEdgeIdFn(edgeId); }
float DataField::valueOf(Helper<float>, const GraphComponent& component) const { return _floatComponentFn(component); }

QString DataField::valueOf(Helper<QString>, NodeId nodeId) const { return _stringNodeIdFn(nodeId); }
QString DataField::valueOf(Helper<QString>, EdgeId edgeId) const { return _stringEdgeIdFn(edgeId); }
QString DataField::valueOf(Helper<QString>, const GraphComponent& component) const { return _stringComponentFn(component); }

DataField& DataField::setIntValueFn(ValueFn<int, NodeId> valueFn) { clearFunctions(); _intNodeIdFn = valueFn; return *this; }
DataField& DataField::setIntValueFn(ValueFn<int, EdgeId> valueFn) { clearFunctions(); _intEdgeIdFn = valueFn; return *this; }
DataField& DataField::setIntValueFn(ValueFn<int, const GraphComponent&> valueFn) { clearFunctions(); _intComponentFn = valueFn; return *this; }

DataField& DataField::setFloatValueFn(ValueFn<float, NodeId> valueFn) { clearFunctions(); _floatNodeIdFn = valueFn; return *this; }
DataField& DataField::setFloatValueFn(ValueFn<float, EdgeId> valueFn) { clearFunctions(); _floatEdgeIdFn = valueFn; return *this; }
DataField& DataField::setFloatValueFn(ValueFn<float, const GraphComponent&> valueFn) { clearFunctions(); _floatComponentFn = valueFn; return *this; }

DataField& DataField::setStringValueFn(ValueFn<QString, NodeId> valueFn) { clearFunctions(); _stringNodeIdFn = valueFn; return *this; }
DataField& DataField::setStringValueFn(ValueFn<QString, EdgeId> valueFn) { clearFunctions(); _stringEdgeIdFn = valueFn; return *this; }
DataField& DataField::setStringValueFn(ValueFn<QString, const GraphComponent&> valueFn) { clearFunctions(); _stringComponentFn = valueFn; return *this; }

DataFieldType DataField::type() const
{
    if(_intNodeIdFn != nullptr) return DataFieldType::IntNode;
    if(_intEdgeIdFn != nullptr) return DataFieldType::IntEdge;
    if(_intComponentFn != nullptr) return DataFieldType::IntComponent;

    if(_floatNodeIdFn != nullptr) return DataFieldType::FloatNode;
    if(_floatEdgeIdFn != nullptr) return DataFieldType::FloatEdge;
    if(_floatComponentFn != nullptr) return DataFieldType::FloatComponent;

    if(_stringNodeIdFn != nullptr) return DataFieldType::StringNode;
    if(_stringEdgeIdFn != nullptr) return DataFieldType::StringEdge;
    if(_stringComponentFn != nullptr) return DataFieldType::StringComponent;

    return DataFieldType::Unknown;
}

DataFieldElementType DataField::elementType() const
{
    switch(type())
    {
    case DataFieldType::IntNode:            return DataFieldElementType::Node;
    case DataFieldType::FloatNode:          return DataFieldElementType::Node;
    case DataFieldType::StringNode:         return DataFieldElementType::Node;

    case DataFieldType::IntEdge:            return DataFieldElementType::Edge;
    case DataFieldType::FloatEdge:          return DataFieldElementType::Edge;
    case DataFieldType::StringEdge:         return DataFieldElementType::Edge;

    case DataFieldType::IntComponent:       return DataFieldElementType::Component;
    case DataFieldType::FloatComponent:     return DataFieldElementType::Component;
    case DataFieldType::StringComponent:    return DataFieldElementType::Component;

    default: return DataFieldElementType::Unknown;
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

bool DataField::hasFloatMin() const { return _floatMin != std::numeric_limits<float>::max(); }
bool DataField::hasFloatMax() const { return _floatMax != std::numeric_limits<float>::min(); }
bool DataField::hasFloatRange() const { return hasFloatMin() && hasFloatMax(); }

float DataField::floatMin() const { return hasFloatMin() ? _floatMin : std::numeric_limits<float>::min(); }
float DataField::floatMax() const { return hasFloatMax() ? _floatMax : std::numeric_limits<float>::max(); }
DataField& DataField::setFloatMin(float floatMin) { _floatMin = floatMin; return *this; }
DataField& DataField::setFloatMax(float floatMax) { _floatMax = floatMax; return *this; }

bool DataField::floatValueInRange(float value) const
{
    if(hasFloatMin() && value < floatMin())
        return false;

    if(hasFloatMax() && value > floatMax())
        return false;

    return true;
}

std::vector<ConditionFnOp> DataField::validConditionFnOps() const
{
    switch(type())
    {
    case DataFieldType::IntNode:
    case DataFieldType::IntEdge:
    case DataFieldType::IntComponent:
    case DataFieldType::FloatNode:
    case DataFieldType::FloatEdge:
    case DataFieldType::FloatComponent:
        return
        {
            ConditionFnOp::Equal,
            ConditionFnOp::NotEqual,
            ConditionFnOp::LessThan,
            ConditionFnOp::GreaterThan,
            ConditionFnOp::LessThanOrEqual,
            ConditionFnOp::GreaterThanOrEqual
        };

    case DataFieldType::StringNode:
    case DataFieldType::StringEdge:
    case DataFieldType::StringComponent:
        return
        {
            ConditionFnOp::Equal,
            ConditionFnOp::NotEqual,
            ConditionFnOp::Contains,
            ConditionFnOp::StartsWith,
            ConditionFnOp::EndsWith,
            ConditionFnOp::MatchesRegex
        };

    default:
        return {};
    }
}
