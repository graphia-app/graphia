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

// What the edge shader's builds share: the edge's attributes, its shape in view
// space, and where each of its parts lies on screen

layout (location = 0) in vec3 vertexPosition; // x: which side, y: which end, z: 0 shaft, 1 head

layout (location = 3) in int    sourceIndex; // Indexes the source node's geometry
layout (location = 4) in int    targetIndex; // Indexes the target node's geometry
layout (location = 7) in int    edgeType; // The type of the edge

layout (location = 8) in int    component; // The component index

layout (location = 9)  in float size; // The size of the edge
layout (location = 10) in vec4  outerColor; // The outside color of the edge
layout (location = 11) in vec4  innerColor; // The inside color of the edge (used for multi edges)
layout (location = 12) in float selected;

flat out vec2  lightPlacement;
flat out mat2  zToClip;

// The shape, in view space
flat out vec3  shaftBase;
flat out vec3  edgeAxis;
flat out float shaftRadius;
flat out float shaftLength;
flat out vec3  headApex;
flat out float headLength;
flat out float headRadius;

// The stretch of the axis, measured from shaftBase, drawn in the inner colour
flat out vec2  innerColorStretch;

#include "componentdata.glsl"
#include "nodegeometry.glsl"
#include "projection.glsl"
#include "viewport.glsl"

uniform float minimumApproximateTracedWidthPixels;
uniform float minimumTracedWidthPixels;

const int FAST_SPLATTED_DETAIL = 0;
const int APPROXIMATE_TRACED_DETAIL = 1;
const int TRACED_DETAIL = 2;

const int EDGE_VISUAL_TYPE_ARROW = 1;

// A stretch of the edge's axis, and the radius of the cylinder that contains
// the shape along it
struct EdgePart
{
    vec3 from;
    vec3 to;
    float radius;
};

// The circle where the part is cut square to its axis
struct CrossSection
{
    vec3 centre;
    float radius;
};

int detailFor(float shaftWidth, float widestWidth)
{
    if(widestWidth < minimumApproximateTracedWidthPixels)
        return FAST_SPLATTED_DETAIL;

    if(shaftWidth < minimumTracedWidthPixels)
        return APPROXIMATE_TRACED_DETAIL;

    return TRACED_DETAIL;
}

// How far from a node's centre a cylinder of the given radius leaves its surface
float surfaceInset(float nodeRadius, float radius)
{
    return sqrt(max((nodeRadius * nodeRadius) - (radius * radius), 0.0));
}

bool clipToNearPlane(inout EdgePart part, float nearZ)
{
    if(part.from.z > nearZ && part.to.z > nearZ)
        return false;

    if(part.from.z > nearZ)
        part.from = mix(part.from, part.to, (nearZ - part.from.z) / (part.to.z - part.from.z));
    else if(part.to.z > nearZ)
        part.to = mix(part.to, part.from, (nearZ - part.to.z) / (part.from.z - part.to.z));

    return true;
}

vec2 pixelPosition(mat4 projectionMatrix, vec3 position)
{
    vec4 clip = projectionMatrix * vec4(position, 1.0);
    return (clip.xy / clip.w) * (viewportSize * 0.5);
}

// How far from its projected centre a sphere's outline reaches on screen, which
// away from the middle of the view is further than its angular size alone says
float outlineRadiusPixels(vec3 centre, float radius, float pixelsPerUnit)
{
    vec3 towards = towardsEye(zToClip, centre);
    float wPerZ = zToClip[0].y;
    float widestUseful = length(viewportSize);

    float eyeDistance = max(length(towards), 0.0001);
    float tangentDistance = sqrt(max((eyeDistance * eyeDistance) - (radius * radius * wPerZ * wPerZ), 0.0001));

    float tanOffAxis = length(towards.xy) / max(towards.z, 0.0001);
    float tanRadius = radius * abs(wPerZ) / tangentDistance;

    // The outline's far side runs off to infinity as it turns parallel to the screen
    float closing = 1.0 - (tanOffAxis * tanRadius);

    if(closing <= 0.0)
        return widestUseful;

    return min(radius * pixelsPerUnit * (1.0 + (tanOffAxis * tanOffAxis)) /
        (tangentDistance * closing), widestUseful);
}

// The point on the part's axis a fraction of the way along it on screen
// On screen, 1/w varies evenly along it, but distance doesn't
vec3 axisPointAt(EdgePart part, float fraction)
{
    float wFrom = clipWAt(zToClip, part.from.z);
    float wTo = clipWAt(zToClip, part.to.z);

    return mix(part.from, part.to, (fraction * wFrom) / mix(wTo, wFrom, fraction));
}

