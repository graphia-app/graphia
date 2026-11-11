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

// What the splatted nodes leave to be seen of a shape drawn by coverage, from the
// depths and occlusion SplatCompositor gathers of them

uniform highp sampler2D nearestNode;
uniform highp sampler2D farthestNode;
uniform sampler2D nodeOcclusion;

// Falls by e^FALLOFF each component radius behind the pixel's nearest node, so the
// front hides the back; measured from it so the front stays in half float range
float coverageWeight(float viewZ, vec2 frontAndRadius, mat2 zToClip)
{
    const float FALLOFF = 5.0;
    const float MOST_AHEAD = 4.0;

    float nearest = texelFetch(nearestNode, ivec2(gl_FragCoord.xy), 0).r;
    float from = nearest < 1.0 ? -zAtDepth(zToClip, nearest) : frontAndRadius.x;

    return exp(-max(FALLOFF * (-viewZ - from) / frontAndRadius.y, -MOST_AHEAD));
}

// How far a depth is from the pixel's nearest node to its farthest
float amongNodes(float depth, float nearest)
{
    float farthest = texelFetch(farthestNode, ivec2(gl_FragCoord.xy), 0).r;
    return clamp((depth - nearest) / max(farthest - nearest, 0.0000001), 0.0, 1.0);
}

// The occlusion is spread evenly over the depths its mean and spread give
// Then a few nodes in front of a dense stack don't spread its occlusion into the gap
float leftByNodes(float depth)
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float nearest = texelFetch(nearestNode, pixel, 0).r;

    // Nothing is in front where there are no nodes
    if(depth <= nearest)
        return 1.0;

    float among = amongNodes(depth, nearest);
    vec3 occlusion = texelFetch(nodeOcclusion, pixel, 0).rgb;
    float mean = occlusion.g / occlusion.r;
    float halfWidth = sqrt(3.0 * max((occlusion.b / occlusion.r) - (mean * mean), 0.0));
    float inFront = clamp((among - mean + halfWidth) / max(2.0 * halfWidth, 0.0001), 0.0, 1.0);

    return exp(-occlusion.r * inFront);
}
