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

#include "texelbuffer.h"

#include <QtGlobal>

#ifdef OPENGL_ES
#include <algorithm>
#include <cmath>
#endif

TexelBuffer::TexelBuffer()
{
    resolveOpenGLFunctions();

    glGenTextures(1, &_texture);
    glBindTexture(Target, _texture);

#ifdef OPENGL_ES
    // ES 3.0 has no buffer textures, so a 2D texture of roughly square
    // dimensions stands in for one, which the shader reads a row at a time
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maximumWidth);
#else
    glGenBuffers(1, &_buffer);
    glBindBuffer(Target, _buffer);
    glTexBuffer(Target, GL_RGBA32F, _buffer);
    glBindBuffer(Target, 0);
#endif

    glBindTexture(Target, 0);
}

TexelBuffer::~TexelBuffer()
{
#ifndef OPENGL_ES
    glDeleteBuffers(1, &_buffer);
#endif

    glDeleteTextures(1, &_texture);
}

void TexelBuffer::upload(std::span<const GLfloat> texels)
{
    Q_ASSERT(texels.size() % 4 == 0);

#ifdef OPENGL_ES
    const auto numTexels = static_cast<GLint>(texels.size() / 4);
    const auto width = std::clamp(static_cast<GLint>(std::ceil(std::sqrt(numTexels))),
        1, std::max(1, _maximumWidth));
    const auto height = (numTexels + width - 1) / width;

    glBindTexture(Target, _texture);

    if(width != _width || height != _height)
    {
        glTexImage2D(Target, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        _width = width;
        _height = height;
    }

    const auto numFullRows = numTexels / width;
    const auto lastRowLength = numTexels % width;

    if(numFullRows > 0)
    {
        glTexSubImage2D(Target, 0, 0, 0, width, numFullRows,
            GL_RGBA, GL_FLOAT, texels.data());
    }

    if(lastRowLength > 0)
    {
        glTexSubImage2D(Target, 0, 0, numFullRows, lastRowLength, 1,
            GL_RGBA, GL_FLOAT, &texels[static_cast<size_t>(numFullRows * width * 4)]);
    }

    glBindTexture(Target, 0);
#else
    glBindBuffer(Target, _buffer);
    glBufferData(Target, static_cast<GLsizeiptr>(texels.size_bytes()),
        texels.data(), GL_STREAM_DRAW);
    glBindBuffer(Target, 0);
#endif
}

void TexelBuffer::bind(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(Target, _texture);
}

void TexelBuffer::release(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(Target, 0);
}
