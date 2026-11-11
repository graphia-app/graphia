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

// SMAA's edge detection, finding silhouettes where the element or sub-element
// shown changes, transcribed from SMAA.hlsl

#ifdef GL_ES
// Element numbers are too large for medium precision to hold exactly
precision highp float;
#endif

layout (location = 0) out vec2 fragEdges;

uniform sampler2D elements;

uniform vec4 imageMetrics; // 1 / width, 1 / height, width, height

vec2 elementAt(ivec2 p)
{
    return texelFetch(elements, clamp(p, ivec2(0), ivec2(imageMetrics.zw) - ivec2(1)), 0).rg;
}

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);

    vec2 element = elementAt(p);
    vec2 edges = vec2(
        elementAt(p + ivec2(-1, 0)) != element ? 1.0 : 0.0,
        elementAt(p + ivec2(0, -1)) != element ? 1.0 : 0.0);

    if(dot(edges, vec2(1.0)) == 0.0)
        discard;

    fragEdges = edges;
}
