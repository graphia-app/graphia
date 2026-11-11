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

#ifdef GL_ES
// Element numbers are too large for medium precision to hold exactly
precision highp float;
#endif

layout (location = 0) out vec4 fragColor;

#include "outline.glsl"
#include "smaablending.glsl"

uniform sampler2D elementTexture;
uniform sampler2D blendWeights;

uniform vec4 outlineColor;
uniform float alpha;

// Whether the picture is to be blended across its silhouettes, by blendWeights
uniform bool smoothed;

vec2 elementAt(ivec2 p)
{
    ivec2 highest = textureSize(elementTexture, 0) - ivec2(1);
    return texelFetch(elementTexture, clamp(p, ivec2(0), highest), 0).rg;
}

// The elements a pixel shows once blended across the silhouettes
void shownAt(ivec2 p, out vec3 elements, out vec3 projectionScales, out vec3 amounts)
{
    vec2 own = elementAt(p);
    elements = vec3(own.r);
    projectionScales = vec3(own.g);
    amounts = vec3(1.0, 0.0, 0.0);

    if(!smoothed)
        return;

    vec4 a = smaaBlendingWeights(blendWeights, p);

    if(dot(a, vec4(1.0)) < 1e-5)
        return;

    bool horizontal = smaaBlendsHorizontally(a);
    vec2 weights = horizontal ? a.xz : a.yw;
    ivec2 along = horizontal ? ivec2(1, 0) : ivec2(0, 1);

    // Each neighbour is read part of the way towards it, by its share of the blend
    vec2 toward = (weights * weights) / dot(weights, vec2(1.0));

    vec2 positive = elementAt(p + along);
    vec2 negative = elementAt(p - along);

    elements = vec3(own.r, positive.r, negative.r);
    projectionScales = vec3(own.g, positive.g, negative.g);
    amounts = vec3(1.0 - toward.x - toward.y, toward.x, toward.y);
}

// How much of what two pixels will show is the same element
float sameShown(vec3 elements, vec3 amounts, vec3 otherElements, vec3 otherAmounts)
{
    float same = 0.0;

    for(int k = 0; k < 3; k++)
    {
        for(int l = 0; l < 3; l++)
        {
            if(elements[k] == otherElements[l])
                same += amounts[k] * otherAmounts[l];
        }
    }

    return same;
}

float edgeStrengthAt(ivec2 p)
{
    // What a pixel and its neighbours show is blended from no further than
    // two pixels away, so where all of that is one element there is no outline
    if(smoothed)
    {
        float own = elementAt(p).r;
        bool alone = true;

        for(int i = -2; i <= 2; i++)
        {
            for(int j = -2; j <= 2; j++)
                alone = alone && elementAt(p + ivec2(i, j)).r == own;
        }

        if(alone)
            discard;
    }

    vec3 elements, projectionScales, amounts;
    shownAt(p, elements, projectionScales, amounts);

    // How many neighbours are likely to show another element than the pixel
    float numDiffPixels = 0.0;
    float projectionScale = 0.0;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            vec3 neighbourElements, neighbourProjectionScales, neighbourAmounts;
            shownAt(p + ivec2(i, j), neighbourElements, neighbourProjectionScales, neighbourAmounts);

            float same = sameShown(elements, amounts, neighbourElements, neighbourAmounts);

            for(int k = 0; k < 3; k++)
            {
                if(neighbourAmounts[k] > 0.0)
                    projectionScale = max(projectionScale, neighbourProjectionScales[k]);
            }

            numDiffPixels += max(1.0 - same, 0.0);
        }
    }

    if(numDiffPixels < 1e-4)
        discard;

    // Clamp to a maximum difference, otherwise we get abnormally dark
    // spots at the junction of many elements
    const float maxNumDiffPixels = 5.0;
    numDiffPixels = min(numDiffPixels, maxNumDiffPixels);

    return outlineOpacity(numDiffPixels / maxNumDiffPixels, outlineProjection(projectionScale));
}

float wideEdgeStrengthAt(ivec2 p, int apart)
{
    vec2 own = elementAt(p);

    // The nearest pixel of another element, as far out as anything shown within
    // the outline's pixel can be blended from
    int reach = smoothed ? apart + 1 : apart;
    float nearest = float(reach) + 1.0;
    ivec2 towards = ivec2(0);

    for(int i = -reach; i <= reach; i++)
    {
        for(int j = -reach; j <= reach; j++)
        {
            float d = length(vec2(i, j));

            if(d < nearest && elementAt(p + ivec2(i, j)).r != own.r)
            {
                nearest = d;
                towards = ivec2(i, j);
            }
        }
    }

    if(nearest > float(reach) || (!smoothed && nearest > float(apart)))
        discard;

    // How much of the outline's pixel about it is likely to show another element
    float within = 1.0;

    if(smoothed)
    {
        vec3 elements, projectionScales, amounts;
        shownAt(p, elements, projectionScales, amounts);
        within = 0.0;

        for(int i = -apart; i <= apart; i++)
        {
            for(int j = -apart; j <= apart; j++)
            {
                if(length(vec2(i, j)) > float(apart))
                    continue;

                vec3 otherElements, otherProjectionScales, otherAmounts;
                shownAt(p + ivec2(i, j), otherElements, otherProjectionScales, otherAmounts);
                within = max(within, 1.0 - sameShown(elements, amounts, otherElements, otherAmounts));
            }
        }

        if(within < 1e-4)
            discard;
    }

    vec2 other = elementAt(p + towards);
    float projectionScale = max(own.g, other.g);

    // Another element the other way, within two of the outline's pixels, makes
    // what is between them a line that thin, with more of its neighbours differing
    vec2 away = -normalize(vec2(towards));
    float differing = 0.6;

    for(int k = 1; k <= 2 * apart; k++)
    {
        vec2 beyond = elementAt(p + ivec2(round(away * float(k))));

        if(beyond.r != own.r)
        {
            float halfWidth = (nearest + float(k) - 1.0) / (2.0 * float(apart));
            differing = clamp(1.4 - (0.8 * halfWidth), 0.6, 1.0);
            projectionScale = max(projectionScale, beyond.g);
            break;
        }
    }

    return within * outlineOpacity(differing, outlineProjection(projectionScale));
}

void main()
{
    int apart = int(outlinePixel());
    float outlineAlpha = apart > 1 ? wideEdgeStrengthAt(ivec2(gl_FragCoord.xy), apart) :
        edgeStrengthAt(ivec2(gl_FragCoord.xy));
    fragColor = vec4(outlineColor.rgb, 1.0) * (outlineAlpha * alpha);
}
