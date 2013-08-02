#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>

class Utils
{
public:
    template<typename T> static T rand(T low, T high)
    {
        T range = high - low;
        T value = ((::rand() * range) / RAND_MAX) + low;

        return value;
    }
};

#endif // UTILS_H
