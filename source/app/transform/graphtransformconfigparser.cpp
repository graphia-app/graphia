#include "graphtransformconfigparser.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

#include <QRegularExpression>

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::TerminalCondition,
    (GraphTransformConfig::TerminalValue, _lhs),
    (GraphTransformConfig::TerminalOp, _op),
    (GraphTransformConfig::TerminalValue, _rhs)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::UnaryCondition,
    (GraphTransformConfig::TerminalValue, _lhs),
    (ConditionFnOp::Unary, _op)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::CompoundCondition,
    (GraphTransformConfig::Condition, _lhs),
    (ConditionFnOp::Logical, _op),
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

const x3::rule<class AttributeName, QString> attributeName = "attributeName";
const auto attributeName_def = char_('$') >> (quotedString | identifier);

struct equality_op_ : x3::symbols<ConditionFnOp::Equality>
{
    equality_op_()
    {
        add
        ("==", ConditionFnOp::Equality::Equal)
        ("!=", ConditionFnOp::Equality::NotEqual)
        ;
    }
} equality_op;

struct numerical_op_ : x3::symbols<ConditionFnOp::Numerical>
{
    numerical_op_()
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
    string_op_()
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
    unary_op_()
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
    logical_op_()
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
const auto operand = terminalCondition | unaryCondition | (x3::lit('(') >> condition >> x3::lit(')'));
const auto condition_def = (operand >> logical_op >> operand) | operand;

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

BOOST_SPIRIT_DEFINE(quotedString, identifier, attributeName, transform, parameter, condition, terminalCondition, unaryCondition);
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

QString GraphTransformConfigParser::attributeNameFor(const QString& variable)
{
    QString attributeName = variable;
    attributeName.replace(QRegularExpression(QStringLiteral("^\\$")), QLatin1String(""));

    return attributeName;
}
