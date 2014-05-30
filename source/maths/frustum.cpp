#include "frustum.h"

Frustum::Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4)
{
    planes[0] = Plane(line1.start(), line4.start(), line2.start());
    planes[1] = Plane(line1.start(), line2.start(), line1.end());
    planes[2] = Plane(line2.start(), line3.start(), line2.end());
    planes[3] = Plane(line3.start(), line4.start(), line3.end());
    planes[4] = Plane(line4.start(), line1.start(), line4.end());
    planes[5] = Plane(line2.end(),   line3.end(),   line1.end());
}

bool Frustum::containsPoint(const QVector3D& point) const
{
    //FIXME: optimise this

    for(int i = 0; i < 6; i++)
    {
        if(planes[i].sideForPoint(point) == Plane::Side::Front)
            return false;
    }

    return true;
}

bool Frustum::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}
