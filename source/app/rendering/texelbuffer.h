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

#ifndef TEXELBUFFER_H
#define TEXELBUFFER_H

#include "openglfunctions.h"

#include <span>

// An array of RGBA32F texels, read in a shader by texelAt(), from texelbuffer.glsl
class TexelBuffer : public OpenGLFunctions
{
public:
    TexelBuffer();
    ~TexelBuffer() override;

    TexelBuffer(const TexelBuffer&) = delete;
    TexelBuffer& operator=(const TexelBuffer&) = delete;

    // Four floats per texel
    void upload(std::span<const GLfloat> texels);

    void bind(GLenum textureUnit);
    void release(GLenum textureUnit);

private:
#ifdef OPENGL_ES
    static constexpr GLenum Target = GL_TEXTURE_2D;
#else
    static constexpr GLenum Target = GL_TEXTURE_BUFFER;
#endif

    GLuint _texture = 0;

#ifdef OPENGL_ES
    GLint _maximumWidth = 0;
    GLint _width = 0;
    GLint _height = 0;
#else
    GLuint _buffer = 0;
#endif
};

#endif // TEXELBUFFER_H
