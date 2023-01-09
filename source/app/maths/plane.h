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

#ifndef PLANE_H
#define PLANE_H

#include "maths/ray.h"

#include <QVector3D>

class Plane
{
private:
    float _distance = 0.0f;
    QVector3D _normal{0.0f, 0.0f, 1.0f};

public:
    Plane() = default;
    Plane(float distance, const QVector3D& normal) :
        _distance(distance), _normal(normal)
    {}

    Plane(const QVector3D& point, const QVector3D &normal);
    Plane(const QVector3D& pointA, const QVector3D& pointB, const QVector3D& pointC);

    float distance() const { return _distance; }
    const QVector3D& normal() const { return _normal; }

    enum class Side
    {
        Front,
        Back
    };

    Side sideForPoint(const QVector3D& point) const;
    QVector3D rayIntersection(const Ray& ray) const;
    float distanceToPoint(const QVector3D& point) const;
    QVector3D project(const QVector3D& point) const;
};

#endif // PLANE_H
