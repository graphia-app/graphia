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

#include "lighting.glsl"

uniform float flatness;

flat in float element;
in vec3 vPosition;
in vec3 vNormal;
in vec3 innerVColor;
in vec3 outerVColor;
in float vSelected;
in vec3 vOutlineColor;
in vec2 uv;
in float lightOffset;
in float lightScale;
in float projectionScale;

layout (location = 0) out vec4  outColor;
layout (location = 1) out vec2  outElement;
layout (location = 2) out vec4  outSelection;

void main()
{
    // Hemisphere projection means the UVs are NOT linear

    // Centre color of Node calculation
    // We want 0, 0 to be the centre for dot
    vec2 scaledUV = (uv.xy - 0.5) * 2.0;

    // Step threshold indicates the size of dot
    vec3 fragColor = mix(outerVColor, innerVColor, step(length(scaledUV), 0.2));

    vec3 normal = normalize(vNormal);
    vec3 color = (flatness * fragColor) +
        ((1.0 - flatness) * adsModel(vPosition, normal, fragColor, lightOffset, lightScale));

    outColor = vec4(color, 1.0);
    outElement.r = element;
    outElement.g = projectionScale;
    outSelection = vec4(vec3(vSelected), 1.0);
}
