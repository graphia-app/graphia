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

// Shared by the splatted builds and the approximate traced build

flat in vec3 shadingAcross[3];

flat in float splitAtBoundaries;
flat in vec4 axisPixels;
flat in vec2 radiusPixels;

// The traced share of a split edge's pixels keeps this far inside those it
// wholly covers, in pixels, so that none of its rays miss
const float BOUNDARY_MARGIN = 0.05;

// In pixels: x how far across the axis the pixel is, y the shaft's half width there
// z is 1 where the pixel isn't wholly inside the shaft, so is splatted
vec3 boundaryAt()
{
    vec2 pixel = gl_FragCoord.xy - (viewportSize * 0.5);
    vec2 between = axisPixels.zw - axisPixels.xy;
    float betweenLength = length(between);
    vec2 along = betweenLength > 0.0001 ? between / betweenLength : vec2(1.0, 0.0);
    vec2 across = vec2(along.y, -along.x);

    float end = betweenLength > 0.0001 ? dot(pixel - axisPixels.xy, along) / betweenLength : 0.5;
    float fromAxis = dot(pixel - axisPixels.xy, across);
    float halfWidth = mix(radiusPixels.x, radiusPixels.y, clamp(end, 0.0, 1.0));
    float reach = 0.5 * (abs(across.x) + abs(across.y));
    bool boundary = end >= 0.0 && end <= 1.0 && abs(fromAxis) > halfWidth - reach - BOUNDARY_MARGIN;

    return vec3(fromAxis, halfWidth, boundary ? 1.0 : 0.0);
}

// The shading's mean over a stretch of the width, its ends in fractions of the
// half width either side of the axis
vec3 meanShadingAcross(vec2 covering)
{
    float meanU = 0.5 * (covering.x + covering.y);
    float meanUU = (dot(covering, covering) + (covering.x * covering.y)) / 3.0;

    return shadingAcross[0] + (shadingAcross[1] * meanU) + (shadingAcross[2] * meanUU);
}
