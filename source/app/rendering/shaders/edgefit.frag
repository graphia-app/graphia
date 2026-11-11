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

// Keeps the shading across one edge's width, averaged over each third of it

flat in vec3 fitThirds[3];

layout (location = 0) out vec4 outFirstThird;
layout (location = 1) out vec4 outSecondThird;
layout (location = 2) out vec4 outThirdThird;

void main()
{
    outFirstThird = vec4(fitThirds[0], 1.0);
    outSecondThird = vec4(fitThirds[1], 1.0);
    outThirdThird = vec4(fitThirds[2], 1.0);
}
