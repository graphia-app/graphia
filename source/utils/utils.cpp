#include "utils.h"

QVector2D Utils::randQVector2D(float low, float high)
{
    return QVector2D(rand(low, high), rand(low, high));
}

QVector3D Utils::randQVector3D(float low, float high)
{
    return QVector3D(rand(low, high), rand(low, high), rand(low, high));
}

QColor Utils::randQColor()
{
    int r = rand(0, 255);
    int g = rand(0, 255);
    int b = rand(0, 255);

    return QColor(r, g, b);
}

float Utils::fast_rsqrt(float number)
{
    typedef union
    {
        float f;
        int i;
        unsigned int ui;
    } floatint_t;

    floatint_t t;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    t.f  = number;
    t.i  = 0x5f3759df -(t.i >> 1);               // what the fuck?
    y  = t.f;
    y  = y *(threehalfs -(x2 * y * y));   // 1st iteration
    //	y  = y *(threehalfs -(x2 * y * y));   // 2nd iteration, this can be removed

    return y;
}

QVector3D Utils::fastNormalize(QVector3D& v, float len)
{
    float rsqrtLen = fast_rsqrt(len);
    return v * rsqrtLen;
}
