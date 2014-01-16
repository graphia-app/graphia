#include "plane.h"

Plane::Plane(const QVector3D& point, const QVector3D& normal) :
    _normal(normal)
{
    float negDistance =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z();

    _distance = -negDistance;
}

Plane::Side Plane::sideForPoint(const QVector3D& point)
{
    float result =
            _normal.x() * point.x() +
            _normal.y() * point.y() +
            _normal.z() * point.z() + distance();

    if (result >= 0.0f)
        return Plane::Side::Front;

    return Plane::Side::Back;
}
