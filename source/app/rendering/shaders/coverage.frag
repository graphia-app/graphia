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

// What SplatCompositor summed, as a premultiplied colour and selection to blend over a layer

layout (location = 0) out vec4 outColor;
layout (location = 3) out vec4 outSelection;

uniform sampler2D sums;
uniform sampler2D weights;
uniform sampler2D overlap;

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(sums, p, 0);

    if(sum.a <= 0.0)
        discard;

    vec4 weight = texelFetch(weights, p, 0);
    vec3 where = texelFetch(overlap, p, 0).rgb;

    // The cover if the shapes lay apart, anywhere in the pixel
    float apart = 1.0 - exp(-sum.a);

    // Shapes spread with this standard deviation cover at most their mean width plus √12 times it
    // So shapes lying over each other cover little more than one of them
    float shapes = max(weight.b, 0.0001);
    vec2 mean = where.xy / shapes;
    float spread = sqrt(max((where.z / shapes) - dot(mean, mean), 0.0));
    float covered = min(apart, (weight.a / shapes) + (sqrt(12.0) * spread));

    outColor = vec4((sum.rgb / weight.r) * covered, covered);
    outSelection = vec4(vec3(weight.g / weight.r) * covered, covered);
}
