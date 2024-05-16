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

#include "textvisual.h"

#include "app/maths/boundingsphere.h"
#include "app/layout/nodepositions.h"

void TextVisual::updatePositions(const NodePositions& nodePositions)
{
    // This must be called when the layout is paused or otherwise locked
    std::vector<QVector3D> points;
    points.reserve(_nodeIds.size());

    for(const NodeId nodeId : _nodeIds)
    {
        auto nodePosition = nodePositions.get(nodeId);
        points.push_back(nodePosition);
    }

    const BoundingSphere boundingSphere(points);
    _centre = boundingSphere.centre();
    _radius = boundingSphere.radius();
}
