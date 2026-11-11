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

// Where the texels EdgeDataTexels holds for each edge are, which a build writes by
// drawing a point at each

// Two texels an edge, for its shaft and for its head
uniform vec2 visibleRangesSize;

// A texel an edge in each of three textures
uniform vec2 approximateShadingThirdsSize;

ivec2 edgeTexel(int index, vec2 texturesSize)
{
    int width = int(texturesSize.x);
    return ivec2(index % width, index / width);
}

ivec2 visibleRangeTexel(bool isHead)
{
    return edgeTexel((gl_InstanceID * 2) + (isHead ? 1 : 0), visibleRangesSize);
}

ivec2 approximateShadingThirdsTexel()
{
    return edgeTexel(gl_InstanceID, approximateShadingThirdsSize);
}

void drawPointAt(ivec2 texel, vec2 texturesSize)
{
    vec2 texelCentre = vec2(texel) + vec2(0.5);
    gl_Position = vec4(((texelCentre / texturesSize) * 2.0) - 1.0, 0.0, 1.0);
    gl_PointSize = 1.0;
}

// A part that can't be seen at all ends before it starts
const vec2 HIDDEN_RANGE = vec2(1.0, 0.0);

bool visibleRangeEmpty(vec2 range)
{
    return range.y <= range.x;
}
