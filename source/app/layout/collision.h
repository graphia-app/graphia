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

#ifndef COLLISION_H
#define COLLISION_H

#include "shared/graph/elementid.h"
#include "layout.h"

#include <QVector3D>

#include <vector>
#include <memory>

class GraphModel;

class Collision
{
private:
    const GraphModel* _graphModel = nullptr;
    ComponentId _componentId;
    QVector3D _offset;
    bool _includeNotFound = false;

public:
    Collision(const GraphModel& graphModel, ComponentId componentId, bool includeNotFound = false) :
        _graphModel(&graphModel),
        _componentId(componentId),
        _offset(0.0f, 0.0f, 0.0f),
        _includeNotFound(includeNotFound)
    {}

    void setOffset(QVector3D offset) { _offset = offset; }

    NodeId nodeClosestToLine(const std::vector<NodeId>& nodeIds, const QVector3D& point, const QVector3D& direction);
    NodeId nodeClosestToLine(const QVector3D& point, const QVector3D& direction);

    void nodesIntersectingLine(const QVector3D& point, const QVector3D& direction, std::vector<NodeId>& intersectingNodeIds);
    void nodesInsideCylinder(const QVector3D& point, const QVector3D& direction,
                             float radius, std::vector<NodeId>& containedNodeIds);

    NodeId nearestNodeIntersectingLine(const QVector3D& point, const QVector3D& direction);
    NodeId nearestNodeInsideCylinder(const QVector3D& point, const QVector3D& direction, float radius);
};

#endif // COLLISION_H
