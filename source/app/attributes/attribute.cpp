/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "attribute.h"

#include "shared/utils/container.h"

#include <QDebug>

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

void Attribute::clearSetValueFunctions()
{
    _.setValueNodeIdFn = nullptr;
    _.setValueEdgeIdFn = nullptr;
    _.setValueComponentFn = nullptr;
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

Attribute& Attribute::setSetValueFn(SetValueFn<NodeId> setValueFn)
{
    clearSetValueFunctions();
    _.setValueNodeIdFn = setValueFn;
    if(elementType() != ElementType::Node)
        qDebug() << "Setting set value function with mismatched element type";
    return *this;
}

Attribute& Attribute::setSetValueFn(SetValueFn<EdgeId> setValueFn)
{
    clearSetValueFunctions();
    _.setValueEdgeIdFn = setValueFn;
    if(elementType() != ElementType::Edge)
        qDebug() << "Setting set value function with mismatched element type";
    return *this;
}

Attribute& Attribute::setSetValueFn(SetValueFn<const IGraphComponent&> setValueFn)
{
    clearSetValueFunctions();
    _.setValueComponentFn = setValueFn;
    if(elementType() != ElementType::Component)
        qDebug() << "Setting set value function with mismatched element type";
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

void Attribute::setValueOf(NodeId nodeId, const QString& value) const
{
    Q_ASSERT(_.setValueNodeIdFn != nullptr);

    if(_.setValueNodeIdFn != nullptr)
        _.setValueNodeIdFn(nodeId, value);
}

void Attribute::setValueOf(EdgeId edgeId, const QString& value) const
{
    Q_ASSERT(_.setValueEdgeIdFn != nullptr);

    if(_.setValueEdgeIdFn != nullptr)
        _.setValueEdgeIdFn(edgeId, value);
}

void Attribute::setValueOf(const IGraphComponent& graphComponent, const QString& value) const
{
    Q_ASSERT(_.setValueComponentFn != nullptr);

    if(_.setValueComponentFn != nullptr)
        _.setValueComponentFn(graphComponent, value);
}

ValueType Attribute::valueType() const
{
    switch(type())
    {
    case Type::IntNode:
    case Type::IntEdge:
    case Type::IntComponent:
        return ValueType::Int;

    case Type::FloatNode:
    case Type::FloatEdge:
    case Type::FloatComponent:
        return ValueType::Float;

    case Type::StringNode:
    case Type::StringEdge:
    case Type::StringComponent:
        return ValueType::String;

    default: return ValueType::Unknown;
    }
}

ElementType Attribute::elementType() const
{
    switch(type())
    {
    case Type::IntNode:
    case Type::FloatNode:
    case Type::StringNode:
        return ElementType::Node;

    case Type::IntEdge:
    case Type::FloatEdge:
    case Type::StringEdge:
        return ElementType::Edge;

    case Type::IntComponent:
    case Type::FloatComponent:
    case Type::StringComponent:
        return ElementType::Component;

    default:
        return ElementType::None;
    }
}

bool Attribute::editable() const
{
    return _.setValueNodeIdFn != nullptr ||
        _.setValueEdgeIdFn != nullptr ||
        _.setValueComponentFn != nullptr;
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
    auto dotIndex = name.indexOf('.');
    if(dotIndex >= 0)
    {
        parameter = name.mid(dotIndex + 1);
        name = name.mid(0, dotIndex);
    }

    name.replace(QStringLiteral(R"(")"), QLatin1String(""));
    parameter.replace(QStringLiteral(R"(")"), QLatin1String(""));

    return {type, name, parameter};
}

QString Attribute::enquoteAttributeName(const QString& name)
{
    const QString sourceString = QStringLiteral("source.");
    const QString targetString = QStringLiteral("target.");
    QString prefix;
    QString postfix;

    auto parsedAttributeName = parseAttributeName(name);

    switch(parsedAttributeName._type)
    {
    case EdgeNodeType::Source: prefix = sourceString; break;
    case EdgeNodeType::Target: prefix = targetString; break;
    default: break;
    }

    if(!parsedAttributeName._parameter.isEmpty())
        postfix = QStringLiteral(R"(."%1")").arg(parsedAttributeName._parameter);

    return QStringLiteral(R"(%1"%2"%3)").arg(prefix,
        parsedAttributeName._name, postfix);
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
