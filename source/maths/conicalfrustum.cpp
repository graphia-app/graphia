#include "conicalfrustum.h"

#include "../utils/utils.h"

ConicalFrustum::ConicalFrustum(const Line3D& centreLine, const Line3D& surfaceLine) :
    _centreLine(centreLine)
{
    //FIXME: check direction of normals
    _nearPlane = Plane(_centreLine.start(), -_centreLine.dir());
    _farPlane = Plane(_centreLine.end(), _centreLine.dir());

    _nearRadius = (_centreLine.start() - surfaceLine.start()).length();
    _farRadius = (_centreLine.end() - surfaceLine.end()).length();
}

bool ConicalFrustum::containsPoint(const QVector3D& point) const
{
    if(_nearPlane.sideForPoint(point) == Plane::Side::Front ||
       _farPlane.sideForPoint(point) == Plane::Side::Front)
        return false;

    float height = _centreLine.length();
    float distanceToPoint = _nearPlane.distanceToPoint(point); //FIXME: check sign
    float testRadius = Utils::interpolate(_nearRadius, _farRadius,
                                          height / distanceToPoint);

    Ray ray(_centreLine.start(), _centreLine.end());

    return ray.distanceTo(point) < testRadius;
}

bool ConicalFrustum::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}
