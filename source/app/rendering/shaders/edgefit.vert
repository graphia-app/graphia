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

// A point at each edge's texel, holding the shading across its width as the pixels see it

#include "edgegeometry.glsl"
#include "edgedatatexels.glsl"
#include "lighting.glsl"

// The outer colour's shading across the width, averaged over each third of it
flat out vec3 fitThirds[3];

void main()
{
    drawPointAt(approximateShadingThirdsTexel(), approximateShadingThirdsSize);

    fitThirds[0] = fitThirds[1] = fitThirds[2] = vec3(0.0);

    bool isArrow = edgeType == EDGE_VISUAL_TYPE_ARROW;
    EdgeEnds ends = edgeEnds();
    int detail = detailFor(ends.shaftWidth, isArrow ? 2.0 * ends.shaftWidth : ends.shaftWidth);

    if(ends.length <= 0.0 || (detail != SPLATTED_DETAIL && detail != APPROXIMATE_TRACED_DETAIL))
        return;

    edgeAxis = (ends.target - ends.source) / ends.length;

    vec3 onAxis = mix(ends.source, ends.target, 0.5);
    vec3 across, facing;
    crossSectionFacingEye(onAxis, across, facing);

    vec3 thirds[3];
    adsModelAcrossCylinderThirds(onAxis, across, facing, outerColor.rgb, lightPlacement, thirds);

    for(int i = 0; i < 3; i++)
        fitThirds[i] = flattened(thirds[i], outerColor.rgb);
}
