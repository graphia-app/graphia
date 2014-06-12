#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include <QVector3D>

#include "line.h"
#include "constants.h"

#include <vector>

class Ray;

class BoundingSphere
{
private:
    QVector3D _centre;
    float _radius;

public:
    BoundingSphere();
    BoundingSphere(const QVector3D centre, float radius);
    BoundingSphere(const std::vector<QVector3D>& points);

    const QVector3D& centre() const { return _centre; }
    float radius() const { return _radius; }

    void scale(float s);
    BoundingSphere scaled(float s) const;

    float volume() const { return (4.0f * Constants::Pi() * _radius * _radius * _radius) / 3.0f; }

    void set(const QVector3D centre, float radius);
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const BoundingSphere& other);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const Line3D& line) const;
    bool containsSphere(const BoundingSphere& other) const;
    std::vector<QVector3D> rayIntersection(const Ray& ray) const;
};

#endif // BOUNDINGSPHERE_H
