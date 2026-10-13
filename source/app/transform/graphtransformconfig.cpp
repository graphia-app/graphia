/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "graphtransform.h"
#include "graphtransformconfigparser.h"

#include "app/attributes/attribute.h"

#include "shared/utils/container.h"

#include <QLocale>
#include <QObject>
#include <QStringList>
#include <QtGlobal>
#include <QVariantList>

#include <algorithm>
#include <cmath>
#include <map>

using namespace Qt::Literals::StringLiterals;

// Enforce consistency in rendering doubles to string
static QString doubleAsString(double d)
{
    auto string = QString::number(d, 'g', QLocale::FloatingPointShortest);

    if(std::isfinite(d) && !string.contains(u'.') && !string.contains(u'e'))
        string += u".0"_s;

    return string;
}

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
    return std::ranges::find_if(_parameters,
    [&name](const auto& parameter) { return name == parameter._name; }) != _parameters.end();
}

const GraphTransformConfig::Parameter* GraphTransformConfig::parameterByName(const QString &name) const
{
    auto it = std::ranges::find_if(_parameters,
        [&name](const auto& parameter) { return name == parameter._name; });

    if(it != _parameters.end())
        return &(*it);

    qFatal("Parameter not found");
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
    auto it = std::ranges::find_if(_parameters,
        [&name](const auto& parameter) { return name == parameter._name; });

    if(it != _parameters.end())
        it->_value = value;
    else
        _parameters.emplace_back(Parameter{._name = name, ._value = value});
}

