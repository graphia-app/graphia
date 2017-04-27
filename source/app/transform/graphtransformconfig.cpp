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
    return _attributeName == other._attributeName &&
            _opValue == other._opValue;
}

QString GraphTransformConfig::TerminalCondition::opAsString() const
{
    struct Visitor
    {
        QString operator()(const FloatOpValue& v) const     { return GraphTransformConfigParser::opToString(v._op); }
        QString operator()(const IntOpValue& v) const       { return GraphTransformConfigParser::opToString(v._op); }
        QString operator()(const StringOpValue& v) const    { return GraphTransformConfigParser::opToString(v._op); }
        QString operator()(const UnaryOp& v) const          { return GraphTransformConfigParser::opToString(v); }
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
        QString operator()(const UnaryOp&) const            { return {}; }
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

const GraphTransformConfig::Parameter* GraphTransformConfig::parameterByName(const QString &name) const
{
    auto it = std::find_if(_parameters.begin(), _parameters.end(),
        [&name](const auto& parameter) { return name == parameter._name; });

    if(it != _parameters.end())
        return &(*it);

    Q_ASSERT(!"Parameter not found");
    return nullptr;
}

QString GraphTransformConfig::Parameter::valueAsString() const
{
    struct Visitor
    {
        QString operator()(double d) const { return QString::number(d); }
        QString operator()(int i) const { return QString::number(i); }
        QString operator()(const QString& s) const { return s; }
    };

    return boost::apply_visitor(Visitor(), _value);
}

bool GraphTransformConfig::hasCondition() const
{
    struct ConditionVisitor
    {
        bool operator()(const TerminalCondition& terminalCondition) const
        {
            return !terminalCondition._attributeName.isEmpty();
        }

        bool operator()(const CompoundCondition& compoundCondition) const
        {
            auto lhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._rhs);

            return lhs && rhs;
        }
    };

    return boost::apply_visitor(ConditionVisitor(), _condition);
}

QVariantMap GraphTransformConfig::conditionAsVariantMap() const
{
    struct ConditionVisitor
    {
        QVariantMap operator()(const TerminalCondition& terminalCondition) const
        {
            QVariantMap map;

            map.insert("lhs", terminalCondition._attributeName);
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

    QVariantList flags;
    for(const auto& flag : _flags)
        flags.append(flag);
    map.insert("flags", flags);

    map.insert("action", _action);

    QVariantMap parameters;
    for(const auto& parameter : _parameters)
        parameters.insert(parameter._name, parameter.valueAsString());
    map.insert("parameters", parameters);

    if(hasCondition())
        map.insert("condition", conditionAsVariantMap());

    return map;
}

std::vector<QString> GraphTransformConfig::attributeNames() const
{
    std::vector<QString> names;

    struct ConditionVisitor
    {
        explicit ConditionVisitor(std::vector<QString>* innerNames) :
            _names(innerNames) {}

        std::vector<QString>* _names;

        void operator()(const TerminalCondition& terminalCondition) const
        {
            if(!terminalCondition._attributeName.isEmpty())
                _names->emplace_back(terminalCondition._attributeName);
        }

        void operator()(const CompoundCondition& compoundCondition) const
        {
            boost::apply_visitor(ConditionVisitor(_names), compoundCondition._lhs);
            boost::apply_visitor(ConditionVisitor(_names), compoundCondition._rhs);
        }
    };

    boost::apply_visitor(ConditionVisitor(&names), _condition);

    return names;
}

bool GraphTransformConfig::operator==(const GraphTransformConfig& other) const
{
    // Note: _flags is deliberately ignored
    // when comparing GraphTransformConfigs
    return _action == other._action &&
            _parameters == other._parameters &&
            _condition == other._condition;
}

bool GraphTransformConfig::operator!=(const GraphTransformConfig& other) const
{
    return !operator==(other);
}

bool GraphTransformConfig::isFlagSet(const QString& flag) const
{
    return u::contains(_flags, flag);
}
