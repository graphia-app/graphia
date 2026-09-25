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

void NodeGeometry::resize(size_t numNodes)
{
    _geometry.resize(numNodes * 4);
}

void NodeGeometry::setPosition(NodeId nodeId, const QVector3D& position)
{
    auto* texel = texelFor(nodeId);

    texel[0] = position.x();
    texel[1] = position.y();
    texel[2] = position.z();
}

void NodeGeometry::setSize(NodeId nodeId, float size)
{
    texelFor(nodeId)[3] = size;
}

void NodeGeometry::copyFrom(const NodeGeometry& other)
{
    _geometry = other._geometry;
}

void NodeGeometry::upload()
{
    _texels.upload(_geometry);
}

GLfloat* NodeGeometry::texelFor(NodeId nodeId)
{
    return &_geometry.at(static_cast<size_t>(nodeId) * 4);
}
