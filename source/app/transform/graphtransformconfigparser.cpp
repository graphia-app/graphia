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

#include "graphtransformconfigparser.h"

#include "app/configgrammar.h"

#include "shared/utils/recursivevalue.h"

#include <lexy_utils.h>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

#include <QByteArray>
#include <QDebug>

#include <utility>
#include <vector>

namespace LexyGraphTransformParser
{
namespace dsl = lexy::dsl;

using lexyutils::QuotedString;

using LexyConfigGrammar::AttributeParameters;
using LexyConfigGrammar::Flags;
using LexyConfigGrammar::idPattern;
using LexyConfigGrammar::Name;

constexpr auto equalityOps = lexy::symbol_table<ConditionFnOp::Equality>
    .map<LEXY_SYMBOL("==")>(ConditionFnOp::Equality::Equal)
    .map<LEXY_SYMBOL("!=")>(ConditionFnOp::Equality::NotEqual);

constexpr auto numericalOps = lexy::symbol_table<ConditionFnOp::Numerical>
    .map<LEXY_SYMBOL("<")>(ConditionFnOp::Numerical::LessThan)
    .map<LEXY_SYMBOL(">")>(ConditionFnOp::Numerical::GreaterThan)
    .map<LEXY_SYMBOL("<=")>(ConditionFnOp::Numerical::LessThanOrEqual)
    .map<LEXY_SYMBOL(">=")>(ConditionFnOp::Numerical::GreaterThanOrEqual);

constexpr auto stringOps = lexy::symbol_table<ConditionFnOp::String>
    .map<LEXY_SYMBOL("includes")>(ConditionFnOp::String::Includes)
    .map<LEXY_SYMBOL("excludes")>(ConditionFnOp::String::Excludes)
    .map<LEXY_SYMBOL("starts")>(ConditionFnOp::String::Starts)
    .map<LEXY_SYMBOL("ends")>(ConditionFnOp::String::Ends)
    .map<LEXY_SYMBOL("matches")>(ConditionFnOp::String::MatchesRegex)
    .map<LEXY_SYMBOL("matchesCaseInsensitive")>(ConditionFnOp::String::MatchesRegexCaseInsensitive);

constexpr auto logicalOps = lexy::symbol_table<ConditionFnOp::Logical>
    .map<LEXY_SYMBOL("or")>(ConditionFnOp::Logical::Or)
    .map<LEXY_SYMBOL("||")>(ConditionFnOp::Logical::Or)
    .map<LEXY_SYMBOL("and")>(ConditionFnOp::Logical::And)
    .map<LEXY_SYMBOL("&&")>(ConditionFnOp::Logical::And);

constexpr auto unaryOps = lexy::symbol_table<ConditionFnOp::Unary>
    .map<LEXY_SYMBOL("hasValue")>(ConditionFnOp::Unary::HasValue);

constexpr auto keywordUsing = LEXY_KEYWORD("using", idPattern);
constexpr auto keywordWith = LEXY_KEYWORD("with", idPattern);
constexpr auto keywordWhere = LEXY_KEYWORD("where", idPattern);

// Note the attribute name productions are token productions, so that no
// whitespace is skipped between the name and any of its parameters

struct AttributeNameNoDollar : lexy::token_production
{
    static constexpr auto rule = dsl::lit_c<'$'> >>
        (dsl::p<Name> + dsl::p<AttributeParameters>);

    static constexpr auto value = LexyConfigGrammar::asAttributeName;
};

struct AttributeName : lexy::token_production
{
    static constexpr auto rule = dsl::p<AttributeNameNoDollar>;

    static constexpr auto value = lexy::callback<QString>(
        [](const QString& name) { return "$" + name; });
};

struct Number
{
    static constexpr auto rule = dsl::capture(lexyutils::number);
    static constexpr auto value = lexyutils::asNumber<GraphTransformConfig::TerminalValue>;
};

struct ValueOperand
{
    static constexpr auto rule = dsl::p<AttributeName> | dsl::p<Number> | dsl::p<QuotedString>;
    static constexpr auto value = lexy::construct<GraphTransformConfig::TerminalValue>;
};

struct TerminalOp
{
    static constexpr auto rule =
        dsl::symbol<equalityOps> | dsl::symbol<numericalOps> | dsl::symbol<stringOps>;

