#version 330 core

in vec2 vPosition;

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;
uniform int multisamples;
uniform float alpha;
uniform int disableAlphaBlending;

vec4 multisampledValue(ivec2 coord)
{
    vec3 rgb = vec3(0.0);
    float a = 0.0;
    int numTexels = 0;
    for(int s = 0; s < multisamples; s++)
    {
        vec4 texel = texelFetch(frameBufferTexture, coord, s);

        if(texel.a > 0.0)
        {
            rgb += texel.rgb;
            a += (texel.a / multisamples);
            numTexels++;
        }
    }

    if(disableAlphaBlending != 0 && numTexels > 0)
        a = 1.0;

    return vec4(rgb / numTexels, a);
}

void main()
{
    ivec2 coord = ivec2(vPosition);
    vec4 color = multisampledValue(coord);
    fragColor = vec4(color.rgb, color.a * alpha);
}
