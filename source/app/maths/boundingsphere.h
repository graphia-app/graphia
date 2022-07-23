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

#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include <QVector3D>

#include "line.h"
#include "shared/utils/constants.h"

#include <vector>

class Ray;

class BoundingSphere
{
private:
    QVector3D _centre;
    float _radius = 0.0f;

public:
    BoundingSphere() = default;
    BoundingSphere(QVector3D centre, float radius);
    explicit BoundingSphere(const std::vector<QVector3D>& points);
    BoundingSphere(QVector3D centre, const std::vector<QVector3D>& points);

    const QVector3D& centre() const { return _centre; }
    float radius() const { return _radius; }

    void scale(float s);
    Q_REQUIRED_RESULT BoundingSphere scaled(float s) const;

    float volume() const { return (4.0f * Constants::Pi() * _radius * _radius * _radius) / 3.0f; }

    void set(QVector3D centre, float radius);
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const BoundingSphere& other);

    bool containsPoint(const QVector3D& point) const;
    bool containsLine(const Line3D& line) const;
    bool containsSphere(const BoundingSphere& other) const;
    std::vector<QVector3D> rayIntersection(const Ray& ray) const;
};

#endif // BOUNDINGSPHERE_H
