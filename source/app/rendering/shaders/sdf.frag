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

// The proportion of the source texel with the given integer index that the glyph
// covers. The texel's centre lies at index + 0.5, and tex is sampled with
// GL_NEAREST, so this is the coverage of precisely that texel, rather than a
// blend of the texels surrounding the sample point
float coverageAt(vec2 index)
{
    return texture(tex, (index + 0.5) / texSize.xy).a;
}

void main()
{
    // gl_FragCoord is in destination pixels, so scaling it gives the position, in
    // source pixels, that this texel's centre corresponds to once sampled. Note
    // this is a position and not an index; it doesn't generally coincide with the
    // centre of any one source texel
    vec2 fragPosition = gl_FragCoord.xy * scaleFactor;

    const float range = 8.0;
    int scalediRange = int(range * scaleFactor);
    float scaledHalfRange = (range / 2.0) * scaleFactor;
    vec2 startIndex = floor(fragPosition - scaledHalfRange);

    // The distance at which the field saturates; the half diagonal of the search
    // box, so that an edge which isn't found encodes as entirely inside or outside
    float maxDistance = scaledHalfRange * sqrt(2.0);

    // A straight edge is only a good fit for the outline in the immediate vicinity
    // of the texel it crosses, so only fit one within a destination texel's width
    float fitDistance = scaleFactor;

    bool fragIsIn = coverageAt(floor(fragPosition)) > 0.5;

    // Find the nearest partially covered texel, which the edge passes through, and
    // the nearest texel on the other side of the edge, the latter being a fallback
    // for where the source has no partial coverage to work with
    vec2 edgeIndex = vec2(0.0);
    float edgeDistance = maxDistance;
    bool edgeFound = false;

    float oppositeDistance = maxDistance;
    float oppositeCoverage = 0.0;
    bool oppositeFound = false;

    for(int dx = 0; dx < scalediRange; dx++)
    {
        for(int dy = 0; dy < scalediRange; dy++)
        {
            vec2 index = startIndex + vec2(dx, dy);
            float coverage = coverageAt(index);

            // Distances are measured to the texel's centre, not its index
            float d = distance(fragPosition, index + 0.5);

            if(coverage > 0.0 && coverage < 1.0 && d < edgeDistance)
            {
                edgeDistance = d;
                edgeIndex = index;
                edgeFound = true;
            }

            if((coverage > 0.5) != fragIsIn && d < oppositeDistance)
            {
                oppositeDistance = d;
                oppositeCoverage = coverage;
                oppositeFound = true;
            }
        }
    }

    // Positive outside the glyph, negative within it
    float distanceToEdge = fragIsIn ? -maxDistance : maxDistance;
    bool fitted = false;

    if(edgeFound && edgeDistance <= fitDistance)
    {
        // Coverage increases towards the inside of the glyph, so the normalised
        // gradient of the coverage is the edge's inward normal
        vec2 gradient = vec2(
            coverageAt(edgeIndex + vec2(1.0, 0.0)) - coverageAt(edgeIndex - vec2(1.0, 0.0)),
            coverageAt(edgeIndex + vec2(0.0, 1.0)) - coverageAt(edgeIndex - vec2(0.0, 1.0)));

        if(length(gradient) > 0.0001)
        {
            // For a straight edge crossing a texel, the coverage of that texel is
            // the signed distance from its centre to the edge. Together with the
            // normal this gives the edge's position to sub-texel precision, so
            // measure to that, instead of quantising to the texel grid
            vec2 inwards = normalize(gradient);
            float centreToEdge = coverageAt(edgeIndex) - 0.5;
            distanceToEdge = -(centreToEdge + dot(fragPosition - (edgeIndex + 0.5), inwards));
            fitted = true;
        }
    }

    if(!fitted && oppositeFound)
    {
        // No edge to fit, so fall back to the distance to the nearest texel on the
        // other side, brought in by that texel's own distance from the edge
        distanceToEdge = oppositeDistance +
            (fragIsIn ? (oppositeCoverage - 0.5) : (0.5 - oppositeCoverage));

        if(fragIsIn)
            distanceToEdge = -distanceToEdge;
    }

    // 0.5 at the edge, reaching 0 and 1 at maxDistance/2 either side of it, with
    // anything beyond that clamped by the framebuffer
    float normalised = 0.5 - (distanceToEdge / maxDistance);

    // Uncomment for outline
    //if(normalised > 0.5)
    //    outColor = vec4(1.0);
    //else
        outColor = vec4(normalised, normalised, normalised, 1.0);
}
