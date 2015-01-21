#version 330 core

in vec2 vPosition;
in vec2 vTexCoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2DMS frameBufferTexture;

uniform mat3 G[2] = mat3[](
    mat3(1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0),
    mat3(1.0, 0.0, -1.0, 2.0, 0.0, -2.0, 1.0, 0.0, -1.0)
);

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
    mat3 I;
    float cnv[2];

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            I[i][j] = multisampledValue(coord + ivec2(i - 1, j - 1)).r;
        }
    }

    for(int i = 0; i < 2; i++)
    {
        float dp3 = dot(G[i][0], I[0]) + dot(G[i][1], I[1]) + dot(G[i][2], I[2]);
        cnv[i] = dp3 * dp3;
    }

    float outlineAlpha = 0.5 * abs(cnv[0]) + abs(cnv[1]);
    float interiorAlpha = I[1][1];
    vec3 highlightColor = vec3(1.0, 1.0, 1.0);
    fragColor = vec4(highlightColor, (outlineAlpha + interiorAlpha) * 0.5);
}
