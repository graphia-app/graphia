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

#ifndef FIXEDSIZESTACK_H
#define FIXEDSIZESTACK_H

#include <array>
#include <cstdlib>

template<typename T> class FixedSizeStack
{
private:
    std::vector<T> _vector;
    size_t _size;
    int _top = -1;

public:
    explicit FixedSizeStack(size_t size) :
        _vector(size),
        _size(size)
    {}

    void push(const T& t)
    {
        if(_top + 1 < static_cast<int>(_size))
            _vector[static_cast<size_t>(++_top)] = t;
        else
            std::abort(); // Top of stack breached
    }

    T& top() { return _vector.at(static_cast<size_t>(_top)); }
    const T& top() const { return _vector.at(static_cast<size_t>(_top)); }

    T pop() { assert(_top >= 0); return _vector.at(static_cast<size_t>(_top--)); }

    size_t size() const { return _size; }
    bool empty() const { return _top < 0; }
};

#endif // FIXEDSIZESTACK_H
