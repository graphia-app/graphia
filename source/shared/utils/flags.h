/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include <utility>

template<typename Enum>
class Flags
{
private:
    Enum _value = static_cast<Enum>(0);

public:
    Flags() = default;
    // cppcheck-suppress noExplicitConstructor
    Flags(Enum value) : // NOLINT
        _value(value)
    {}

    template<typename... Tail>
    Flags(Enum value, Tail... values) // NOLINT
    {
        set(value);
        set(values...);
    }

    void set(Enum value)
    {
        _value = static_cast<Enum>(static_cast<int>(_value) | static_cast<int>(value));
    }

    template<typename... Tail>
    void set(Enum value, Tail... values)
    {
        set(value);
        set(values...);
    }

    void reset(Enum value)
    {
        _value = static_cast<Enum>(static_cast<int>(_value) & ~static_cast<int>(value));
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
        return (static_cast<int>(_value) & static_cast<int>(value)) != 0;
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

    Enum operator*() const { return _value; }

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
