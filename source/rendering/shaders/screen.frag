#version 330 core

in vec2 vPosition;
in vec2 vTexCoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;

vec4 multisampledValue(ivec2 coord)
{
    vec4 accumulator = vec4(0.0);
    for(int s = 0; s < 4; s++)
    {
        accumulator += texelFetch(frameBufferTexture, coord, s);
    }
    accumulator /= 4;

    return accumulator;
}

void main()
{
    ivec2 coord = ivec2(vTexCoord);
    fragColor = multisampledValue(coord);
}
