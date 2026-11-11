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

#include "screenpass.h"

ScreenPass::ScreenPass()
{
    resolveOpenGLFunctions();
    _triangle.create();
}

void ScreenPass::draw(QOpenGLShaderProgram& shader, std::initializer_list<GLuint> textures)
{
    shader.bind();

    GLenum textureUnit = GL_TEXTURE0;

    for(auto texture : textures)
    {
        glActiveTexture(textureUnit++);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    _triangle.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    _triangle.release();

    while(textureUnit-- > GL_TEXTURE0)
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    shader.release();
}
