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

// A level of DepthPyramid: the furthest depth of what it covers, either of
// the depth texture itself or of the level before

layout (location = 0) out float furthestDepth;

#ifdef RESOLVE_DEPTH

uniform sampler2D depthTexture;

void main()
{
    furthestDepth = texelFetch(depthTexture, ivec2(gl_FragCoord.xy), 0).r;
}

#else

uniform sampler2D previousLevel;
uniform vec2 previousLevelSize;

void main()
{
    ivec2 size = ivec2(previousLevelSize);
    ivec2 first = ivec2(gl_FragCoord.xy) * 2;

    // Account for texels lost during odd-dimension halving
    ivec2 span = ivec2(2) + (size % 2);

    furthestDepth = 0.0;

    for(int y = 0; y < span.y; y++)
    {
        for(int x = 0; x < span.x; x++)
        {
            ivec2 texel = min(first + ivec2(x, y), size - ivec2(1));
            furthestDepth = max(furthestDepth, texelFetch(previousLevel, texel, 0).r);
        }
    }
}

#endif
