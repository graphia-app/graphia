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

// An edge is two quads, one each for its shaft and head, each as small as covers it
// An edge's width on screen picks the one build that draws it; the others collapse it

#if defined(EDGE_COVERAGE) && defined(EDGE_ACROSS)
#define DRAWN_DETAIL SPLATTED_DETAIL
#elif defined(EDGE_COVERAGE)
#define DRAWN_DETAIL FAST_SPLATTED_DETAIL
#elif defined(EDGE_APPROXIMATE_TRACED)
#define DRAWN_DETAIL APPROXIMATE_TRACED_DETAIL
#elif defined(EDGE_TRACED)
#define DRAWN_DETAIL TRACED_DETAIL
#else
#error "No edge detail is defined"
#endif

flat out float element;
flat out vec3  innerVColor;
flat out vec3  outerVColor;
flat out float vSelected;
out float projectionScale;

out vec3 impostorPosition; // In view space

#include "edgegeometry.glsl"
#include "edgedatatexels.glsl"
#include "element.glsl"

#ifdef EDGE_ACROSS
// What the fit build found, a texel per edge in each of three textures
uniform sampler2D approximateShadingThirds0;
uniform sampler2D approximateShadingThirds1;
uniform sampler2D approximateShadingThirds2;

// The outer colour's shading across the width, as a + b u + c u², u the
// fraction of the half width across, fitted to its average over each third
flat out vec3 shadingAcross[3];

// A quadratic with the averages over the thirds of the width the fit build found
void readShadingAcross()
{
    ivec2 texel = approximateShadingThirdsTexel();
    vec3 first = texelFetch(approximateShadingThirds0, texel, 0).rgb;
    vec3 second = texelFetch(approximateShadingThirds1, texel, 0).rgb;
    vec3 third = texelFetch(approximateShadingThirds2, texel, 0).rgb;

    shadingAcross[2] = 1.125 * (first + third - (2.0 * second));
    shadingAcross[1] = 0.75 * (third - first);
    shadingAcross[0] = second - (shadingAcross[2] / 27.0);
}
#endif

#ifdef EDGE_COVERAGE
#include "splatcompositor.glsl"
#include "lighting.glsl"

#ifndef EDGE_ACROSS
flat out vec3 averagedOuterColor;
#endif

flat out vec3 averagedInnerColor;
out vec3 shaftCoordinate;
flat out vec2 coverageDepths;

// The direction across the shaft on screen
flat out vec2 acrossScreen;

vec3 averagedColor(vec3 color, vec3 onAxis)
{
    vec3 across, facing;
    crossSectionFacingEye(onAxis, across, facing);

    return flattened(adsModelAcrossCylinder(onAxis, across, facing, color, lightPlacement), color);
}
#else
// The part's cross-section about a point on its axis; the head's narrows to
// nothing at its apex
CrossSection crossSectionAt(EdgePart part, bool isHead, vec3 centre)
{
    float radius = isHead ? headRadius * min(distance(centre, headApex) / headLength, 1.0) : part.radius;
    return CrossSection(centre, radius);
}

// The span of a cross-section's outline along a direction on screen: where the
// lines across that direction that touch it are, the roots of a quadratic
vec2 outlineSpan(mat4 projectionMatrix, CrossSection section, vec2 direction)
{
    // How far along direction a point is, in pixels, is the ratio of these
    vec4 projected = vec4(direction * viewportSize * 0.5, 0.0, 0.0) * projectionMatrix;
    vec4 w = vec4(0.0, 0.0, 0.0, 1.0) * projectionMatrix;

    float projectedCentre = dot(projected, vec4(section.centre, 1.0));
    float wCentre = dot(w, vec4(section.centre, 1.0));

    // Around the circle, only the components square to the axis vary
    vec3 projectedAcross = projected.xyz - (edgeAxis * dot(projected.xyz, edgeAxis));
    vec3 wAcross = w.xyz - (edgeAxis * dot(w.xyz, edgeAxis));
    float radiusSquared = section.radius * section.radius;

    float a = (wCentre * wCentre) - (radiusSquared * dot(wAcross, wAcross));
    float halfB = (projectedCentre * wCentre) - (radiusSquared * dot(projectedAcross, wAcross));
    float widestUseful = length(viewportSize);

    // Some of the circle is behind the eye, so its outline has no bound
    if(a <= 0.0)
        return vec2(-widestUseful, widestUseful);

    // The discriminant, factored so that it doesn't cancel
    vec3 gradient = (wCentre * projectedAcross) - (projectedCentre * wAcross);
    vec3 skew = cross(projectedAcross, wAcross);
    float spread = section.radius * sqrt(max(dot(gradient, gradient) - (radiusSquared * dot(skew, skew)), 0.0));

    return clamp(vec2(halfB - spread, halfB + spread) / a, -widestUseful, widestUseful);
}

