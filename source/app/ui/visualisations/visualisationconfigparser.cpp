/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "visualisationconfigparser.h"

#define BOOST_SPIRIT_X3_UNICODE

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include <QDebug>

BOOST_FUSION_ADAPT_STRUCT(
    VisualisationConfig::Parameter,
    _name,
    _value
)

BOOST_FUSION_ADAPT_STRUCT(
    VisualisationConfig,
    _flags,
    _attributeName,
    _channelName,
    _parameters
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
const auto escapedQuote = lit('\\') >> char_('"');
const auto quotedString_def = lexeme['"' >> *(escapedQuote | ~char_('"')) >> '"'];

const x3::rule<class Identifier, QString> identifier = "identifier";
const auto identifier_def = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];

const x3::rule<class Parameter, VisualisationConfig::Parameter> parameter = "parameter";
const auto parameterName = quotedString | identifier;
const auto parameter_def = parameterName >> lit('=') >> (double_ | quotedString);

const auto identifierList = identifier % lit(',');
const auto flags = lit('[') >> -identifierList >> lit(']');

const x3::rule<class AttributeParameter, QString> attributeParameter = "attributeParameter";
const auto attributeParameter_def = lexeme[char_('.') >> (quotedString | identifier)];

const x3::rule<class AttributeName, QString> attributeName = "attributeName";
const auto attributeName_def = lexeme[(quotedString | identifier) >> *attributeParameter];

const x3::rule<class Visualisation, VisualisationConfig> visualisation = "visualisation";
const auto channelName = quotedString | identifier;
const auto visualisation_def =
    -flags >>
    attributeName >> channelName >>
    -(lit("with") >> +parameter);

BOOST_SPIRIT_DEFINE(quotedString, identifier, attributeParameter, attributeName, visualisation, parameter)
} // namespace SpiritVisualisationParser

bool VisualisationConfigParser::parse(const QString& text, bool warnOnFailure)
{
    QStringSpiritUnicodeConstIterator begin(text.begin());
    const QStringSpiritUnicodeConstIterator end(text.end());
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

QString VisualisationConfigParser::parseForDisplay(const QString& text)
{
    VisualisationConfigParser parser;

    if(parser.parse(text))
        return parser.result().asString(true);

    return {};
}
