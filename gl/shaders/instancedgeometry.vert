#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec3 point; // The position of the data point. Varies per instance

out vec3 position;
out vec3 normal;

uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;

void main()
{
    // Calculate position by offseting vertex by instance position
    position = ( modelViewMatrix * vec4( point + vertexPosition, 1.0 ) ).xyz;

    // Transform the normal as usual
    normal = normalMatrix * vertexNormal;

    // Draw at the current position
    gl_Position = projectionMatrix * vec4( position, 1.0 );
}
