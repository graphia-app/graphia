#ifndef UTILS_H
#define UTILS_H

#include <cmath>
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
} // namespace u

#define ARRAY_SIZEOF(x) (sizeof(x)/sizeof((x)[0]))

#endif // UTILS_H
