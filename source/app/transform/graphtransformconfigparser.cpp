#include "graphtransformconfigparser.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::IntOpValue,
    (ConditionFnOp::Numerical, _op),
    (int, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::FloatOpValue,
    (ConditionFnOp::Numerical, _op),
    (double, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::StringOpValue,
    (ConditionFnOp::String, _op),
    (QString, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::TerminalCondition,
    (QString, _attributeName),
    (GraphTransformConfig::OpValue, _opValue)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::CompoundCondition,
    (GraphTransformConfig::Condition, _lhs),
    (ConditionFnOp::Binary, _op),
    (GraphTransformConfig::Condition, _rhs)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::Parameter,
    (QString, _name),
    (GraphTransformConfig::ParameterValue, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig,
    (std::vector<QString>, _flags),
    (QString, _action),
    (std::vector<GraphTransformConfig::Parameter>, _parameters),
    (GraphTransformConfig::Condition, _condition)
)

namespace SpiritGraphTranformConfigParser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using ascii::char_;

const x3::rule<class QuotedString, QString> quotedString = "quotedString";
const auto escapedQuote = x3::lit('\\') >> char_('"');
const auto quotedString_def = lexeme['"' >> *(escapedQuote | ~char_('"')) >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier";
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const auto attributeName = quotedString | identifier;

struct numerical_op_ : x3::symbols<ConditionFnOp::Numerical>
{
    numerical_op_()
    {
        add
        ("==", ConditionFnOp::Numerical::Equal)
        ("!=", ConditionFnOp::Numerical::NotEqual)
        ("<",  ConditionFnOp::Numerical::LessThan)
        (">",  ConditionFnOp::Numerical::GreaterThan)
        ("<=", ConditionFnOp::Numerical::LessThanOrEqual)
        (">=", ConditionFnOp::Numerical::GreaterThanOrEqual)
        ;
    }
} numerical_op;

struct string_op_ : x3::symbols<ConditionFnOp::String>
{
    string_op_()
    {
        add
        ("==",       ConditionFnOp::String::Equal)
        ("!=",       ConditionFnOp::String::NotEqual)
        ("includes", ConditionFnOp::String::Includes)
        ("excludes", ConditionFnOp::String::Excludes)
        ("starts",   ConditionFnOp::String::Starts)
        ("ends",     ConditionFnOp::String::Ends)
        ("matches",  ConditionFnOp::String::MatchesRegex)
        ;
    }
} string_op;

struct unary_op_ : x3::symbols<ConditionFnOp::Unary>
{
    unary_op_()
    {
        add
        ("hasValue", ConditionFnOp::Unary::HasValue)
        ;
    }
} unary_op;

const auto floatCondition = numerical_op >> double_;
const auto intCondition = numerical_op >> int_;
const auto stringCondition = string_op >> quotedString;
const auto unaryCondition = unary_op;

const x3::rule<class TerminalCondition, GraphTransformConfig::TerminalCondition> terminalCondition = "terminalCondition";
const auto terminalCondition_def = attributeName >> (floatCondition | intCondition | stringCondition | unaryCondition);

struct binary_op_ : x3::symbols<ConditionFnOp::Binary>
{
    binary_op_()
    {
        add
        ("or",  ConditionFnOp::Binary::Or)
        ("and", ConditionFnOp::Binary::And)
        ("||",  ConditionFnOp::Binary::Or)
        ("&&",  ConditionFnOp::Binary::And)
        ;
    }
} binary_op;

const x3::rule<class Condition, GraphTransformConfig::Condition> condition = "condition";

const auto operand = terminalCondition | (x3::lit('(') >> condition >> x3::lit(')'));
const auto condition_def = (operand >> binary_op >> operand) | operand;

const x3::rule<class Parameter, GraphTransformConfig::Parameter> parameter = "parameter";
const auto parameterName = quotedString | identifier;
const auto parameter_def = parameterName >> x3::lit('=') >> (double_ | int_ | quotedString);

const auto identifierList = identifier % x3::lit(',');
const auto flags = x3::lit('[') >> -identifierList >> x3::lit(']');

const x3::rule<class Transform, GraphTransformConfig> transform = "transform";
const auto transformName = quotedString | identifier;
const auto transform_def =
    -flags >>
    transformName >>
    -(x3::lit("with") >> +parameter) >>
    -(x3::lit("where") >> condition);

BOOST_SPIRIT_DEFINE(quotedString, identifier, transform, parameter, condition, terminalCondition);
} // namespace SpiritGraphTranformConfigParser

bool GraphTransformConfigParser::parse(const QString& text)
{
    auto stdString = text.toStdString();
    auto begin = stdString.begin();
    auto end = stdString.end();
    _result = {};
    _success = SpiritGraphTranformConfigParser::x3::phrase_parse(begin, end,
                    SpiritGraphTranformConfigParser::transform,
                    SpiritGraphTranformConfigParser::ascii::space, _result);

    if(begin != end)
    {
        _success = false;
        _failedInput = QString::fromStdString(std::string(begin, end));
    }

    return _success;
}

QStringList GraphTransformConfigParser::ops(ValueType valueType)
{
    QStringList list;

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

QString GraphTransformConfigParser::opToString(ConditionFnOp::Binary op)
{
    QString result;

    SpiritGraphTranformConfigParser::binary_op.for_each([&](auto& v, auto)
    {
        if(SpiritGraphTranformConfigParser::binary_op.at(v) == op)
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

bool GraphTransformConfigParser::opIsUnary(const QString& op)
{
    return SpiritGraphTranformConfigParser::unary_op.find(op.toStdString()) != nullptr;
}
