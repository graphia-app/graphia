#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;

layout (location = 3) in vec3  nodePosition; // The position of the node

layout (location = 4) in int   component; // The component index

layout (location = 5) in float size; // The size of the node
layout (location = 6) in vec3  color; // The color of the node
layout (location = 7) in vec3  outlineColor; // The outline color of the node

out vec3 position;
out vec3 normal;
out vec3 vColor;
out vec3 vOutlineColor;

uniform samplerBuffer componentData;

void main()
{
    int index = component * 8;

    mat4 modelViewMatrix = mat4(texelFetch(componentData, index + 0),
                                texelFetch(componentData, index + 1),
                                texelFetch(componentData, index + 2),
                                texelFetch(componentData, index + 3));

    mat4 projectionMatrix = mat4(texelFetch(componentData, index + 4),
                                 texelFetch(componentData, index + 5),
                                 texelFetch(componentData, index + 6),
                                 texelFetch(componentData, index + 7));

    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));

    position = (modelViewMatrix * vec4(nodePosition + (vertexPosition * size), 1.0)).xyz;
    normal = normalMatrix * vertexNormal;
    vColor = color;
    vOutlineColor = outlineColor;
    gl_Position = projectionMatrix * vec4(position, 1.0);
}
