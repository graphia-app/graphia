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

// The walk along the depth pyramid that finds, for each part of each traced
// edge, the stretch of it that may be seen; a point for each part

#include "edgegeometry.glsl"
#include "edgedatatexels.glsl"

uniform sampler2D depthPyramid;
uniform int depthPyramidLevels;

flat out vec2 visibleRange;

// A part of an edge as the walk sees it, in pixels from the corner of the view
struct Walk
{
    EdgePart part;
    vec2 fromPixel;
    vec2 toPixel;
    float widthPixels;
    float nearZ;
};

// Longer parts are walked in coarser steps, so the walk costs the same for any length
const int MAX_WALK_STEPS = 32;

// A patch this many texels square covers anything spanning one fewer, however it falls
const int PATCH_SIZE = 6;

int levelFor(Walk walk, float stretchPixels)
{
    float texelPixels = (walk.widthPixels + stretchPixels) / float(PATCH_SIZE - 1);
    return int(clamp(ceil(log2(max(texelPixels, 1.0))), 0.0, float(depthPyramidLevels - 1)));
}

// Whether a stretch of the part is certainly behind what is already drawn: the
// nearest it comes is further than the furthest of everything it could cover
bool stretchHidden(Walk walk, float from, float to, int level)
{
    float nearerEndZ = max(axisPointAt(walk.part, from).z, axisPointAt(walk.part, to).z);
    float nearestZ = min(nearerEndZ + walk.part.radius, walk.nearZ);

    vec2 a = mix(walk.fromPixel, walk.toPixel, from);
    vec2 b = mix(walk.fromPixel, walk.toPixel, to);
    float texelPixels = exp2(float(level));
    ivec2 firstTexel = ivec2(floor((min(a, b) - (walk.widthPixels * 0.5)) / texelPixels));
    ivec2 lastTexel = ivec2(floor((max(a, b) + (walk.widthPixels * 0.5)) / texelPixels));
    ivec2 levelLastTexel = textureSize(depthPyramid, level) - ivec2(1);

    float furthest = 0.0;

    for(int y = 0; y < PATCH_SIZE; y++)
    {
        for(int x = 0; x < PATCH_SIZE; x++)
        {
            ivec2 texel = clamp(min(firstTexel + ivec2(x, y), lastTexel), ivec2(0), levelLastTexel);
            furthest = max(furthest, texelFetch(depthPyramid, texel, level).r);
        }
    }

    return depthAt(zToClip, nearestZ) > furthest;
}

vec2 visibleRangeOf(Walk walk)
{
    float lengthPixels = distance(walk.fromPixel, walk.toPixel);

    // In a dense view most parts are wholly hidden, which one test of the whole part finds
    if(stretchHidden(walk, 0.0, 1.0, levelFor(walk, lengthPixels)))
        return HIDDEN_RANGE;

    int level = levelFor(walk, lengthPixels / float(MAX_WALK_STEPS));
    float stepPixels = max((float(PATCH_SIZE - 1) * exp2(float(level))) - walk.widthPixels, 1.0);
    int steps = int(clamp(ceil(lengthPixels / stepPixels), 1.0, float(MAX_WALK_STEPS)));

    // Only the outermost stretches that may be seen decide the range, so the
    // walk comes in from each end and stops at the first it finds
    int first = steps;

    for(int stretch = 0; stretch < steps; stretch++)
    {
        if(!stretchHidden(walk, float(stretch) / float(steps), float(stretch + 1) / float(steps), level))
        {
            first = stretch;
            break;
        }
    }

    if(first == steps)
        return HIDDEN_RANGE;

    int last = first;

    for(int stretch = steps - 1; stretch > first; stretch--)
    {
        if(!stretchHidden(walk, float(stretch) / float(steps), float(stretch + 1) / float(steps), level))
        {
            last = stretch;
            break;
        }
    }

    return vec2(float(first), float(last + 1)) / float(steps);
}

void main()
{
    bool isArrow = edgeType == EDGE_VISUAL_TYPE_ARROW;
    bool isHead = gl_VertexID == 1;

    drawPointAt(visibleRangeTexel(isHead), visibleRangesSize);
    visibleRange = HIDDEN_RANGE;

    if(isHead && !isArrow)
        return;

    EdgeEnds ends = edgeEnds();
    int detail = detailFor(ends.shaftWidth, isArrow ? 2.0 * ends.shaftWidth : ends.shaftWidth);

    PartOnScreen onScreen;

    if(ends.length <= 0.0 || detail != TRACED_DETAIL || !partOnScreen(ends, isHead, isArrow, onScreen))
        return;

    float widestRadius = max(onScreen.radiusFrom, onScreen.radiusTo);
    visibleRange = visibleRangeOf(Walk(onScreen.part, onScreen.pixelFrom + (viewportSize * 0.5),
        onScreen.pixelTo + (viewportSize * 0.5), (2.0 * widestRadius) + 1.0, onScreen.nearZ));
}
