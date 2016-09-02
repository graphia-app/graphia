#version 330 core

// Instanced Data
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexTangent;
// VBO Data
in int component; // The component index
in vec2 texturePosition; // Y index in the atlas
in vec3 targetPosition;
in vec2 positionOffset;
in vec2 glyphSize;
in vec3 color;
in float textScale;
in float stringWidth;
in vec2 targetOffset;

uniform samplerBuffer componentData;
uniform sampler2DArray tex;
uniform int textAlignment;

out vec2 texcoord;
out vec3 textColor;
out vec4 idcolor;
flat out int layer;

void main()
{
   int index = component * 8;

   mat4 modelViewMatrix = mat4(texelFetch(componentData, index + 0),
                               texelFetch(componentData, index + 1),
                               texelFetch(componentData, index + 2),
                               texelFetch(componentData, index + 3));

   mat4 projectionMatrix = mat4(texelFetch(componentData, index + 4),
                                texelFetch(componentData, index + 5),
                                texelFetch(componentData, index + 6),
                                texelFetch(componentData, index + 7));

  const int ALIGN_RIGHT = 0;
  const int ALIGN_LEFT = 1;
  const int ALIGN_CENTER = 2;
  const int ALIGN_TOP_CENTER = 3;
  const int ALIGN_BOTTOM_CENTER = 4;

  // 3D Text
  const float glyphHeight = 1.0;
  float textSizeScale = textScale / 12.0;
  ivec3 texSize = textureSize(tex, 0);

  float aspectRatio = float(texSize.x) / float(texSize.y);
  float scaleRatio = glyphHeight / glyphSize.y;

  vec2 quadSize = vec2(glyphSize.x * scaleRatio * aspectRatio, glyphHeight) * textSizeScale;

  vec3 cameraUp = normalize(vec3(modelViewMatrix[0].y, modelViewMatrix[1].y, modelViewMatrix[2].y));
  vec3 cameraRight = normalize(vec3(modelViewMatrix[0].x, modelViewMatrix[1].x, modelViewMatrix[2].x));

  vec3 offset;
  switch(textAlignment)
  {
  case ALIGN_RIGHT:
      offset = (cameraRight * targetOffset.x) + (cameraUp * -quadSize.y * 0.5);
      break;
  case ALIGN_LEFT:
      offset = (-cameraRight * targetOffset.x) +
              (-cameraRight * stringWidth * scaleRatio * aspectRatio * textSizeScale) +
              (cameraUp * -quadSize.y * 0.5);
      break;
  case ALIGN_CENTER:
      offset = (-cameraRight * stringWidth * scaleRatio * aspectRatio * textSizeScale * 0.5) +
              (cameraUp * -quadSize.y * 0.5);
      break;
  case ALIGN_TOP_CENTER:
      offset = (-cameraRight * stringWidth * scaleRatio * aspectRatio * textSizeScale * 0.5) +
              (cameraUp * targetOffset.y);
      break;
  case ALIGN_BOTTOM_CENTER:
      offset = (-cameraRight * stringWidth * scaleRatio * aspectRatio * textSizeScale * 0.5) +
              (cameraUp * -targetOffset.y) + (cameraUp * -glyphSize.y);
      break;
  default:
      break;
  }

  vec3 billboardVertPosition = (cameraRight * vertexPosition.x * quadSize.x) + (cameraUp * vertexPosition.y * quadSize.y);

  vec3 scaledGlyphOffset = positionOffset.x * scaleRatio * aspectRatio * cameraRight * textSizeScale;

  vec3 position = targetPosition + offset + billboardVertPosition + scaledGlyphOffset;

  position = (modelViewMatrix * vec4(position, 1.0)).xyz;
  gl_Position = projectionMatrix * vec4(position, 1.0);


  float distance = -position.z;

  idcolor = vec4(0.0, texturePosition.y, 0.0, 1.0);

  layer = 0; //FIXME Only ever using the first texture layer?

  texcoord = vec2(texturePosition.x + (vertexPosition.x * glyphSize.x), (((1.0 - texturePosition.y)) + (vertexPosition.y * glyphSize.y))) ;
  textColor = color;
}
