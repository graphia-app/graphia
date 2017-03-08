#include "visualisationconfigparser.h"

#include "thirdparty/boost/boost_disable_warnings.h"
#include "boost/spirit/home/x3.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "thirdparty/boost/boost_spirit_qstring_adapter.h"

BOOST_FUSION_ADAPT_STRUCT(
    VisualisationConfig,
    (std::vector<QString>, _flags),
    (QString, _dataFieldName),
    (QString, _channelName)
)

namespace Parser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
using x3::string;
using x3::lexeme;
using ascii::char_;

const x3::rule<class QuotedString, QString> quotedString = "quotedString";
const auto quotedString_def = lexeme['"' >> +(char_ - '"') >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier";
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const auto identifierList = identifier % x3::lit(",");
const auto flags = x3::lit("[") >> -identifierList >> x3::lit("]");

const x3::rule<class Visualisation, VisualisationConfig> visualisation = "visualisation";
const auto dataFieldName = quotedString | identifier;
const auto channelName = quotedString | identifier;
const auto visualisation_def =
    -flags >>
    dataFieldName >> channelName;

BOOST_SPIRIT_DEFINE(quotedString, identifier, visualisation);
} // namespace Parser

bool VisualisationConfigParser::parse(const QString& text)
{
    auto stdString = text.toStdString();
    auto begin = stdString.begin();
    auto end = stdString.end();
    _result = {};
    _success = Parser::x3::phrase_parse(begin, end, Parser::visualisation,
                                        Parser::ascii::space, _result);

    if(begin != end)
    {
        _success = false;
        _failedInput = QString::fromStdString(std::string(begin, end));
    }

    return _success;
}
