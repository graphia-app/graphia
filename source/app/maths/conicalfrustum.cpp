#include "conicalfrustum.h"

#include "shared/utils/utils.h"

ConicalFrustum::ConicalFrustum(const Line3D& centreLine, const Line3D& surfaceLine) :
    _centreLine(centreLine)
{
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

    float distanceBetweenPlanes = _centreLine.length();
    float distanceFromNearPlane = _nearPlane.distanceToPoint(point);
    float testRadius = u::interpolate(_nearRadius, _farRadius,
                                      distanceFromNearPlane / distanceBetweenPlanes);

    float distanceToCentreLine = point.distanceToLine(_centreLine.start(),
        (_centreLine.end() - _centreLine.start()).normalized());

    return distanceToCentreLine < testRadius;
}
