#ifndef CONICALFRUSTUM_H
#define CONICALFRUSTUM_H

#include "plane.h"
#include "line.h"

#include <QVector3D>

class ConicalFrustum
{
private:
    Line3D _centreLine;

    Plane _nearPlane;
    float _nearRadius;

    Plane _farPlane;
    float _farRadius;

public:
    ConicalFrustum(const Line3D& centreLine, const Line3D& surfaceLine);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const Line3D& line) const;
};

#endif // CONICALFRUSTUM_H
