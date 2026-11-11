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

// Traces the shaft and the arrow head through each fragment of the quads the
// vertex shader laid out

#include "lighting.glsl"
#include "projection.glsl"

uniform float flatness;

flat in float element;
flat in vec3  innerVColor;
flat in vec3  outerVColor;
flat in float vSelected;
flat in vec2  lightPlacement;
in float projectionScale;

flat in vec3  shaftBase;
flat in vec3  edgeAxis;
flat in float shaftRadius;
flat in float shaftLength;
flat in vec3  headApex;
flat in float headLength;
flat in float headRadius;

flat in vec2  innerColorStretch;

flat in mat2  zToClip;

in vec3 impostorPosition;

layout (location = 0) out vec4  outColor;
layout (location = 1) out vec2  outElement;
layout (location = 2) out vec4  outSelection;

const float EPSILON = 0.000001;
const float NO_HIT = 3.402823466e+38;

struct Hit
{
    float t;
    vec3 position;
    vec3 normal;
};

// Only surfaces facing the ray count: the shapes are open where the nodes
// swallow them, and what would show through is their inside
void consider(inout Hit nearest, float t, vec3 position, vec3 normal, vec3 rayDirection)
{
    if(t > 0.0 && t < nearest.t && dot(normal, rayDirection) < 0.0)
        nearest = Hit(t, position, normal);
}

// The rays start so far back that the quadratics lose the radius to rounding, so each
// is solved from the point on the ray nearest the shape

void traceShaft(vec3 rayOrigin, vec3 rayDirection, inout Hit nearest)
{
    float rayAlong = dot(rayDirection, edgeAxis);
    vec3 rayAcross = rayDirection - (edgeAxis * rayAlong);
    float a = dot(rayAcross, rayAcross);

    if(a < EPSILON)
        return;

    // Where the ray passes nearest the axis
    float closest = -dot(rayOrigin - shaftBase, rayAcross) / a;
    vec3 fromBase = (rayOrigin + (rayDirection * closest)) - shaftBase;
    float originAlong = dot(fromBase, edgeAxis);
    vec3 originAcross = fromBase - (edgeAxis * originAlong);

    float b = 2.0 * dot(rayAcross, originAcross);
    float c = dot(originAcross, originAcross) - (shaftRadius * shaftRadius);
    float discriminant = (b * b) - (4.0 * a * c);

    if(discriminant < 0.0)
        return;

    float fromClosest = (-b - sqrt(discriminant)) / (2.0 * a);
    float t = closest + fromClosest;
    float along = originAlong + (fromClosest * rayAlong);

    if(along < 0.0 || along > shaftLength)
        return;

    vec3 position = rayOrigin + (rayDirection * t);
    consider(nearest, t, position, normalize(position - (shaftBase + (edgeAxis * along))), rayDirection);
}

void traceHeadBase(vec3 rayOrigin, vec3 rayDirection, vec3 centre, vec3 normal, inout Hit nearest)
{
    float facing = dot(rayDirection, normal);

    if(facing >= -EPSILON)
        return;

    float t = dot(centre - rayOrigin, normal) / facing;
    vec3 position = rayOrigin + (rayDirection * t);

    if(distance(position, centre) <= headRadius)
        consider(nearest, t, position, normal, rayDirection);
}

void traceHead(vec3 rayOrigin, vec3 rayDirection, inout Hit nearest)
{
    // From the tip back towards the source
    vec3 axis = -edgeAxis;

    traceHeadBase(rayOrigin, rayDirection, headApex + (axis * headLength), axis, nearest);

    // Where the ray passes nearest the apex
    float closest = dot(headApex - rayOrigin, rayDirection);
    vec3 fromApex = (rayOrigin + (rayDirection * closest)) - headApex;
    float cosSquared = (headLength * headLength) / ((headLength * headLength) + (headRadius * headRadius));
    float rayAlong = dot(rayDirection, axis);
    float originAlong = dot(fromApex, axis);

    float a = (rayAlong * rayAlong) - cosSquared;
    float b = 2.0 * ((rayAlong * originAlong) - (dot(rayDirection, fromApex) * cosSquared));
    float c = (originAlong * originAlong) - (dot(fromApex, fromApex) * cosSquared);
    float discriminant = (b * b) - (4.0 * a * c);

    if(abs(a) < EPSILON || discriminant < 0.0)
        return;

    // a can be negative, which puts the roots the other way round
    float root = sqrt(discriminant);
    vec2 fromClosest = vec2((-b - root) / (2.0 * a), (-b + root) / (2.0 * a));
    fromClosest = vec2(min(fromClosest.x, fromClosest.y), max(fromClosest.x, fromClosest.y));
    vec2 t = fromClosest + closest;

    for(int i = 0; i < 2; i++)
    {
        float along = originAlong + (fromClosest[i] * rayAlong);

        if(t[i] > 0.0 && along >= 0.0 && along <= headLength)
        {
            vec3 position = rayOrigin + (rayDirection * t[i]);
            vec3 radial = (position - headApex) - (axis * along);
            float radialLength = length(radial);

            vec3 normal = radialLength > EPSILON ?
                normalize(((radial / radialLength) * headLength) - (axis * headRadius)) : -axis;

            consider(nearest, t[i], position, normal, rayDirection);
            return;
        }
    }
}

void main()
{
    vec3 rayDirection = -normalize(towardsEye(zToClip, impostorPosition));

    // Far enough back to be outside the shape, so its nearer crossings are ahead
    vec3 shaftEnd = shaftBase + (edgeAxis * shaftLength);
    float reach = max(max(distance(impostorPosition, shaftBase), distance(impostorPosition, shaftEnd)),
        distance(impostorPosition, headApex)) + max(shaftRadius, headRadius);
    vec3 rayOrigin = impostorPosition - (rayDirection * reach);

    Hit nearest = Hit(NO_HIT, vec3(0.0), vec3(0.0));
    traceShaft(rayOrigin, rayDirection, nearest);

    if(headLength > 0.0)
        traceHead(rayOrigin, rayDirection, nearest);

    if(nearest.t == NO_HIT)
        discard;

    float along = dot(nearest.position - shaftBase, edgeAxis);
    bool inner = along >= innerColorStretch.x && along <= innerColorStretch.y;
    vec3 fragColor = inner ? innerVColor : outerVColor;

    vec3 color = (flatness * fragColor) +
        ((1.0 - flatness) * adsModel(nearest.position, nearest.normal, fragColor, lightPlacement));

    gl_FragDepth = depthAt(zToClip, nearest.position.z);

    outColor = vec4(color, 1.0);
    outElement.r = element;
    outElement.g = projectionScale;
    outSelection = vec4(vec3(vSelected), 1.0);
}
