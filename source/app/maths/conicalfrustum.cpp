/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    const float distanceBetweenPlanes = _centreLine.length();
    const float distanceFromNearPlane = _nearPlane.distanceToPoint(point);
    const float testRadius = u::interpolate(_nearRadius, _farRadius,
                                      distanceFromNearPlane / distanceBetweenPlanes);

    const float distanceToCentreLine = point.distanceToLine(_centreLine.start(),
        (_centreLine.end() - _centreLine.start()).normalized());

    return distanceToCentreLine < testRadius;
}
