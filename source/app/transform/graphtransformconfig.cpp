#include "graphtransformconfig.h"
#include "graphtransformconfigparser.h"

#include "attributes/attribute.h"

#include "shared/utils/utils.h"

#include <QVariantList>

bool GraphTransformConfig::TerminalCondition::operator==(const GraphTransformConfig::TerminalCondition& other) const
{
    return _lhs == other._lhs &&
            _op == other._op &&
            _rhs == other._rhs;
}


bool GraphTransformConfig::UnaryCondition::operator==(const GraphTransformConfig::UnaryCondition& other) const
{
    return _lhs == other._lhs &&
            _op == other._op;
}

QString GraphTransformConfig::TerminalCondition::opAsString() const
{
    struct Visitor
    {
        QString operator()(ConditionFnOp::Equality v) const  { return GraphTransformConfigParser::opToString(v); }
        QString operator()(ConditionFnOp::Numerical v) const { return GraphTransformConfigParser::opToString(v); }
        QString operator()(ConditionFnOp::String v) const    { return GraphTransformConfigParser::opToString(v); }
    };

    return boost::apply_visitor(Visitor(), _op);
}

QString GraphTransformConfig::UnaryCondition::opAsString() const
{
    return GraphTransformConfigParser::opToString(_op);
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
        bool operator()(GraphTransformConfig::NoCondition) const { return false; }
        bool operator()(const TerminalCondition&) const { return true; }
        bool operator()(const UnaryCondition&) const { return true; }
        bool operator()(const CompoundCondition&) const { return true; }
    };

    return boost::apply_visitor(ConditionVisitor(), _condition);
}

QVariantMap GraphTransformConfig::conditionAsVariantMap() const
{
    struct ConditionVisitor
    {
        QString terminalValueAsString(const GraphTransformConfig::TerminalValue& terminalValue) const
        {
            struct Visitor
            {
                QString operator()(const double& v) const   { return QString::number(v); }
                QString operator()(const int& v) const      { return QString::number(v); }
                QString operator()(const QString& v) const  { return v; }
            };

            return boost::apply_visitor(Visitor(), terminalValue);
        }

        QVariantMap operator()(GraphTransformConfig::NoCondition) const { return {}; }
        QVariantMap operator()(const TerminalCondition& terminalCondition) const
        {
            QVariantMap map;

            map.insert("lhs", terminalValueAsString(terminalCondition._lhs));
            map.insert("op", terminalCondition.opAsString());
            map.insert("rhs", terminalValueAsString(terminalCondition._rhs));

            return map;
        }

        QVariantMap operator()(const UnaryCondition& unaryCondition) const
        {
            QVariantMap map;

            map.insert("lhs", terminalValueAsString(unaryCondition._lhs));
            map.insert("op", unaryCondition.opAsString());

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

        void operator()(GraphTransformConfig::NoCondition) const {};

        QString attributeFromTerminalValue(const TerminalValue& terminalValue) const
        {
            const QString* s = boost::get<QString>(&terminalValue);
            if(s != nullptr && GraphTransformConfigParser::isAttributeName(*s))
                return GraphTransformConfigParser::attributeNameFor(*s);

            return {};
        }

        void operator()(const TerminalCondition& terminalCondition) const
        {
            auto lhs = attributeFromTerminalValue(terminalCondition._lhs);
            auto rhs = attributeFromTerminalValue(terminalCondition._rhs);

            if(!lhs.isEmpty())
            {
                auto attributeName = Attribute::parseAttributeName(lhs);
                _names->emplace_back(attributeName._name);
            }

            if(!rhs.isEmpty())
            {
                auto attributeName = Attribute::parseAttributeName(rhs);
                _names->emplace_back(attributeName._name);
            }
        }

        void operator()(const UnaryCondition& unaryCondition) const
        {
            auto lhs = attributeFromTerminalValue(unaryCondition._lhs);

            if(!lhs.isEmpty())
                _names->emplace_back(lhs);
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
    // These flags do not cause a change in a transform's effect,
    // so ignore them for comparison purposes
    const std::vector<QString> flagsToIgnore = {"locked", "pinned"};

    auto flags = u::setDifference(_flags, flagsToIgnore);
    auto otherFlags = u::setDifference(other._flags, flagsToIgnore);

    return _action == other._action &&
            !u::setsDiffer(_parameters, other._parameters) &&
            !u::setsDiffer(flags, otherFlags) &&
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
