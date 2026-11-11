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

#ifndef APPROXIMATEEDGESHADING_H
#define APPROXIMATEEDGESHADING_H

#include "edgedatatexels.h"

#include <QOpenGLShaderProgram>

#include <cstddef>

// A texel per edge in each of three textures, holding the shading across its
// width averaged over each third of it
class ApproximateEdgeShading : public OpenGLFunctions
{
public:
    ApproximateEdgeShading();

    void reserve(size_t numEdges) { _thirds.reserve(numEdges); }
    void bindForWriting(QOpenGLShaderProgram& shader);

    // Binds a texture to each of three units from firstTextureUnit
    void bind(GLenum firstTextureUnit, QOpenGLShaderProgram& shader);
    void release(GLenum firstTextureUnit);

private:
    EdgeDataTexels _thirds;
};

#endif // APPROXIMATEEDGESHADING_H
