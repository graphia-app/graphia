#ifndef UTILS_H
#define UTILS_H

namespace u
{
    template<typename T> T interpolate(const T& a, const T& b, float f)
    {
        return a + ((b - a) * f);
    }

    template<typename T> T clamp(T min, T max, T value)
    {
        if(value < min)
            return min;
        else if(value > max)
            return max;

        return value;
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

    static inline int smallestPowerOf2GreaterThan(int x)
    {
        if(x < 0)
            return 0;

        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return x + 1;
    }

    template<typename T>
    bool exclusiveOr(T a, T b)
    {
        return !a != !b;
    }
}

#define ARRAY_SIZEOF(x) (sizeof(x)/sizeof((x)[0]))

#endif // UTILS_H
