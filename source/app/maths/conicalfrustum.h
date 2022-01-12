/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
