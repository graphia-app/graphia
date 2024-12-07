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

#ifndef RAY_H
#define RAY_H

#include <QVector3D>

#include "line.h"

#include <array>

class Ray
{
private:
    QVector3D _origin;
    QVector3D _dir;
    QVector3D _invDir;
    std::array<int, 3> _sign{{0, 0, 0}};

    void initialise()
    {
        _invDir = QVector3D(1.0f / _dir.x(), 1.0f / _dir.y(), 1.0f/ _dir.z());
        _sign[0] = (_invDir.x() < 0.0f) ? 1 : 0;
        _sign[1] = (_invDir.y() < 0.0f) ? 1 : 0;
        _sign[2] = (_invDir.z() < 0.0f) ? 1 : 0;
    }

public:
    Ray(const QVector3D& origin, const QVector3D& dir) :
        _origin(origin), _dir(dir)
    {
        initialise();
    }

    explicit Ray(const Line3D& line) :
        _origin(line.start()), _dir(line.dir())
    {
        initialise();
    }

    const QVector3D& origin() const { return _origin; }
    const QVector3D& dir() const { return _dir; }
    const QVector3D& invDir() const { return _invDir; }
    auto sign() const { return _sign; }

    QVector3D closestPointTo(const QVector3D& point) const;
    QVector3D closestPointTo(const Ray& other) const;

    float distanceTo(const QVector3D& point) const;
    float distanceTo(const Ray& other) const;
};

#endif // RAY_H
