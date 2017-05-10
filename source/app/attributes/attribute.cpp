#include "attribute.h"

#include <QRegularExpression>

void Attribute::clearValueFunctions()
{
    _.intNodeIdFn = nullptr;
    _.intEdgeIdFn = nullptr;
    _.intComponentFn = nullptr;

    _.floatNodeIdFn = nullptr;
    _.floatEdgeIdFn = nullptr;
    _.floatComponentFn = nullptr;

    _.stringNodeIdFn = nullptr;
    _.stringEdgeIdFn = nullptr;
    _.stringComponentFn = nullptr;
}

void Attribute::clearMissingFunctions()
{
    _.valueMissingNodeIdFn = nullptr;
    _.valueMissingEdgeIdFn = nullptr;
    _.valueMissingComponentFn = nullptr;
}

int Attribute::valueOf(Helper<int>, NodeId nodeId) const { Q_ASSERT(_.intNodeIdFn != nullptr); return _.intNodeIdFn(nodeId); }
int Attribute::valueOf(Helper<int>, EdgeId edgeId) const { Q_ASSERT(_.intEdgeIdFn != nullptr); return _.intEdgeIdFn(edgeId); }
int Attribute::valueOf(Helper<int>, const IGraphComponent& component) const { Q_ASSERT(_.intComponentFn != nullptr); return _.intComponentFn(component); }

double Attribute::valueOf(Helper<double>, NodeId nodeId) const { Q_ASSERT(_.floatNodeIdFn != nullptr); return _.floatNodeIdFn(nodeId); }
double Attribute::valueOf(Helper<double>, EdgeId edgeId) const { Q_ASSERT(_.floatEdgeIdFn != nullptr); return _.floatEdgeIdFn(edgeId); }
double Attribute::valueOf(Helper<double>, const IGraphComponent& component) const { Q_ASSERT(_.floatComponentFn != nullptr); return _.floatComponentFn(component); }

QString Attribute::valueOf(Helper<QString>, NodeId nodeId) const { Q_ASSERT(_.stringNodeIdFn != nullptr); return _.stringNodeIdFn(nodeId); }
QString Attribute::valueOf(Helper<QString>, EdgeId edgeId) const { Q_ASSERT(_.stringEdgeIdFn != nullptr); return _.stringEdgeIdFn(edgeId); }
QString Attribute::valueOf(Helper<QString>, const IGraphComponent& component) const { Q_ASSERT(_.stringComponentFn != nullptr); return _.stringComponentFn(component); }

bool Attribute::valueMissingOf(NodeId nodeId) const
{
    if(_.valueMissingNodeIdFn != nullptr)
        return _.valueMissingNodeIdFn(nodeId);

    return false;
}

bool Attribute::valueMissingOf(EdgeId edgeId) const
{
    if(_.valueMissingEdgeIdFn != nullptr)
        return _.valueMissingEdgeIdFn(edgeId);

    return false;
}

bool Attribute::valueMissingOf(const IGraphComponent& component) const
{
    if(_.valueMissingComponentFn != nullptr)
        return _.valueMissingComponentFn(component);

    return false;
}

