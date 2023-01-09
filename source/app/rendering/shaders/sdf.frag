/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#version 330 core

// This Shader Generates an SDF map of a large texture
// the resulting texture will be texSize.x/scaleFactor wide and texSize.y/scaleFactor talln
// tex is the texture to SDF
// texSize is the size of the texture in pixels
// scaleFactor is how much smaller the resultant texture will be in height + width

uniform sampler2D tex;
uniform vec2 texSize;
uniform float scaleFactor;

layout(location = 0) out vec4 outColor;

in vec2 vPosition;

bool isIn(vec2 uv)
{
   return (texture(tex, uv)).a > 0.1;
}

float squaredDistanceBetween(vec2 uv1, vec2 uv2)
{
    vec2 delta = uv1 - uv2;
    float dist = (delta.x * delta.x) + (delta.y * delta.y);
    return dist;
}

void main()
{
    // gl_FragCoord is in pixels so will be smaller than texSize
    vec2 scaledFragCoord = gl_FragCoord.xy * scaleFactor;
    vec2 uv = scaledFragCoord / texSize.xy;

    const float range = 8.0;
    // Have to set manually to scaleFactor! (4.0 in this case)
    const int scalediRange = int(range * 4.0);
    float scaledHalfRange = (range / 2.0) * scaleFactor;
    vec2 startPosition = vec2(scaledFragCoord.x - scaledHalfRange, scaledFragCoord.y - scaledHalfRange);

    bool fragIsIn = isIn(uv);
    float squaredDistanceToEdge = (scaledHalfRange * scaledHalfRange) * 2.0;

    for(int dx = 0; dx < scalediRange; dx++)
    {
        for(int dy = 0; dy < scalediRange; dy++)
        {
            vec2 scanPosition = startPosition + vec2(dx, dy);

            bool scanIsIn = isIn(scanPosition / texSize.xy);
            if(scanIsIn != fragIsIn)
            {
                float scanDistance = squaredDistanceBetween(scaledFragCoord.xy, scanPosition);
                if(scanDistance < squaredDistanceToEdge)
                    squaredDistanceToEdge = scanDistance;
            }
        }
    }

    float normalised = squaredDistanceToEdge / (scaledHalfRange * scaledHalfRange) * 2.0;
    float distanceToEdge = sqrt(normalised);
    if(fragIsIn)
        distanceToEdge = -distanceToEdge;
    normalised = 0.5 - (distanceToEdge / 2.0);

    // Uncomment for outline
    //if(normalised > 0.5)
    //   fragColor = vec4(1.0);
    //else
        outColor = vec4(normalised, normalised, normalised, 1.0);
}


