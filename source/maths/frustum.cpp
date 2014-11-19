#include "frustum.h"

Frustum::Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4)
{
    _planes[0] = Plane(line1.start(), line4.start(), line2.start());
    _planes[1] = Plane(line1.start(), line2.start(), line1.end());
    _planes[2] = Plane(line2.start(), line3.start(), line2.end());
    _planes[3] = Plane(line3.start(), line4.start(), line3.end());
    _planes[4] = Plane(line4.start(), line1.start(), line4.end());
    _planes[5] = Plane(line2.end(),   line3.end(),   line1.end());
}

bool Frustum::containsPoint(const QVector3D& point) const
{
    //FIXME: optimise this

    for(int i = 0; i < 6; i++)
    {
        if(_planes[i].sideForPoint(point) == Plane::Side::Front)
            return false;
    }

    return true;
}

bool Frustum::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}
