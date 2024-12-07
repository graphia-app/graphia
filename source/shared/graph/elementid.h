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

#ifndef ELEMENTID_H
#define ELEMENTID_H

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <compare>

template<typename T> class ElementId
{
private:
    static const int NullValue = -1;
    int _value;

public:
    // cppcheck-suppress noExplicitConstructor
    ElementId(int value = NullValue) : // NOLINT google-explicit-constructor
        _value(value)
    {
        static_assert(sizeof(ElementId) == sizeof(_value), "ElementId should not be larger than an int");
    }

    template<typename Integer> using EnableIfConvertibleToInt = typename std::enable_if_t<
        std::is_integral_v<Integer> && std::is_convertible_v<Integer, int>>;

    // Prevent warnings when initialising an ElementId with other integral types
    template<typename U, typename = EnableIfConvertibleToInt<U>>
    explicit ElementId(U value) : ElementId(static_cast<int>(value)) {}

    explicit operator int() const { return _value; }
    explicit operator size_t() const { assert(_value >= 0); return static_cast<size_t>(_value); }

    ElementId(const ElementId<T>& other) = default;
    ElementId(ElementId<T>&& other) noexcept = default;
    ElementId& operator=(const ElementId<T>& other) = default;
    ElementId& operator=(ElementId<T>&& other) noexcept = default;

    T& operator++() { ++_value; return static_cast<T&>(*this); }
    T operator++(int) { T previous = static_cast<T&>(*this); ++_value; return previous; }
    T& operator--() { --_value; return static_cast<T&>(*this); }
    T operator--(int) { T previous = static_cast<T&>(*this); --_value; return previous; }
    T operator+(int value) const { return _value + value; }
    T operator-(int value) const { return _value - value; }
    auto operator<=>(const ElementId<T>& other) const = default;

    bool isNull() const { assert(_value >= NullValue); return _value == NullValue; }
    void setToNull() { _value = NullValue; }
};

class NodeId :      public ElementId<NodeId>      { using ElementId::ElementId; };
class EdgeId :      public ElementId<EdgeId>      { using ElementId::ElementId; };
class ComponentId : public ElementId<ComponentId> { using ElementId::ElementId; };

#endif // ELEMENTID_H