Attribute& Attribute::setIntValueFn(ValueFn<int, NodeId> valueFn) { clearValueFunctions(); _.intNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, EdgeId> valueFn) { clearValueFunctions(); _.intEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setIntValueFn(ValueFn<int, const IGraphComponent&> valueFn) { clearValueFunctions(); _.intComponentFn = valueFn; return *this; }

Attribute& Attribute::setFloatValueFn(ValueFn<double, NodeId> valueFn) { clearValueFunctions(); _.floatNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setFloatValueFn(ValueFn<double, EdgeId> valueFn) { clearValueFunctions(); _.floatEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setFloatValueFn(ValueFn<double, const IGraphComponent&> valueFn) { clearValueFunctions(); _.floatComponentFn = valueFn; return *this; }

Attribute& Attribute::setStringValueFn(ValueFn<QString, NodeId> valueFn) { clearValueFunctions(); _.stringNodeIdFn = valueFn; return *this; }
Attribute& Attribute::setStringValueFn(ValueFn<QString, EdgeId> valueFn) { clearValueFunctions(); _.stringEdgeIdFn = valueFn; return *this; }
Attribute& Attribute::setStringValueFn(ValueFn<QString, const IGraphComponent&> valueFn) { clearValueFunctions(); _.stringComponentFn = valueFn; return *this; }

Attribute& Attribute::setValueMissingFn(ValueFn<bool, NodeId> missingFn)
{
    clearMissingFunctions();
    _.valueMissingNodeIdFn = missingFn;
    if(elementType() != ElementType::Node)
        qDebug() << "Setting value missing function with mismatched element type";
    return *this;
}

Attribute& Attribute::setValueMissingFn(ValueFn<bool, EdgeId> missingFn)
{
    clearMissingFunctions();
    _.valueMissingEdgeIdFn = missingFn;
    if(elementType() != ElementType::Edge)
        qDebug() << "Setting value missing function with mismatched element type";
    return *this;
}

Attribute& Attribute::setValueMissingFn(ValueFn<bool, const IGraphComponent&> missingFn)
{
    clearMissingFunctions();
    _.valueMissingComponentFn = missingFn;
    if(elementType() != ElementType::Component)
        qDebug() << "Setting value missing function with mismatched element type";
    return *this;
}

Attribute::Type Attribute::type() const
{
    if(_.intNodeIdFn != nullptr)         return Type::IntNode;
    if(_.intEdgeIdFn != nullptr)         return Type::IntEdge;
    if(_.intComponentFn != nullptr)      return Type::IntComponent;

    if(_.floatNodeIdFn != nullptr)       return Type::FloatNode;
    if(_.floatEdgeIdFn != nullptr)       return Type::FloatEdge;
    if(_.floatComponentFn != nullptr)    return Type::FloatComponent;

    if(_.stringNodeIdFn != nullptr)      return Type::StringNode;
    if(_.stringEdgeIdFn != nullptr)      return Type::StringEdge;
    if(_.stringComponentFn != nullptr)   return Type::StringComponent;

    return Type::Unknown;
}

void Attribute::disableAutoRange()
{
    _.flags.reset(AttributeFlag::AutoRangeMutable, AttributeFlag::AutoRangeTransformed);
}

bool Attribute::hasMissingValues() const
{
    switch(elementType())
    {
    case ElementType::Node:         return _.valueMissingNodeIdFn != nullptr;
    case ElementType::Edge:         return _.valueMissingEdgeIdFn != nullptr;
    case ElementType::Component:    return _.valueMissingComponentFn != nullptr;
    default:                        return false;
    }

    return false;
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

bool Attribute::isValid() const
{
    return type() != Type::Unknown;
}

bool AttributeRange<int>::hasMin() const { return _attribute->_.intMin != std::numeric_limits<int>::max(); }
bool AttributeRange<int>::hasMax() const { return _attribute->_.intMax != std::numeric_limits<int>::min(); }
bool AttributeRange<int>::hasRange() const { return hasMin() && hasMax(); }

int AttributeRange<int>::min() const { return hasMin() ? _attribute->_.intMin : std::numeric_limits<int>::min(); }
int AttributeRange<int>::max() const { return hasMax() ? _attribute->_.intMax : std::numeric_limits<int>::max(); }
IAttribute& AttributeRange<int>::setMin(int min) { _attribute->_.intMin = min; _attribute->disableAutoRange(); return *_attribute; }
IAttribute& AttributeRange<int>::setMax(int max) { _attribute->_.intMax = max; _attribute->disableAutoRange(); return *_attribute; }

bool AttributeRange<double>::hasMin() const { return _attribute->_.floatMin != std::numeric_limits<double>::max(); }
bool AttributeRange<double>::hasMax() const { return _attribute->_.floatMax != std::numeric_limits<double>::min(); }
bool AttributeRange<double>::hasRange() const { return hasMin() && hasMax(); }

double AttributeRange<double>::min() const { return hasMin() ? _attribute->_.floatMin : std::numeric_limits<double>::min(); }
double AttributeRange<double>::max() const { return hasMax() ? _attribute->_.floatMax : std::numeric_limits<double>::max(); }
IAttribute& AttributeRange<double>::setMin(double min) { _attribute->_.floatMin = min; _attribute->disableAutoRange(); return *_attribute; }
IAttribute& AttributeRange<double>::setMax(double max) { _attribute->_.floatMax = max; _attribute->disableAutoRange(); return *_attribute; }

bool AttributeNumericRange::hasMin() const
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: return _attribute->_intRange.hasMin();
    case ValueType::Float: return _attribute->_floatRange.hasMin();
    default: return false;
    }
}

bool AttributeNumericRange::hasMax() const
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: return _attribute->_intRange.hasMax();
    case ValueType::Float: return _attribute->_floatRange.hasMax();
    default: return false;
    }
}

