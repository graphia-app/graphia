/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <algorithm>
#include <array>

#include <QtGlobal>

// Test if there is an operator* for T and Arg
namespace Test
{
    struct NotFound {};
    template<typename T, typename Arg> NotFound operator*(const T&, const Arg&);

    template<typename T, typename Arg = T>
    struct MultiplyExists
    {
        enum { value = !std::is_same_v<decltype(std::declval<T>() * std::declval<Arg>()), NotFound> };
    };
} // namespace Test

template<typename T, size_t Size> class CircularBuffer
{
private:
    std::array<T, Size> _array{};
    size_t _size = 0;
    size_t _current = 0;
    size_t _next = 0;

    size_t indexToPosition(size_t index) const
    {
        Q_ASSERT(_size > 0);
        auto base = _current - _size + 1;
        return (base + Size + index) % Size;
    }

public:
    void push_back(const T& t)
    {
        _array.at(_next) = t;
        _current = _next;
        _next++;
        _size = std::max(_size, _next);
        _next = _next % Size;
    }

    void fill(const T& t)
    {
        _array.fill(t);
        _size = Size;
        _current = 0;
        _next = 1;
    }

    // An index of 0 will get the oldest T
    // An index of size() - 1 will get the newest T
    const T& at(size_t index) const
    {
        return _array.at(indexToPosition(index));
    }

    T& at(size_t index)
    {
        return _array.at(indexToPosition(index));
    }

    const T& newest() const
    {
        Q_ASSERT(_size > 0);
        return at(_size - 1);
    }

    const T& oldest() const
    {
        Q_ASSERT(_size > 0);
        return at(0);
    }

    T mean(size_t samples) const
    {
        samples = std::min(samples, _size);

        T result = T();

        using Reciprocal = std::conditional_t<Test::MultiplyExists<T, float>::value, float, double>;
        auto reciprocal = static_cast<Reciprocal>(1.0 / static_cast<double>(samples));

        for(auto i = _size - samples; i < _size; i++)
            result += at(i) * reciprocal;

        return result;
    }

    T mean() const { return mean(_size); }

    size_t size() const { return _size; }

    bool full() const { return _size >= Size; }

    void clear()
    {
        _size = 0;
        _next = 0;
        _current = 0;
    }
};

#endif // CIRCULARBUFFER_H
