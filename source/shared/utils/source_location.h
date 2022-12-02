/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#if defined(__clang__) || !__has_include(<source_location>)

#include <cstdint>

struct source_location
{
    std::uint_least32_t _line;
    std::uint_least32_t _column;
    const char* _file_name;
    const char* _function_name;

    constexpr std::uint_least32_t line() const noexcept { return _line; }
    constexpr std::uint_least32_t column() const noexcept { return _column; }
    constexpr const char* file_name() const noexcept { return _file_name; }
    constexpr const char* function_name() const noexcept { return _function_name; }
};

#define CURRENT_SOURCE_LOCATION source_location{__LINE__, 0, __FILE__, __PRETTY_FUNCTION__}

#else

#include <source_location>

struct source_location : std::source_location {};

#define CURRENT_SOURCE_LOCATION static_cast<source_location>(source_location::current())

#endif

#endif // SOURCE_LOCATION_H
