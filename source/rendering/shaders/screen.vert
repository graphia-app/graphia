#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;

uniform mat4 projectionMatrix;

out vec2 vPosition;
out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    vPosition = position;
    vTexCoord = texCoord;
    vColor = color;
    gl_Position = projectionMatrix * vec4(position, 0.0, 1.0);
}
