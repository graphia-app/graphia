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

#version 330 core

in vec2 vPosition;

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;
uniform int width;
uniform int height;
uniform int multisamples;
uniform vec4 outlineColor;
uniform float alpha;

// Find the mean of the available samples using a method that avoids integer overflow
float meanOfMSFor(ivec2 coord, int channel, uint s)
{
    coord.x = clamp(coord.x, 0, width - 1);
    coord.y = clamp(coord.y, 0, height - 1);

    float mean = 0.0;
    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, coord, i)[channel];
        mean += (v / s);
    }

    return mean;
}

float meanOfNonZeroMSFor(ivec2 coord, int channel)
{
    coord.x = clamp(coord.x, 0, width - 1);
    coord.y = clamp(coord.y, 0, height - 1);

    uint numNonZeroSamples = 0u;
    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, coord, i)[channel];

        if(v > 0.0)
            numNonZeroSamples++;
    }

    if(numNonZeroSamples == 0u)
        return 0.0;

    return meanOfMSFor(coord, channel, numNonZeroSamples);
}

float quantOfMSFor(ivec2 coord, int channel)
{
    coord.x = clamp(coord.x, 0, width - 1);
    coord.y = clamp(coord.y, 0, height - 1);

    float mean = meanOfMSFor(coord, channel, uint(multisamples));

    // Find the sample value closest to the mean,
    // thereby quantising it, so that we can identify the
    // most representative sample from those available
    float min = 1.0 / 0.0; //3.402823466e+38;
    int qi = 0;

    for(int i = 0; i < multisamples; i++)
    {
        float v = texelFetch(frameBufferTexture, coord, i)[channel];
        float diff = v < mean ? mean - v : v - mean;

        if(diff < min)
        {
            min = diff;
            qi = i;
        }
    }

    return texelFetch(frameBufferTexture, coord, qi)[channel];
}

float edgeStrengthAt(ivec2 coord)
{
    float s = quantOfMSFor(coord, 0);

    float numDiffPixels = 0.0;
    float projectionScale = 0.0;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            if(s != quantOfMSFor(coord + ivec2(i, j), 0))
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

            float p = float(meanOfNonZeroMSFor(coord + ivec2(i, j), 1));

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
    ivec2 coord = ivec2(vPosition);
    float outlineAlpha = edgeStrengthAt(coord);

    fragColor = vec4(outlineColor.rgb, outlineAlpha * alpha);
}


