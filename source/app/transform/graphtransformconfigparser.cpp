/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "graphtransformconfigparser.h"

#define BOOST_SPIRIT_X3_UNICODE

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include <QRegularExpression>
#include <QDebug>

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::TerminalCondition,
    _lhs,
    _op,
    _rhs
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::UnaryCondition,
    _lhs,
    _op
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::CompoundCondition,
    _lhs,
    _op,
    _rhs
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::Parameter,
    _name,
    _value
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig,
    _flags,
    _action,
    _attributes,
    _parameters,
    _condition
)

namespace SpiritGraphTranformConfigParser
{
namespace x3 = boost::spirit::x3;
namespace unicode = boost::spirit::x3::unicode;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using unicode::char_;

const x3::rule<class QuotedString, QString> quotedString = "quotedString"; // NOLINT bugprone-forward-declaration-namespace
const auto escapedQuote = lit('\\') >> char_('"');
const auto quotedString_def = lexeme['"' >> *(escapedQuote | ~char_('"')) >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier"; // NOLINT bugprone-forward-declaration-namespace
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const x3::rule<class AttributeParameter, QString> attributeParameter = "attributeParameter"; // NOLINT bugprone-forward-declaration-namespace
const auto attributeParameter_def = lexeme[char_('.') >> (quotedString | identifier)];

const x3::rule<class AttributeName, QString> attributeName = "attributeName"; // NOLINT bugprone-forward-declaration-namespace
const auto attributeName_def = lexeme[char_('$') >> (quotedString | identifier) >> *attributeParameter];

struct equality_op_ : x3::symbols<ConditionFnOp::Equality>
{
    equality_op_() noexcept
    {
        add
        ("==", ConditionFnOp::Equality::Equal)
        ("!=", ConditionFnOp::Equality::NotEqual)
        ;
    }
} equality_op;

struct numerical_op_ : x3::symbols<ConditionFnOp::Numerical>
{
    numerical_op_() noexcept
    {
        add
        ("<",  ConditionFnOp::Numerical::LessThan)
        (">",  ConditionFnOp::Numerical::GreaterThan)
        ("<=", ConditionFnOp::Numerical::LessThanOrEqual)
        (">=", ConditionFnOp::Numerical::GreaterThanOrEqual)
        ;
    }
} numerical_op;

struct string_op_ : x3::symbols<ConditionFnOp::String>
{
    string_op_() noexcept
    {
        add
        ("includes",                ConditionFnOp::String::Includes)
        ("excludes",                ConditionFnOp::String::Excludes)
        ("starts",                  ConditionFnOp::String::Starts)
        ("ends",                    ConditionFnOp::String::Ends)
        ("matches",                 ConditionFnOp::String::MatchesRegex)
        ("matchesCaseInsensitive",  ConditionFnOp::String::MatchesRegexCaseInsensitive)
        ;
    }
} string_op;

struct unary_op_ : x3::symbols<ConditionFnOp::Unary>
{
    unary_op_() noexcept
    {
        add
        ("hasValue", ConditionFnOp::Unary::HasValue)
        ;
    }
} unary_op;

const auto terminalBinaryOperator = equality_op | numerical_op | string_op;
const auto valueOperand = (attributeName | double_ | int_ | quotedString);

const x3::rule<class TerminalCondition, GraphTransformConfig::TerminalCondition> terminalCondition = "terminalCondition";
const auto terminalCondition_def = valueOperand >> terminalBinaryOperator >> valueOperand;

const x3::rule<class UnaryCondition, GraphTransformConfig::UnaryCondition> unaryCondition = "unaryCondition";
const auto unaryCondition_def = valueOperand >> unary_op;

struct logical_op_ : x3::symbols<ConditionFnOp::Logical>
{
    logical_op_() noexcept
    {
        add
        ("or",  ConditionFnOp::Logical::Or)
        ("and", ConditionFnOp::Logical::And)
        ("||",  ConditionFnOp::Logical::Or)
        ("&&",  ConditionFnOp::Logical::And)
        ;
    }
} logical_op;

const x3::rule<class Condition, GraphTransformConfig::Condition> condition = "condition";
const auto operand = terminalCondition | unaryCondition | (lit('(') >> condition >> lit(')'));
const auto condition_def = (operand >> logical_op >> operand) | operand;

const auto attributeNameNoDollarCapture = lexeme[lit('$') >> (quotedString | identifier) >> *attributeParameter];

const x3::rule<class Parameter, GraphTransformConfig::Parameter> parameter = "parameter"; // NOLINT bugprone-forward-declaration-namespace
const auto parameterName = quotedString | identifier;
const auto parameter_def = parameterName >> lit('=') >> (double_ | int_ | quotedString);

const auto identifierList = identifier % lit(',');
const auto flags = lit('[') >> -identifierList >> lit(']');

const x3::rule<class Transform, GraphTransformConfig> transform = "transform";
const auto transformName = quotedString | identifier;
const auto transform_def =
    -flags >>
    transformName >>
    -(lit("using") >> +attributeNameNoDollarCapture) >>
    -(lit("with") >> +parameter) >>
    -(lit("where") >> condition);

BOOST_SPIRIT_DEFINE(quotedString, identifier, attributeParameter, attributeName,
    transform, parameter, condition, terminalCondition, unaryCondition)
} // namespace SpiritGraphTranformConfigParser

bool GraphTransformConfigParser::parse(const QString& text, bool warnOnFailure)
{
    QStringSpiritUnicodeConstIterator begin(text.begin());
    const QStringSpiritUnicodeConstIterator end(text.end());
    _result = {};
    _success = SpiritGraphTranformConfigParser::x3::phrase_parse(begin, end,
                    SpiritGraphTranformConfigParser::transform,
                    SpiritGraphTranformConfigParser::unicode::space, _result);

    if(begin != end)
    {
        _success = false;
        _failedInput = QString::fromStdString(std::string(begin, end));

        if(warnOnFailure)
            qWarning() << "Failed to parse" << _failedInput;
    }

    return _success;
}

QStringList GraphTransformConfigParser::ops(ValueType valueType)
{
    QStringList list;

    SpiritGraphTranformConfigParser::equality_op.for_each([&list](auto& v, auto) { list.append(QString::fromStdString(v)); });

    switch(valueType)
    {
    case ValueType::Float:
    case ValueType::Int:
        SpiritGraphTranformConfigParser::numerical_op.for_each([&list](auto& v, auto) { list.append(QString::fromStdString(v)); });
        break;

    case ValueType::String:
        SpiritGraphTranformConfigParser::string_op.for_each([&list](auto& v, auto) { list.append(QString::fromStdString(v)); });
        break;

    default: break;
    }

    return list;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Equality op)
{
    QString result;

    SpiritGraphTranformConfigParser::equality_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::equality_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Numerical op)
{
    QString result;

    SpiritGraphTranformConfigParser::numerical_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::numerical_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::String op)
{
    QString result;

    SpiritGraphTranformConfigParser::string_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::string_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Logical op)
{
    QString result;

    SpiritGraphTranformConfigParser::logical_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::logical_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Unary op)
{
    QString result;

    SpiritGraphTranformConfigParser::unary_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::unary_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

GraphTransformConfig::TerminalOp GraphTransformConfigParser::stringToOp(const QString& s)
{
    auto* equalityOp = SpiritGraphTranformConfigParser::equality_op.find(s.toStdString());
    if(equalityOp != nullptr)
        return *equalityOp;

    auto* numericalOp = SpiritGraphTranformConfigParser::numerical_op.find(s.toStdString());
    if(numericalOp != nullptr)
        return *numericalOp;

    auto* stringOp = SpiritGraphTranformConfigParser::string_op.find(s.toStdString());
    if(stringOp != nullptr)
        return *stringOp;

    return {};
}

bool GraphTransformConfigParser::opIsUnary(const QString& op)
{
    return SpiritGraphTranformConfigParser::unary_op.find(op.toStdString()) != nullptr;
}

bool GraphTransformConfigParser::isAttributeName(const QString& variable)
{
    return !variable.isEmpty() && variable[0] == '$';
}

QString GraphTransformConfigParser::parseForDisplay(const QString& text)
{
    GraphTransformConfigParser parser;

    if(parser.parse(text))
        return parser.result().asString(true);

    return {};
}
