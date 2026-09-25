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

#ifndef TEXELBUFFER_GLSL
#define TEXELBUFFER_GLSL

// The shader side of TexelBuffer

#ifdef GL_ES
#define TexelBuffer sampler2D

vec4 texelAt(TexelBuffer texels, int index)
{
    int width = textureSize(texels, 0).x;
    return texelFetch(texels, ivec2(index % width, index / width), 0);
}
#else
#define TexelBuffer samplerBuffer

vec4 texelAt(TexelBuffer texels, int index)
{
    return texelFetch(texels, index);
}
#endif

#endif
