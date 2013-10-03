#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 point; // The position of marker
layout (location = 2) in float scale; // The scale of marker

out vec3 position;
out vec3 centre;
out float radius;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec3 point3 = vec3(point, 0.0);
    position = point3 + (vertexPosition * scale * 2.0);
    vec3 transformedPosition = ( modelViewMatrix * vec4( position, 1.0 ) ).xyz;
    centre = vec3(point, 0.0);
    radius = scale;

    gl_Position = projectionMatrix * vec4( transformedPosition, 1.0 );
}