QString GraphTransformConfig::Parameter::valueAsString(bool addQuotes) const
{
    struct Visitor
    {
        bool _addQuotes;

        explicit Visitor(bool addQuotes_) : _addQuotes(addQuotes_) {}

        QString operator()(double d) const          { return doubleAsString(d); }
        QString operator()(int i) const             { return QString::number(i); }
        QString operator()(const QString& s) const
        {
            if(_addQuotes)
            {
                QString escapedString = s;
                escapedString.replace(u"\""_s, u"\\\""_s);

                return u"\"%1\""_s.arg(escapedString);
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
        bool operator()(const RecursiveValue<CompoundCondition>&) const { return true; }
    };

    return std::visit(ConditionVisitor(), _condition);
}

QVariantMap GraphTransformConfig::conditionAsVariantMap() const
{
    struct ConditionVisitor
    {
        static QString terminalValueAsString(const GraphTransformConfig::TerminalValue& terminalValue)
        {
            struct Visitor
            {
                QString operator()(const double& d) const   { return doubleAsString(d); }
                QString operator()(const int& i) const      { return QString::number(i); }
                QString operator()(const QString& s) const  { return s; }
            };

            return std::visit(Visitor(), terminalValue);
        }

        QVariantMap operator()(GraphTransformConfig::NoCondition) const { return {}; }
        QVariantMap operator()(const TerminalCondition& terminalCondition) const
        {
            QVariantMap map;

            map.insert(u"lhs"_s, terminalValueAsString(terminalCondition._lhs));
            map.insert(u"op"_s, terminalCondition.opAsString());
            map.insert(u"rhs"_s, terminalValueAsString(terminalCondition._rhs));

            return map;
        }

        QVariantMap operator()(const UnaryCondition& unaryCondition) const
        {
            QVariantMap map;

            map.insert(u"lhs"_s, terminalValueAsString(unaryCondition._lhs));
            map.insert(u"op"_s, unaryCondition.opAsString());

            return map;
        }

        QVariantMap operator()(const RecursiveValue<CompoundCondition>& wrapper) const
        {
            const auto& compoundCondition = *wrapper;
            QVariantMap map;
            auto lhs = std::visit(ConditionVisitor(), compoundCondition._lhs);
            auto rhs = std::visit(ConditionVisitor(), compoundCondition._rhs);

            map.insert(u"lhs"_s, lhs);
            map.insert(u"op"_s, compoundCondition.opAsString());
            map.insert(u"rhs"_s, rhs);

            return map;
        }
    };

    return std::visit(ConditionVisitor(), _condition);
}

QString GraphTransformConfig::conditionAsString(bool forDisplay, const AttributeDisplayNameFn& attributeDisplayName) const
{
    struct ConditionVisitor
    {
        ConditionVisitor(bool forDisplay, const AttributeDisplayNameFn& attributeDisplayName) :
            _forDisplay(forDisplay), _attributeDisplayName(attributeDisplayName) {}

        bool _forDisplay;
        AttributeDisplayNameFn _attributeDisplayName;

        QString terminalValueAsString(const GraphTransformConfig::TerminalValue& terminalValue) const
        {
            struct Visitor
            {
                Visitor(bool forDisplay, const AttributeDisplayNameFn& attributeDisplayName) :
                    _forDisplay(forDisplay), _attributeDisplayName(attributeDisplayName) {}

                bool _forDisplay;
                AttributeDisplayNameFn _attributeDisplayName;

                QString operator()(const double& d) const   { return doubleAsString(d); }
                QString operator()(const int& i) const      { return QString::number(i); }
                QString operator()(const QString& s) const
                {
                    const auto attributeWithParameterTemplace = _forDisplay ? QObject::tr("%1%2 › %3") : u"$\"%1%2\".\"%3\""_s;
                    const auto attributeTemplate = _forDisplay ? QObject::tr("%1%2") : u"$\"%1%2\""_s;
                    const auto valueTemplate = _forDisplay ? QObject::tr("%1") : u"\"%1\""_s;

                    if(GraphTransformConfigParser::isAttributeName(s))
                    {
                        if(_forDisplay && _attributeDisplayName)
                            return _attributeDisplayName(s);

                        auto info = Attribute::parseAttributeName(s);

                        QString prefix;
                        switch(info._type)
                        {
                        case Attribute::EdgeNodeType::Source: prefix = _forDisplay ? QObject::tr("Source › ") : u"source."_s; break;
                        case Attribute::EdgeNodeType::Target: prefix = _forDisplay ? QObject::tr("Target › ") : u"target."_s; break;
                        default: break;
                        }

                        if(!info._parameter.isEmpty())
                            return attributeWithParameterTemplace.arg(prefix, info._name, info._parameter);

                        return attributeTemplate.arg(prefix, info._name);
                    }

                    return valueTemplate.arg(s);
                }
            };

            return std::visit(Visitor(_forDisplay, _attributeDisplayName), terminalValue);
        }

        QString prettifyOp(const QString& op) const
        {
            if(!_forDisplay)
                return op;

            std::map<QString, QString> replacements =
            {
                {"==",                       QObject::tr("=")},
                {"!=",                       QObject::tr("≠")},
                {"<=",                       QObject::tr("≤")},
                {">=",                       QObject::tr("≥")},
                {"&&",                       QObject::tr("and")},
                {"||",                       QObject::tr("or")},
                {"includes",                 QObject::tr("Includes")},
                {"excludes",                 QObject::tr("Excludes")},
                {"starts",                   QObject::tr("Starts With")},
                {"ends",                     QObject::tr("Ends With")},
                {"matches",                  QObject::tr("Matches Regex")},
                {"matchesCaseInsensitive",   QObject::tr("Matches Case Insensitive Regex")},
                {"hasValue",                 QObject::tr("Has Value")}
            };

            if(u::contains(replacements, op))
                return replacements.at(op);

            return op;
        }

        QString operator()(GraphTransformConfig::NoCondition) const { return {}; }
        QString operator()(const TerminalCondition& terminalCondition) const
        {
            return u"%1 %2 %3"_s.arg(
                terminalValueAsString(terminalCondition._lhs),
                prettifyOp(terminalCondition.opAsString()),
                terminalValueAsString(terminalCondition._rhs));
        }

        QString operator()(const UnaryCondition& unaryCondition) const
        {
            return u"%1 %2"_s.arg(
                terminalValueAsString(unaryCondition._lhs),
                prettifyOp(unaryCondition.opAsString()));
        }

        QString operator()(const RecursiveValue<CompoundCondition>& wrapper) const
        {
            const auto& compoundCondition = *wrapper;

            auto operandAsString = [this](const Condition& operand)
            {
                auto s = std::visit(ConditionVisitor(_forDisplay, _attributeDisplayName), operand);

                if(std::holds_alternative<RecursiveValue<CompoundCondition>>(operand))
                    return u"(%1)"_s.arg(s);

                return s;
            };

            auto lhs = operandAsString(compoundCondition._lhs);
            auto rhs = operandAsString(compoundCondition._rhs);

            return u"%1 %2 %3"_s.arg(lhs, prettifyOp(compoundCondition.opAsString()), rhs);
        }
    };

    return std::visit(ConditionVisitor(forDisplay, attributeDisplayName), _condition);
}

QVariantMap GraphTransformConfig::asVariantMap(const GraphTransformFactory* factory) const
{
    QVariantMap map;

    QVariantList flags;
    flags.reserve(static_cast<int>(_flags.size()));
    for(const auto& flag : _flags)
        flags.append(flag);
    map.insert(u"flags"_s, flags);

    map.insert(u"action"_s, _action);
    map.insert(u"displayName"_s, factory != nullptr ? factory->displayName() : _action);

    QStringList attributes;
    attributes.reserve(static_cast<int>(_attributes.size()));
    for(const auto& attribute : _attributes)
        attributes.append(Attribute::enquoteAttributeName(attribute));
    map.insert(u"attributes"_s, attributes);

    QVariantList parameters;
    for(const auto& parameter : _parameters)
    {
        QVariantMap parameterMap;
        parameterMap.insert(u"name"_s, parameter._name);
        parameterMap.insert(u"value"_s, parameter.valueAsString());
        parameters.append(parameterMap);
    }
    map.insert(u"parameters"_s, parameters);

    if(hasCondition())
        map.insert(u"condition"_s, conditionAsVariantMap());

    return map;
}

QString GraphTransformConfig::asString(bool forDisplay, const GraphTransformFactory* factory,
    const AttributeDisplayNameFn& attributeDisplayName) const
{
    QString s;

    if(!_flags.empty())
    {
        if(!forDisplay)
        {
            s += u"["_s;
            for(const auto& flag : _flags)
            {
                if(s[s.length() - 1] != '[')
                    s += u", "_s;

                s += flag;
            }
            s += u"] "_s;
        }
        else if(isFlagSet(u"pinned"_s))
            s += QObject::tr("📌 ");
    }

    const auto actionTemplate = forDisplay ? QObject::tr("%1") : u"\"%1\""_s;
    const auto attributeTemplate = forDisplay ? QObject::tr(" %1") : u" $\"%1\""_s;
    const auto parameterTemplate = forDisplay ?  QObject::tr(" %1 = %2") : u" \"%1\" = %2"_s;

    s += actionTemplate.arg(forDisplay && factory != nullptr ? factory->displayName() : _action);

    if(!_attributes.empty())
    {
        s += forDisplay ? QObject::tr(" using") : u" using"_s;

        for(const auto& attribute : _attributes)
        {
            s += attributeTemplate.arg(forDisplay && attributeDisplayName ?
                attributeDisplayName(attribute) : attribute);
        }
    }

    if(!_parameters.empty())
    {
        s += forDisplay ? QObject::tr(" with") : u" with"_s;

        for(const auto& parameter : _parameters)
        {
            auto name = forDisplay && factory != nullptr ?
                factory->parameter(parameter._name).displayName() :
                QString();

            if(name.isEmpty())
                name = parameter._name;

            s += parameterTemplate.arg(name, parameter.valueAsString(!forDisplay));
        }
    }

    if(hasCondition())
    {
        s += forDisplay ? QObject::tr(" where ") : u" where "_s;
        s += conditionAsString(forDisplay, attributeDisplayName);
    }

    return s;
}

std::vector<QString> GraphTransformConfig::referencedAttributeNames() const
{
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

        void operator()(const RecursiveValue<CompoundCondition>& wrapper) const
        {
            const auto& compoundCondition = *wrapper;
            std::visit(ConditionVisitor(_names), compoundCondition._lhs);
            std::visit(ConditionVisitor(_names), compoundCondition._rhs);
        }
    };

    std::visit(ConditionVisitor(&names), _condition);

    return names;
}

bool GraphTransformConfig::equals(const GraphTransformConfig& other,
    NamedBool<"ignoreInertFlags"> ignoreInertFlags) const
{
    std::vector<QString> flagsToIgnore;

    if(ignoreInertFlags)
    {
        // These flags do not cause a change in a transform's effect,
        // so ignore them for comparison purposes
        flagsToIgnore = {"locked", "pinned"};
    }

    auto flags = u::setDifference(_flags, flagsToIgnore);
    auto otherFlags = u::setDifference(other._flags, flagsToIgnore);

    return _action == other._action &&
            !u::setsDiffer(_parameters, other._parameters) &&
            !u::setsDiffer(flags, otherFlags) &&
            _attributes == other._attributes &&
            _condition == other._condition;
}

bool GraphTransformConfig::isFlagSet(const QString& flag) const
{
    return u::contains(_flags, flag);
}
