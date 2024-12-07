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

in vec2 vPosition;

layout (location = 0) out vec4 fragColor;

#ifdef GL_ES
uniform sampler2D frameBufferTexture;
#else
uniform sampler2DMS frameBufferTexture;
#endif

uniform int multisamples;
uniform float alpha;
uniform int disableAlphaBlending;

#ifdef GL_ES
vec4 resolvedValue(vec2 coord)
{
    return texture(frameBufferTexture, coord);
}
#else
vec4 resolvedValue(vec2 coord)
{
    vec3 rgb = vec3(0.0);
    float a = 0.0;
    int numTexels = 0;
    for(int s = 0; s < multisamples; s++)
    {
        vec4 texel = texelFetch(frameBufferTexture, ivec2(coord), s);

        if(texel.a > 0.0)
        {
            rgb += texel.rgb;
            a += (texel.a / float(multisamples));
            numTexels++;
        }
    }

    if(disableAlphaBlending != 0 && numTexels > 0)
        a = 1.0;

    return vec4(rgb / float(numTexels), a);
}
#endif

void main()
{
    vec4 color = resolvedValue(vPosition);
    fragColor = vec4(color.rgb, color.a * alpha);
}
