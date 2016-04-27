#version 330 core

in vec3 position;
in vec3 centre;
in float radius;

layout (location = 0) out vec4 fragColor;

void main()
{
    const float LINE_WIDTH = 0.5;
    const float STEP_WIDTH = 0.1;

    const float HALF_LINE_WIDTH = LINE_WIDTH * 0.5;
    const float LINE_BOUNDARY_WIDTH = HALF_LINE_WIDTH + STEP_WIDTH;

    float distanceFromCentre = distance(centre, position);
    float distanceFromLine = abs(distanceFromCentre - (radius - LINE_BOUNDARY_WIDTH));

    float i;
    if(distanceFromLine < HALF_LINE_WIDTH)
        i = 1.0;
    else if(distanceFromLine >= HALF_LINE_WIDTH && distanceFromLine < LINE_BOUNDARY_WIDTH)
        i = (distanceFromLine - LINE_BOUNDARY_WIDTH) / (HALF_LINE_WIDTH - LINE_BOUNDARY_WIDTH);
    else if(distanceFromLine >= LINE_BOUNDARY_WIDTH)
        discard;

    vec3 color = vec3(1.0, 1.0, 1.0);
    fragColor = vec4(color, i);
}
