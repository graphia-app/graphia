/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
uniform vec4 outlineColor;
uniform float alpha;

#ifdef GL_ES
float meanOfNonZeroMSFor(vec2 coord, int channel)
{
    return texture(frameBufferTexture, coord)[channel];
}

float quantOfMSFor(vec2 coord, int channel)
{
    return texture(frameBufferTexture, coord)[channel];
}
#else
vec2 clampCoord(vec2 coord)
{
    vec2 clamped;

    clamped.x = clamp(coord.x, 0.0, float(width - 1));
    clamped.y = clamp(coord.y, 0.0, float(height - 1));

    return clamped;
}

float meanOfMSFor(vec2 coord, int channel, uint s)
{
    coord = clampCoord(coord);

    float mean = 0.0;
    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, ivec2(coord), i)[channel];
        mean += (v / float(s));
    }

    return mean;
}

float meanOfNonZeroMSFor(vec2 coord, int channel)
{
    coord = clampCoord(coord);

    uint numNonZeroSamples = 0u;
    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, ivec2(coord), i)[channel];

        if(v > 0.0)
            numNonZeroSamples++;
    }

    if(numNonZeroSamples == 0u)
        return 0.0;

    return meanOfMSFor(coord, channel, numNonZeroSamples);
}

float quantOfMSFor(vec2 coord, int channel)
{
    coord = clampCoord(coord);

    float mean = meanOfMSFor(coord, channel, uint(multisamples));

    // Find the sample value closest to the mean,
    // thereby quantising it, so that we can identify the
    // most representative sample from those available
    float min = 1.0 / 0.0; //3.402823466e+38;
    int qi = 0;

    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, ivec2(coord), i)[channel];
        float diff = v < mean ? mean - v : v - mean;

        if(diff < min)
        {
            min = diff;
            qi = i;
        }
    }

    return texelFetch(frameBufferTexture, ivec2(coord), qi)[channel];
}
#endif

float edgeStrengthAt(vec2 coord)
{
    float pixelWidth = 1.0;
    float pixelHeight = 1.0;

#ifdef GL_ES
    pixelWidth /= float(width);
    pixelHeight /= float(height);
#endif

    float s = quantOfMSFor(coord, 0);

    float numDiffPixels = 0.0;
    float projectionScale = 0.0;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            vec2 offset = vec2(float(i) * pixelWidth, float(j) * pixelHeight);

            if(s != quantOfMSFor(coord + offset, 0))
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

            vec2 offset = vec2(float(i) * pixelWidth, float(j) * pixelHeight);

            float p = float(meanOfNonZeroMSFor(coord + offset, 1));

            if(p > projectionScale)
                projectionScale = p;
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
    float outlineAlpha = edgeStrengthAt(vPosition);
    fragColor = vec4(outlineColor.rgb, outlineAlpha * alpha);
}
