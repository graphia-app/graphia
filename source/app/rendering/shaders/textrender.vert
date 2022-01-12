/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

// Instanced Data
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;

// VBO Data
layout (location = 3) in int component; // The component index
layout (location = 4) in vec2 textureCoord;
layout (location = 5) in int textureLayer;
layout (location = 6) in vec3 basePosition;
layout (location = 7) in vec2 glyphOffset;
layout (location = 8) in vec2 glyphSize;
layout (location = 9) in vec3 color;

uniform samplerBuffer componentData;
uniform int componentDataElementSize;
uniform sampler2DArray tex;
uniform float textScale;

out vec2 texCoord;
flat out int texLayer;
out vec3 textColor;

int componentDataOffset()
{
    return component * componentDataElementSize;
}

mat4 mat4FromComponentData(int offset)
{
    mat4 m;
    int index = componentDataOffset() + offset;

    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < 4; i++)
        {
            m[j][i] = texelFetch(componentData, index).r;
            index++;
        }
    }

    return m;
}

void main()
{
   mat4 modelViewMatrix = mat4FromComponentData(0);
   mat4 projectionMatrix = mat4FromComponentData(16);

  vec3 cameraUp =    normalize(vec3(modelViewMatrix[0].y, modelViewMatrix[1].y, modelViewMatrix[2].y));
  vec3 cameraRight = normalize(vec3(modelViewMatrix[0].x, modelViewMatrix[1].x, modelViewMatrix[2].x));

  vec2 scaledGlyphSize = glyphSize * textScale;
  vec3 billboardPosition = (cameraRight * vertexPosition.x * scaledGlyphSize.x) +
                           (cameraUp *    vertexPosition.y * scaledGlyphSize.y);
  vec3 billboardOffset = (cameraRight * glyphOffset.x) + (cameraUp * glyphOffset.y);
  vec3 position = basePosition + billboardPosition + billboardOffset;

  gl_Position = projectionMatrix * vec4((modelViewMatrix * vec4(position, 1.0)).xyz, 1.0);

  texLayer = textureLayer;

  float u = textureCoord.x + (vertexPosition.x * glyphSize.x);
  float v = (1.0 - textureCoord.y) + (vertexPosition.y * glyphSize.y);
  texCoord = vec2(u, v);

  textColor = color;
}
