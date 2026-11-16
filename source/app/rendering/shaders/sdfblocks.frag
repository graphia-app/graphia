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

// Summarises each blockSize square of source texels for the SDF shader's searches
uniform sampler2D tex;
uniform int blockSize;

layout(location = 0) out vec4 outColor;

void main()
{
    ivec2 origin = ivec2(gl_FragCoord.xy) * blockSize;

    float minimum = 1.0;
    float maximum = 0.0;
    bool partial = false;

    for(int x = 0; x < blockSize; x++)
    {
        for(int y = 0; y < blockSize; y++)
        {
            float coverage = texelFetch(tex, origin + ivec2(x, y), 0).a;
            minimum = min(minimum, coverage);
            maximum = max(maximum, coverage);
            partial = partial || (coverage > 0.0 && coverage < 1.0);
        }
    }

    outColor = vec4(minimum, maximum, partial ? 1.0 : 0.0, 1.0);
}