bool AttributeNumericRange::hasRange() const
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: return _attribute->_intRange.hasRange();
    case ValueType::Float: return _attribute->_floatRange.hasRange();
    default: return false;
    }
}

double AttributeNumericRange::min() const
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: return static_cast<double>(_attribute->_intRange.min());
    case ValueType::Float: return _attribute->_floatRange.min();
    default: return std::numeric_limits<double>::max();
    }
}

double AttributeNumericRange::max() const
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: return static_cast<double>(_attribute->_floatRange.max());
    case ValueType::Float: return _attribute->_floatRange.max();
    default: return std::numeric_limits<double>::min();
    }
}

IAttribute& AttributeNumericRange::setMin(double min)
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: _attribute->_intRange.setMin(static_cast<int>(min));
    case ValueType::Float: _attribute->_intRange.setMin(min);
    default: break;
    }

    return *_attribute;
}

IAttribute& AttributeNumericRange::setMax(double max)
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: _attribute->_intRange.setMax(static_cast<int>(max));
    case ValueType::Float: _attribute->_intRange.setMax(max);
    default: break;
    }

    return *_attribute;
}

Attribute::Name Attribute::parseAttributeName(const QString& name)
{
    QRegularExpression edgeNodeRe("^(source|target)\\.(.+)$");
    auto match = edgeNodeRe.match(name);

    if(match.hasMatch())
    {
        auto edgeNodeTypeString = match.captured(1);
        auto resolvedName = match.captured(2);

        EdgeNodeType edgeNodeType = EdgeNodeType::None;
        if(edgeNodeTypeString == "source")
            edgeNodeType = EdgeNodeType::Source;
        else if(edgeNodeTypeString == "target")
            edgeNodeType = EdgeNodeType::Target;

        return {edgeNodeType, resolvedName};
    }

    return {Attribute::EdgeNodeType::None, name};
}

Attribute Attribute::edgeNodesAttribute(const IGraph& graph, const Attribute& nodeAttribute,
                                        Attribute::EdgeNodeType edgeNodeType)
{
    if(nodeAttribute.elementType() != ElementType::Node)
        return {};

    Attribute attribute = nodeAttribute;

    NodeId(IEdge::*pFn)() const = nullptr;

    switch(edgeNodeType)
    {
    case Attribute::EdgeNodeType::None:   return {};
    case Attribute::EdgeNodeType::Source: pFn = &IEdge::sourceId; break;
    case Attribute::EdgeNodeType::Target: pFn = &IEdge::targetId; break;
    }

    switch(nodeAttribute.valueType())
    {
    case ValueType::Int:
        attribute.setIntValueFn([nodeAttribute, pFn, &graph](EdgeId edgeId)
        {
            auto nodeId = (graph.edgeById(edgeId).*pFn)();
            return nodeAttribute.valueOf<int>(nodeId);
        });
        break;

    case ValueType::Float:
        attribute.setFloatValueFn([nodeAttribute, pFn, &graph](EdgeId edgeId)
        {
            auto nodeId = (graph.edgeById(edgeId).*pFn)();
            return nodeAttribute.valueOf<double>(nodeId);
        });
        break;

    case ValueType::String:
        attribute.setStringValueFn([nodeAttribute, pFn, &graph](EdgeId edgeId)
        {
            auto nodeId = (graph.edgeById(edgeId).*pFn)();
            return nodeAttribute.valueOf<QString>(nodeId);
        });
        break;

    default: return {};
    }

    return attribute;
}
