#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QColor>

namespace Utils
{
    float rand(float low, float high);
    int rand(int low, int high);

    QVector2D randQVector2D(float low, float high);
    QVector3D randQVector3D(float low, float high);
    QColor randQColor();

    template<typename T> T interpolate(const T& a, const T& b, float f)
    {
        return a + ((b - a) * f);
    }

    template<typename T> bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }

    float fast_rsqrt(float number);
    QVector3D fastNormalize(const QVector3D& v);

    template<typename T> T clamp(T min, T max, T value)
    {
        if(value < min)
            return min;
        else if(value > max)
            return max;

        return value;
    }

    template<typename T> bool signsMatch(T a, T b)
    {
        return (a > 0 && b > 0) || (a <= 0 && b <= 0);
    }

    int smallestPowerOf2GreaterThan(int x);

    int currentThreadId();

    QQuaternion matrixToQuaternion(const QMatrix4x4& m);

    template<typename T> void checkEqual(T a, T b)
    {
        // Hopefully this function call elides to nothing when in release mode
        Q_ASSERT(a == b);
    }
}

#endif // UTILS_H
