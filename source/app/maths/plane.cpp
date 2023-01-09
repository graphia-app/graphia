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

#include "plane.h"

#include <utility>

Plane::Plane(const QVector3D& point, const QVector3D& normal) :
    _normal(normal)
{
    const float negDistance =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z();

    _distance = -negDistance;
}

Plane::Plane(const QVector3D& pointA, const QVector3D& pointB, const QVector3D& pointC)
{
    const QVector3D a = pointB - pointA;
    const QVector3D b = pointC - pointA;

    _normal = QVector3D::crossProduct(a, b).normalized();
    _distance = -QVector3D::dotProduct(_normal, pointA);
}

Plane::Side Plane::sideForPoint(const QVector3D& point) const
{
    const float result =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z() + _distance;

    if(result >= 0.0f)
        return Plane::Side::Front;

    return Plane::Side::Back;
}

QVector3D Plane::rayIntersection(const Ray& ray) const
{
    const float o_n = QVector3D::dotProduct(ray.origin(), _normal);
    const float d_n = QVector3D::dotProduct(ray.dir(), _normal);
    const float t = -(o_n + _distance) / d_n;

    return ray.origin() + (t * ray.dir());
}

float Plane::distanceToPoint(const QVector3D& point) const
{
    const float n =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z() + _distance;

    return -n / _normal.length();
}

QVector3D Plane::project(const QVector3D& point) const
{
    auto dot = QVector3D::dotProduct(point, _normal);
    return point - (dot + _distance) * _normal;
}
