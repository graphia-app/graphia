#include "graphtransformconfigparser.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::IntOpValue,
    (GraphTransformConfig::NumericalOp, _op),
    (int, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::FloatOpValue,
    (GraphTransformConfig::NumericalOp, _op),
    (double, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::StringOpValue,
    (GraphTransformConfig::StringOp, _op),
    (QString, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::TerminalCondition,
    (QString, _field),
    (GraphTransformConfig::OpValue, _opValue)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::CompoundCondition,
    (GraphTransformConfig::Condition, _lhs),
    (GraphTransformConfig::BinaryOp, _op),
    (GraphTransformConfig::Condition, _rhs)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig::Parameter,
    (QString, _name),
    (GraphTransformConfig::ParameterValue, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    GraphTransformConfig,
    (std::vector<QString>, _metaAttributes),
    (QString, _action),
    (std::vector<GraphTransformConfig::Parameter>, _parameters),
    (GraphTransformConfig::Condition, _condition)
)

namespace Parser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::string;
using x3::lexeme;
using ascii::char_;

const x3::rule<class QuotedString, QString> quotedString = "quotedString";
const auto quotedString_def = lexeme['"' >> +(char_ - '"') >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier";
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const auto fieldName = quotedString | identifier;

struct numerical_op_ : x3::symbols<GraphTransformConfig::NumericalOp>
{
    numerical_op_()
    {
        add
        ("==", GraphTransformConfig::NumericalOp::Equal)
        ("!=", GraphTransformConfig::NumericalOp::NotEqual)
        ("<",  GraphTransformConfig::NumericalOp::LessThan)
        (">",  GraphTransformConfig::NumericalOp::GreaterThan)
        ("<=", GraphTransformConfig::NumericalOp::LessThanOrEqual)
        (">=", GraphTransformConfig::NumericalOp::GreaterThanOrEqual)
        ;
    }
} numerical_op;

struct string_op_ : x3::symbols<GraphTransformConfig::StringOp>
{
    string_op_()
    {
        add
        ("==",       GraphTransformConfig::StringOp::Equal)
        ("!=",       GraphTransformConfig::StringOp::NotEqual)
        ("includes", GraphTransformConfig::StringOp::Includes)
        ("excludes", GraphTransformConfig::StringOp::Excludes)
        ("starts",   GraphTransformConfig::StringOp::Starts)
        ("ends",     GraphTransformConfig::StringOp::Ends)
        ("matches",  GraphTransformConfig::StringOp::MatchesRegex)
        ;
    }
} string_op;

const auto floatCondition = numerical_op >> double_;
const auto intCondition = numerical_op >> int_;
const auto stringCondition = string_op >> quotedString;

const x3::rule<class TerminalCondition, GraphTransformConfig::TerminalCondition> terminalCondition = "terminalCondition";
const auto terminalCondition_def = fieldName >> (floatCondition | intCondition | stringCondition);

struct binary_op_ : x3::symbols<GraphTransformConfig::BinaryOp>
{
    binary_op_()
    {
        add
        ("or",  GraphTransformConfig::BinaryOp::Or)
        ("and", GraphTransformConfig::BinaryOp::And)
        ("||",  GraphTransformConfig::BinaryOp::Or)
        ("&&",  GraphTransformConfig::BinaryOp::And)
        ;
    }
} binary_op;

const x3::rule<class Condition, GraphTransformConfig::Condition> condition = "condition";

const auto operand = terminalCondition | (x3::lit("(") >> condition >> x3::lit(")"));
const auto condition_def = (operand >> binary_op >> operand) | operand;

const x3::rule<class Parameter, GraphTransformConfig::Parameter> parameter = "parameter";
const auto parameterName = quotedString | identifier;
const auto parameter_def = parameterName >> x3::lit("=") >> (double_ | quotedString);

const auto identifierList = identifier % x3::lit(",");
const auto metaAttributes = x3::lit("[") >> -identifierList >> x3::lit("]");

const x3::rule<class Transform, GraphTransformConfig> transform = "transform";
const auto transformName = quotedString | identifier;
const auto transform_def =
    -metaAttributes >>
    transformName >>
    -(x3::lit("with") >> +parameter) >>
    -(x3::lit("where") >> condition);

BOOST_SPIRIT_DEFINE(quotedString, identifier, transform, parameter, condition, terminalCondition);
} // namespace Parser

bool GraphTransformConfigParser::parse(const QString& text)
{
    auto stdString = text.toStdString();
    auto begin = stdString.begin();
    auto end = stdString.end();
    _result = {};
    _success = Parser::x3::phrase_parse(begin, end, Parser::transform,
                                        Parser::ascii::space, _result);

    if(begin != end)
    {
        _success = false;
        _failedInput = QString::fromStdString(std::string(begin, end));
    }

    return _success;
}

QStringList GraphTransformConfigParser::ops(FieldType fieldType)
{
    QStringList list;

    switch(fieldType)
    {
    case FieldType::Float:
    case FieldType::Int:
        Parser::numerical_op.for_each([&list](auto& v, auto) { list.append(QString::fromStdString(v)); });
        break;

    case FieldType::String:
        Parser::string_op.for_each([&list](auto& v, auto) { list.append(QString::fromStdString(v)); });
        break;

    default: break;
    }

    return list;
}

QString GraphTransformConfigParser::opToString(GraphTransformConfig::NumericalOp op)
{
    QString result;

    Parser::numerical_op.for_each([&](auto& v, auto)
    {
        if(Parser::numerical_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(GraphTransformConfig::StringOp op)
{
    QString result;

    Parser::string_op.for_each([&](auto& v, auto)
    {
        if(Parser::string_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}

QString GraphTransformConfigParser::opToString(GraphTransformConfig::BinaryOp op)
{
    QString result;

    Parser::binary_op.for_each([&](auto& v, auto)
    {
        if(Parser::binary_op.at(v) == op)
            result = QString::fromStdString(v);
    });

    return result;
}
