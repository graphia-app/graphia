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

// A camera facing quad over the node's sphere, for the fragment shader to trace

layout (location = 0) in vec3 vertexPosition; // Which corner of the quad

layout (location = 3) in int   nodeIndex; // Indexes the node geometry

layout (location = 4) in int   component; // The component index

layout (location = 6) in vec4  outerColor; // The outside color of the node
layout (location = 7) in vec4  innerColor; // The inside color of the node
layout (location = 8) in float selected;

flat out float element;
flat out vec3  innerVColor;
flat out vec3  outerVColor;
flat out float vSelected;
flat out vec2  lightPlacement;
flat out float projectionScale;

flat out vec3  nodeCentre; // In view space
flat out float nodeRadius;
flat out mat2  zToClip;

out vec3 impostorPosition; // In view space

#ifdef NODE_DEPTH_PREPASS
out vec2 quadCoordinate;
#endif

// Outlines are measured at the width of the widest edge, so that a node's
// matches an edge's at the same depth
uniform float widestEdgeSize;

#include "componentdata.glsl"
#include "element.glsl"
#include "nodegeometry.glsl"
#include "nodeimpostor.glsl"
#include "projection.glsl"

void main()
{
    element = nodeElement(gl_InstanceID);

    mat4 modelViewMatrix = componentModelViewMatrix(component);
    mat4 projectionMatrix = componentProjectionMatrix(component);
    lightPlacement = componentLightPlacement(component);

    outerVColor = outerColor.rgb;
    innerVColor = innerColor.rgb;
    vSelected = selected;

    nodeCentre = (modelViewMatrix * vec4(nodePosition(nodeIndex), 1.0)).xyz;
    nodeRadius = nodeSize(nodeIndex);
    zToClip = zToClipOf(projectionMatrix);

    // Seen from the eye, the sphere fills a cone, which is wider where it
    // passes the sphere's centre than the sphere itself is
    vec3 towards = towardsEye(zToClip, nodeCentre);
    float eyeDistance = max(length(towards), 0.0001);
    float wPerZ = zToClip[0].y;
    float silhouetteRadius = nodeRadius * eyeDistance /
        sqrt(max((eyeDistance * eyeDistance) - (nodeRadius * nodeRadius * wPerZ * wPerZ), 0.0001));

    vec3 facing = towards / eyeDistance;
    vec3 right, up;
    facingBasis(facing, right, up);

    impostorPosition = nodeCentre + (silhouetteRadius * ((right * vertexPosition.x) + (up * vertexPosition.y)));
    gl_Position = projectionMatrix * vec4(impostorPosition, 1.0);

#ifdef NODE_DEPTH_PREPASS
    quadCoordinate = vertexPosition.xy;
#endif

    projectionScale = projectionScaleFor(projectionMatrix, nodeCentre,
        widestEdgeSize > 0.0 ? widestEdgeSize : nodeRadius);
}
