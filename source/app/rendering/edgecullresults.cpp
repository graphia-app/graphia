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

#include "edgecullresults.h"

EdgeCullResults::EdgeCullResults()
{
    resolveOpenGLFunctions();
}

void EdgeCullResults::reserve(size_t numEdges)
{
    if(_walk.reserve(numEdges))
        _written = false;
}

void EdgeCullResults::bindForWriting(QOpenGLShaderProgram& shader)
{
    _walk.bindForWriting();
    setSizeUniform(shader);

    _written = true;
}

void EdgeCullResults::bind(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _walk.texture(0));
    setSizeUniform(shader);
}

void EdgeCullResults::release(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void EdgeCullResults::setSizeUniform(QOpenGLShaderProgram& shader) const
{
    shader.setUniformValue("visibleRangesSize", _walk.size());
}
