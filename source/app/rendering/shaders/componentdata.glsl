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

// Each component's model view matrix, projection matrix and lighting parameters,
// in that order, as laid out by GraphRendererCore::uploadGPUComponentData

#include "texelbuffer.glsl"

uniform TexelBuffer componentData;

vec4 componentDataTexel(int component, int texel)
{
    const int texelsPerComponent = 9;
    return texelAt(componentData, (component * texelsPerComponent) + texel);
}

mat4 componentModelViewMatrix(int component)
{
    return mat4(componentDataTexel(component, 0), componentDataTexel(component, 1),
        componentDataTexel(component, 2), componentDataTexel(component, 3));
}

mat4 componentProjectionMatrix(int component)
{
    return mat4(componentDataTexel(component, 4), componentDataTexel(component, 5),
        componentDataTexel(component, 6), componentDataTexel(component, 7));
}

vec2 componentLightPlacement(int component) { return componentDataTexel(component, 8).xy; }
