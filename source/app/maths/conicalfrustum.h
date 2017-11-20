#ifndef CONICALFRUSTUM_H
#define CONICALFRUSTUM_H

#include "frustum.h"
#include "plane.h"
#include "line.h"

#include <QVector3D>

class ConicalFrustum : public BaseFrustum
{
private:
    Line3D _centreLine;

    Plane _nearPlane;
    float _nearRadius;

    Plane _farPlane;
    float _farRadius;

public:
    ConicalFrustum(const Line3D &centreLine, const Line3D& surfaceLine);

    bool containsPoint(const QVector3D& point) const override;
    Line3D centreLine() const override { return _centreLine; }
};

#endif // CONICALFRUSTUM_H
