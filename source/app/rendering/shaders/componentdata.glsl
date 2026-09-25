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

// Each component's matrices and lighting parameters, one after another

#ifdef GL_ES
uniform sampler2D componentData;
#else
uniform samplerBuffer componentData;
#endif

uniform int componentDataElementSize;
uniform int componentDataTextureMaxDimension;

float floatFromComponentData(int component, int offset)
{
    int index = (component * componentDataElementSize) + offset;
#ifdef GL_ES
    int y = index / componentDataTextureMaxDimension;
    int x = index % componentDataTextureMaxDimension;
    ivec2 coord = ivec2(x, y);

    return texelFetch(componentData, coord, 0).r;
#else
    return texelFetch(componentData, index).r;
#endif
}

mat4 mat4FromComponentData(int component, int offset)
{
    mat4 m;
    int index = offset;

    for(int j = 0; j < 4; j++)
        for(int i = 0; i < 4; i++)
            m[j][i] = floatFromComponentData(component, index++);

    return m;
}

mat4 componentModelViewMatrix(int component)    { return mat4FromComponentData(component, 0); }
mat4 componentProjectionMatrix(int component)   { return mat4FromComponentData(component, 16); }
float componentLightOffset(int component)       { return floatFromComponentData(component, 32); }
float componentLightScale(int component)        { return floatFromComponentData(component, 33); }
