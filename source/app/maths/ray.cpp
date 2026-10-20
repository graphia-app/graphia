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

#include "ray.h"

// Not inline, as a direction parallel to an axis deliberately divides by zero, giving
// infinities, and where the direction is a constant, MSVC warns of it when inlined
void Ray::initialise()
{
    _invDir = QVector3D(1.0f / _dir.x(), 1.0f / _dir.y(), 1.0f/ _dir.z());
    _sign[0] = (_invDir.x() < 0.0f) ? 1 : 0;
    _sign[1] = (_invDir.y() < 0.0f) ? 1 : 0;
    _sign[2] = (_invDir.z() < 0.0f) ? 1 : 0;
}

QVector3D Ray::closestPointTo(const QVector3D& point) const
{
    const float t = QVector3D::dotProduct(point - _origin, _dir);
    return _origin + (_dir * t);
}

QVector3D Ray::closestPointTo(const Ray& other) const
{
    const QVector3D u = _origin - other._origin;
    const QVector3D v = other._dir;
    const QVector3D w = _dir;

    const float a = QVector3D::dotProduct(u, v);
    const float b = QVector3D::dotProduct(v, w);
    const float c = QVector3D::dotProduct(v, v);
    const float d = QVector3D::dotProduct(u, w);
    const float e = QVector3D::dotProduct(w, w);
    const float t = (a * b - c * d) / (c * e - b * b);

    return _origin + (_dir * t);
}

float Ray::distanceTo(const QVector3D& point) const
{
    return (closestPointTo(point) - point).length();
}

float Ray::distanceTo(const Ray& other) const
{
    return (closestPointTo(other) - other.closestPointTo(*this)).length();
}