    static constexpr auto value = lexy::construct<GraphTransformConfig::TerminalOp>;
};

struct UnaryOp
{
    static constexpr auto rule = dsl::symbol<unaryOps>;
    static constexpr auto value = lexy::forward<ConditionFnOp::Unary>;
};

struct LogicalOp
{
    static constexpr auto rule = dsl::symbol<logicalOps>;
    static constexpr auto value = lexy::forward<ConditionFnOp::Logical>;
};

// Both terminal and unary conditions start with a value operand,
// so they're only distinguishable by the operator that follows
struct TerminalOrUnaryCondition
{
    static constexpr auto rule = dsl::p<ValueOperand> >>
        ((dsl::p<TerminalOp> >> dsl::p<ValueOperand>) | dsl::p<UnaryOp>);

    static constexpr auto value = lexy::callback<GraphTransformConfig::Condition>(
        [](GraphTransformConfig::TerminalValue&& lhs,
           GraphTransformConfig::TerminalOp op,
           GraphTransformConfig::TerminalValue&& rhs)
        {
            return GraphTransformConfig::Condition(GraphTransformConfig::TerminalCondition{
                std::move(lhs), op, std::move(rhs)});
        },
        [](GraphTransformConfig::TerminalValue&& lhs, ConditionFnOp::Unary op)
        {
            return GraphTransformConfig::Condition(
                GraphTransformConfig::UnaryCondition{std::move(lhs), op});
        });
};

struct Condition;

struct Operand
{
    static constexpr auto rule =
        (dsl::lit_c<'('> >> (dsl::recurse<Condition> + dsl::lit_c<')'>)) |
        dsl::p<TerminalOrUnaryCondition>;

    static constexpr auto value = lexy::forward<GraphTransformConfig::Condition>;
};

struct Condition
{
    static constexpr auto rule = dsl::p<Operand> >>
        dsl::opt(dsl::p<LogicalOp> >> dsl::p<Operand>);

    static constexpr auto value = lexy::callback<GraphTransformConfig::Condition>(
        [](GraphTransformConfig::Condition&& condition, lexy::nullopt)
        {
            return std::move(condition);
        },
        [](GraphTransformConfig::Condition&& lhs, ConditionFnOp::Logical op,
            GraphTransformConfig::Condition&& rhs)
        {
            return GraphTransformConfig::Condition(
                RecursiveValue<GraphTransformConfig::CompoundCondition>(
                GraphTransformConfig::CompoundCondition{std::move(lhs), op, std::move(rhs)}));
        });
};

struct Parameter
{
    // A parameter is only committed to once its = has been seen, otherwise a
    // trailing where clause would be mistaken for the start of another parameter
    static constexpr auto rule =
        dsl::peek(dsl::p<Name> + dsl::while_(dsl::ascii::space) + dsl::lit_c<'='>) >>
        (dsl::p<Name> + dsl::lit_c<'='> + (dsl::p<Number> | dsl::p<QuotedString>));

    static constexpr auto value = lexy::construct<GraphTransformConfig::Parameter>;
};

struct Attributes
{
    static constexpr auto rule =
        dsl::opt(keywordUsing >> dsl::list(dsl::p<AttributeNameNoDollar>));

    static constexpr auto value = lexy::as_list<std::vector<QString>>;
};

struct Parameters
{
    static constexpr auto rule = dsl::opt(keywordWith >> dsl::list(dsl::p<Parameter>));
    static constexpr auto value = lexy::as_list<std::vector<GraphTransformConfig::Parameter>>;
};

struct OptionalCondition
{
    static constexpr auto rule = dsl::opt(keywordWhere >> dsl::p<Condition>);

    static constexpr auto value = lexy::bind(
        lexy::forward<GraphTransformConfig::Condition>,
        lexy::_1 || GraphTransformConfig::NoCondition{});
};

struct Transform
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = dsl::p<Flags> + dsl::p<Name> + dsl::p<Attributes> +
        dsl::p<Parameters> + dsl::p<OptionalCondition> + dsl::eof;

