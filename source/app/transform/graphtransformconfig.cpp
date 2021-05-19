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

#include "graphtransformconfig.h"
#include "graphtransformconfigparser.h"

#include "attributes/attribute.h"

#include "shared/utils/container.h"

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

    return std::visit(Visitor(), _op);
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

const std::vector<QString>& GraphTransformConfig::attributeNames() const
{
    return _attributes;
}

bool GraphTransformConfig::hasParameter(const QString& name) const
{
    return std::find_if(_parameters.begin(), _parameters.end(),
    [&name](const auto& parameter) { return name == parameter._name; }) != _parameters.end();
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

bool GraphTransformConfig::parameterHasValue(const QString& name, const QString& value) const
{
    if(!hasParameter(name))
        return false;

    return parameterByName(name)->valueAsString() == value;
}

void GraphTransformConfig::setParameterValue(const QString& name,
    const GraphTransformConfig::ParameterValue& value)
{
    auto it = std::find_if(_parameters.begin(), _parameters.end(),
        [&name](const auto& parameter) { return name == parameter._name; });

    if(it != _parameters.end())
        it->_value = value;
    else
        _parameters.emplace_back(Parameter{name, value});
}

QString GraphTransformConfig::Parameter::valueAsString(bool addQuotes) const
{
    struct Visitor
    {
        bool _addQuotes;

        explicit Visitor(bool addQuotes_) : _addQuotes(addQuotes_) {}

        QString operator()(double d) const          { return QString::number(d, 'f'); }
        QString operator()(int i) const             { return QString::number(i); }
        QString operator()(const QString& s) const
        {
            if(_addQuotes)
            {
                QString escapedString = s;
                escapedString.replace(QStringLiteral(R"(")"), QStringLiteral(R"(\")"));

                return QStringLiteral(R"("%1")").arg(escapedString);
            }

            return s;
        }
    };

    return std::visit(Visitor(addQuotes), _value);
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
        static QString terminalValueAsString(const GraphTransformConfig::TerminalValue& terminalValue)
        {
            struct Visitor
            {
                QString operator()(const double& d) const   { return QString::number(d, 'f'); }
                QString operator()(const int& i) const      { return QString::number(i); }
                QString operator()(const QString& s) const  { return s; }
            };

            return std::visit(Visitor(), terminalValue);
        }

        QVariantMap operator()(GraphTransformConfig::NoCondition) const { return {}; }
        QVariantMap operator()(const TerminalCondition& terminalCondition) const
        {
            QVariantMap map;

            map.insert(QStringLiteral("lhs"), terminalValueAsString(terminalCondition._lhs));
            map.insert(QStringLiteral("op"), terminalCondition.opAsString());
            map.insert(QStringLiteral("rhs"), terminalValueAsString(terminalCondition._rhs));

            return map;
        }

        QVariantMap operator()(const UnaryCondition& unaryCondition) const
        {
            QVariantMap map;

            map.insert(QStringLiteral("lhs"), terminalValueAsString(unaryCondition._lhs));
            map.insert(QStringLiteral("op"), unaryCondition.opAsString());

            return map;
        }

        QVariantMap operator()(const CompoundCondition& compoundCondition) const
        {
            QVariantMap map;
            auto lhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._rhs);

            map.insert(QStringLiteral("lhs"), lhs);
            map.insert(QStringLiteral("op"), compoundCondition.opAsString());
            map.insert(QStringLiteral("rhs"), rhs);

            return map;
        }
    };

    return boost::apply_visitor(ConditionVisitor(), _condition);
}

