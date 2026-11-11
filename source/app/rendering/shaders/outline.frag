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

#ifdef GL_ES
// Element numbers are too large for medium precision to hold exactly
precision highp float;
#endif

layout (location = 0) out vec4 fragColor;

uniform sampler2D elementTexture;

uniform vec4 outlineColor;
uniform float alpha;

vec2 elementAt(ivec2 p)
{
    ivec2 highest = textureSize(elementTexture, 0) - ivec2(1);
    return texelFetch(elementTexture, clamp(p, ivec2(0), highest), 0).rg;
}

float edgeStrengthAt(ivec2 p)
{
    float s = elementAt(p).r;

    float numDiffPixels = 0.0;
    float projectionScale = 0.0;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            if(s != elementAt(p + ivec2(i, j)).r)
                numDiffPixels += 1.0;
        }
    }

    if(numDiffPixels == 0.0)
        discard;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            projectionScale = max(projectionScale, elementAt(p + ivec2(i, j)).g);
        }
    }

    const float MIN_PS = 0.001;
    const float MAX_PS = 0.015;
    projectionScale = clamp((projectionScale - MIN_PS) / (MAX_PS - MIN_PS), 0.0, 1.0);

    // Clamp to a maximum difference, otherwise we get abnormally dark
    // spots at the junction of many elements
    const float maxNumDiffPixels = 5.0;
    numDiffPixels = min(numDiffPixels, maxNumDiffPixels);

    // Normalise
    float value = numDiffPixels / maxNumDiffPixels;

    // Give the value a bit of ramp; this thins the edges out a bit
    value = pow(value, 1.0 + ((1.0 - projectionScale) * 2.0));

    // Dial the value back a bit, helping the color come through
    // when edges are densely packed
    const float MIN_VALUE = 0.1;
    value *= MIN_VALUE + (projectionScale * (1.0 - MIN_VALUE));

    return value;
}

void main()
{
    float outlineAlpha = edgeStrengthAt(ivec2(gl_FragCoord.xy));
    fragColor = vec4(outlineColor.rgb, outlineAlpha * alpha);
}
