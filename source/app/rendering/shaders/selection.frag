/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

in vec2 vPosition;

layout (location = 0) out vec4 fragColor;

#ifdef GL_ES
uniform sampler2D frameBufferTexture;
#else
uniform sampler2DMS frameBufferTexture;
#endif

uniform int width;
uniform int height;
uniform int multisamples;
uniform vec4 highlightColor;
uniform float alpha;

#ifdef GL_ES
vec4 resolvedValue(vec2 coord)
{
    vec4 value = texture(frameBufferTexture, coord);
    value.a *= alpha;

    return value;
}
#else
vec4 resolvedValue(vec2 coord)
{
    vec4 accumulator = vec4(0.0);
    for(int s = 0; s < multisamples; s++)
    {
        accumulator += texelFetch(frameBufferTexture, ivec2(coord), s);
    }
    accumulator /= float(multisamples);
    accumulator.a *= alpha;

    return accumulator;
}
#endif

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

    float pixelWidth = 1.0;
    float pixelHeight = 1.0;

#ifdef GL_ES
    pixelWidth /= float(width);
    pixelHeight /= float(height);
#endif

    // Sobel edge detection
    mat3 I;
    float cnv[2];

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            vec2 offset = vec2(float(i) * pixelWidth, float(j) * pixelHeight);
            vec2 offsetPosition = vPosition + offset;
            I[i + 1][j + 1] = resolvedValue(offsetPosition).r;
        }
    }

    for(int i = 0; i < 2; i++)
    {
        float dp3 = dot(G[i][0], I[0]) + dot(G[i][1], I[1]) + dot(G[i][2], I[2]);
        cnv[i] = dp3;
    }

    float outlineAlpha = 1.0 / inversesqrt(cnv[0] * cnv[0] + cnv[1] * cnv[1]);
    outlineAlpha *= 0.25;

    float interiorAlpha = I[1][1] * 0.2;

    fragColor = vec4(highlightColor.rgb, (interiorAlpha + outlineAlpha) * alpha);
}