// The span along a direction on screen of the shape between two cross-sections,
// which lies within their outlines and what joins them
vec2 stretchSpan(mat4 projectionMatrix, CrossSection from, CrossSection to, vec2 direction)
{
    vec2 fromSpan = outlineSpan(projectionMatrix, from, direction);
    vec2 toSpan = outlineSpan(projectionMatrix, to, direction);

    return vec2(min(fromSpan.x, toSpan.x), max(fromSpan.y, toSpan.y));
}

// Whether some of a cross-section is beyond the near plane, where its outline
// on screen may have no bound
bool reachesPastNearPlane(CrossSection section, float nearZ)
{
    return section.centre.z + (section.radius * length(edgeAxis.xy)) > nearZ;
}

// The span along a direction on screen of what is in front of the near plane
// of a box about the axis between two points, which holds the shape there
vec2 nearClippedSpan(mat4 projectionMatrix, vec3 from, vec3 to, float radius, vec2 direction, float nearZ)
{
    // Across the axis, only v moves towards or away from the eye
    vec3 u = cross(edgeAxis, vec3(0.0, 0.0, 1.0));
    u = length(u) > 0.0001 ? normalize(u) : vec3(1.0, 0.0, 0.0);
    vec3 v = cross(edgeAxis, u);

    vec3 corners[4] = vec3[4](from - (v * radius), to - (v * radius), to + (v * radius), from + (v * radius));
    vec2 span = vec2(length(viewportSize), -length(viewportSize));

    for(int i = 0; i < 4; i++)
    {
        vec3 corner = corners[i];
        vec3 nextCorner = corners[(i + 1) % 4];
        bool cornerInFront = corner.z <= nearZ;
        bool nextCornerInFront = nextCorner.z <= nearZ;

        vec3 kept[2];
        int numKept = 0;

        if(cornerInFront)
            kept[numKept++] = corner;

        if(cornerInFront != nextCornerInFront)
            kept[numKept++] = mix(corner, nextCorner, (nearZ - corner.z) / (nextCorner.z - corner.z));

        for(int k = 0; k < numKept; k++)
        {
            float lower = dot(pixelPosition(projectionMatrix, kept[k] - (u * radius)), direction);
            float upper = dot(pixelPosition(projectionMatrix, kept[k] + (u * radius)), direction);
            span = vec2(min(span.x, min(lower, upper)), max(span.y, max(lower, upper)));
        }
    }

    return clamp(span, -length(viewportSize), length(viewportSize));
}
#endif

#ifdef EDGE_TRACED
// What the cull found; the depth prepass reads the last walk's
uniform sampler2D visibleRanges;
uniform bool hasVisibleRanges;

vec2 visibleRangeOfPart(bool isHead)
{
    return texelFetch(visibleRanges, visibleRangeTexel(isHead), 0).rg;
}
#endif

void collapse()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
    element = edgeElement(gl_InstanceID);

    bool isArrow = edgeType == EDGE_VISUAL_TYPE_ARROW;
    bool isHead = vertexPosition.z > 0.5;

    if(isHead && !isArrow)
    {
        collapse();
        return;
    }

#ifdef EDGE_DEPTH_PREPASS
    // A part the last walk found hidden is very likely hidden still
    // Leaving it out of the prepass can only make the cull find less
    if(hasVisibleRanges && visibleRangeEmpty(visibleRangeOfPart(isHead)))
    {
        collapse();
        return;
    }
#endif

    EdgeEnds ends = edgeEnds();

    outerVColor = outerColor.rgb;
    innerVColor = innerColor.rgb;
    vSelected = selected;

    int detail = detailFor(ends.shaftWidth, isArrow ? 2.0 * ends.shaftWidth : ends.shaftWidth);

    PartOnScreen onScreen;

    if(ends.length <= 0.0 || detail != DRAWN_DETAIL || !partOnScreen(ends, isHead, isArrow, onScreen))
    {
        collapse();
        return;
    }

    mat4 projectionMatrix = ends.projectionMatrix;
    EdgePart part = onScreen.part;

    vec2 between = onScreen.pixelTo - onScreen.pixelFrom;
    vec2 along = length(between) > 0.0001 ? normalize(between) : vec2(1.0, 0.0);
    vec2 across = vec2(along.y, -along.x);

    // The stretch of the part the quad covers, in fractions of it on screen
    vec2 stretch = vec2(0.0, 1.0);

