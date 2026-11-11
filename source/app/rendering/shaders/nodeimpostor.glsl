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

// What the node impostor's vertex and fragment shaders share

#define PI 3.14159265359

// How far the dot in a node's middle reaches, as a fraction of the right angle
// the normal turns through from the middle to the rim
const float DOT_REACH = 0.2;

// About how much of the disc the dot covers
const float DOT_AREA = pow(sin(DOT_REACH * PI * 0.5), 2.0);

// Right and up across a direction facing the eye, square to it and each other
void facingBasis(vec3 facing, out vec3 right, out vec3 up)
{
    vec3 notParallel = abs(facing.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    right = normalize(cross(notParallel, facing));
    up = cross(facing, right);
}
