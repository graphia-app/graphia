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

// Traces the node's sphere through each fragment of the quad the vertex shader laid out

#include "lighting.glsl"
#include "nodeimpostor.glsl"
#include "projection.glsl"

uniform float flatness;

flat in float element;
flat in vec3  innerVColor;
flat in vec3  outerVColor;
flat in float vSelected;
flat in vec2  lightPlacement;
flat in float projectionScale;

flat in vec3  nodeCentre;
flat in float nodeRadius;
flat in mat2  zToClip;

in vec3 impostorPosition;

layout (location = 0) out vec4  outColor;
layout (location = 1) out vec2  outElement;
layout (location = 2) out vec4  outSelection;

void main()
{
    vec3 rayDirection = -normalize(towardsEye(zToClip, impostorPosition));

    // Far enough back to be outside the sphere, so its nearer crossing is ahead
    vec3 rayOrigin = impostorPosition -
        (rayDirection * (distance(impostorPosition, nodeCentre) + nodeRadius));

    vec3 centreToOrigin = rayOrigin - nodeCentre;
    float along = dot(centreToOrigin, rayDirection);
    float discriminant = (along * along) -
        (dot(centreToOrigin, centreToOrigin) - (nodeRadius * nodeRadius));

    if(discriminant < 0.0)
        discard;

    vec3 position = rayOrigin + (rayDirection * (-along - sqrt(discriminant)));
    vec3 normal = normalize(position - nodeCentre);

    // Hemisphere projection means the UVs are NOT linear
    vec3 right, up;
    facingBasis(normalize(towardsEye(zToClip, nodeCentre)), right, up);
    vec2 uv = vec2(0.5 + (asin(clamp(dot(right, normal), -1.0, 1.0)) / PI),
                   0.5 + (asin(clamp(dot(up, normal), -1.0, 1.0)) / PI));

    // We want 0, 0 to be the centre for dot
    vec2 scaledUV = (uv.xy - 0.5) * 2.0;

    // Step threshold indicates the size of dot
    vec3 fragColor = mix(outerVColor, innerVColor, step(length(scaledUV), DOT_REACH));

    vec3 color = (flatness * fragColor) +
        ((1.0 - flatness) * adsModel(position, normal, fragColor, lightPlacement.x, lightPlacement.y));

    gl_FragDepth = depthAt(zToClip, position.z);

    outColor = vec4(color, 1.0);
    outElement.r = element;
    outElement.g = projectionScale;
    outSelection = vec4(vec3(vSelected), 1.0);
}
