#version 330 core
#define PI 3.14159265359

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;

layout (location = 3) in vec3  nodePosition; // The position of the node

layout (location = 4) in int   component; // The component index

layout (location = 5) in float size; // The size of the node
layout (location = 6) in vec3  outerColor; // The outside color of the node
layout (location = 7) in vec3  innerColor; // The inside color of the node
layout (location = 8) in float selected;

out vec3 position;
out vec2 uv;
out vec3 normal;
out vec3 innerVColor;
out vec3 outerVColor;
out float vSelected;
out float lightOffset;

uniform samplerBuffer componentData;

mat4 makeOrientationMatrix(vec3 forward)
{
    vec3 worldUp = vec3(0.0, 1.0, 0.0);

    vec3 right = cross(forward, worldUp);
    vec3 up = cross(forward, right);
    mat3 m;

    m[1] = up;
    m[0] = right;
    m[2] = forward;

    return mat4(m);
}

int componentDataOffset()
{
    return component * 33;
}

mat4 mat4FromComponentData(int offset)
{
    mat4 m;
    int index = componentDataOffset() + offset;

    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < 4; i++)
        {
            m[j][i] = texelFetch(componentData, index).r;
            index++;
        }
    }

    return m;
}

float floatFromComponentData(int offset)
{
    int index = componentDataOffset() + offset;
    return texelFetch(componentData, index).r;
}

void main()
{
    mat4 modelViewMatrix = mat4FromComponentData(0);
    mat4 projectionMatrix = mat4FromComponentData(16);
    lightOffset = floatFromComponentData(32);

    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));

    position = (modelViewMatrix * vec4(nodePosition + (vertexPosition * size), 1.0)).xyz;
    normal = normalMatrix * vertexNormal;
    outerVColor = outerColor;
    innerVColor = innerColor;
    vSelected = selected;
    gl_Position = projectionMatrix * vec4(position, 1.0);

    // Map 2D UVs to node Hemisphere
    // Create orientation matrix based on vector from eyespace nodePosition
    // This keeps the node "facing" the camera
    vec3 eyeNodeCentre = (modelViewMatrix * vec4(nodePosition, 1.0)).xyz;
    mat4 perspectiveOrientationMatrix = makeOrientationMatrix(normalize(-eyeNodeCentre));
    vec4 eyeNodeNormal = perspectiveOrientationMatrix * vec4(normal, 1.0);

    // Project hemisphere normal to UVs
    uv = vec2(0.5 + asin(eyeNodeNormal.x) / PI, 0.5 + asin(eyeNodeNormal.y) / PI);
}
