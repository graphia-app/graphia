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

// SMAA's neighbourhood blending, transcribed from SMAA.hlsl

#ifdef GL_ES
// Fractions of a pixel across the whole image need more than medium precision
precision highp float;
#endif

layout (location = 0) out vec4 fragColor;

#include "smaablending.glsl"

uniform sampler2D colors;
uniform sampler2D weights;

uniform vec4 imageMetrics; // 1 / width, 1 / height, width, height

void main()
{
    vec2 texcoord = gl_FragCoord.xy * imageMetrics.xy;
    vec4 a = smaaBlendingWeights(weights, ivec2(gl_FragCoord.xy));

    if(dot(a, vec4(1.0)) < 1e-5)
    {
        fragColor = vec4(textureLod(colors, texcoord, 0.0).rgb, 1.0);
        return;
    }

    bool horizontal = smaaBlendsHorizontally(a);
    vec4 blendingOffset = horizontal ? vec4(a.x, 0.0, a.z, 0.0) : vec4(0.0, a.y, 0.0, a.w);
    vec2 blendingWeight = horizontal ? a.xz : a.yw;
    blendingWeight /= dot(blendingWeight, vec2(1.0));

    vec4 blendingCoord = (blendingOffset * vec4(imageMetrics.xy, -imageMetrics.xy)) + texcoord.xyxy;

    vec3 color = blendingWeight.x * textureLod(colors, blendingCoord.xy, 0.0).rgb;
    color += blendingWeight.y * textureLod(colors, blendingCoord.zw, 0.0).rgb;

    fragColor = vec4(color, 1.0);
}
