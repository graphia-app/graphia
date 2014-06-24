#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QColor>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <memory>

class Utils
{
public:
    template<typename T> static T rand(T low, T high)
    {
        T range = high - low;
        int64_t randValue = ::rand();
        int64_t numerator = randValue * range;
        T value = (numerator / static_cast<T>(RAND_MAX)) + low;

        return value;
    }

    static QVector2D randQVector2D(float low, float high)
    {
        return QVector2D(rand(low, high), rand(low, high));
    }

    static QVector3D randQVector3D(float low, float high)
    {
        return QVector3D(rand(low, high), rand(low, high), rand(low, high));
    }

    static QColor randQColor()
    {
        int r = rand(0, 255);
        int g = rand(0, 255);
        int b = rand(0, 255);

        return QColor(r, g, b);
    }

    template<typename T> static bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }

    static float fast_rsqrt(float number)
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

    static QVector3D fastNormalize(QVector3D& v, float len)
    {
        float rsqrtLen = fast_rsqrt(len);
        return v * rsqrtLen;
    }

    template<typename T> static T clamp(T min, T max, T value)
    {
        if(value < min)
            return min;
        else if(value > max)
            return max;

        return value;
    }

    template<typename T> static bool signsMatch(T a, T b)
    {
        return (a > 0 && b > 0) || (a <= 0 && b <= 0);
    }
};

#if __cplusplus <= 201103L
namespace std {
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}
}
#else
#error std::make_unique should now be available
#endif

#endif // UTILS_H
