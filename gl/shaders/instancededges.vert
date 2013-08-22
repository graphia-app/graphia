#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec3 source; // The position of the source node
layout (location = 4) in vec3 target; // The position of the target node

out vec3 position;
out vec3 normal;

uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec3 midpoint = mix(source, target, 0.5);
    position = ( modelViewMatrix * vec4( midpoint + vertexPosition, 1.0 ) ).xyz;

    normal = normalMatrix * vertexNormal;
    gl_Position = projectionMatrix * vec4( position, 1.0 );
}
