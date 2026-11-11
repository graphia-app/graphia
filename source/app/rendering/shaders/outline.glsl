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

// How many pixels drawn stand for each of the outline's, which are logical
// pixels on screen; more on a high DPI display and in a screenshot
uniform float outlineScale;

// The outline's pixel, in whole pixels drawn, as far as its neighbours are apart
float outlinePixel()
{
    return max(round(outlineScale), 1.0);
}

// Determines how much of an element's outline is visible based on its
// projected size
float outlineProjection(float projectionScale)
{
    const float MIN_PS = 0.001;
    const float MAX_PS = 0.015;

    return clamp((projectionScale - MIN_PS) / (MAX_PS - MIN_PS), 0.0, 1.0);
}

// Calculates outline opacity based on the proportion of neighbouring
// pixels that belong to different elements
float outlineOpacity(float differing, float projection)
{
    // Give the value a bit of ramp; this thins the edges out a bit
    float value = pow(differing, 1.0 + ((1.0 - projection) * 2.0));

    // Dial the value back a bit, helping the color come through
    // when edges are densely packed
    const float MIN_VALUE = 0.1;
    return value * (MIN_VALUE + (projection * (1.0 - MIN_VALUE)));
}

// Calculates outline opacity based on how far inside the element the
// current pixel lies
float outlineOpacityInside(float inside, float differing, float projectionScale)
{
    return clamp(1.5 - (inside / outlinePixel()), 0.0, 1.0) *
        outlineOpacity(differing, outlineProjection(projectionScale));
}
