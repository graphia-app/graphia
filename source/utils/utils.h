#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QColor>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <memory>
#include <random>

class Utils
{
private:
    static std::random_device _rd;
    static std::mt19937 _gen;

public:
    static float rand(float low, float high);
    static int rand(int low, int high);

    static QVector2D randQVector2D(float low, float high);
    static QVector3D randQVector3D(float low, float high);
    static QColor randQColor();

    template<typename T> static T interpolate(const T& a, const T& b, float f)
    {
        return a + ((b - a) * f);
    }

    template<typename T> static bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }

    static float fast_rsqrt(float number);
    static QVector3D fastNormalize(const QVector3D& v);

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

    static int smallestPowerOf2GreaterThan(int x);
};

#endif // UTILS_H
