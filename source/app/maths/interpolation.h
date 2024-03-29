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

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <cstdlib>
#include <cmath>
#include <numbers>

class Interpolation
{
public:
    static float linear(float a, float b, float f)
    {
        return a + ((b - a) * f);
    }

    static float easeInEaseOut(float a, float b, float f)
    {
        return a + (0.5f * (1.0f - std::cos(f * std::numbers::pi_v<float>)) * (b - a));
    }

    static float power(float a, float b, float f, int power = 3)
    {
        float f2 = 1.0f;

        for(int i = 0; i < power; i++)
            f2 *= f;

        return linear(a, b, f2);
    }

    static float inversePower(float a, float b, float f, int power = 3)
    {
        float f2 = 1.0f;

        for(int i = 0; i < power; i++)
            f2 *= (1.0f - f);

        f2 = 1.0f - f2;

        return linear(a, b, f2);
    }
};

#endif // INTERPOLATION_H
