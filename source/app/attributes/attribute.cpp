#include "attribute.h"

#include "shared/utils/container.h"

#include <QDebug>
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

int Attribute::valueOf(Helper<int>, NodeId nodeId) const { return callValueFn(_.intNodeIdFn, nodeId); }
int Attribute::valueOf(Helper<int>, EdgeId edgeId) const { return callValueFn(_.intEdgeIdFn, edgeId); }
int Attribute::valueOf(Helper<int>, const IGraphComponent& component) const
{ return callValueFn<int, const IGraphComponent&>(_.intComponentFn, component); }

double Attribute::valueOf(Helper<double>, NodeId nodeId) const { return callValueFn(_.floatNodeIdFn, nodeId); }
double Attribute::valueOf(Helper<double>, EdgeId edgeId) const { return callValueFn(_.floatEdgeIdFn, edgeId); }
double Attribute::valueOf(Helper<double>, const IGraphComponent& component) const
{ return callValueFn<double, const IGraphComponent&>(_.floatComponentFn, component); }

QString Attribute::valueOf(Helper<QString>, NodeId nodeId) const { return callValueFn(_.stringNodeIdFn, nodeId); }
QString Attribute::valueOf(Helper<QString>, EdgeId edgeId) const { return callValueFn(_.stringEdgeIdFn, edgeId); }
QString Attribute::valueOf(Helper<QString>, const IGraphComponent& component) const
{ return callValueFn<QString, const IGraphComponent&>(_.stringComponentFn, component); }

bool Attribute::valueMissingOf(NodeId nodeId) const
{
    if(valueFnIsSet(_.valueMissingNodeIdFn))
        return callValueFn(_.valueMissingNodeIdFn, nodeId);

    return false;
}

bool Attribute::valueMissingOf(EdgeId edgeId) const
{
    if(valueFnIsSet(_.valueMissingEdgeIdFn))
        return callValueFn(_.valueMissingEdgeIdFn, edgeId);

    return false;
}

bool Attribute::valueMissingOf(const IGraphComponent& component) const
{
    if(valueFnIsSet(_.valueMissingComponentFn))
        return callValueFn<bool, const IGraphComponent&>(_.valueMissingComponentFn, component);

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
    if(valueFnIsSet(_.intNodeIdFn))         return Type::IntNode;
    if(valueFnIsSet(_.intEdgeIdFn))         return Type::IntEdge;
    if(valueFnIsSet(_.intComponentFn))      return Type::IntComponent;

    if(valueFnIsSet(_.floatNodeIdFn))       return Type::FloatNode;
    if(valueFnIsSet(_.floatEdgeIdFn))       return Type::FloatEdge;
    if(valueFnIsSet(_.floatComponentFn))    return Type::FloatComponent;

    if(valueFnIsSet(_.stringNodeIdFn))      return Type::StringNode;
    if(valueFnIsSet(_.stringEdgeIdFn))      return Type::StringEdge;
    if(valueFnIsSet(_.stringComponentFn))   return Type::StringComponent;

    return Type::Unknown;
}

void Attribute::disableAutoRange()
{
    _.flags.reset(AttributeFlag::AutoRange);
}

bool Attribute::hasMissingValues() const
{
    switch(elementType())
    {
    case ElementType::Node:         return valueFnIsSet(_.valueMissingNodeIdFn);
    case ElementType::Edge:         return valueFnIsSet(_.valueMissingEdgeIdFn);
    case ElementType::Component:    return valueFnIsSet(_.valueMissingComponentFn);
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

QString Attribute::parameterValue() const
{
    int numValidValues = static_cast<int>(_.validParameterValues.size());

    // Parameter not yet set
    if(_.parameterIndex < 0 || _.parameterIndex >= numValidValues)
        return {};

    return _.validParameterValues.at(_.parameterIndex);
}

bool Attribute::setParameterValue(const QString& value)
{
    if(!u::contains(_.validParameterValues, value))
        return false;

    _.parameterIndex = u::indexOf(_.validParameterValues, value);
    return true;
}

bool Attribute::hasParameter() const
{
    return !_.validParameterValues.empty();
}

QStringList Attribute::validParameterValues() const
{
    return _.validParameterValues;
}

IAttribute& Attribute::setValidParameterValues(const QStringList& values)
{
    _.validParameterValues = values;
    return *this;
}

bool AttributeRange<int>::hasMin() const { return _attribute->_.intMin != std::numeric_limits<int>::max(); }
bool AttributeRange<int>::hasMax() const { return _attribute->_.intMax != std::numeric_limits<int>::lowest(); }
bool AttributeRange<int>::hasRange() const { return hasMin() && hasMax(); }

int AttributeRange<int>::min() const { return hasMin() ? _attribute->_.intMin : std::numeric_limits<int>::lowest(); }
int AttributeRange<int>::max() const { return hasMax() ? _attribute->_.intMax : std::numeric_limits<int>::max(); }
IAttribute& AttributeRange<int>::setMin(int min) { _attribute->_.intMin = min; _attribute->disableAutoRange(); return *_attribute; }
IAttribute& AttributeRange<int>::setMax(int max) { _attribute->_.intMax = max; _attribute->disableAutoRange(); return *_attribute; }

bool AttributeRange<double>::hasMin() const { return _attribute->_.floatMin != std::numeric_limits<double>::max(); }
bool AttributeRange<double>::hasMax() const { return _attribute->_.floatMax != std::numeric_limits<double>::lowest(); }
bool AttributeRange<double>::hasRange() const { return hasMin() && hasMax(); }

double AttributeRange<double>::min() const { return hasMin() ? _attribute->_.floatMin : std::numeric_limits<double>::lowest(); }
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
    case ValueType::Int: return static_cast<double>(_attribute->_intRange.max());
    case ValueType::Float: return _attribute->_floatRange.max();
    default: return std::numeric_limits<double>::lowest();
    }
}

IAttribute& AttributeNumericRange::setMin(double min)
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: _attribute->_intRange.setMin(static_cast<int>(min)); break;
    case ValueType::Float: _attribute->_floatRange.setMin(min); break;
    default: break;
    }

    return *_attribute;
}

IAttribute& AttributeNumericRange::setMax(double max)
{
    switch(_attribute->valueType())
    {
    case ValueType::Int: _attribute->_intRange.setMax(static_cast<int>(max)); break;
    case ValueType::Float: _attribute->_floatRange.setMax(max); break;
    default: break;
    }

    return *_attribute;
}

Attribute::Name Attribute::parseAttributeName(QString name)
{
    const QString sourceString = QStringLiteral("source.");
    const QString targetString = QStringLiteral("target.");

    EdgeNodeType type = EdgeNodeType::None;

    // Strip off leading $, if present
    if(name.startsWith('$'))
        name.remove(0, 1);

    if(name.startsWith(sourceString))
    {
        type = EdgeNodeType::Source;
        name = name.mid(sourceString.length());
    }
    else if(name.startsWith(targetString))
    {
        type = EdgeNodeType::Target;
        name = name.mid(targetString.length());
    }

    QString parameter;
    QRegularExpression re(QStringLiteral(R"(^([^\.]*)(?:\.(.+))?$)"));
    auto match = re.match(name);

    if(match.hasMatch())
    {
        name = match.captured(1);
        name.replace("\"", "");
        auto parameterString = match.captured(2);
        if(!parameterString.isNull())
        {
            parameter = parameterString;
            parameter.replace("\"", "");
        }
    }

    return {type, name, parameter};
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
