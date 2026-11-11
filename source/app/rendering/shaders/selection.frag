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

layout (location = 0) out vec4 fragColor;

uniform sampler2D selectionTexture;

uniform vec4 highlightColor;
uniform float alpha;

// Whole pixels, so that the outline is hard in each sample gathered, wherever
// between pixels the selection ends
const float OUTSIDE = 2.0;
const float INSIDE = 1.0;

float selectedAt(vec2 offset)
{
    vec2 size = vec2(textureSize(selectionTexture, 0));
    vec2 p = clamp(gl_FragCoord.xy + offset, vec2(0.5), size - vec2(0.5));
    return textureLod(selectionTexture, p / size, 0.0).r;
}

void main()
{
    mat3 G[2] = mat3[](
        mat3( 1.0,  2.0,  1.0,
              0.0,  0.0,  0.0,
             -1.0, -2.0, -1.0),
        mat3( 1.0,  0.0, -1.0,
              2.0,  0.0, -2.0,
              1.0,  0.0, -1.0)
    );

    // Sobel, with its taps far enough apart to reach the selection from the
    // outline's outer side, for which way is out
    vec2 gradient = vec2(0.0);

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            float tap = selectedAt(vec2(i, j) * OUTSIDE);
            gradient += vec2(G[0][i + 1][j + 1], G[1][i + 1][j + 1]) * tap;
        }
    }

    float outlineAlpha = 0.0;
    float gradientLength = length(gradient);

    if(gradientLength > 0.05)
    {
        // The outline is what lies between OUTSIDE out from the boundary and INSIDE in
        vec2 inward = -gradient / gradientLength;
        outlineAlpha = clamp(selectedAt(inward * OUTSIDE) - selectedAt(-inward * INSIDE), 0.0, 1.0);
    }

    // The interior shows only through what the outline leaves, as it does where
    // each of the samples gathered is clamped
    float selected = texelFetch(selectionTexture, ivec2(gl_FragCoord.xy), 0).r;
    float interiorAlpha = selected * 0.2 * (1.0 - outlineAlpha);

    fragColor = vec4(highlightColor.rgb, 1.0) * ((interiorAlpha + outlineAlpha) * alpha);
}
