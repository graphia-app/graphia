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

#ifndef LINE_H
#define LINE_H

#include <QVector2D>
#include <QVector3D>

#include <utility>

template <typename Vector> class Line
{
private:
    Vector _start;
    Vector _end;
    mutable float _length = -1.0f;

public:
    Line() = default;
    Line(Vector start, Vector end) :
        _start(std::move(start)), _end(std::move(end))
    {}

    const Vector& start() const { return _start; }
    const Vector& end() const { return _end; }

    Vector dir() const { return (_end - _start).normalized(); }

    float length() const
    {
        // Calculate on demand
        if(_length < 0.0f)
            _length = _start.distanceToPoint(_end);

        return _length;
    }

    void setStart(const Vector& start)
    {
        _start = start;
        _length = -1.0f;
    }

    void setEnd(const Vector& end)
    {
        _end = end;
        _length = -1.0f;
    }
};

using Line3D = Line<QVector3D>;
using Line2D = Line<QVector2D>;

#endif // LINE_H
