/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

in vec2 texCoord;
flat in int texLayer;
in vec3 textColor;

layout (location = 0) out vec4  outColor;
layout (location = 1) out vec2  outElement;
layout (location = 2) out vec4  outSelection;

uniform mediump sampler2DArray tex;

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
    outElement = vec2(0.0, 0.0);
    outSelection = vec4(0, 0, 0, 0);
}
