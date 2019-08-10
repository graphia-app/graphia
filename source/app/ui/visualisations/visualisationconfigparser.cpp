#include "visualisationconfigparser.h"

#define BOOST_SPIRIT_X3_UNICODE

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include <QDebug>

BOOST_FUSION_ADAPT_STRUCT(
    VisualisationConfig::Parameter,
    (QString, _name),
    (VisualisationConfig::ParameterValue, _value)
)

BOOST_FUSION_ADAPT_STRUCT(
    VisualisationConfig,
    (std::vector<QString>, _flags),
    (QString, _attributeName),
    (QString, _channelName),
    (std::vector<VisualisationConfig::Parameter>, _parameters)
)

namespace SpiritVisualisationParser
{
namespace x3 = boost::spirit::x3;
namespace unicode = boost::spirit::x3::unicode;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::lexeme;
using unicode::char_;

const x3::rule<class QuotedString, QString> quotedString = "quotedString";
const auto escapedQuote = x3::lit('\\') >> char_('"');
const auto quotedString_def = lexeme['"' >> *(escapedQuote | ~char_('"')) >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier";
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const x3::rule<class Parameter, VisualisationConfig::Parameter> parameter = "parameter";
const auto parameterName = quotedString | identifier;
const auto parameter_def = parameterName >> x3::lit('=') >> (double_ | quotedString);

const auto identifierList = identifier % x3::lit(',');
const auto flags = x3::lit('[') >> -identifierList >> x3::lit(']');

const x3::rule<class AttributeParameter, QString> attributeParameter = "attributeParameter";
const auto attributeParameter_def = lexeme[char_('.') >> (quotedString | identifier)];

const x3::rule<class AttributeName, QString> attributeName = "attributeName";
const auto attributeName_def = lexeme[(quotedString | identifier) >> -attributeParameter];

const x3::rule<class Visualisation, VisualisationConfig> visualisation = "visualisation";
const auto channelName = quotedString | identifier;
const auto visualisation_def =
    -flags >>
    attributeName >> channelName >>
    -(x3::lit("with") >> +parameter);

BOOST_SPIRIT_DEFINE(quotedString, identifier, attributeParameter, attributeName, visualisation, parameter)
} // namespace SpiritVisualisationParser

bool VisualisationConfigParser::parse(const QString& text, bool warnOnFailure)
{
    QStringSpiritUnicodeConstIterator begin(text.begin());
    QStringSpiritUnicodeConstIterator end(text.end());
    _result = {};
    _success = SpiritVisualisationParser::x3::phrase_parse(begin, end,
                    SpiritVisualisationParser::visualisation,
                    SpiritVisualisationParser::unicode::space, _result);

    if(begin != end)
    {
        _success = false;
        _failedInput = QString::fromStdString(std::string(begin, end));

        if(warnOnFailure)
            qWarning() << "Failed to parse" << _failedInput;
    }

    return _success;
}
