#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"
#include "line.h"

#include <QVector3D>

class Frustum
{
private:
    Plane _planes[6];

public:
    Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const Line3D& line) const;
};

#endif // FRUSTUM_H
