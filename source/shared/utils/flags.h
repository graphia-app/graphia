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

#ifndef FLAGS_H
#define FLAGS_H

#include <type_traits>
#include <utility>

template<typename Enum>
class Flags
{
private:
    // The value is stored using the underlying type so that bitwise
    // combinations never result in an out of range enumerator value
    using Underlying = std::underlying_type_t<Enum>;

    Underlying _value = {};

    static constexpr Underlying underlyingValueOf(Enum value)
    {
        return static_cast<Underlying>(value);
    }

public:
    Flags() = default;
    // cppcheck-suppress noExplicitConstructor
    Flags(Enum value) : // NOLINT google-explicit-constructor
        _value(underlyingValueOf(value))
    {}

    template<typename... Tail>
    Flags(Enum value, Tail... values) // NOLINT google-explicit-constructor
    {
        set(value);
        set(values...);
    }

    void set(Enum value)
    {
        _value = static_cast<Underlying>(_value | underlyingValueOf(value));
    }

    template<typename... Tail>
    void set(Enum value, Tail... values)
    {
        set(value);
        set(values...);
    }

    void reset(Enum value)
    {
        _value = static_cast<Underlying>(_value & ~underlyingValueOf(value));
    }

    template<typename... Tail>
    void reset(Enum value, Tail... values)
    {
        reset(value);
        reset(values...);
    }

    void setState(Enum value, bool state)
    {
        if(state)
            set(value);
        else
            reset(value);
    }

    bool test(Enum value) const
    {
        return (_value & underlyingValueOf(value)) != 0;
    }

    bool operator!=(const Flags& other) const
    {
        return _value != other._value;
    }

    bool anyOf(Enum value) const { return test(value); }
    template<typename... Tail>
    bool anyOf(Enum value, Tail... values) const
    {
        return test(value) || anyOf(values...);
    }

    bool allOf(Enum value) const { return test(value); }
    template<typename... Tail>
    bool allOf(Enum value, Tail... values) const
    {
        return allOf(value) && allOf(values...);
    }

    // NOLINTNEXTLINE clang-analyzer-optin.core.EnumCastOutOfRange
    Enum operator*() const { return static_cast<Enum>(_value); }

    //FIXME: Should be able to replace this with a C++17 template deduction constructor
    template<typename... Args>
    static Enum combine(Args... values)
    {
        Flags<Enum> flags;
        flags.set(std::forward<Args>(values)...);
        return *flags;
    }
};

#endif // FLAGS_H
