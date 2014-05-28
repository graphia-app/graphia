#include "boundingsphere.h"

#include "ray.h"

#include <QList>
#include <cmath>

BoundingSphere::BoundingSphere() :
    _centre(), _radius(0.0f)
{
}

BoundingSphere::BoundingSphere(const QVector3D centre, float radius) :
    _centre(centre), _radius(radius)
{
}

void BoundingSphere::scale(float s)
{
    _radius *= s;
}

BoundingSphere BoundingSphere::scaled(float s) const
{
    return BoundingSphere(_centre, _radius * s);
}

void BoundingSphere::set(const QVector3D centre, float radius)
{
    _centre = centre;
    _radius = radius;
}

void BoundingSphere::expandToInclude(const QVector3D& point)
{
    float d = point.distanceToPoint(_centre);
    _radius = std::max(d, _radius);
}

void BoundingSphere::expandToInclude(const BoundingSphere& other)
{
    float d = other.centre().distanceToPoint(_centre) + other.radius();
    _radius = std::max(d, _radius);
}

bool BoundingSphere::containsPoint(const QVector3D& point) const
{
    float d = point.distanceToPoint(_centre);
    return d <= _radius;
}

bool BoundingSphere::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}

bool BoundingSphere::containsSphere(const BoundingSphere& other) const
{
    float d = other.centre().distanceToPoint(_centre) + other.radius();
    return d <= _radius;
}

QList<QVector3D> BoundingSphere::rayIntersection(const Ray& ray) const
{
    QList<QVector3D> result;
    const QVector3D& origin = ray.origin();
    const QVector3D& dir = ray.dir();
    QVector3D oc = _centre - origin;

    if(QVector3D::dotProduct(oc, dir) < 0.0f)
    {
        // Ray starts behind sphere
        float ocLength = oc.length();

        if(ocLength == _radius)
            result.append(origin);
        else if(ocLength < _radius)
        {
            QVector3D centreProjectedOnRay = ray.closestPointTo(_centre);
            float rayDistFromCentre = (centreProjectedOnRay - _centre).length();
            float d = std::sqrt((_radius * _radius) - (rayDistFromCentre * rayDistFromCentre));
            float di1 = d - (centreProjectedOnRay - origin).length();
            result.append(origin + (dir * di1));
        }
    }
    else
    {
        // Ray starts in front of sphere
        QVector3D centreProjectedOnRay = ray.closestPointTo(_centre);
        float rayDistFromCentre = (centreProjectedOnRay - _centre).length();

        if(rayDistFromCentre <= _radius)
        {
            float d = std::sqrt((_radius * _radius) - (rayDistFromCentre * rayDistFromCentre));
            float di1 = (centreProjectedOnRay - origin).length() - d;
            float di2 = (centreProjectedOnRay - origin).length() + d;
            float ocLength = oc.length();

            if(ocLength >= _radius)
            {
                result.append(origin + (dir * di1));
                result.append(origin + (dir * di2));
            }
            else
                result.append(origin + (dir * di2));
        }
    }

    return result;
}
