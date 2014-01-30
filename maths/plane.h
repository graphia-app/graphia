#ifndef PLANE_H
#define PLANE_H

#include <QVector3D>

#include "../maths/ray.h"

class Plane
{
private:
    float _distance;
    QVector3D _normal;

public:
    Plane(float distance, const QVector3D& normal) :
        _distance(distance), _normal(normal)
    {}

    Plane(const QVector3D& point, const QVector3D& normal);

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
};

#endif // PLANE_H