    static constexpr auto value = lexy::construct<GraphTransformConfig>;
};

template<typename Table>
void appendOps(QStringList& list, const Table& table)
{
    for(const auto& op : table)
        list.append(QString::fromUtf8(op.symbol));
}

template<typename Table, typename Op>
QString opAsString(const Table& table, Op op)
{
    for(const auto& entry : table)
    {
        if(entry.value == op)
            return QString::fromUtf8(entry.symbol);
    }

    return {};
}

template<typename Table>
const typename Table::mapped_type* findOp(const Table& table, const QString& s)
{
    const auto bytes = s.toUtf8();
    const auto input = lexy::string_input<lexy::default_encoding>(
        bytes.constData(), bytes.constData() + bytes.size());

    const auto index = table.parse(input);

    if(!index)
        return nullptr;

    return &table[index];
}
} // namespace LexyGraphTransformParser

bool GraphTransformConfigParser::parse(const QString& text, bool warnOnFailure)
{
    const auto bytes = text.toUtf8();
    const auto* const begin = bytes.constData();
    const auto* const end = begin + bytes.size();

    const auto input = lexy::string_input<lexy::default_encoding>(begin, end);

    _result = {};
    _failedInput.clear();

    const char* failurePosition = nullptr;
    const auto onError = lexy::callback<void>([&failurePosition](const auto&, const auto& error)
    {
        // Only the first (i.e. earliest) failure is of interest
        if(failurePosition == nullptr)
            failurePosition = error.position();
    });

    auto result = lexy::parse<LexyGraphTransformParser::Transform>(input, onError);
    _success = result.is_success();

    if(_success)
        _result = result.value();
    else
    {
        if(failurePosition == nullptr)
            failurePosition = begin;

        _failedInput = QString::fromUtf8(failurePosition,
            static_cast<qsizetype>(end - failurePosition));

        if(warnOnFailure)
            qWarning() << "Failed to parse" << _failedInput;
    }

    return _success;
}

QStringList GraphTransformConfigParser::ops(ValueType valueType)
{
    QStringList list;

    LexyGraphTransformParser::appendOps(list, LexyGraphTransformParser::equalityOps);

    switch(valueType)
    {
    case ValueType::Float:
    case ValueType::Int:
        LexyGraphTransformParser::appendOps(list, LexyGraphTransformParser::numericalOps);
        break;

    case ValueType::String:
        LexyGraphTransformParser::appendOps(list, LexyGraphTransformParser::stringOps);
        break;

    default: break;
    }

    return list;
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Equality op)
{
    return LexyGraphTransformParser::opAsString(LexyGraphTransformParser::equalityOps, op);
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Numerical op)
{
    return LexyGraphTransformParser::opAsString(LexyGraphTransformParser::numericalOps, op);
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::String op)
{
    return LexyGraphTransformParser::opAsString(LexyGraphTransformParser::stringOps, op);
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Logical op)
{
    return LexyGraphTransformParser::opAsString(LexyGraphTransformParser::logicalOps, op);
}

QString GraphTransformConfigParser::opToString(ConditionFnOp::Unary op)
{
    return LexyGraphTransformParser::opAsString(LexyGraphTransformParser::unaryOps, op);
}

GraphTransformConfig::TerminalOp GraphTransformConfigParser::stringToOp(const QString& s)
{
    const auto* equalityOp = LexyGraphTransformParser::findOp(
        LexyGraphTransformParser::equalityOps, s);
    if(equalityOp != nullptr)
        return *equalityOp;

    const auto* numericalOp = LexyGraphTransformParser::findOp(
        LexyGraphTransformParser::numericalOps, s);
    if(numericalOp != nullptr)
        return *numericalOp;

    const auto* stringOp = LexyGraphTransformParser::findOp(
        LexyGraphTransformParser::stringOps, s);
    if(stringOp != nullptr)
        return *stringOp;

    return {};
}

bool GraphTransformConfigParser::opIsUnary(const QString& op)
{
    return LexyGraphTransformParser::findOp(LexyGraphTransformParser::unaryOps, op) != nullptr;
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
