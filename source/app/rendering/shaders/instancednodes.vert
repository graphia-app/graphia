/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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

flat out float element;
out vec3 vPosition;
out vec2 uv;
out vec3 vNormal;
out vec3 innerVColor;
out vec3 outerVColor;
out float vSelected;
out float lightOffset;
out float lightScale;
out float projectionScale;

uniform sampler2D componentData;
uniform int componentDataElementSize;
uniform int componentDataTextureMaxDimension;

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

float floatFromComponentData(int offset)
{
    int index = (component * componentDataElementSize) + offset;
    int y = index / componentDataTextureMaxDimension;
    int x = index % componentDataTextureMaxDimension;
    ivec2 coord = ivec2(x, y);

    return texelFetch(componentData, coord, 0).r;
}

mat4 mat4FromComponentData(int offset)
{
    mat4 m;
    int index = offset;

    for(int j = 0; j < 4; j++)
        for(int i = 0; i < 4; i++)
            m[j][i] = floatFromComponentData(index++);

    return m;
}

float approxProjectionScaleFor(vec3 position, float extent, mat4 p)
{
    vec4 a = p * vec4(position.x + extent, position.y, position.z, 1.0);
    vec4 b = p * vec4(position.x - extent, position.y, position.z, 1.0);

    return abs((a.x / a.w) - (b.x / b.w)) / 2.0;
}

void main()
{
    element = float(gl_InstanceID + 1);

    mat4 modelViewMatrix = mat4FromComponentData(0);
    mat4 projectionMatrix = mat4FromComponentData(16);
    lightOffset = floatFromComponentData(32);
    lightScale = floatFromComponentData(33);

    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));

    vNormal = normalMatrix * vertexNormal;
    outerVColor = outerColor;
    innerVColor = innerColor;
    vSelected = selected;

    vec3 nodeVertexPosition = nodePosition + (vertexPosition * size);
    vec3 nodeVertexPositionViewSpace = vPosition =
        (modelViewMatrix * vec4(nodeVertexPosition, 1.0)).xyz;
    gl_Position = projectionMatrix * vec4(nodeVertexPositionViewSpace, 1.0);

    // This value is 0.0 in the perspective case and 1.0 in ortho
    // It doesn't change linearly, but it's probably better to
    // use it to tween than switching over suddenly
    float proj33 = projectionMatrix[3][3];

    // Map 2D UVs to node Hemisphere
    // Create orientation matrix based on vector from view space nodePosition
    // This keeps the node "facing" the camera
    vec3 nodePositionViewSpace = (modelViewMatrix * vec4(nodePosition, 1.0)).xyz;
    vec3 nodeToCameraDir = mix(normalize(-nodePositionViewSpace), vec3(0.0, 0.0, 1.0), proj33);
    mat4 orientationMatrix = makeOrientationMatrix(nodeToCameraDir);
    vec4 eyeNodeNormal = orientationMatrix * vec4(vNormal, 1.0);

    // Project hemisphere normal to UVs
    uv = vec2(0.5 + asin(eyeNodeNormal.x) / PI, 0.5 + asin(eyeNodeNormal.y) / PI);

    projectionScale = approxProjectionScaleFor(nodeVertexPositionViewSpace,
        size, projectionMatrix);
}
