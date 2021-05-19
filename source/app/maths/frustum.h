/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"
#include "line.h"

#include <QVector3D>

#include <array>

class BaseFrustum
{
public:
    virtual ~BaseFrustum() = default;

    virtual bool containsPoint(const QVector3D& point) const = 0;
    bool containsLine(const Line3D& line) const;

    virtual Line3D centreLine() const = 0;
};

class Frustum : public BaseFrustum
{
private:
    std::array<Plane, 6> _planes;
    Line3D _centreLine;

public:
    Frustum(const Line3D& line1, const Line3D& line2, const Line3D& line3, const Line3D& line4);

    bool containsPoint(const QVector3D& point) const override;
    Line3D centreLine() const override { return _centreLine; }
};

#endif // FRUSTUM_H
