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

#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <vector>

#include <QString>

namespace u
{
    template<typename T> T interpolate(const T& a, const T& b, float f)
    {
        return a + ((b - a) * f);
    }

    template<typename T> T normalise(T min, T max, T value)
    {
        if(max == min)
        {
            // There is no sensible way to normalise when the range is zero
            return static_cast<T>(-1);
        }

        return (value - min) / (max - min);
    }

    template<typename T> bool signsMatch(T a, T b)
    {
        return (a > 0 && b > 0) || (a <= 0 && b <= 0);
    }

    int smallestPowerOf2GreaterThan(int x);

    template<typename T>
    bool exclusiveOr(T a, T b)
    {
        return !a != !b;
    }

    template<typename T>
    std::vector<T> evenDivisionOf(T n, size_t d)
    {
        if(d == 0)
            return {};

        const auto q = static_cast<T>(n / d);
        const auto r = n % d;
        const auto c = d - r;

        std::vector<T> out(d);
        size_t a = 1;
        size_t b = 1;

        for(size_t i = 0; i < d; i++)
        {
            // Make the ratio a/b tend towards c/r
            if((a * r) < (b * c))
            {
                a++;
                out[i] = q;
            }
            else
            {
                b++;
                out[i] = q + 1;
            }
        }

        return out;
    }

    float normaliseAngle(float radians);
} // namespace u

#endif // UTILS_H