#if defined(EDGE_TRACED) && !defined(EDGE_DEPTH_PREPASS)
    if(hasVisibleRanges)
    {
        // The quad covers only the stretch that may be seen, but the shape
        // traced through it is the whole of it
        stretch = visibleRangeOfPart(isHead);

        if(visibleRangeEmpty(stretch))
        {
            collapse();
            return;
        }
    }
#endif

    float end = mix(stretch.x, stretch.y, (vertexPosition.y + 1.0) * 0.5);
    vec3 onAxis = mix(part.from, part.to, end);

#ifdef EDGE_COVERAGE
    float endRadius = mix(onScreen.radiusFrom, onScreen.radiusTo, end);
    float quadRadius = endRadius + (0.5 * (abs(across.x) + abs(across.y)));
    float side = vertexPosition.x;
    vec2 cornerPixel = mix(onScreen.pixelFrom, onScreen.pixelTo, end) + (across * (side * quadRadius));

    float innerColorLength = innerColorStretch.y - innerColorStretch.x;
    float alongInnerColor = innerColorLength > 0.0 ?
        (dot(onAxis - shaftBase, edgeAxis) - innerColorStretch.x) / innerColorLength : -1.0;

    shaftCoordinate = vec3(side * quadRadius, alongInnerColor, endRadius);
    acrossScreen = across;

#ifdef EDGE_ACROSS
    readShadingAcross();
    vec3 averagedOuterColor = shadingAcross[0] + (shadingAcross[2] / 3.0);
#else
    averagedOuterColor = averagedColor(outerColor.rgb, onAxis);
#endif

    averagedInnerColor = innerColor.rgb == outerColor.rgb ? averagedOuterColor : averagedColor(innerColor.rgb, onAxis);
#else
    // As little as covers the shape between the cross-sections at each end
    CrossSection fromSection = crossSectionAt(part, isHead, axisPointAt(part, stretch.x));
    CrossSection toSection = crossSectionAt(part, isHead, axisPointAt(part, stretch.y));
    vec2 alongSpan;
    vec2 acrossSpan;

    if(reachesPastNearPlane(fromSection, onScreen.nearZ) || reachesPastNearPlane(toSection, onScreen.nearZ))
    {
        // Where the part was cut at the near plane, its surface reaches in
        // front of it from further back along the axis
        vec3 boxFrom = stretch.x <= 0.0 ? onScreen.uncutPart.from : fromSection.centre;
        vec3 boxTo = stretch.y >= 1.0 ? onScreen.uncutPart.to : toSection.centre;

        alongSpan = nearClippedSpan(projectionMatrix, boxFrom, boxTo, part.radius, along, onScreen.nearZ);
        acrossSpan = nearClippedSpan(projectionMatrix, boxFrom, boxTo, part.radius, across, onScreen.nearZ);
    }
    else
    {
        alongSpan = stretchSpan(projectionMatrix, fromSection, toSection, along);
        acrossSpan = stretchSpan(projectionMatrix, fromSection, toSection, across);
    }

    vec2 cornerPixel = (along * (vertexPosition.y < 0.0 ? alongSpan.x : alongSpan.y)) +
        (across * (vertexPosition.x < 0.0 ? acrossSpan.x : acrossSpan.y));
#endif

#ifdef EDGE_TRACED
    float cornerZ = onAxis.z;
#else
    // The depth is interpolated, which is nearest right on the shape's near surface
    float cornerZ = min(onAxis.z + part.radius, onScreen.nearZ);
#endif

    impostorPosition = unproject(projectionMatrix, zToClip, cornerPixel / (viewportSize * 0.5), cornerZ);
    gl_Position = projectionMatrix * vec4(impostorPosition, 1.0);

#ifdef EDGE_COVERAGE
    coverageDepths = coverageDepthsOf(component);
#endif

    projectionScale = projectionScaleFor(projectionMatrix, onAxis, size);
}
