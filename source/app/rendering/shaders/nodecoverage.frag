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

// Draws a node too small to trace by how much of each pixel its disc covers

#include "lighting.glsl"
#include "nodeimpostor.glsl"
#include "projection.glsl"
#include "outline.glsl"

flat in vec3  innerVColor;
flat in vec3  outerVColor;
flat in float vSelected;
flat in vec2  lightPlacement;
flat in float projectionScale;

flat in vec3  nodeCentre;
flat in float nodeRadius;
flat in mat2  zToClip;

in vec3 impostorPosition;

uniform vec4 outlineColor;

flat in vec3 coveredDisc;
flat in float coverageWeight;
flat in vec3 averagedColor;

// For SplatCompositor
layout (location = 0) out vec4 outCoveredColor;
layout (location = 1) out vec4 outCoverage;
layout (location = 2) out vec4 outOverlap;

// How much of a pixel's width a span covers, the span's middle being offset from the pixel's
float boxOverlap(float offset, float width)
{
    return clamp(min(offset + (width * 0.5), 0.5) - max(offset - (width * 0.5), -0.5), 0.0, 1.0);
}

float pixelCoverage(vec2 fromCentre, float radius)
{
    float side = radius * sqrt(PI);
    float asSquare = boxOverlap(fromCentre.x, side) * boxOverlap(fromCentre.y, side);

    // A pixel wide, and narrowed to keep the disc's area
    float rampRadius = sqrt(max((radius * radius) - (1.0 / 12.0), 0.25));
    float asDisc = clamp(rampRadius + 0.5 - length(fromCentre), 0.0, 1.0);

    return mix(asSquare, asDisc, smoothstep(0.5, 1.0, radius));
}

void main()
{
    vec2 fromCentre = gl_FragCoord.xy - coveredDisc.xy;
    float radius = coveredDisc.z;
    float covered = pixelCoverage(fromCentre, radius);

    if(covered <= 0.0)
        discard;

    // Where the pixel is over little of the disc the local shading shows, but a disc
    // under half a pixel across is only ever averaged
    float ofDisc = min(covered / (PI * radius * radius), 1.0);
    float local = (1.0 - (ofDisc * ofDisc)) * smoothstep(0.5, 1.5, 2.0 * radius);
    vec3 shaded = averagedColor;

    if(local > 0.0)
    {
        vec3 facing = normalize(towardsEye(zToClip, nodeCentre));
        vec3 right, up;
        facingBasis(facing, right, up);

        // A pixel past the rim is shaded as the disc is just inside it
        vec2 onDisc = fromCentre / radius;
        onDisc *= min(1.0, 0.95 / max(length(onDisc), 0.0001));
        vec3 normal = (right * onDisc.x) + (up * onDisc.y) + (facing * sqrt(1.0 - dot(onDisc, onDisc)));
        vec3 color = dot(onDisc, onDisc) < DOT_AREA ? innerVColor : outerVColor;
        vec3 lit = min(adsModel(nodeCentre + (normal * nodeRadius), normal, color, lightPlacement), 1.0);
        shaded = mix(shaded, flattened(lit, color), local);
    }

    // As for edges: every neighbour differs on a disc two pixels across
    // This falls to three of five by about five across
    float differing = clamp(1.3 - (0.3 * radius), 0.6, 1.0);
    float inside = radius - length(fromCentre);
    shaded = mix(shaded, outlineColor.rgb, flatness * outlineOpacityInside(inside, differing, projectionScale));

    // Blending by coverage can't tell that a node is in front of its edges' ends,
    // so the more of a pixel it covers the more it counts, up to about as solid
    // as it is once traced
    const float SOLIDITY = 8.0;
    float weighted = covered * coverageWeight * (1.0 + (SOLIDITY * smoothstep(0.5, 1.0, covered)));
    outCoveredColor = vec4(shaded * weighted, -log(max(1.0 - covered, 0.0001)));
    outCoverage = vec4(weighted, weighted * vSelected, covered, covered * covered);

    // The middle of the part of the pixel the disc's square covers
    float side = radius * sqrt(PI);
    vec2 where = 0.5 * (max(-fromCentre - (side * 0.5), -0.5) + min(-fromCentre + (side * 0.5), 0.5));
    outOverlap = vec4(where * covered, dot(where, where) * covered, 0.0);
}
