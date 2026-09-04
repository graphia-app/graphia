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

#ifndef CONFIGGRAMMAR_H
#define CONFIGGRAMMAR_H

#include <lexy_utils.h>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/grammar.hpp>

#include <QString>

#include <utility>
#include <vector>

// Grammar common to the transform and visualisation configuration languages
namespace LexyConfigGrammar
{
namespace dsl = lexy::dsl;

constexpr auto idPattern =
    dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);

struct Identifier : lexy::token_production
{
    static constexpr auto rule = idPattern;
    static constexpr auto value = lexyutils::asQString;
};

struct Name
{
    static constexpr auto rule = dsl::p<lexyutils::QuotedString> | dsl::p<Identifier>;
    static constexpr auto value = lexy::forward<QString>;
};

struct AttributeParameter
{
    static constexpr auto rule = dsl::lit_c<'.'> >> dsl::p<Name>;
    static constexpr auto value = lexy::callback<QString>(
        [](const QString& name) { return "." + name; });
};

struct AttributeParameters
{
    static constexpr auto rule = dsl::opt(dsl::list(dsl::p<AttributeParameter>));
    static constexpr auto value = lexy::as_list<std::vector<QString>>;
};

inline constexpr auto asAttributeName = lexy::callback<QString>(
    [](QString&& name, const std::vector<QString>& parameters)
    {
        for(const auto& parameter : parameters)
            name += parameter;

        return std::move(name);
    });

struct Flags
{
    static constexpr auto rule = dsl::opt(dsl::lit_c<'['> >>
        (dsl::opt(dsl::list(dsl::p<Identifier>, dsl::sep(dsl::lit_c<','>))) + dsl::lit_c<']'>));

    static constexpr auto value = lexy::as_list<std::vector<QString>>;
};

} // namespace LexyConfigGrammar

#endif // CONFIGGRAMMAR_H
