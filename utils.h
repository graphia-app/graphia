#ifndef UTILS_H
#define UTILS_H

#include <QVector3D>
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

    static QVector3D randQVector3D(float low, float high)
    {
        return QVector3D(rand(low, high), rand(low, high), rand(low, high));
    }

    template<typename T> static bool valueIsCloseToZero(T value)
    {
        return qFuzzyCompare(1, value + 1);
    }
};

#endif // UTILS_H
