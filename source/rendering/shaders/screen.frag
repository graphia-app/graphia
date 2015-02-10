#version 330 core

in vec2 vPosition;
in vec2 vTexCoord;
in vec4 vColor;

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;

vec4 multisampledValue(ivec2 coord)
{
    vec3 rgb = vec3(0.0);
    float a = 0.0;
    for(int s = 0; s < 4; s++)
    {
        vec4 texel = texelFetch(frameBufferTexture, coord, s);
        rgb += texel.rgb;
        a += texel.a;
    }

    vec4 color = vec4(rgb / a, a / 4.0);

    return color * vColor;
}

void main()
{
    ivec2 coord = ivec2(vTexCoord);
    fragColor = multisampledValue(coord);
}
