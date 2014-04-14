#version 330 core

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;

void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 result = vec4(0.0);

    for(int i = 0; i < 4; i++)
    {
        result += texelFetch(frameBufferTexture, coord, i);
    }

    result /= 4;
    fragColor = result;
}
