#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <cstdlib>
#include <cmath>

#include "constants.h"

class Interpolation
{
public:
    static float linear(float a, float b, float f)
    {
        return a + ((b - a) * f);
    }

    static float easeInEaseOut(float a, float b, float f)
    {
        return a + (0.5f * (1.0f - std::cos(f * Constants::Pi())) * (b - a));
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
