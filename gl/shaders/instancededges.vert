#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec3 source; // The position of the source node
layout (location = 4) in vec3 target; // The position of the target node
layout (location = 5) in float size; // The size of the edge
layout (location = 6) in vec3 color; // The color of the edge
layout (location = 7) in vec4 outlineColor; // The outline color of the node

out vec3 position;
out vec3 normal;
out vec3 vColor;
out vec4 vOutlineColor;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

mat4 makeOrientationMatrix(vec3 up)
{
    mat3 m;

    m[1] = up;
    m[0].y = -m[1].x;
    m[0].z = m[1].y;
    m[0].x = m[1].z;

    float d = dot(m[0], m[1]);
    m[0] -= (d * m[1]);
    m[0] = normalize(m[0]);
    m[2] = cross(m[0], m[1]);

    return mat4(m);
}

void main()
{
    float edgeLength = distance(source, target);
    vec3 midpoint = mix(source, target, 0.5);
    mat4 modelMatrix = makeOrientationMatrix(normalize(target - source));

    vec3 scaledVertexPosition = vertexPosition;
    scaledVertexPosition.xz *= size;
    scaledVertexPosition.y *= edgeLength;

    position = (modelMatrix * vec4(scaledVertexPosition, 1.0)).xyz;
    position = (viewMatrix * vec4(position + midpoint, 1.0)).xyz;

    mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));
    normal = normalMatrix * vertexNormal;
    vColor = color;
    vOutlineColor = outlineColor;
    gl_Position = projectionMatrix * vec4( position, 1.0 );
}