// Across the edge, and towards the eye, at a point on its axis and square to it
void crossSectionFacingEye(vec3 onAxis, out vec3 across, out vec3 facing)
{
    vec3 toEye = normalize(towardsEye(zToClip, onAxis));
    facing = normalize(toEye - (edgeAxis * dot(toEye, edgeAxis)));
    across = normalize(cross(edgeAxis, toEye));
}

// The edge's ends in view space, and how wide its shaft is on screen at its
// midpoint, in pixels
struct EdgeEnds
{
    mat4 projectionMatrix;
    vec3 source;
    vec3 target;
    float length;
    float pixelsPerUnit;
    float shaftWidth;
};

EdgeEnds edgeEnds()
{
    EdgeEnds ends;

    mat4 modelViewMatrix = componentModelViewMatrix(component);
    ends.projectionMatrix = componentProjectionMatrix(component);
    lightPlacement = componentLightPlacement(component);
    zToClip = zToClipOf(ends.projectionMatrix);

    ends.source = (modelViewMatrix * vec4(nodePosition(sourceIndex), 1.0)).xyz;
    ends.target = (modelViewMatrix * vec4(nodePosition(targetIndex), 1.0)).xyz;
    ends.length = distance(ends.source, ends.target);
    ends.pixelsPerUnit = focalLengthPixels(ends.projectionMatrix);

    float midpointW = clipWAt(zToClip, mix(ends.source.z, ends.target.z, 0.5));
    ends.shaftWidth = 2.0 * size * ends.pixelsPerUnit / max(abs(midpointW), 0.0001);

    return ends;
}

// Where a part of the edge lies on screen, in pixels from the middle of the
// view, cut where it passes the near plane
struct PartOnScreen
{
    EdgePart part;
    EdgePart uncutPart;
    float nearZ;
    vec2 pixelFrom;
    vec2 pixelTo;
    float radiusFrom;
    float radiusTo;
};

// Lays out the edge's shape, and where the part lies on screen; false where
// none of the part can be seen
bool partOnScreen(EdgeEnds ends, bool isHead, bool isArrow, out PartOnScreen onScreen)
{
    float sourceRadius = nodeSize(sourceIndex);
    float targetRadius = nodeSize(targetIndex);

    edgeAxis = (ends.target - ends.source) / ends.length;
    shaftRadius = size;
    headRadius = 2.0 * size;
    headLength = isArrow ? min(4.0 * size, ends.length * 0.25) : 0.0;

    // Distances along the axis from the source's centre; the shaft starts where it
    // parts from the node's surface, since its open end would show
    float headTo = ends.length - targetRadius;
    float headFrom = headTo - headLength;
    float shaftFrom = surfaceInset(sourceRadius, shaftRadius);
    float shaftTo = isArrow ? headFrom : ends.length - surfaceInset(targetRadius, shaftRadius);

    shaftBase = ends.source + (edgeAxis * shaftFrom);
    shaftLength = max(shaftTo - shaftFrom, 0.0);
    headApex = ends.source + (edgeAxis * headTo);

    // The middle quarter between the nodes, where a multi edge shows its inner colour
    innerColorStretch = mix(vec2(sourceRadius), vec2(ends.length - targetRadius), vec2(0.375, 0.625)) - shaftFrom;

    bool hiddenInNodes = isHead ?
        (headLength <= 0.0 || headTo <= surfaceInset(sourceRadius, headRadius)) :
        (shaftTo <= shaftFrom);

    if(isHead)
        onScreen.part = EdgePart(ends.source + (edgeAxis * headFrom), headApex, headRadius);
    else
        onScreen.part = EdgePart(shaftBase, ends.source + (edgeAxis * shaftTo), shaftRadius);

    onScreen.nearZ = min(nearPlaneZ(zToClip), -0.0001);
    onScreen.uncutPart = onScreen.part;

    if(hiddenInNodes || !clipToNearPlane(onScreen.part, onScreen.nearZ))
        return false;

    // Seen from the eye the part lies within a disc about each end and
    // everything between
    onScreen.pixelFrom = pixelPosition(ends.projectionMatrix, onScreen.part.from);
    onScreen.pixelTo = pixelPosition(ends.projectionMatrix, onScreen.part.to);
    onScreen.radiusFrom = outlineRadiusPixels(onScreen.part.from, onScreen.part.radius, ends.pixelsPerUnit);
    onScreen.radiusTo = outlineRadiusPixels(onScreen.part.to, onScreen.part.radius, ends.pixelsPerUnit);
    float widestRadius = max(onScreen.radiusFrom, onScreen.radiusTo);

    vec2 lowest = min(onScreen.pixelFrom, onScreen.pixelTo) - vec2(widestRadius);
    vec2 highest = max(onScreen.pixelFrom, onScreen.pixelTo) + vec2(widestRadius);

    return !any(lessThan(highest, -viewportSize * 0.5)) && !any(greaterThan(lowest, viewportSize * 0.5));
}
