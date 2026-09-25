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

#ifndef NODEGEOMETRY_H
#define NODEGEOMETRY_H

#include "texelbuffer.h"

#include "shared/graph/elementid.h"

#include <QVector3D>

#include <cstddef>
#include <vector>

// Where each node is and how big it is, indexed by NodeId, for anything drawn
// that is positioned by the nodes; read in a shader through nodegeometry.glsl
class NodeGeometry
{
public:
    void resize(size_t numNodes);

    void setPosition(NodeId nodeId, const QVector3D& position);
    void setSize(NodeId nodeId, float size);

    void copyFrom(const NodeGeometry& other);
    void upload();

    void bind(GLenum textureUnit) { _texels.bind(textureUnit); }
    void release(GLenum textureUnit) { _texels.release(textureUnit); }

private:
    std::vector<GLfloat> _geometry;
    TexelBuffer _texels;

    GLfloat* texelFor(NodeId nodeId);
};

#endif // NODEGEOMETRY_H
