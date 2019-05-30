#include "plane.h"

#include <utility>

Plane::Plane(const QVector3D& point, const QVector3D& normal) :
    _normal(normal)
{
    float negDistance =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z();

    _distance = -negDistance;
}

Plane::Plane(const QVector3D& pointA, const QVector3D& pointB, const QVector3D& pointC)
{
    const QVector3D a = pointB - pointA;
    const QVector3D b = pointC - pointA;

    // cppcheck-suppress useInitializationList
    _normal = QVector3D::crossProduct(a, b).normalized();
    _distance = -QVector3D::dotProduct(_normal, pointA);
}

Plane::Side Plane::sideForPoint(const QVector3D& point) const
{
    float result =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z() + distance();

    if(result >= 0.0f)
        return Plane::Side::Front;

    return Plane::Side::Back;
}

QVector3D Plane::rayIntersection(const Ray& ray) const
{
    float o_n = QVector3D::dotProduct(ray.origin(), _normal);
    float d_n = QVector3D::dotProduct(ray.dir(), _normal);
    float t = -(o_n + _distance) / d_n;

    return ray.origin() + (t * ray.dir());
}

float Plane::distanceToPoint(const QVector3D& point) const
{
    float n =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z() + distance();

    return -n / _normal.length();
}
