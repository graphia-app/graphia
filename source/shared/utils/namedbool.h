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

#ifndef NAMEDBOOL_H
#define NAMEDBOOL_H

#include <cstddef>

template<std::size_t N>
struct String
{
    char _data[N] = {0};

    consteval String(const char (&string)[N]) // NOLINT google-explicit-constructor
    {
        char *dst = _data;
        const char *src = string;
        while((*dst++ = *src++) != '\0');
    }
};

template<String string>
class NamedBool
{
private:
    bool _value = false;

public:
    constexpr NamedBool() = delete;
    constexpr explicit NamedBool(bool value) : _value(value) {}
    constexpr NamedBool(const NamedBool& other) = default;
    constexpr NamedBool(NamedBool&& other) = default;
    constexpr NamedBool& operator=(bool value) { _value = value; return *this; }

    constexpr explicit operator bool() const { return _value; }
};

template<String string>
consteval auto operator""_true()
{
    return NamedBool<string>(true);
}

template<String string>
consteval auto operator""_false()
{
    return NamedBool<string>(false);
}

template<String string>
consteval auto operator""_yes()
{
    return NamedBool<string>(true);
}

template<String string>
consteval auto operator""_no()
{
    return NamedBool<string>(false);
}

#endif // NAMEDBOOL_H
