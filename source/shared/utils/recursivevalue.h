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

#ifndef RECURSIVEVALUE_H
#define RECURSIVEVALUE_H

#include <memory>
#include <utility>

// A value semantics wrapper that adds a layer of indirection, thereby allowing
// a type to (indirectly) contain itself; std::variant requires that its
// alternatives are complete types, so a directly recursive one is not possible
template<typename T>
class RecursiveValue
{
private:
    std::unique_ptr<T> _value;

public:
    RecursiveValue() : _value(std::make_unique<T>()) {}

    // cppcheck-suppress noExplicitConstructor
    RecursiveValue(const T& value) : // NOLINT google-explicit-constructor
        _value(std::make_unique<T>(value)) {}

    // cppcheck-suppress noExplicitConstructor
    RecursiveValue(T&& value) : // NOLINT google-explicit-constructor
        _value(std::make_unique<T>(std::move(value))) {}

    RecursiveValue(const RecursiveValue& other) :
        _value(std::make_unique<T>(*other._value)) {}

    RecursiveValue(RecursiveValue&& other) noexcept = default;

    RecursiveValue& operator=(const RecursiveValue& other)
    {
        if(this != &other)
            _value = std::make_unique<T>(*other._value);

        return *this;
    }

    RecursiveValue& operator=(RecursiveValue&& other) noexcept = default;

    ~RecursiveValue() = default;

    T& get() { return *_value; }
    const T& get() const { return *_value; }

    T& operator*() { return *_value; }
    const T& operator*() const { return *_value; }

    T* operator->() { return _value.get(); }
    const T* operator->() const { return _value.get(); }

    bool operator==(const RecursiveValue& other) const { return *_value == *other._value; }
};

#endif // RECURSIVEVALUE_H
