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

// These work from the projection matrix itself, so they are exact for perspective,
// orthographic, and the transitions between

// The part of a projection matrix that takes view space z to clip space z and w
mat2 zToClipOf(mat4 projectionMatrix)
{
    return mat2(projectionMatrix[2].zw, projectionMatrix[3].zw);
}

float clipWAt(mat2 zToClip, float z)
{
    return (zToClip * vec2(z, 1.0)).y;
}

float depthAt(mat2 zToClip, float z)
{
    vec2 clip = zToClip * vec2(z, 1.0);
    return ((clip.x / clip.y) * 0.5) + 0.5;
}

// From a point towards where the rays meet, scaled by w per unit of z,
// which keeps it finite when that is infinitely far away
vec3 towardsEye(mat2 zToClip, vec3 position)
{
    return vec3(zToClip[0].y * position.xy, clipWAt(zToClip, position.z));
}

// Half the width, in normalised device coordinates, of an extent at a point
float projectionScaleFor(mat4 projectionMatrix, vec3 position, float extent)
{
    vec4 a = projectionMatrix * vec4(position.x + extent, position.y, position.z, 1.0);
    vec4 b = projectionMatrix * vec4(position.x - extent, position.y, position.z, 1.0);

    return abs((a.x / a.w) - (b.x / b.w)) / 2.0;
}
