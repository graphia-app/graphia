#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec3 point; // The position of the node
layout (location = 4) in float size; // The size of the node
layout (location = 5) in vec3 color; // The color of the node

out vec3 position;
out vec3 normal;
out vec3 vColor;

uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;

void main()
{
    position = ( modelViewMatrix * vec4( point + (vertexPosition * size), 1.0 ) ).xyz;

    normal = normalMatrix * vertexNormal;
    vColor = color;
    gl_Position = projectionMatrix * vec4( position, 1.0 );
}