QString GraphTransformConfig::conditionAsString() const
{
    struct ConditionVisitor
    {
        static QString terminalValueAsString(const GraphTransformConfig::TerminalValue& terminalValue)
        {
            struct Visitor
            {
                QString operator()(const double& d) const   { return QString::number(d, 'f'); }
                QString operator()(const int& i) const      { return QString::number(i); }
                QString operator()(const QString& s) const
                {
                    if(GraphTransformConfigParser::isAttributeName(s))
                    {
                        auto info = Attribute::parseAttributeName(s);

                        const char* prefix = "";
                        switch(info._type)
                        {
                        case Attribute::EdgeNodeType::Source: prefix = "source."; break;
                        case Attribute::EdgeNodeType::Target: prefix = "target."; break;
                        default: break;
                        }

                        if(!info._parameter.isEmpty())
                            return QString(R"($"%1%2"."%3")").arg(prefix, info._name, info._parameter);

                        return QString(R"($"%1%2")").arg(prefix, info._name);
                    }

                    return QString(R"("%1")").arg(s);
                }
            };

            return std::visit(Visitor(), terminalValue);
        }

        QString operator()(GraphTransformConfig::NoCondition) const { return {}; }
        QString operator()(const TerminalCondition& terminalCondition) const
        {
            return QString("%1 %2 %3").arg(
                terminalValueAsString(terminalCondition._lhs),
                terminalCondition.opAsString(),
                terminalValueAsString(terminalCondition._rhs));
        }

        QString operator()(const UnaryCondition& unaryCondition) const
        {
            return QString("%1 %2").arg(
                terminalValueAsString(unaryCondition._lhs),
                unaryCondition.opAsString());
        }

        QString operator()(const CompoundCondition& compoundCondition) const
        {
            auto lhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._lhs);
            auto rhs = boost::apply_visitor(ConditionVisitor(), compoundCondition._rhs);

            return QString("%1 %2 %3").arg(lhs, compoundCondition.opAsString(), rhs);
        }
    };

    return boost::apply_visitor(ConditionVisitor(), _condition);
}

QVariantMap GraphTransformConfig::asVariantMap() const
{
    QVariantMap map;

    QVariantList flags;
    flags.reserve(static_cast<int>(_flags.size()));
    for(const auto& flag : _flags)
        flags.append(flag);
    map.insert(QStringLiteral("flags"), flags);

    map.insert(QStringLiteral("action"), _action);

    QStringList attributes;
    attributes.reserve(static_cast<int>(_attributes.size()));
    for(const auto& attribute : _attributes)
        attributes.append(Attribute::enquoteAttributeName(attribute));
    map.insert(QStringLiteral("attributes"), attributes);

    QVariantList parameters;
    for(const auto& parameter : _parameters)
    {
        QVariantMap parameterMap;
        parameterMap.insert(QStringLiteral("name"), parameter._name);
        parameterMap.insert(QStringLiteral("value"), parameter.valueAsString());
        parameters.append(parameterMap);
    }
    map.insert(QStringLiteral("parameters"), parameters);

    if(hasCondition())
        map.insert(QStringLiteral("condition"), conditionAsVariantMap());

    return map;
}

QString GraphTransformConfig::asString() const
{
    QString s;

    if(!_flags.empty())
    {
        s += QStringLiteral("[");
        for(const auto& flag : _flags)
        {
            if(s[s.length() - 1] != '[')
                s += QStringLiteral(", ");

            s += flag;
        }
        s += QStringLiteral("] ");
    }

    s += QStringLiteral(R"("%1")").arg(_action);

    if(!_attributes.empty())
    {
        s += QStringLiteral(" using");

        for(const auto& attribute : _attributes)
            s += QStringLiteral(R"( $"%1")").arg(attribute);
    }

    if(!_parameters.empty())
    {
        s += QStringLiteral(" with");

        for(const auto& parameter : _parameters)
        {
            // cppcheck-suppress useStlAlgorithm
            s += QStringLiteral(R"( "%1" = %2)").arg(parameter._name,
                parameter.valueAsString(true));
        }
    }

    if(hasCondition())
    {
        s += QStringLiteral(" where ");
        s += conditionAsString();
    }

    return s;
}

std::vector<QString> GraphTransformConfig::referencedAttributeNames() const
{
    // cppcheck-suppress unreadVariable
    auto names = _attributes;

    struct ConditionVisitor
    {
        explicit ConditionVisitor(std::vector<QString>* innerNames) :
            _names(innerNames) {}

        std::vector<QString>* _names;

        void operator()(GraphTransformConfig::NoCondition) const {};

        static QString attributeFromTerminalValue(const TerminalValue& terminalValue)
        {
            const auto* s = std::get_if<QString>(&terminalValue);
            if(s != nullptr && GraphTransformConfigParser::isAttributeName(*s))
                return s->mid(1); // Strip leading $

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
            _attributes == other._attributes &&
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
