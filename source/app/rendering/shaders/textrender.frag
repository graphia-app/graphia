#version 330 core

in vec2 texCoord;
flat in int texLayer;
in vec3 textColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out uint outElement;
layout (location = 2) out vec4 outSelection;

uniform sampler2DArray tex;

void main()
{
    vec4 texColor = texture(tex, vec3(texCoord, texLayer));
    float distance = texColor.r;
    float smoothing = fwidth(distance);
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);

    vec4 firstData = texColor;
    if(firstData.r < 0.5 - smoothing)
        discard;

    outColor = vec4(textColor.rgb, alpha);
    outElement = 0xFFFFFFFFu; // The default clear color for the element buffer
    outSelection = vec4(0, 0, 0, 0);
}
