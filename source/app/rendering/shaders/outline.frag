#version 330 core

in vec2 vPosition;

layout (location = 0) out vec4 fragColor;

uniform usampler2DMS frameBufferTexture;
uniform int width;
uniform int height;
uniform int multisamples;
uniform vec4 outlineColor;
uniform float alpha;

uint multisampledValue(ivec2 coord)
{
    coord.x = clamp(coord.x, 0, width - 1);
    coord.y = clamp(coord.y, 0, height - 1);

    // First, find the mean of the available samples...
    // (using a method that avoids integer overflow)
    uint s = uint(multisamples);
    uint sM1 = s - 1u;

    uint quotientSum = 0u;
    uint remainderSum = 0u;
    for(int i = 0; i < multisamples; i++)
    {
        uint v = texelFetch(frameBufferTexture, coord, i).r;

        quotientSum += (v / s);
        remainderSum += (v % s);
    }

    uint mean = quotientSum +
        ((remainderSum + (s * sM1)) / s) - sM1;

    // ...then, find the sample value closest to the mean,
    // thereby quantising it, so that we can identify the
    // most representative sample from those available
    uint min = (1u << 31);
    int qi = 0;

    for(int i = 0; i < multisamples; i++)
    {
        uint v = texelFetch(frameBufferTexture, coord, i).r;
        uint diff = abs(mean - v);

        if(diff < min)
        {
            min = diff;
            qi = i;
        }
    }

    return texelFetch(frameBufferTexture, coord, qi).r;
}

float edgeStrengthAt(ivec2 coord)
{
    uint s = multisampledValue(coord);

    // Count the number of surrounding values that differ
    float numDiffPixels =
        (s != multisampledValue(coord + ivec2(-1, -1)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2( 0, -1)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2( 1, -1)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2(-1,  0)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2( 1,  0)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2(-1,  1)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2( 0,  1)) ? 1.0 : 0.0) +
        (s != multisampledValue(coord + ivec2( 1,  1)) ? 1.0 : 0.0);

    // Clamp to a maximum difference, otherwise we get abnormally dark
    // spots at the junction of many elements
    const float maxNumDiffPixels = 5.0;
    numDiffPixels = min(numDiffPixels, maxNumDiffPixels);

    // Normalise
    float value = numDiffPixels / maxNumDiffPixels;

    // Give the value a bit of ramp; this thins the edges out a bit
    value = pow(value, 1.5);

    // Dial the value back a bit, helping the color come through
    // when edges are densely packed
    value *= 0.7;

    return value;
}

void main()
{
    ivec2 coord = ivec2(vPosition);
    float outlineAlpha = edgeStrengthAt(coord);

    fragColor = vec4(outlineColor.rgb, outlineAlpha * alpha);
}


