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

// Draws an edge too narrow to trace by how much of each pixel its shaft covers

#include "lighting.glsl"
#include "outline.glsl"

flat in float vSelected;
in float projectionScale;

uniform vec4 outlineColor;

flat in vec3 averagedOuterColor;
flat in vec3 averagedInnerColor;
in vec3 shaftCoordinate;
in float coverageWeight;
flat in vec2 acrossScreen;

// For SplatCompositor
layout (location = 0) out vec4 outCoveredColor;
layout (location = 1) out vec4 outCoverage;
layout (location = 2) out vec4 outOverlap;

// How much of a pixel's box lies within an offset across the shaft from its middle
float boxWithin(float offset)
{
    float wider = max(abs(acrossScreen.x), abs(acrossScreen.y));
    float narrower = min(abs(acrossScreen.x), abs(acrossScreen.y));

    if(narrower < 0.0001)
        return clamp(0.5 + (offset / wider), 0.0, 1.0);

    // Projected across the shaft, the box is a trapezium
    float outer = (wider + narrower) * 0.5;
    float inner = (wider - narrower) * 0.5;

    if(offset <= -outer)
        return 0.0;

    if(offset < -inner)
        return ((offset + outer) * (offset + outer)) / (2.0 * wider * narrower);

    if(offset <= inner)
        return 0.5 + (offset / wider);

    if(offset < outer)
        return 1.0 - (((outer - offset) * (outer - offset)) / (2.0 * wider * narrower));

    return 1.0;
}

void main()
{
    float fromAxis = shaftCoordinate.x;
    float halfWidth = max(shaftCoordinate.z, 0.00001);
    float covered = boxWithin(halfWidth - fromAxis) - boxWithin(-halfWidth - fromAxis);

    if(covered <= 0.0)
        discard;

    bool inner = shaftCoordinate.y >= 0.0 && shaftCoordinate.y <= 1.0;

    vec3 shaded = inner ? averagedInnerColor : averagedOuterColor;

    // outline.frag counts all the neighbours along a line one pixel wide differing,
    // and three of five along one two wide
    float differing = clamp(1.4 - (0.8 * halfWidth), 0.6, 1.0);
    shaded = mix(shaded, outlineColor.rgb,
        flatness * outlineOpacityInside(halfWidth - abs(fromAxis), differing, projectionScale));

    float weighted = covered * coverageWeight;
    outCoveredColor = vec4(shaded * weighted, -log(max(1.0 - covered, 0.0001)));
    outCoverage = vec4(weighted, weighted * vSelected, covered, covered * covered);

    // The middle of the stretch across the pixel the shaft covers
    float reach = 0.5 * (abs(acrossScreen.x) + abs(acrossScreen.y));
    vec2 where = acrossScreen * 0.5 * (max(-fromAxis - halfWidth, -reach) + min(-fromAxis + halfWidth, reach));
    outOverlap = vec4(where * covered, dot(where, where) * covered, 0.0);
}
