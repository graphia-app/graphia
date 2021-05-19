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

#include "ray.h"

QVector3D Ray::closestPointTo(const QVector3D& point) const
{
    float t = QVector3D::dotProduct(point - _origin, _dir);
    return _origin + (_dir * t);
}

QVector3D Ray::closestPointTo(const Ray& other) const
{
    QVector3D u = _origin - other._origin;
    QVector3D v = other._dir;
    QVector3D w = _dir;

    float a = QVector3D::dotProduct(u, v);
    float b = QVector3D::dotProduct(v, w);
    float c = QVector3D::dotProduct(v, v);
    float d = QVector3D::dotProduct(u, w);
    float e = QVector3D::dotProduct(w, w);
    float t = (a * b - c * d) / (c * e - b * b);

    return _origin + (_dir * t);
}

float Ray::distanceTo(const QVector3D& point) const
{
    return closestPointTo(point).length();
}

float Ray::distanceTo(const Ray& other) const
{
    return closestPointTo(other).length();
}
