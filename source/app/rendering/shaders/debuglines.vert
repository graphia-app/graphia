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

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec3 vColor;

void main()
{
    vColor = color;
    gl_Position = projectionMatrix * vec4((modelViewMatrix * vec4(position, 1.0)).xyz, 1.0);
}
