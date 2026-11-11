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

// The weights SMAA mixes the pixel at p with its right, top, left and bottom neighbours by
vec4 smaaBlendingWeights(sampler2D weights, ivec2 p)
{
    ivec2 highest = textureSize(weights, 0) - ivec2(1);

    vec4 a;
    a.x = texelFetch(weights, clamp(p + ivec2(1, 0), ivec2(0), highest), 0).a;
    a.y = texelFetch(weights, clamp(p + ivec2(0, 1), ivec2(0), highest), 0).g;
    a.wz = texelFetch(weights, clamp(p, ivec2(0), highest), 0).xz;

    return a;
}

bool smaaBlendsHorizontally(vec4 weights)
{
    return max(weights.x, weights.z) > max(weights.y, weights.w);
}
