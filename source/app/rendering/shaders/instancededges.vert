/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

layout (location = 0) in vec3   vertexPosition;
layout (location = 1) in vec3   vertexNormal;
layout (location = 2) in vec2   vertexTexCoord;

layout (location = 3) in int    sourceIndex; // Indexes the source node's geometry
layout (location = 4) in int    targetIndex; // Indexes the target node's geometry
layout (location = 7) in int    edgeType; // The type of the edge

layout (location = 8) in int    component; // The component index

layout (location = 9)  in float size; // The size of the edge
layout (location = 10) in vec3  outerColor; // The outside color of the edge
layout (location = 11) in vec3  innerColor; // The inside color of the edge (used for multi edges)
layout (location = 12) in float selected;

flat out float element;
out vec3 vPosition;
out vec3 vNormal;
out vec3 innerVColor;
out vec3 outerVColor;
out float vSelected;
out vec2 uv;
out float lightOffset;
out float lightScale;
out float projectionScale;

#include "componentdata.glsl"
#include "nodegeometry.glsl"

const float ARROW_HEAD_CUTOFF_Y = 0.24;
const float MAX_ARROW_HEAD_LENGTH = 0.25;

bool equals(float value, float target)
{
    const float EPSILON = 0.001;
    return (abs(value - target) <= EPSILON);
}

mat4 makeOrientationMatrix(vec3 up)
{
    mat3 m;

    m[1] = up;
    m[0].y = -m[1].x;
    m[0].z = m[1].y;
    m[0].x = m[1].z;

    float d = dot(m[0], m[1]);
    m[0] -= (d * m[1]);
    m[0] = normalize(m[0]);
    m[2] = cross(m[0], m[1]);

    return mat4(m);
}

float approxProjectionScaleFor(vec3 position, float extent, mat4 p)
{
    vec4 a = p * vec4(position.x + extent, position.y, position.z, 1.0);
    vec4 b = p * vec4(position.x - extent, position.y, position.z, 1.0);

    return abs((a.x / a.w) - (b.x / b.w)) / 2.0;
}

// Whether the edge lies wholly within the lens where its two nodes intersect
bool hiddenByNodes(float edgeLengthSq, float sourceSize, float targetSize)
{
    float nodeRadiusSum = sourceSize + targetSize;

    if(edgeLengthSq <= 0.0 || edgeLengthSq >= nodeRadiusSum * nodeRadiusSum)
        return false;

    float sourceRadiusSq = sourceSize * sourceSize;
    float targetRadiusSq = targetSize * targetSize;

    float n = edgeLengthSq - sourceRadiusSq + targetRadiusSq;
    float lensRadiusSq = targetRadiusSq - ((n * n) / (4.0 * edgeLengthSq));

    return (size * size) < lensRadiusSq;
}

void main()
{
    // Make the index negative so that it doesn't overlap with the node indices
    element = float(-(gl_InstanceID + 1));

    vec3 sourcePosition = nodePosition(sourceIndex);
    vec3 targetPosition = nodePosition(targetIndex);
    float sourceSize = nodeSize(sourceIndex);
    float targetSize = nodeSize(targetIndex);

    vec3 sourceToTarget = targetPosition - sourcePosition;

    if(hiddenByNodes(dot(sourceToTarget, sourceToTarget), sourceSize, targetSize))
    {
        gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float edgeLength = distance(sourcePosition, targetPosition);
    float edgeLengthMinusNodeRadii = edgeLength - (sourceSize + targetSize);
    vec3 midpoint = mix(sourcePosition, targetPosition, 0.5);
    mat4 orientationMatrix = makeOrientationMatrix(normalize(targetPosition - sourcePosition));

    // Apply inverse to the normals
    vec3 scaledVertexNormal = vertexNormal;
    scaledVertexNormal.xz /= size;
    scaledVertexNormal.y /= edgeLength;

    mat4 modelViewMatrix = componentModelViewMatrix(component);
    mat4 projectionMatrix = componentProjectionMatrix(component);
    lightOffset = componentLightOffset(component);
    lightScale = componentLightScale(component);

    vec3 edgeVertexPosition = vec3(0.0);

    const int EdgeVisualTypeCylinder = 0;
    const int EdgeVisualTypeArrow = 1;

    // Cylinder Edge
    if(edgeType == EdgeVisualTypeCylinder && !equals(length(vertexPosition.xz), 1.0))
    {
        // These vertices form the cone of the arrow mesh, which we
        // don't want to render at all, when in cylinder mode
    }
    else if(edgeType == EdgeVisualTypeCylinder && vertexPosition.y > 0.0)
    {
        // Reposition the top of the cylinder to the target
        vec3 scaledVertexPosition = vertexPosition;
        scaledVertexPosition.xz *= size;
        scaledVertexPosition.y = (edgeLength * 0.5);
        uv = vec2(0.0, (scaledVertexPosition.y / edgeLengthMinusNodeRadii) + 0.5);

        edgeVertexPosition = (orientationMatrix * vec4(scaledVertexPosition, 1.0)).xyz + midpoint;
    }
    // Arrow Head Edge
    else if(edgeType == EdgeVisualTypeArrow && vertexPosition.y > ARROW_HEAD_CUTOFF_Y)
    {
        vec3 conePosition = vertexPosition;
        // Position so it points to origin
        conePosition.y -= 0.5;
        // Scale to match edgeSize
        conePosition *= size;
        // Scale arrow head (cone) length
        conePosition.y *= 16.0;

        // Limit the cone size to MAX_ARROW_HEAD_LENGTH * edge length
        if(abs(conePosition.y) > edgeLength * MAX_ARROW_HEAD_LENGTH)
            conePosition.y = -edgeLength * MAX_ARROW_HEAD_LENGTH;

        uv = vec2(0.0, 1.0 + ((conePosition.y) / edgeLengthMinusNodeRadii));

        // Offset cone position to point at edge of node
        conePosition.y -= targetSize;

        edgeVertexPosition = (orientationMatrix * vec4(conePosition, 1.0)).xyz + targetPosition;
    }
    else
    {
        // Scale edge vertices
        vec3 scaledVertexPosition = vertexPosition;
        scaledVertexPosition.xz *= size;
        scaledVertexPosition.y *= edgeLength;

        uv = vec2(0.0, -sourceSize / edgeLengthMinusNodeRadii);

        edgeVertexPosition = (orientationMatrix * vec4(scaledVertexPosition, 1.0)).xyz + midpoint;
    }

    vec3 edgeVertexPositionViewSpace = vPosition =
        (modelViewMatrix * vec4(edgeVertexPosition, 1.0)).xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix * orientationMatrix)));
    vNormal = normalMatrix * scaledVertexNormal;
    innerVColor = innerColor;
    outerVColor = outerColor;
    vSelected = selected;
    gl_Position = projectionMatrix * vec4(edgeVertexPositionViewSpace, 1.0);

    float normalisedPositionAlongEdge = vertexPosition.y + 0.5;
    projectionScale = approxProjectionScaleFor(edgeVertexPositionViewSpace,
        mix(sourceSize, targetSize, normalisedPositionAlongEdge), projectionMatrix);
}
