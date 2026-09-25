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
layout (location = 9) in float glyphScale;
layout (location = 10) in vec3 color;

// Text belonging to a node or an edge sits midway between these two nodes, which
// for a node's text are the same node; other text has none, and uses basePosition
layout (location = 11) in ivec2 anchorNodes;

layout (location = 12) in float textHeight;

#include "componentdata.glsl"
#include "nodegeometry.glsl"

uniform mediump sampler2DArray tex;

uniform float viewportHeight;

uniform float illegibleTextPixels;
uniform float legibleTextPixels;

out vec2 texCoord;
flat out int texLayer;
out vec3 textColor;
flat out float legibility;

vec3 anchorPosition()
{
    if(anchorNodes.x < 0)
        return basePosition;

    return mix(nodePosition(anchorNodes.x), nodePosition(anchorNodes.y), 0.5);
}

void main()
{
  mat4 modelViewMatrix = componentModelViewMatrix(component);
  mat4 projectionMatrix = componentProjectionMatrix(component);

  vec3 anchor = anchorPosition();
  vec2 scaledGlyphSize = glyphSize * glyphScale;

  float anchorW = (projectionMatrix * modelViewMatrix * vec4(anchor, 1.0)).w;
  float textPixels = (textHeight * projectionMatrix[1][1] * viewportHeight * 0.5) / anchorW;
  legibility = smoothstep(illegibleTextPixels, legibleTextPixels, textPixels);

  if(legibility <= 0.0)
  {
      // Degenerate, so nothing is drawn
      gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
      return;
  }

  vec3 cameraUp =    normalize(vec3(modelViewMatrix[0].y, modelViewMatrix[1].y, modelViewMatrix[2].y));
  vec3 cameraRight = normalize(vec3(modelViewMatrix[0].x, modelViewMatrix[1].x, modelViewMatrix[2].x));

  vec3 billboardPosition = (cameraRight * vertexPosition.x * scaledGlyphSize.x) +
                           (cameraUp *    vertexPosition.y * scaledGlyphSize.y);
  vec3 billboardOffset = (cameraRight * glyphOffset.x) + (cameraUp * glyphOffset.y);
  vec3 position = anchor + billboardPosition + billboardOffset;

  gl_Position = projectionMatrix * vec4((modelViewMatrix * vec4(position, 1.0)).xyz, 1.0);

  texLayer = textureLayer;

  float u = textureCoord.x + (vertexPosition.x * glyphSize.x);
  float v = (1.0 - textureCoord.y) + (vertexPosition.y * glyphSize.y);
  texCoord = vec2(u, v);

  textColor = color;
}
