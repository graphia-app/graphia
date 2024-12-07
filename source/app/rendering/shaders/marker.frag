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
