#version 330 core

in vec2 vTexCoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2D frameBufferTexture;

void main()
{
    fragColor = texture(frameBufferTexture, vTexCoord);
}
