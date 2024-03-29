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

#include "frustum.h"

#include <algorithm>

Frustum::Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4)
{
    _planes[0] = Plane(line1.start(), line4.start(), line2.start());
    _planes[1] = Plane(line1.start(), line2.start(), line1.end());
    _planes[2] = Plane(line2.start(), line3.start(), line2.end());
    _planes[3] = Plane(line3.start(), line4.start(), line3.end());
    _planes[4] = Plane(line4.start(), line1.start(), line4.end());
    _planes[5] = Plane(line2.end(),   line3.end(),   line1.end());

    _centreLine = Line3D(0.5f * (line1.start() + line3.start()),
                         0.5f * (line1.end() + line3.end()));
}

bool Frustum::containsPoint(const QVector3D& point) const
{
    return std::all_of(_planes.begin(), _planes.end(), [&point](const auto& plane)
    {
        return plane.sideForPoint(point) == Plane::Side::Back;
    });
}

bool BaseFrustum::containsLine(const Line3D& line) const
{
    return containsPoint(line.start()) && containsPoint(line.end());
}
