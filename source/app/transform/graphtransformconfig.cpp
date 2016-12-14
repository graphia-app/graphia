#include "graphtransformconfig.h"
#include "graphtransformconfigparser.h"

#include "shared/utils/utils.h"

#include <QVariantList>

bool GraphTransformConfig::FloatOpValue::operator==(const GraphTransformConfig::FloatOpValue& other) const
{
    return _op == other._op &&
            _value == other._value;
}

bool GraphTransformConfig::IntOpValue::operator==(const GraphTransformConfig::IntOpValue& other) const
{
    return _op == other._op &&
            _value == other._value;
}

bool GraphTransformConfig::StringOpValue::operator==(const GraphTransformConfig::StringOpValue& other) const
{
    return _op == other._op &&
            _value == other._value;
}

bool GraphTransformConfig::TerminalCondition::operator==(const GraphTransformConfig::TerminalCondition& other) const
{
    return _field == other._field &&
            _opValue == other._opValue;
}

QString GraphTransformConfig::TerminalCondition::opAsString() const
{
    struct Visitor
    {
        QString operator()(const FloatOpValue& v) const     { return GraphTransformConfigParser::opToString(v._op); }
        QString operator()(const IntOpValue& v) const       { return GraphTransformConfigParser::opToString(v._op); }
        QString operator()(const StringOpValue& v) const    { return GraphTransformConfigParser::opToString(v._op); }
    };

    return boost::apply_visitor(Visitor(), _opValue);
}

QString GraphTransformConfig::TerminalCondition::valueAsString() const
{
    struct Visitor
    {
        QString operator()(const FloatOpValue& v) const     { return QString::number(v._value); }
        QString operator()(const IntOpValue& v) const       { return QString::number(v._value); }
        QString operator()(const StringOpValue& v) const    { return v._value; }
    };

    return boost::apply_visitor(Visitor(), _opValue);
}

bool GraphTransformConfig::CompoundCondition::operator==(const GraphTransformConfig::CompoundCondition& other) const
{
    return _lhs == other._lhs &&
            _op == other._op &&
            _rhs == other._rhs;
}

QString GraphTransformConfig::CompoundCondition::opAsString() const
{
    return GraphTransformConfigParser::opToString(_op);
}

bool GraphTransformConfig::Parameter::operator==(const GraphTransformConfig::Parameter& other) const
{
    return _name == other._name &&
            _value == other._value;
}

QString GraphTransformConfig::Parameter::valueAsString() const
{
    struct Visitor
    {
        QString operator()(double d) const { return QString::number(d); }
        QString operator()(const QString& s) const { return s; }
    };

    return boost::apply_visitor(Visitor(), _value);
}

QVariantMap GraphTransformConfig::conditionAsVariantMap() const
{
    struct ConditionVisitor
    {
        QVariantMap operator()(const TerminalCondition& terminalCondition) const
        {
            QVariantMap map;

            map.insert("lhs", terminalCondition._field);
            map.insert("op", terminalCondition.opAsString());
            map.insert("rhs", terminalCondition.valueAsString());

            return map;
        }

        QVariantMap operator()(const CompoundCondition& compoundCondition) const
        {
            QVariantMap map;
            auto lhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._rhs);

            map.insert("lhs", lhs);
            map.insert("op", compoundCondition.opAsString());
            map.insert("rhs", rhs);

            return map;
        }
    };

    return boost::apply_visitor(ConditionVisitor(), _condition);
}

QVariantMap GraphTransformConfig::asVariantMap() const
{
    QVariantMap map;

    QVariantList metaAttributes;
    for(const auto& metaAttribute : _metaAttributes)
        metaAttributes.append(metaAttribute);
    map.insert("metaAttributes", metaAttributes);

    map.insert("action", _action);

    QVariantList parameters;
    for(const auto& parameter : _parameters)
    {
        QVariantMap parameterObject;
        parameterObject.insert("name", parameter._name);
        parameterObject.insert("value", parameter.valueAsString());
        parameters.append(parameterObject);
    }
    map.insert("parameters", parameters);

    map.insert("condition", conditionAsVariantMap());

    return map;
}

bool GraphTransformConfig::operator==(const GraphTransformConfig& other) const
{
    // Note: _metaAttributes is deliberately ignored
    // when comparing GraphTransformConfigs
    return _action == other._action &&
            _parameters == other._parameters &&
            _condition == other._condition;
}

bool GraphTransformConfig::operator!=(const GraphTransformConfig& other) const
{
    return !operator==(other);
}

bool GraphTransformConfig::isMetaAttributeSet(const QString& metaAttribute) const
{
    return u::contains(_metaAttributes, metaAttribute);
}
