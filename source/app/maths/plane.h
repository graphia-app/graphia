#ifndef PLANE_H
#define PLANE_H

#include "maths/ray.h"

#include <QVector3D>

class Plane
{
private:
    float _distance = 0.0f;
    QVector3D _normal;

public:
    Plane() : _normal(0.0f, 0.0f, 1.0f) {}

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
