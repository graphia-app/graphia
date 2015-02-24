#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 projectionMatrix;

out vec2 vPosition;

void main()
{
    vPosition = position;
    gl_Position = projectionMatrix * vec4(position, 0.0, 1.0);
}
