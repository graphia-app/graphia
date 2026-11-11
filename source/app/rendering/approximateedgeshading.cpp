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

#include "approximateedgeshading.h"

ApproximateEdgeShading::ApproximateEdgeShading() :
    _thirds(1, GL_RGBA32F, GL_RGBA, 3)
{
    resolveOpenGLFunctions();
}

void ApproximateEdgeShading::bindForWriting(QOpenGLShaderProgram& shader)
{
    _thirds.bindForWriting();
    shader.setUniformValue("approximateShadingThirdsSize", _thirds.size());
}

void ApproximateEdgeShading::bind(GLenum firstTextureUnit, QOpenGLShaderProgram& shader)
{
    for(size_t i = 0; i < 3; i++)
    {
        glActiveTexture(firstTextureUnit + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, _thirds.texture(i));
    }

    shader.setUniformValue("approximateShadingThirdsSize", _thirds.size());
}

void ApproximateEdgeShading::release(GLenum firstTextureUnit)
{
    for(size_t i = 0; i < 3; i++)
    {
        glActiveTexture(firstTextureUnit + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
