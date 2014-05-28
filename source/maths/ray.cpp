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
