#version 330 core

in vec2 texcoord;
in vec3 textColor;
flat in int layer;
in vec4 idcolor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outSelection;

uniform sampler2DArray tex;
const float smoothing = 0.0f;


void main()
{
    vec4 texColor = texture(tex, vec3(texcoord, layer));
    float distance = texColor.r;
    float smoothing = 0.015;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    alpha = alpha;

    vec4 firstData = texColor;
    if(firstData.r < 0.5 - smoothing)
        discard;

    outColor = vec4(textColor.rgb, alpha);
    outSelection = vec4(0, 0, 0, 0);
}
