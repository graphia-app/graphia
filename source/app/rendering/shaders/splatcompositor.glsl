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

// What the shapes drawn into SplatCompositor share; include after componentdata.glsl

// How much what is drawn by coverage at a depth counts, falling by e³ for each
// radius of its component it is behind the front, so that the front hides the back
float coverageWeightAt(int component, float viewZ)
{
    const float FALLOFF = 3.0;

    float radius = max(componentRadius(component), 0.0001);
    float front = componentDistance(component) - radius;

    return exp(-FALLOFF * max(-viewZ - front, 0.0) / radius);
}
