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

#include "boundingsphere.h"

#include "ray.h"

#include <cmath>
#include <vector>
#include <numeric>

BoundingSphere::BoundingSphere(QVector3D centre, float radius) :
    _centre(centre), _radius(radius)
{
}

static float maxDistanceFromCentre(const QVector3D& centre, const std::vector<QVector3D>& points)
{
    float radius = 0.0f;

    for(const auto& point : points)
    {
        const float lengthSquared = (centre - point).lengthSquared();
        if(lengthSquared > radius)
            radius = lengthSquared;
    }

    return std::sqrt(radius);
}

BoundingSphere::BoundingSphere(const std::vector<QVector3D>& points) :
    // This is only an approximation; Welzl's algorithm will get a better result
    // Find barycentre
    _centre(std::accumulate(points.begin(), points.end(), QVector3D(),
        [](const auto& centre, const auto& point) { return centre + point; }) /
        static_cast<float>(points.size())),
    _radius(maxDistanceFromCentre(_centre, points))
{
}

BoundingSphere::BoundingSphere(QVector3D centre, const std::vector<QVector3D>& points) :
    _centre(centre), _radius(maxDistanceFromCentre(_centre, points))
{}

void BoundingSphere::scale(float s)
{
    _radius *= s;
}

BoundingSphere BoundingSphere::scaled(float s) const
{
    return {_centre, _radius * s};
}

void BoundingSphere::set(QVector3D centre, float radius)
{
    _centre = centre;
    _radius = radius;
}

void BoundingSphere::expandToInclude(const QVector3D& point)
{
    const float d = point.distanceToPoint(_centre);
    _radius = std::max(d, _radius);
}

void BoundingSphere::expandToInclude(const BoundingSphere& other)
{
    const float d = other.centre().distanceToPoint(_centre) + other.radius();
    _radius = std::max(d, _radius);
}

bool BoundingSphere::containsPoint(const QVector3D& point) const
{
    const float d = point.distanceToPoint(_centre);
    return d <= _radius;
}

bool BoundingSphere::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}

bool BoundingSphere::containsSphere(const BoundingSphere& other) const
{
    const float d = other.centre().distanceToPoint(_centre) + other.radius();
    return d <= _radius;
}

std::vector<QVector3D> BoundingSphere::rayIntersection(const Ray& ray) const
{
    std::vector<QVector3D> result;
    const QVector3D& origin = ray.origin();
    const QVector3D& dir = ray.dir();
    const QVector3D oc = _centre - origin;

    if(QVector3D::dotProduct(oc, dir) < 0.0f)
    {
        // Ray starts behind sphere
        const float ocLength = oc.length();

        if(ocLength == _radius)
            result.push_back(origin);
        else if(ocLength < _radius)
        {
            const QVector3D centreProjectedOnRay = ray.closestPointTo(_centre);
            const float rayDistFromCentre = (centreProjectedOnRay - _centre).length();
            const float d = std::sqrt((_radius * _radius) - (rayDistFromCentre * rayDistFromCentre));
            const float di1 = d - (centreProjectedOnRay - origin).length();
            result.push_back(origin + (dir * di1));
        }
    }
    else
    {
        // Ray starts in front of sphere
        const QVector3D centreProjectedOnRay = ray.closestPointTo(_centre);
        const float rayDistFromCentre = (centreProjectedOnRay - _centre).length();

        if(rayDistFromCentre <= _radius)
        {
            const float d = std::sqrt((_radius * _radius) - (rayDistFromCentre * rayDistFromCentre));
            const float di1 = (centreProjectedOnRay - origin).length() - d;
            const float di2 = (centreProjectedOnRay - origin).length() + d;
            const float ocLength = oc.length();

            if(ocLength >= _radius)
            {
                result.push_back(origin + (dir * di1));
                result.push_back(origin + (dir * di2));
            }
            else
                result.push_back(origin + (dir * di2));
        }
    }

    return result;
}
