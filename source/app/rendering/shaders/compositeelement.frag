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

// Element numbers can't be blended, since one half way between two is neither

#ifdef GL_ES
// Element numbers are too large for medium precision to hold exactly
precision highp float;
#endif

#include "element.glsl"

layout (location = 0) out vec2 compositeElement;

uniform sampler2D elementTexture;
uniform sampler2D subElementTexture;

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    float element = texelFetch(elementTexture, p, 0).r;

    if(element == NO_ELEMENT)
        discard;

    compositeElement = vec2(element, texelFetch(subElementTexture, p, 0).r);
}
