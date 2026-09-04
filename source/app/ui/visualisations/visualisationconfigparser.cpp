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

#include "visualisationconfigparser.h"

#include "app/configgrammar.h"

#include <lexy_utils.h>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

#include <QByteArray>
#include <QDebug>

#include <vector>

namespace LexyVisualisationParser
{
namespace dsl = lexy::dsl;

using lexyutils::asDouble;
using lexyutils::QuotedString;

using LexyConfigGrammar::AttributeParameters;
using LexyConfigGrammar::Flags;
using LexyConfigGrammar::Name;

constexpr auto keywordWith = LEXY_KEYWORD("with", LexyConfigGrammar::idPattern);

struct Double
{
    static constexpr auto rule = dsl::capture(lexyutils::real);
    static constexpr auto value = asDouble;
};

struct Parameter
{
    static constexpr auto rule = dsl::p<Name> >>
        (dsl::lit_c<'='> + (dsl::p<Double> | dsl::p<QuotedString>));

    static constexpr auto value = lexy::construct<VisualisationConfig::Parameter>;
};

struct Parameters
{
    static constexpr auto rule = dsl::opt(keywordWith >> dsl::list(dsl::p<Parameter>));
    static constexpr auto value = lexy::as_list<std::vector<VisualisationConfig::Parameter>>;
};

// Note this is a token production, so that no whitespace is
// skipped between the attribute name and its parameters
struct AttributeName : lexy::token_production
{
    static constexpr auto rule = dsl::p<Name> >> dsl::p<AttributeParameters>;
    static constexpr auto value = LexyConfigGrammar::asAttributeName;
};

struct Visualisation
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = dsl::p<Flags> + dsl::p<AttributeName> +
        dsl::p<Name> + dsl::p<Parameters> + dsl::eof;

    static constexpr auto value = lexy::construct<VisualisationConfig>;
};
} // namespace LexyVisualisationParser

bool VisualisationConfigParser::parse(const QString& text, bool warnOnFailure)
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

    auto result = lexy::parse<LexyVisualisationParser::Visualisation>(input, onError);
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

QString VisualisationConfigParser::parseForDisplay(const QString& text)
{
    VisualisationConfigParser parser;

    if(parser.parse(text))
        return parser.result().asString(true);

    return {};
}
