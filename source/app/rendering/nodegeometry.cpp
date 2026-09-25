/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "nodegeometry.h"

#include "app/layout/nodepositions.h"

void NodeGeometry::setPositions(const std::vector<NodeId>& nodeIds, const NodePositions& nodePositions)
{
    _geometry.resize(nodePositions.size() * 4);

    if(!nodeIds.empty())
    {
        const auto position = nodePositions.get(nodeIds.front());
        _bounds.set(position, position);
    }

    for(auto nodeId : nodeIds)
    {
        const auto position = nodePositions.get(nodeId);
        auto* texel = texelFor(nodeId);

        texel[0] = position.x();
        texel[1] = position.y();
        texel[2] = position.z();

        _bounds.expandToInclude(position);
    }
}

void NodeGeometry::setSize(NodeId nodeId, float size)
{
    texelFor(nodeId)[3] = size;
}

void NodeGeometry::copyFrom(const NodeGeometry& other)
{
    _geometry = other._geometry;
    _bounds = other._bounds;
}

void NodeGeometry::upload()
{
    _texels.upload(_geometry);
}

GLfloat* NodeGeometry::texelFor(NodeId nodeId)
{
    return &_geometry.at(static_cast<size_t>(nodeId) * 4);
}
