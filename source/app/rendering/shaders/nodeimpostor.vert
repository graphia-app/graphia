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

// A camera facing quad over the node's sphere

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

#ifdef NODE_COVERAGE
const bool DRAWS_NARROW_NODES = true;

flat out vec3 coveredDisc; // Where on screen, and its radius, in pixels
flat out vec2 coverageDepths;
flat out vec3 averagedColor;

#include "lighting.glsl"

// A pixel partly covered by the disc can be this far from it
const float COVERAGE_MARGIN = 0.75;
#else
const bool DRAWS_NARROW_NODES = false;
#endif

// Outlines are measured at the width of the widest edge, so that a node's
// matches an edge's at the same depth
uniform float widestEdgeSize;

// Narrower than this on screen, a node is drawn by its coverage
uniform float minimumTracedWidthPixels;

#include "componentdata.glsl"
#include "splatcompositor.glsl"
#include "element.glsl"
#include "nodegeometry.glsl"
#include "nodeimpostor.glsl"
#include "projection.glsl"
#include "viewport.glsl"

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

    vec4 centreClip = projectionMatrix * vec4(nodeCentre, 1.0);
    float pixelsPerUnit = focalLengthPixels(projectionMatrix) / max(abs(centreClip.w), 0.0001);
    bool narrow = 2.0 * nodeRadius * pixelsPerUnit < minimumTracedWidthPixels;

    // Moved outside the view, since a point is drawn wherever it lands
    if(narrow != DRAWS_NARROW_NODES)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return;
    }

    vec3 facing = towards / eyeDistance;
    vec3 right, up;
    facingBasis(facing, right, up);

#ifdef NODE_COVERAGE
    impostorPosition = nodeCentre;

    vec2 centrePixel = (centreClip.xy / centreClip.w) * viewportSize * 0.5;
    float radiusPixels = silhouetteRadius * pixelsPerUnit;
    coveredDisc = vec3(centrePixel + (viewportSize * 0.5), radiusPixels);
    coverageDepths = coverageDepthsOf(component);

#if !defined(NODE_DEPTHS) && !defined(NODE_OCCLUSION)
    // Shaded as a sphere, averaged over four points that integrate a cubic over
    // the disc exactly, each clipped at 1 first as a traced picture's would be
    vec3 color = mix(outerVColor, innerVColor, DOT_AREA);
    averagedColor = vec3(0.0);

    for(int i = 0; i < 4; i++)
    {
        vec2 onDisc = vec2((i % 2 == 0) ? 0.5 : -0.5, (i < 2) ? 0.5 : -0.5);
        vec3 normal = (right * onDisc.x) + (up * onDisc.y) + (facing * sqrt(0.5));
        vec3 lit = min(adsModel(nodeCentre + (normal * nodeRadius), normal, color, lightPlacement), 1.0);
        averagedColor += flattened(lit, color);
    }

    averagedColor *= 0.25;
#endif

    // A point sprite, which is a quarter of the vertices a quad is, at the
    // sphere's near side, so that what leaves its surface is behind it
    gl_Position = projectionMatrix * vec4(nodeCentre + (facing * nodeRadius), 1.0);
    gl_PointSize = 2.0 * (radiusPixels + COVERAGE_MARGIN);
#else
    impostorPosition = nodeCentre + (silhouetteRadius * ((right * vertexPosition.x) + (up * vertexPosition.y)));
    gl_Position = projectionMatrix * vec4(impostorPosition, 1.0);
#endif

#ifdef NODE_DEPTH_PREPASS
    quadCoordinate = vertexPosition.xy;
#endif

    projectionScale = projectionScaleFor(projectionMatrix, nodeCentre,
        widestEdgeSize > 0.0 ? widestEdgeSize : nodeRadius);
}
